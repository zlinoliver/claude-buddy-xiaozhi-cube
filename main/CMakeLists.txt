# claude-desktop-buddy-esp32 — CMakeLists.txt
# Pure buddy device: display + LED + board abstraction + BLE

set(SOURCES
    "display/display.cc"
    "display/lcd_display.cc"
    "display/oled_display.cc"
    "display/lvgl_display/lvgl_display.cc"
    "display/lvgl_display/emoji_collection.cc"
    "display/lvgl_display/lvgl_theme.cc"
    "display/lvgl_display/lvgl_font.cc"
    "display/lvgl_display/lvgl_image.cc"
    "display/lvgl_display/gif/lvgl_gif.cc"
    "display/lvgl_display/gif/gifdec.c"
    "led/single_led.cc"
    "led/circular_strip.cc"
    "led/gpio_led.cc"
    "boards/common/board.cc"
    "boards/common/backlight.cc"
    "boards/common/buzzer.cc"
    "boards/common/es8311_buzzer.cc"
    "boards/common/i2s_buzzer.cc"
    "boards/common/pdm_buzzer.cc"
    "boards/common/button.cc"
    "boards/common/system_reset.cc"
    "settings.cc"
    "main.cc"
)

set(INCLUDE_DIRS "." "display" "display/lvgl_display" "boards/common" "buddy")

# Font defaults
set(BUILTIN_TEXT_FONT font_puhui_14_1)
set(BUILTIN_ICON_FONT font_awesome_14_1)

# Single target board: Shuzhi 1.54" TFT 4G (XiaoZhi CUBE 4G).
set(BOARD_TYPE "shuzhi-1.54tft-4g")
set(BUILTIN_TEXT_FONT font_puhui_20_4)
set(BUILTIN_ICON_FONT font_awesome_20_4)

# Board-specific sources
file(GLOB BOARD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD_TYPE}/*.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD_TYPE}/*.c
)
list(APPEND SOURCES ${BOARD_SOURCES})

# Buddy sources
file(GLOB_RECURSE BUDDY_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/buddy/*.cc)
list(APPEND SOURCES ${BUDDY_SOURCES})

# Helper: find component by pattern
idf_build_get_property(build_components BUILD_COMPONENTS)
function(find_component_by_pattern PATTERN COMPONENT_VAR PATH_VAR)
    foreach(COMPONENT ${build_components})
        if(COMPONENT MATCHES "${PATTERN}")
            set(${COMPONENT_VAR} ${COMPONENT} PARENT_SCOPE)
            idf_component_get_property(COMPONENT_PATH ${COMPONENT} COMPONENT_DIR)
            set(${PATH_VAR} "${COMPONENT_PATH}" PARENT_SCOPE)
            break()
        endif()
    endforeach()
endfunction()

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${INCLUDE_DIRS}
                    WHOLE_ARCHIVE
                    PRIV_REQUIRES
                        esp_pm
                        esp_psram
                        esp_driver_gpio
                        esp_driver_spi
                        esp_driver_i2c
                        esp_driver_i2s
                        esp_driver_ledc
                        esp_adc
                        spi_flash
                        efuse
                        bt
                        nvs_flash
                        json
                        vfs
                        mbedtls
                        esp_app_format
                    )

if(NOT BOARD_NAME)
    set(BOARD_NAME ${BOARD_TYPE})
endif()
target_compile_definitions(${COMPONENT_LIB}
                    PRIVATE BOARD_TYPE=\"${BOARD_TYPE}\" BOARD_NAME=\"${BOARD_NAME}\"
                    PRIVATE BUILTIN_TEXT_FONT=${BUILTIN_TEXT_FONT} BUILTIN_ICON_FONT=${BUILTIN_ICON_FONT}
                    )

# Find xiaozhi-fonts component
find_component_by_pattern("xiaozhi-fonts" XIAOZHI_FONTS_COMPONENT XIAOZHI_FONTS_COMPONENT_PATH)
if(XIAOZHI_FONTS_COMPONENT_PATH)
    set(XIAOZHI_FONTS_PATH "${XIAOZHI_FONTS_COMPONENT_PATH}")
endif()
