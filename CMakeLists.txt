
idf_component_register(
    SRCS "hoja_frontend.c" "hoja_backend.c"
"hoja_settings.c" "cores/switch/core_switch_spi.c" 
"cores/switch/core_switch_reports.c" "cores/switch/core_switch_input.c"
"cores/core_switch_backend.c" "cores/switch/core_switch_controller.c"
"cores/switch/core_switch_comms.c" "cores/core_snes_backend.c"
"cores/core_gamecube_backend.c" "utilities/util_joybus_rmt.c"
"cores/core_usb_backend.c" "utilities/util_wired_detect.c" 
"utilities/util_battery.c" "utilities/util_common.c"
"utilities/util_i2c.c" "utilities/util_gamepad.c" 
"resources/rsc_descriptors.c" "cores/core_bt_dinput.c" 
"cores/core_bt_xinput.c" "utilities/util_bt_hid.c" 
"esp_hid_gap.c"
    INCLUDE_DIRS "include" "include/cores"
    "include/resources" "include/utilities" "include/cores/switch"
    REQUIRES "bt" "hal" "xtensa"
    "riscv" "esp_rom" "driver" "esp_common" 
    "soc" "esp_hw_support" "esp_system"
    "newlib" "spi_flash" "log" "heap"
    "freertos" "nvs_flash" "esp_hid")
