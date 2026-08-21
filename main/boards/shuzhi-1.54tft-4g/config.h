#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

// Pin map reverse-engineered from the stock xiaozhi "shuzhi-1.54tft-4g"
// firmware (class SHUZHI_1_54TFT_4G, xiaozhi v1.6.6 / ESP-IDF v5.4.1).
// ESP32-S3, 8MB flash, 8MB octal PSRAM, ST7789 240x240 square LCD, 4G (ML307).
// Audio codec and 4G modem are not wired up here — not needed for the buddy.

#define BUILTIN_LED_GPIO        GPIO_NUM_NC

// LCD / peripheral power-enable rail (driven high by the stock firmware)
#define POWER_ENABLE_GPIO       GPIO_NUM_21

// Display - ST7789 240x240 SPI (SPI3_HOST, spi_mode 3, 80MHz, color inverted)
#define DISPLAY_WIDTH   240
#define DISPLAY_HEIGHT  240
#define DISPLAY_MIRROR_X false
#define DISPLAY_MIRROR_Y false
#define DISPLAY_SWAP_XY  false
#define DISPLAY_OFFSET_X 0
#define DISPLAY_OFFSET_Y 0

#define DISPLAY_BACKLIGHT_PIN         GPIO_NUM_13
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false

#define DISPLAY_SPI_SCLK_PIN  GPIO_NUM_9
#define DISPLAY_SPI_MOSI_PIN  GPIO_NUM_10
#define DISPLAY_SPI_CS_PIN    GPIO_NUM_14
#define DISPLAY_SPI_DC_PIN    GPIO_NUM_8
#define DISPLAY_SPI_RESET_PIN GPIO_NUM_18

// Audio — speaker is a standard I2S output (the stock firmware used
// NoAudioCodecSimplexPdm: I2S speaker + PDM mic). Buddy only needs the speaker
// for approval/celebration tones. No separate amp-enable pin (amp sits on the
// GPIO 21 power rail). PDM mic (clk=2, din=3) is unused here.
#define AUDIO_I2S_SPK_BCLK  GPIO_NUM_15
#define AUDIO_I2S_SPK_WS    GPIO_NUM_16
#define AUDIO_I2S_SPK_DOUT  GPIO_NUM_7

// Buttons (all active-low). Confirmed physical layout, top→bottom on the right
// side of the case:
//   1st (top) = GPIO 39 -> Approve / Yes  (changes pet when idle)
//   2nd       = GPIO 40 -> Deny / No      (toggles demo mode when idle)
//   3rd       = GPIO 0  -> Nav            (switch pages; hold = settings)
//   4th       = hardware power/reset (not a GPIO button)
#define BUTTON_APPROVE_GPIO GPIO_NUM_39   // top
#define BUTTON_DENY_GPIO    GPIO_NUM_40   // 2nd
#define BUTTON_NAV_GPIO     GPIO_NUM_0    // 3rd

#endif // _BOARD_CONFIG_H_
