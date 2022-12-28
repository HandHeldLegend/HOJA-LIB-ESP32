#include "core_bt_xinput.h"

TaskHandle_t xinput_bt_task_handle = NULL;

// XINPUT BLE HIDD callback
void xinput_ble_hidd_cb(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    esp_hidd_event_t event = (esp_hidd_event_t)id;
    esp_hidd_event_data_t *param = (esp_hidd_event_data_t *)event_data;
    static const char *TAG = "HID_DEV_BLE";

    switch (event) {
    case ESP_HIDD_START_EVENT: {
        ESP_LOGI(TAG, "START");
        esp_hid_ble_gap_adv_start();
        break;
    }
    case ESP_HIDD_CONNECT_EVENT: {
        ESP_LOGI(TAG, "CONNECT");
        break;
    }
    case ESP_HIDD_PROTOCOL_MODE_EVENT: {
        ESP_LOGI(TAG, "PROTOCOL MODE[%u]: %s", param->protocol_mode.map_index, param->protocol_mode.protocol_mode ? "REPORT" : "BOOT");
        break;
    }
    case ESP_HIDD_CONTROL_EVENT: {
        ESP_LOGI(TAG, "CONTROL[%u]: %sSUSPEND", param->control.map_index, param->control.control ? "EXIT_" : "");
        break;
    }
    case ESP_HIDD_OUTPUT_EVENT: {
        //ESP_LOGI(TAG, "OUTPUT[%u]: %8s ID: %2u, Len: %d, Data:", param->output.map_index, esp_hid_usage_str(param->output.usage), param->output.report_id, param->output.length);
        break;
    }
    case ESP_HIDD_FEATURE_EVENT: {
        ESP_LOGI(TAG, "FEATURE[%u]: %8s ID: %2u, Len: %d, Data:", param->feature.map_index, esp_hid_usage_str(param->feature.usage), param->feature.report_id, param->feature.length);
        break;
    }
    case ESP_HIDD_DISCONNECT_EVENT: {
        ESP_LOGI(TAG, "DISCONNECT: %s", esp_hid_disconnect_reason_str(esp_hidd_dev_transport_get(param->disconnect.dev), param->disconnect.reason));
        xinput_stop_task();
        esp_hid_ble_gap_adv_start();
        break;
    }
    case ESP_HIDD_STOP_EVENT: {
        ESP_LOGI(TAG, "STOP");
        break;
    }
    default:
        break;
    }
    return;
}

// XINPUT GAP BLE event callback
void xinput_ble_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    const char* TAG = "xinput_ble_gap_cb";

    switch (event) {
    /*
     * SCAN
     * */
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
        ESP_LOGV(TAG, "BLE GAP EVENT SCAN_PARAM_SET_COMPLETE");
        //SEND_BLE_CB();
        break;
    }
    case ESP_GAP_BLE_SCAN_RESULT_EVT: {
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        switch (scan_result->scan_rst.search_evt) {
        case ESP_GAP_SEARCH_INQ_RES_EVT: {
            //handle_ble_device_result(&scan_result->scan_rst);
            break;
        }
        case ESP_GAP_SEARCH_INQ_CMPL_EVT:
            ESP_LOGV(TAG, "BLE GAP EVENT SCAN DONE: %d", scan_result->scan_rst.num_resps);
            //SEND_BLE_CB();
            break;
        default:
            break;
        }
        break;
    }
    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT: {
        ESP_LOGV(TAG, "BLE GAP EVENT SCAN CANCELED");
        break;
    }

    /*
     * ADVERTISEMENT
     * */
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        ESP_LOGV(TAG, "BLE GAP ADV_DATA_SET_COMPLETE");
        break;

    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        ESP_LOGV(TAG, "BLE GAP ADV_START_COMPLETE");
        break;

    /*
     * AUTHENTICATION
     * */
    case ESP_GAP_BLE_AUTH_CMPL_EVT:
        if (!param->ble_security.auth_cmpl.success) {
            ESP_LOGE(TAG, "BLE GAP AUTH ERROR: 0x%x", param->ble_security.auth_cmpl.fail_reason);
        } else {
            ESP_LOGI(TAG, "BLE GAP AUTH SUCCESS");
            //ble_hid_task_start_up();//todo: this should be on auth_complete (in GAP)
            xinput_start_task();
        }
        break;

    case ESP_GAP_BLE_KEY_EVT: //shows the ble key info share with peer device to the user.
        //ESP_LOGI(TAG, "BLE GAP KEY type = %s", esp_ble_key_type_str(param->ble_security.ble_key.key_type));
        break;

    case ESP_GAP_BLE_PASSKEY_NOTIF_EVT: // ESP_IO_CAP_OUT
        // The app will receive this evt when the IO has Output capability and the peer device IO has Input capability.
        // Show the passkey number to the user to input it in the peer device.
        //ESP_LOGI(TAG, "BLE GAP PASSKEY_NOTIF passkey:%d", param->ble_security.key_notif.passkey);
        break;

    case ESP_GAP_BLE_NC_REQ_EVT: // ESP_IO_CAP_IO
        // The app will receive this event when the IO has DisplayYesNO capability and the peer device IO also has DisplayYesNo capability.
        // show the passkey number to the user to confirm it with the number displayed by peer device.
        ESP_LOGI(TAG, "BLE GAP NC_REQ passkey:%d", (unsigned int) param->ble_security.key_notif.passkey);
        esp_ble_confirm_reply(param->ble_security.key_notif.bd_addr, true);
        break;

    case ESP_GAP_BLE_PASSKEY_REQ_EVT: // ESP_IO_CAP_IN
        // The app will receive this evt when the IO has Input capability and the peer device IO has Output capability.
        // See the passkey number on the peer device and send it back.
        ESP_LOGI(TAG, "BLE GAP PASSKEY_REQ");
        //esp_ble_passkey_reply(param->ble_security.ble_req.bd_addr, true, 1234);
        break;

    case ESP_GAP_BLE_SEC_REQ_EVT:
        ESP_LOGI(TAG, "BLE GAP SEC_REQ");
        // Send the positive(true) security response to the peer device to accept the security request.
        // If not accept the security request, should send the security response with negative(false) accept value.
        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
        break;

    default:
        //ESP_LOGV(TAG, "BLE GAP EVENT %s", ble_gap_evt_str(event));
        break;
    }
}

