#ifndef HOJA_INCLUDES_H
#define HOJA_INCLUDES_H

#include <hal/clk_gate_ll.h>
#include <hal/rmt_ll.h>
#include "driver/rmt.h"

#include <assert.h>
#include "esp_log.h"
#include "esp_hidd_api.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_bt.h"
#include "esp_err.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_gap_bt_api.h"
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stddef.h>
#include "driver/spi_slave.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/adc.h"

#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "sdkconfig.h"

// HOJA API includes
#include "hoja_types.h"
#include "hoja_backend.h"
#include "hoja_frontend.h"
#include "hoja_settings.h"

#include "core_gamecube_backend.h"
#include "core_usb_backend.h"
#include "core_snes_backend.h"
#include "core_switch_backend.h"

#include "util_joybus_rmt.h"
#include "util_joybus.h"


#endif