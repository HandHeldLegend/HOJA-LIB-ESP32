idf_component_register(SRCS 
                    "hoja_frontend.c" "hoja_backend.c" 
                    "hoja_settings.c" "core_switch_spi.c" 
                    "core_switch_reports.c" "core_switch_input.c" 
                    "core_switch_backend.c" "core_switch_controller.c" 
                    "core_switch_comms.c" "core_snes_backend.c"
                    "core_gamecube_backend.c" "util_joybus_rmt.c"
                    "core_usb_backend.c" "util_rgb.c"
                    "util_wired_detect.c" "util_battery.c"
                    "util_i2c.c"
                    INCLUDE_DIRS "include"
                    REQUIRES bt esp32 freertos)

