#include "util_bt_hid.h"

// TEMPLATE CALLBACK FUNCTIONS
// USE THESE TO PASTE INTO YOUR OWN
// CONTROLLER CORES FOR HANDLING

// BT Classic HID Callbacks

// BTC HIDD Event Callback Template
void util_bt_hidd_cb(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    esp_hidd_event_t event = (esp_hidd_event_t)id;
    esp_hidd_event_data_t *param = (esp_hidd_event_data_t *)event_data;
    static const char *TAG = "util_bt_hidd_cb";

    switch (event) {
    case ESP_HIDD_START_EVENT: {
        if (param->start.status == ESP_OK) {
            ESP_LOGI(TAG, "START OK");
            ESP_LOGI(TAG, "Setting to connectable, discoverable");
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        } else {
            ESP_LOGE(TAG, "START failed!");
        }
        break;
    }
    case ESP_HIDD_CONNECT_EVENT: {
        if (param->connect.status == ESP_OK) {
            ESP_LOGI(TAG, "CONNECT OK");
            ESP_LOGI(TAG, "Setting to non-connectable, non-discoverable");
            esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
            //bt_hid_task_start_up();
        } else {
            ESP_LOGE(TAG, "CONNECT failed!");
        }
        break;
    }
    case ESP_HIDD_PROTOCOL_MODE_EVENT: {
        ESP_LOGI(TAG, "PROTOCOL MODE[%u]: %s", param->protocol_mode.map_index, param->protocol_mode.protocol_mode ? "REPORT" : "BOOT");
        break;
    }
    case ESP_HIDD_OUTPUT_EVENT: {
        ESP_LOGI(TAG, "OUTPUT[%u]: %8s ID: %2u, Len: %d, Data:", param->output.map_index, esp_hid_usage_str(param->output.usage), param->output.report_id, param->output.length);
        //ESP_LOG_BUFFER_HEX(TAG, param->output.data, param->output.length);
        break;
    }
    case ESP_HIDD_FEATURE_EVENT: {
        ESP_LOGI(TAG, "FEATURE[%u]: %8s ID: %2u, Len: %d, Data:", param->feature.map_index, esp_hid_usage_str(param->feature.usage), param->feature.report_id, param->feature.length);
        //ESP_LOG_BUFFER_HEX(TAG, param->feature.data, param->feature.length);
        break;
    }
    case ESP_HIDD_DISCONNECT_EVENT: {
        if (param->disconnect.status == ESP_OK) {
            ESP_LOGI(TAG, "DISCONNECT OK");
            //bt_hid_task_shut_down();
            ESP_LOGI(TAG, "Setting to connectable, discoverable again");
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        } else {
            ESP_LOGE(TAG, "DISCONNECT failed!");
        }
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

// BTC GAP Event Callback Template
void util_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    const char* TAG = "util_bluetooth_gap_cb";
    switch (event) 
    {
        case ESP_BT_GAP_DISC_RES_EVT:
            ESP_LOGI(TAG, "ESP_BT_GAP_DISC_RES_EVT");
            //esp_log_buffer_hex(TAG, param->disc_res.bda, ESP_BD_ADDR_LEN);
            break;
        case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
            ESP_LOGI(TAG, "ESP_BT_GAP_DISC_STATE_CHANGED_EVT");
            break;
        case ESP_BT_GAP_RMT_SRVCS_EVT:
            ESP_LOGI(TAG, "ESP_BT_GAP_RMT_SRVCS_EVT");
            ESP_LOGI(TAG, "%d", param->rmt_srvcs.num_uuids);
            break;
        case ESP_BT_GAP_RMT_SRVC_REC_EVT:
            ESP_LOGI(TAG, "ESP_BT_GAP_RMT_SRVC_REC_EVT");
            break;
        case ESP_BT_GAP_AUTH_CMPL_EVT:{
            if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "authentication success: %s", param->auth_cmpl.device_name);
                //esp_log_buffer_hex(TAG, param->auth_cmpl.bda, ESP_BD_ADDR_LEN);
            } else {
                ESP_LOGI(TAG, "authentication failed, status:%d", param->auth_cmpl.stat);
            }
            break;
        }
        case ESP_BT_GAP_MODE_CHG_EVT:{
            ESP_LOGI(TAG, "power mode change: %d", param->mode_chg.mode);
            break;
        }
        
        default:
            ESP_LOGI(TAG, "UNKNOWN GAP EVT: %d", event);
            break; 
    }
}

