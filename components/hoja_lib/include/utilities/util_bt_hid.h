#ifndef UTIL_BT_HID_H
#define UTIL_BT_HID_H

#include "hoja_includes.h"

typedef enum
{
    UTIL_BT_HID_STATUS_IDLE,
    UTIL_BT_HID_STATUS_INITIALIZED,
    UTIL_BT_HID_STATUS_RUNNING,
    UTIL_BT_HID_STATUS_MAX
} util_bt_hid_status_t;

typedef struct
{
    // CB for BTC
    esp_hd_cb_t hidd_cb;
    esp_bt_gap_cb_t gap_cb;

    // CB for BLE
    esp_event_handler_t ble_hidd_cb;
    esp_gap_ble_cb_t ble_gap_cb;
    
    esp_hidd_dev_t *hid_dev;
    esp_bt_mode_t bt_mode;
    int appearance;
} util_bt_app_params_s;

// Public variables
extern util_bt_hid_status_t util_bt_hid_status;

// Public functions
hoja_err_t util_bluetooth_init(uint8_t *mac_address);

hoja_err_t util_bluetooth_register_app(util_bt_app_params_s *util_bt_app_params, esp_hid_device_config_t *hidd_device_config);

#endif