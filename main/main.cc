#include <esp_log.h>
#include <esp_err.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#if CONFIG_PM_ENABLE
#include <esp_pm.h>
#endif

#include "buddy/core/buddy_app.h"

#define TAG "main"

extern "C" void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Erasing NVS flash to fix corruption");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

#if CONFIG_PM_ENABLE
    // Dynamic frequency scaling (240<->80MHz) + automatic light sleep.
    // light_sleep_enable self-gates: while the backlight PWM is lit or BLE is
    // connected over the main XTAL, ESP-IDF holds a no-light-sleep lock, so the
    // SoC only does DFS (no flicker, BLE stays connected). True CPU light sleep
    // engages when the screen is fully off (eco mode). If you ever see backlight
    // flicker or BLE instability when idle, set .light_sleep_enable = false.
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 80,
        .light_sleep_enable = true,
    };
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
#endif

    auto& app = BuddyApp::GetInstance();
    app.Initialize();
    app.Run();
}
