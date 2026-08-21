#include "board.h"
#include "display/lcd_display.h"
#include "button.h"
#include "i2s_buzzer.h"
#include "config.h"
#include "buddy/core/buddy_app.h"
#include "buddy/core/demo_mode.h"
#include "buddy/pet/buddy_pet.h"
#include "buddy/ui/buddy_ui.h"
#include "buddy/storage/buddy_nvs.h"

#include <esp_log.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>

#define TAG "Shuzhi154Buddy"

// Pin map recovered by reverse-engineering the stock xiaozhi firmware.
// See config.h for the source notes. Display = ST7789 240x240 on SPI3_HOST.

class Shuzhi154Tft4gBoard : public Board {
private:
    Button nav_button_;       // 3rd physical button (GPIO 0)
    Button approve_button_;   // top physical button (GPIO 39)
    Button deny_button_;      // 2nd physical button (GPIO 40)
    Display* display_;
    Buzzer* buzzer_ = nullptr;
    adc_oneshot_unit_handle_t battery_adc_ = nullptr;  // ADC2 ch6 / GPIO17
    adc_cali_handle_t battery_cali_ = nullptr;
    bool battery_cali_tried_ = false;
    static constexpr int kBatteryDivider = 2;  // 2:1 resistor divider (pin = Vbat/2)

    // Battery sense was reverse-engineered from the stock firmware
    // (PowerManager on ADC_UNIT_2, ADC_CHANNEL_6 -> GPIO17, 12-bit, 12dB,
    // raw reads). GPIO17 is unused elsewhere on the buddy, and the buddy has
    // no Wi-Fi, so ADC2 reads are reliable. If init fails we silently leave
    // battery_adc_ null and GetBatteryLevel() reports "unknown" (returns false).
    void InitializeBattery() {
        adc_oneshot_unit_init_cfg_t init_cfg = {};
        init_cfg.unit_id = ADC_UNIT_2;
        if (adc_oneshot_new_unit(&init_cfg, &battery_adc_) != ESP_OK) {
            battery_adc_ = nullptr;
            ESP_LOGW(TAG, "battery ADC unit init failed; level unavailable");
            return;
        }
        adc_oneshot_chan_cfg_t chan_cfg = {};
        chan_cfg.atten = ADC_ATTEN_DB_12;
        chan_cfg.bitwidth = ADC_BITWIDTH_12;
        if (adc_oneshot_config_channel(battery_adc_, ADC_CHANNEL_6, &chan_cfg) != ESP_OK) {
            adc_oneshot_del_unit(battery_adc_);
            battery_adc_ = nullptr;
            ESP_LOGW(TAG, "battery ADC channel init failed; level unavailable");
        }
    }

    void InitializePower() {
        // LCD / peripheral power rail — stock firmware drives this high early.
        gpio_config_t cfg = {};
        cfg.mode = GPIO_MODE_OUTPUT;
        cfg.pin_bit_mask = (1ULL << POWER_ENABLE_GPIO);
        gpio_config(&cfg);
        gpio_set_level(POWER_ENABLE_GPIO, 1);
    }

    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = DISPLAY_SPI_MOSI_PIN;
        buscfg.miso_io_num = GPIO_NUM_NC;
        buscfg.sclk_io_num = DISPLAY_SPI_SCLK_PIN;
        buscfg.quadwp_io_num = GPIO_NUM_NC;
        buscfg.quadhd_io_num = GPIO_NUM_NC;
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    void InitializeDisplay() {
        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;

        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = DISPLAY_SPI_CS_PIN;
        io_config.dc_gpio_num = DISPLAY_SPI_DC_PIN;
        io_config.spi_mode = 3;
        io_config.pclk_hz = 80 * 1000 * 1000;
        io_config.trans_queue_depth = 7;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_config, &panel_io));

        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = DISPLAY_SPI_RESET_PIN;
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
        panel_config.bits_per_pixel = 16;
        ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io, &panel_config, &panel));

        esp_lcd_panel_reset(panel);
        esp_lcd_panel_init(panel);
        esp_lcd_panel_invert_color(panel, true);
        esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);

        display_ = new SpiLcdDisplay(panel_io, panel,
            DISPLAY_WIDTH, DISPLAY_HEIGHT,
            DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y,
            DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY);
    }

    void InitializeButtons() {
        // Nav (GPIO0 / BOOT): click = next page, long press = settings
        nav_button_.OnClick([]() {
            auto& app = BuddyApp::GetInstance();
            if (app.IsScreenOff()) { app.NotifyActivity(); return; }
            app.NotifyActivity();
            if (buddy_ui_in_settings()) {
                buddy_ui_settings_next();
            } else {
                buddy_ui_next_mode();
            }
        });
        nav_button_.OnLongPress([]() {
            auto& app = BuddyApp::GetInstance();
            app.NotifyActivity();
            buddy_ui_show_settings();
        });

        // Approve (top button): approve / switch pet / toggle setting
        approve_button_.OnClick([]() {
            auto& app = BuddyApp::GetInstance();
            if (app.IsScreenOff()) { app.NotifyActivity(); return; }
            app.NotifyActivity();
            if (buddy_ui_in_settings()) {
                buddy_ui_settings_select();
            } else if (app.GetState().has_prompt()) {
                app.Approve();
            } else {
                uint8_t next = (buddy_pet_get_species() + 1) % buddy_pet_species_count();
                buddy_pet_set_species(next);
                buddy_nvs_save_species(next);
            }
        });

        // Deny (2nd button): deny / demo mode / exit settings
        deny_button_.OnClick([]() {
            auto& app = BuddyApp::GetInstance();
            if (app.IsScreenOff()) { app.NotifyActivity(); return; }
            app.NotifyActivity();
            if (buddy_ui_in_settings()) {
                buddy_ui_show_settings();  // toggle off
            } else if (app.GetState().has_prompt()) {
                app.Deny();
            } else {
                if (demo_mode_active()) demo_mode_stop();
                else demo_mode_start();
            }
        });
    }