// BLE Callbacks

// BLE HIDD Event Callback Template
static void util_ble_hidd_cb(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
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
        //ble_hid_task_start_up();//todo: this should be on auth_complete (in GAP)
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
        ESP_LOGI(TAG, "OUTPUT[%u]: %8s ID: %2u, Len: %d, Data:", param->output.map_index, esp_hid_usage_str(param->output.usage), param->output.report_id, param->output.length);
        ESP_LOG_BUFFER_HEX(TAG, param->output.data, param->output.length);
        break;
    }
    case ESP_HIDD_FEATURE_EVENT: {
        ESP_LOGI(TAG, "FEATURE[%u]: %8s ID: %2u, Len: %d, Data:", param->feature.map_index, esp_hid_usage_str(param->feature.usage), param->feature.report_id, param->feature.length);
        ESP_LOG_BUFFER_HEX(TAG, param->feature.data, param->feature.length);
        break;
    }
    case ESP_HIDD_DISCONNECT_EVENT: {
        ESP_LOGI(TAG, "DISCONNECT: %s", esp_hid_disconnect_reason_str(esp_hidd_dev_transport_get(param->disconnect.dev), param->disconnect.reason));
        //ble_hid_task_shut_down();
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

// BLE GAP Event Callback Template
static void util_ble_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    const char* TAG = "util_ble_gap_cb";

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
        }
        break;

    case ESP_GAP_BLE_KEY_EVT: //shows the ble key info share with peer device to the user.
        //ESP_LOGI(TAG, "BLE GAP KEY type = %s", esp_ble_key_type_str(param->ble_security.ble_key.key_type));
        break;

    case ESP_GAP_BLE_PASSKEY_NOTIF_EVT: // ESP_IO_CAP_OUT
        // The app will receive this evt when the IO has Output capability and the peer device IO has Input capability.
        // Show the passkey number to the user to input it in the peer device.
        ESP_LOGI(TAG, "BLE GAP PASSKEY_NOTIF passkey:%d", (unsigned int) param->ble_security.key_notif.passkey);
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

// Public variables

// Status of BT HID Gamepad Utility
util_bt_hid_status_t util_bt_hid_status = UTIL_BT_HID_STATUS_IDLE;

// Private functions

// Register app with BT Classic
hoja_err_t bt_register_app(util_bt_app_params_s *util_bt_app_params, esp_hid_device_config_t *hidd_device_config)
{
    const char* TAG = "bt_register_app";

    esp_err_t ret;
    esp_bt_cod_t hid_cod;
    hid_cod.minor = 0x2;
    hid_cod.major = 0x5;
    hid_cod.service = 0x400;
    esp_bt_gap_set_cod(hid_cod, ESP_BT_SET_COD_MAJOR_MINOR);

    esp_bt_dev_set_device_name(hidd_device_config->device_name);

    esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
    esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_NONE;
    esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));

    if ((ret = esp_bt_gap_register_callback(util_bt_app_params->gap_cb)) != ESP_OK) 
    {
        ESP_LOGE(TAG, "GAP callback register failed: %s\n", esp_err_to_name(ret));
        return HOJA_FAIL;
    }

    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);

    ret = esp_hidd_dev_init(hidd_device_config, ESP_HID_TRANSPORT_BT, util_bt_app_params->hidd_cb, &util_bt_app_params->hid_dev);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "HIDD dev init failed: %s\n", esp_err_to_name(ret));
        return HOJA_FAIL;
    }

    return HOJA_OK;
}

