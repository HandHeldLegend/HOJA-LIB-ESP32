#include "retroblue_backend.h"

//void (*rb_bluetooth_cb_tab)(rb_event_bt_t,  uint16_t);
//void (*rb_controller_cb_tab)(rb_event_ct_t, uint16_t);

// Callback functions
//void rb_bluetooth_event_cb(rb_event_bt_t evt_code, uint16_t *data)
//{
//    const char* TAG = "rb_bluetooth_event_cb";
//    ESP_LOGI(TAG, "RetroBlue Bluetooth callback event: %d", evt_code);
//
//    rb_bluetooth_cb_tab(evt_code, data);
//}

//void rb_controller_event_cb(rb_event_ct_t evt_code, uint16_t *data)
//{
//    const char* TAG = "rb_controller_event_cb";
//    ESP_LOGI(TAG, "RetroBlue controller callback event: %d", evt_code);
//    
//    rb_controller_cb_tab(evt_code, data);
//}