
set(srcs "port")
if(CONFIG_BT_ENABLED)
    list(APPEND srcs "ble" "${LUATOS_ROOT}/components/nimble/src")
endif()

idf_component_register(SRC_DIRS ${LUATOS_ROOT}/luat/modules
                                ${LUATOS_ROOT}/luat/vfs
                                ${LUATOS_ROOT}/lua/src
                                ${srcs}
                                ${LUATOS_ROOT}/luat/freertos
                                ${LUATOS_ROOT}/components/printf
                                ${LUATOS_ROOT}/components/shell
                                # ${LUATOS_ROOT}/components/ymodem
                                ${LUATOS_ROOT}/components/cmux
                                ${LUATOS_ROOT}/components/i2c-tools
                                ${LUATOS_ROOT}/components/lfs
                                # ${LUATOS_ROOT}/components/iotauth
                                ${LUATOS_ROOT}/components/cjson
                                ${LUATOS_ROOT}/components/lua-cjson
                                ${LUATOS_ROOT}/components/crypto
                                ${LUATOS_ROOT}/components/miniz
                                ${LUATOS_ROOT}/components/serialization/protobuf
                                ${LUATOS_ROOT}/components/coremark
                                ${LUATOS_ROOT}/components/fal/src
                                ${LUATOS_ROOT}/components/flashdb/src
                                ${LUATOS_ROOT}/components/minmea
                                # ${LUATOS_ROOT}/components/wlan
                                ${LUATOS_ROOT}/components/statem
                                ${LUATOS_ROOT}/components/u8g2
                                ${LUATOS_ROOT}/components/gtfont
                                ${LUATOS_ROOT}/components/qrcode
                                ${LUATOS_ROOT}/components/lcd
                                ${LUATOS_ROOT}/components/lvgl/gen/lv_core
                                ${LUATOS_ROOT}/components/lvgl/gen/lv_draw
                                ${LUATOS_ROOT}/components/lvgl/gen/lv_font
                                ${LUATOS_ROOT}/components/lvgl/gen/lv_misc
                                ${LUATOS_ROOT}/components/lvgl/gen/lv_widgets
                                ${LUATOS_ROOT}/components/lvgl/font
                                ${LUATOS_ROOT}/components/lvgl/binding
                                ${LUATOS_ROOT}/components/lvgl/src/lv_core
                                ${LUATOS_ROOT}/components/lvgl/src/lv_draw
                                ${LUATOS_ROOT}/components/lvgl/src/lv_font
                                ${LUATOS_ROOT}/components/lvgl/src/lv_hal
                                ${LUATOS_ROOT}/components/lvgl/src/lv_misc
                                ${LUATOS_ROOT}/components/lvgl/src/lv_themes
                                ${LUATOS_ROOT}/components/lvgl/src/lv_widgets
                                ${LUATOS_ROOT}/components/lvgl/exts/lv_qrcode
                                ${LUATOS_ROOT}/components/lvgl/exts/lv_bmp
                                ${LUATOS_ROOT}/components/lvgl/exts/lv_png
                                ${LUATOS_ROOT}/components/lvgl/exts/lv_sjpg
                                ${LUATOS_ROOT}/components/lvgl/lv_demos/src/assets
                                ${LUATOS_ROOT}/components/lvgl/lv_demos/src/lv_demo_benchmark
                                ${LUATOS_ROOT}/components/lvgl/lv_demos/src/lv_demo_keypad_encoder
                                ${LUATOS_ROOT}/components/lvgl/lv_demos/src/lv_demo_music
                                ${LUATOS_ROOT}/components/lvgl/lv_demos/src/lv_demo_printer
                                ${LUATOS_ROOT}/components/lvgl/lv_demos/src/lv_demo_stress
                                ${LUATOS_ROOT}/components/lvgl/lv_demos/src/lv_demo_widgets
                                ${LUATOS_ROOT}/components/lvgl/lv_demos/src/lv_ex_get_started
                                ${LUATOS_ROOT}/components/lvgl/lv_demos/src/lv_ex_style
                                # ${LUATOS_ROOT}/components/eink
                                # ${LUATOS_ROOT}/components/epaper
                                ${LUATOS_ROOT}/components/tjpgd
                                ${LUATOS_ROOT}/components/sfud
                                ${LUATOS_ROOT}/components/mlx90640-library
                                ${LUATOS_ROOT}/components/lora
                                ${LUATOS_ROOT}/components/lora/sx126x
                                ${LUATOS_ROOT}/components/lora2
                                ${LUATOS_ROOT}/components/lora2/sx126x
                                # ${LUATOS_ROOT}/components/network/httpsrv/src
                                ${LUATOS_ROOT}/components/rsa/binding
                                # ${LUATOS_ROOT}/components/network/adapter
                                # ${LUATOS_ROOT}/components/network/adapter_lwip2
                                # ${LUATOS_ROOT}/components/network/websocket
                                # ${LUATOS_ROOT}/components/network/libftp
                                # ${LUATOS_ROOT}/components/network/libhttp
                                # ${LUATOS_ROOT}/components/network/libemqtt
                                # ${LUATOS_ROOT}/components/network/libsntp
                                # ${LUATOS_ROOT}/components/network/ulwip/binding/
                                # ${LUATOS_ROOT}/components/network/ulwip/src/
                                # ${LUATOS_ROOT}/components/ethernet/common
                                # ${LUATOS_ROOT}/components/ethernet/w5500
                                ${LUATOS_ROOT}/components/common
                                ${LUATOS_ROOT}/components/rtos/freertos
                                ${LUATOS_ROOT}/components/luatfonts
                                ${LUATOS_ROOT}/components/fskv
                                ${LUATOS_ROOT}/components/sfd
                                ${LUATOS_ROOT}/components/romfs
                                ${LUATOS_ROOT}/components/fatfs
                                ${LUATOS_ROOT}/components/max30102
                                ${LUATOS_ROOT}/components/iconv
                                ${LUATOS_ROOT}/components/gmssl/src
                                ${LUATOS_ROOT}/components/gmssl/bind
                                ${LUATOS_ROOT}/components/repl
                                # ${LUATOS_ROOT}/components/ymodem
                                ${LUATOS_ROOT}/components/xxtea/src
                                ${LUATOS_ROOT}/components/xxtea/binding
                                INCLUDE_DIRS ../../include
                                ${LUATOS_ROOT}/components/max30102
                                ${LUATOS_ROOT}/lua/include
                                ${LUATOS_ROOT}/luat/include
                                ${LUATOS_ROOT}/components/printf
                                ${LUATOS_ROOT}/components/shell
                                # ${LUATOS_ROOT}/components/ymodem
                                ${LUATOS_ROOT}/components/cmux
                                ${LUATOS_ROOT}/components/i2c-tools
                                ${LUATOS_ROOT}/components/fatfs
                                ${LUATOS_ROOT}/components/lfs
                                # ${LUATOS_ROOT}/components/iotauth
                                ${LUATOS_ROOT}/components/cjson
                                ${LUATOS_ROOT}/components/lua-cjson
                                ${LUATOS_ROOT}/components/crypto
                                ${LUATOS_ROOT}/components/miniz
                                ${LUATOS_ROOT}/components/serialization/protobuf
                                ${LUATOS_ROOT}/components/coremark
                                ${LUATOS_ROOT}/components/fal/inc
                                ${LUATOS_ROOT}/components/flashdb/inc
                                ${LUATOS_ROOT}/components/minmea
                                # ${LUATOS_ROOT}/components/wlan
                                ${LUATOS_ROOT}/components/statem
                                ${LUATOS_ROOT}/components/u8g2
                                ${LUATOS_ROOT}/components/gtfont
                                ${LUATOS_ROOT}/components/qrcode
                                ${LUATOS_ROOT}/components/lcd
                                ${LUATOS_ROOT}/components/lvgl
                                ${LUATOS_ROOT}/components/lvgl/src
                                ${LUATOS_ROOT}/components/lvgl/binding
                                ${LUATOS_ROOT}/components/lvgl/gen
                                ${LUATOS_ROOT}/components/lvgl/font
                                ${LUATOS_ROOT}/components/lvgl/src/lv_font
                                ${LUATOS_ROOT}/components/lvgl/exts/lv_bmp
                                ${LUATOS_ROOT}/components/lvgl/exts/lv_sjpg
                                ${LUATOS_ROOT}/components/lvgl/exts/lv_gif
                                ${LUATOS_ROOT}/components/lvgl/exts/lv_png
                                ${LUATOS_ROOT}/components/lvgl/exts/lv_qrcode
                                ${LUATOS_ROOT}/components/eink
                                # ${LUATOS_ROOT}/components/epaper
                                ${LUATOS_ROOT}/components/tjpgd
                                ${LUATOS_ROOT}/components/sfud
                                ${LUATOS_ROOT}/components/mlx90640-library
                                ${LUATOS_ROOT}/components/lora/sx126x
                                ${LUATOS_ROOT}/components/lora2/sx126x
                                # ${LUATOS_ROOT}/components/network/httpsrv/inc
                                ${LUATOS_ROOT}/components/nimble/inc
                                # ${LUATOS_ROOT}/components/network/adapter
                                # ${LUATOS_ROOT}/components/network/adapter_lwip2
                                # ${LUATOS_ROOT}/components/network/websocket
                                # ${LUATOS_ROOT}/components/network/libftp
                                # ${LUATOS_ROOT}/components/network/libhttp
                                # ${LUATOS_ROOT}/components/network/libemqtt
                                # ${LUATOS_ROOT}/components/network/libsntp
                                # ${LUATOS_ROOT}/components/network/ulwip/include/
                                # ${LUATOS_ROOT}/components/ethernet/common
                                # ${LUATOS_ROOT}/components/ethernet/w5500
                                ${LUATOS_ROOT}/components/common
                                ${LUATOS_ROOT}/components/luatfonts
                                ${LUATOS_ROOT}/components/fskv
                                ${LUATOS_ROOT}/components/sfd
                                ${LUATOS_ROOT}/components/romfs
                                ${LUATOS_ROOT}/components/iconv
                                ${LUATOS_ROOT}/components/gmssl/include
                                ${LUATOS_ROOT}/components/repl
                                # ${LUATOS_ROOT}/components/ymodem
                                ${LUATOS_ROOT}/components/xxtea/include
                    REQUIRES esp-tls lwip esp_http_client mbedtls spiffs driver heap esp_netif esp_event
                             esp_wifi esp_rom http_parser mqtt esp_adc bt console spi_flash esp_psram nvs_flash arduino
                    )