// Register app with BLE
hoja_err_t ble_register_app(util_bt_app_params_s *util_bt_app_params, esp_hid_device_config_t *hidd_device_config)
{
    esp_err_t ret;
    const char* TAG = "ble_register_app";

    // Register GAP callback
    if ((ret = esp_ble_gap_register_callback(util_bt_app_params->ble_gap_cb)) != ESP_OK) {
        ESP_LOGE(TAG, "esp_ble_gap_register_callback failed: %d", ret);
        return HOJA_FAIL;
    }

    // Register GATTS callback
    if ((ret = esp_ble_gatts_register_callback(esp_hidd_gatts_event_handler)) != ESP_OK) {
        ESP_LOGE(TAG, "GATTS register callback failed: %d", ret);
        return HOJA_FAIL;
    }

    const uint8_t hidd_service_uuid128[] = {
        0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x12, 0x18, 0x00, 0x00,
    };

    esp_ble_adv_data_t ble_adv_data = {
        .set_scan_rsp = false,
        .include_name = true,
        .include_txpower = true,
        .min_interval = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
        .max_interval = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
        .appearance = util_bt_app_params->appearance,
        .manufacturer_len = 0,
        .p_manufacturer_data =  NULL,
        .service_data_len = 0,
        .p_service_data = NULL,
        .service_uuid_len = sizeof(hidd_service_uuid128),
        .p_service_uuid = (uint8_t *)hidd_service_uuid128,
        .flag = 0x6,
    };

    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_BOND;
    //esp_ble_io_cap_t iocap = ESP_IO_CAP_OUT;//you have to enter the key on the host
    //esp_ble_io_cap_t iocap = ESP_IO_CAP_IN;//you have to enter the key on the device
    esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;//you have to agree that key matches on both
    //esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;//device is not capable of input or output, unsecure
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t key_size = 16; //the key size should be 7~16 bytes
    uint32_t passkey = 1234;//ESP_IO_CAP_OUT

    if ((ret = esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, 1)) != ESP_OK) {
        ESP_LOGE(TAG, "GAP set_security_param AUTHEN_REQ_MODE failed: %d", ret);
        return HOJA_FAIL;
    }

    if ((ret = esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, 1)) != ESP_OK) {
        ESP_LOGE(TAG, "GAP set_security_param IOCAP_MODE failed: %d", ret);
        return HOJA_FAIL;
    }

    if ((ret = esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, 1)) != ESP_OK) {
        ESP_LOGE(TAG, "GAP set_security_param SET_INIT_KEY failed: %d", ret);
        return HOJA_FAIL;
    }

    if ((ret = esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, 1)) != ESP_OK) {
        ESP_LOGE(TAG, "GAP set_security_param SET_RSP_KEY failed: %d", ret);
        return HOJA_FAIL;
    }

    if ((ret = esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, 1)) != ESP_OK) {
        ESP_LOGE(TAG, "GAP set_security_param MAX_KEY_SIZE failed: %d", ret);
        return HOJA_FAIL;
    }

    if ((ret = esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t))) != ESP_OK) {
        ESP_LOGE(TAG, "GAP set_security_param SET_STATIC_PASSKEY failed: %d", ret);
        return HOJA_FAIL;
    }

    if ((ret = esp_ble_gap_set_device_name(hidd_device_config->device_name)) != ESP_OK) {
        ESP_LOGE(TAG, "GAP set_device_name failed: %d", ret);
        return HOJA_FAIL;
    }

    if ((ret = esp_ble_gap_config_adv_data(&ble_adv_data)) != ESP_OK) {
        ESP_LOGE(TAG, "GAP config_adv_data failed: %d", ret);
        return HOJA_FAIL;
    }

    ESP_ERROR_CHECK(esp_hidd_dev_init(hidd_device_config, ESP_HID_TRANSPORT_BLE, util_bt_app_params->ble_hidd_cb, &util_bt_app_params->hid_dev));
    return HOJA_OK;
}


//-----------------------------
// Public functions