// XInput HID report maps
static esp_hid_raw_report_map_t xinput_report_maps[1] = {
    {
        .data = xinput_hid_report_descriptor,
        .len = (uint16_t) XINPUT_HID_REPORT_MAP_LEN
    }
};

// Bluetooth App setup data
static util_bt_app_params_s xinput_app_params = {
    .ble_hidd_cb        = xinput_ble_hidd_cb,
    .ble_gap_cb         = xinput_ble_gap_cb,
    .bt_mode            = ESP_BT_MODE_BLE,
    .appearance         = ESP_HID_APPEARANCE_GAMEPAD,
};

// Takes in both XINPUT structs and returns if they match.
bool xinput_compare(xi_input_s *one, xi_input_s *two)
{
    bool ret = false;
    ret |= one->buttons_1   != two->buttons_1;
    ret |= one->buttons_2   != two->buttons_2;
    ret |= one->dpad_hat    != two->dpad_hat;

    ret |= one->stick_left_x != two->stick_left_x;
    ret |= one->stick_left_y != two->stick_left_y;

    ret |= one->stick_right_x != two->stick_right_x;
    ret |= one->stick_right_y != two->stick_right_y;

    ret |= one->analog_trigger_l != two->analog_trigger_l;
    ret |= one->analog_trigger_r != two->analog_trigger_r;

    return ret;
}

void xinput_bt_sendinput_task(void * param)
{
    const char* TAG = "xinput_bt_sendinput_task";
    ESP_LOGI(TAG, "Starting input task loop.");

    xi_input_s xi_input = {0};
    xi_input_s xi_input_last = {0};
    uint8_t xi_buffer[XI_HID_LEN];

    for(;;)
    {
        hoja_analog_cb(&hoja_analog_data);
        xi_input.stick_left_x       = hoja_analog_data.ls_x << 4;
        xi_input.stick_left_y       = hoja_analog_data.ls_y << 4;
        xi_input.stick_right_x      = hoja_analog_data.rs_x << 4;
        xi_input.stick_right_y      = hoja_analog_data.rs_y << 4;

        xi_input.analog_trigger_l   = hoja_analog_data.lt_a << 4;
        xi_input.analog_trigger_r   = hoja_analog_data.rt_a << 4;

        xi_input.bumper_l           = hoja_button_data.trigger_l;
        xi_input.bumper_r           = hoja_button_data.trigger_r;

        xi_input.button_a           = hoja_button_data.button_down;
        xi_input.button_b           = hoja_button_data.button_right;
        xi_input.button_x           = hoja_button_data.button_left;
        xi_input.button_y           = hoja_button_data.button_up;
        
        uint8_t lr = (1 - hoja_button_data.dpad_left) + hoja_button_data.dpad_right;
        uint8_t ud = (1 - hoja_button_data.dpad_down) + hoja_button_data.dpad_up;
        xi_input.dpad_hat = util_get_dpad_hat(HAT_MODE_XINPUT, lr, ud);
        hoja_button_reset();

        if (xinput_compare(&xi_input, &xi_input_last))
        {
            memcpy(xi_buffer, &xi_input, XI_HID_LEN);
            esp_hidd_dev_input_set(xinput_app_params.hid_dev, 0, XI_INPUT_REPORT_ID, xi_buffer, XI_HID_LEN);
            memcpy(&xi_input_last, &xi_input, sizeof(xi_input_s));
        }
        vTaskDelay(8/portTICK_PERIOD_MS);
    }
}

void xinput_start_task(void)
{
    if (xinput_bt_task_handle)
    {
        vTaskDelete(xinput_bt_task_handle);
        xinput_bt_task_handle = NULL;
    }
    xTaskCreatePinnedToCore(xinput_bt_sendinput_task, "xinput_bt_task", 2048, NULL, 0, &xinput_bt_task_handle, 0);
}

void xinput_stop_task(void)
{
    if (xinput_bt_task_handle)
    {
        vTaskDelete(xinput_bt_task_handle);
        xinput_bt_task_handle = NULL;
    }
}

static esp_hid_device_config_t xinput_hidd_config = {
    .vendor_id  = HID_VEND_XINPUT,
    .product_id = HID_PROD_XINPUT,
    .version    = 0x0000,
    .device_name = "XInput BLE Gamepad",
    .manufacturer_name = "HHL",
    .serial_number = "000000",
    .report_maps    = xinput_report_maps,
    .report_maps_len = 1,
};

// Public Functions
hoja_err_t core_bt_xinput_start(void)
{
    const char* TAG = "core_bt_xinput_start";
    esp_err_t ret;
    hoja_err_t err = HOJA_OK;

    err = util_bluetooth_init(loaded_settings.xinput_client_bt_address);
    err = util_bluetooth_register_app(&xinput_app_params, &xinput_hidd_config);
    return err;
}
