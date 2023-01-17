#ifndef HOJA_INCLUDES_H
#define HOJA_INCLUDES_H

#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stddef.h>
#include <assert.h>
#include <stdint.h>

#include "sdkconfig.h"

#include <hal/clk_gate_ll.h>
#include <hal/rmt_ll.h>
#include <esp32/rom/ets_sys.h>
#include <esp_cpu.h>


// Bluetooth Stuff

// New HID api
#include "esp_hid_common.h"
#include "esp_hidd.h"

#include "esp_hidd_api.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_gap_bt_api.h"
#include "esp_gap_ble_api.h"

#include "esp_system.h"
#include "esp_mac.h"
#include "esp_random.h"

#include "esp_timer.h"
#include "esp_sleep.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "driver/spi_slave.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/adc.h"
#include "driver/pcnt.h"
#include "driver/rmt.h"
#include "driver/rtc_io.h"

#include "soc/soc.h"
#include "soc/spi_reg.h"
#include "soc/spi_struct.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_struct.h"
#include "soc/io_mux_reg.h"
#include "soc/spi_periph.h"

#include "rom/ets_sys.h"
#include "rom/gpio.h"

#include "soc/gpio_sig_map.h"
#include "soc/gpio_periph.h"
#include "soc/periph_defs.h"
#include "soc/rmt_struct.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/portmacro.h"
#include "freertos/xtensa_rtos.h"



//#include "sdkconfig.h"

// HOJA API includes
#include "hoja_types.h"
#include "hoja_backend.h"
#include "hoja_frontend.h"
#include "hoja_settings.h"

#include "util_i2c.h"
#include "util_joybus_rmt.h"
#include "util_wired_detect.h"
#include "util_battery.h"
#include "util_rmt.h"
#include "util_common.h"
#include "util_gamepad.h"
#include "util_bt_hid.h"

#include "core_gamecube_backend.h"
#include "core_usb_backend.h"
#include "core_snes_backend.h"
#include "core_switch_backend.h"
#include "core_bt_dinput.h"
#include "core_bt_xinput.h"

// ESP libs
#include "esp_hid_gap.h"

#include "esp_log.h"
#include "esp_err.h"

#include "rsc_descriptors.h"

#endif