/**
 * @brief Initialize HID Gamepad Bluetooth app. It will automatically start the BT
 * Controller in the appropriate mode based on SDK Settings.
 * 
 * @param mac_address Pointer to uint8_t array of Mac address (8 long)
*/
hoja_err_t util_bluetooth_init(uint8_t *mac_address)
{
    const char* TAG = "util_bluetooth_init";
    esp_err_t ret;

    if (util_bt_hid_status > UTIL_BT_HID_STATUS_IDLE)
    {
        ESP_LOGE(TAG, "Already initialized or running.");
        return HOJA_FAIL;
    }

    if (sizeof(mac_address) != (8*sizeof(uint8_t)))
    {
        ESP_LOGE(TAG, "Mac address length is not 8 bytes. Using default address.");
    }
    else
    {
        esp_base_mac_addr_set(mac_address);
    }

    #if CONFIG_BTDM_CTRL_MODE_BTDM
        ESP_LOGI(TAG, "BT Dual Mode enabled.");
        esp_bt_mode_t mode = ESP_BT_MODE_BTDM;
    #elif CONFIG_BT_HID_DEVICE_ENABLED
        // Release BT BLE mode memory
        ESP_LOGI(TAG, "BT Classic HID only enabled. Release BLE Memory");
        ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));
        esp_bt_mode_t mode = ESP_BT_MODE_CLASSIC_BT;
    #elif CONFIG_BT_BLE_ENABLED
        // Release BTC mode memory
        ESP_LOGI(TAG, "BT LE only enabled. Release BTC Memory");
        ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
        esp_bt_mode_t mode = ESP_BT_MODE_BLE;
    #endif

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) 
    {
        ESP_LOGE(TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return HOJA_FAIL;
    }

    if ((ret = esp_bt_controller_enable(mode)) != ESP_OK) 
    {
        ESP_LOGE(TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return HOJA_FAIL;
    }

    ESP_LOGI(TAG, "Bluedroid initializing...");
    if ((ret = esp_bluedroid_init()) != ESP_OK) 
    {
        ESP_LOGE(TAG, "Bluedroid failed to initialize: %s\n",  esp_err_to_name(ret));
        return HOJA_FAIL;
    }

    ESP_LOGI(TAG, "Bluedroid enabling...");
    if ((ret = esp_bluedroid_enable()) != ESP_OK) 
    {
        ESP_LOGE(TAG, "Bluedroid failed to enable: %s\n",  esp_err_to_name(ret));
        return HOJA_FAIL;
    }

    util_bt_hid_status = UTIL_BT_HID_STATUS_INITIALIZED;
    return HOJA_OK;
}

/**
 * @brief Takes in an app params struct and starts the HID Gamepad
 * 
 * @param util_bt_app_params Pointer to type of util_bt_app_params_s
*/
hoja_err_t util_bluetooth_register_app(util_bt_app_params_s *util_bt_app_params, esp_hid_device_config_t *hidd_device_config)
{
    const char* TAG = "util_bluetooth_register_app";
    esp_err_t ret;
    hoja_err_t err;

    if (util_bt_hid_status < UTIL_BT_HID_STATUS_INITIALIZED)
    {
        ESP_LOGE(TAG, "Register with util_bluetooth_init() first!");
        return HOJA_FAIL;
    }

    if (util_bt_hid_status > UTIL_BT_HID_STATUS_INITIALIZED)
    {
        ESP_LOGE(TAG, "App is already registered and running.");
        return HOJA_FAIL;
    }

    switch(util_bt_app_params->bt_mode)
    {
        case ESP_BT_MODE_CLASSIC_BT:
            #if CONFIG_BT_HID_DEVICE_ENABLED
            err = bt_register_app(util_bt_app_params, hidd_device_config);
            #else
            ESP_LOGE(TAG, "BT Classic HID disabled. Enable in SDK settings. Also enable BT Dual mode.");
            return HOJA_FAIL;
            #endif
        break;

        case ESP_BT_MODE_BLE:
            #if CONFIG_BT_BLE_ENABLED
            err = ble_register_app(util_bt_app_params, hidd_device_config);
            #else
            ESP_LOGE(TAG, "BLE is disabled. Enable in SDK settings. Also enable BT Dual mode.");
            return HOJA_FAIL;
            #endif
        break;

        default:
            ESP_LOGE(TAG, "Invalid BT Mode.");
            return HOJA_FAIL;
        break;
    }

    if (err == HOJA_OK)
    {
        util_bt_hid_status = UTIL_BT_HID_STATUS_RUNNING;

        // Send callback event saying BT is started.
        if (hoja_event_cb)
        {
            hoja_event_cb(HOJA_EVT_BT, HEVT_BT_STARTED, 0x00);
        }
    }
    return err;
}