public:
    Shuzhi154Tft4gBoard()
        : nav_button_(BUTTON_NAV_GPIO),
          approve_button_(BUTTON_APPROVE_GPIO),
          deny_button_(BUTTON_DENY_GPIO) {
        InitializePower();
        InitializeBattery();
        InitializeSpi();
        InitializeDisplay();
        InitializeButtons();
        // Speaker over I2S (no amp-enable pin; amp is on the GPIO 21 power rail).
        // Gives audible approval/celebration tones via Buzzer::AttentionTone().
        buzzer_ = new I2sBuzzer(AUDIO_I2S_SPK_BCLK, AUDIO_I2S_SPK_WS,
                                AUDIO_I2S_SPK_DOUT, GPIO_NUM_NC);
        // Square ST7789 panel looks dim at the stock 75% default — run it at full
        // brightness. The on-device settings menu can still dim it afterwards.
        GetBacklight()->SetBrightness(100);
    }

    virtual std::string GetBoardType() override { return "shuzhi-1.54tft-4g"; }
    virtual Display* GetDisplay() override { return display_; }
    virtual Buzzer* GetBuzzer() override { return buzzer_; }
    virtual const char* GetApprovalHint() override { return "Top = Yes   2nd = No"; }

    // Reverse-engineered stock mapping: average raw 12-bit ADC2/ch6 reads, then
    // level = clamp((avg - 1969) * 100 / 2120, 0, 100). Charging detection is
    // not wired (the stock charging GPIO was not identified), so we report
    // discharging only — the percentage is what matters here.
    virtual bool GetBatteryLevel(int& level, bool& charging, bool& discharging) override {
        if (battery_adc_ == nullptr) return false;
        // Lazily create the ADC calibration scheme (curve fitting, supported on S3).
        if (!battery_cali_ && !battery_cali_tried_) {
            battery_cali_tried_ = true;
            adc_cali_curve_fitting_config_t cc = {};
            cc.unit_id = ADC_UNIT_2;
            cc.chan = ADC_CHANNEL_6;
            cc.atten = ADC_ATTEN_DB_12;
            cc.bitwidth = ADC_BITWIDTH_12;
            adc_cali_create_scheme_curve_fitting(&cc, &battery_cali_);
        }
        if (battery_cali_ == nullptr) return false;
        int sum = 0, n = 0;
        for (int i = 0; i < 8; i++) {
            int raw = 0;
            if (adc_oneshot_read(battery_adc_, ADC_CHANNEL_6, &raw) == ESP_OK) {
                sum += raw;
                n++;
            }
        }
        if (n == 0) return false;
        int pin_mv = 0;
        if (adc_cali_raw_to_voltage(battery_cali_, sum / n, &pin_mv) != ESP_OK) return false;
        int vbat_mv = pin_mv * kBatteryDivider;
        // Li-ion open-circuit-voltage -> state-of-charge curve (mV, %).
        static const struct { int mv; int pct; } kCurve[] = {
            {3000, 0}, {3300, 5}, {3500, 10}, {3650, 20}, {3700, 30}, {3750, 40},
            {3800, 50}, {3850, 60}, {3950, 70}, {4050, 80}, {4150, 90}, {4200, 100}
        };
        const int kN = sizeof(kCurve) / sizeof(kCurve[0]);
        int pct;
        if (vbat_mv <= kCurve[0].mv) {
            pct = 0;
        } else if (vbat_mv >= kCurve[kN - 1].mv) {
            pct = 100;
        } else {
            pct = 100;
            for (int i = 1; i < kN; i++) {
                if (vbat_mv < kCurve[i].mv) {
                    int span = kCurve[i].mv - kCurve[i - 1].mv;
                    pct = kCurve[i - 1].pct +
                          (vbat_mv - kCurve[i - 1].mv) * (kCurve[i].pct - kCurve[i - 1].pct) / span;
                    break;
                }
            }
        }
        level = pct;
        charging = false;
        discharging = true;
        return true;
    }

    virtual Backlight* GetBacklight() override {
        static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
        return &backlight;
    }
};

DECLARE_BOARD(Shuzhi154Tft4gBoard);
