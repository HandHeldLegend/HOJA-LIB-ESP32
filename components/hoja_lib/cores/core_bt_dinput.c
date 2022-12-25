#include "core_bt_dinput.h"

// Public vars
bt_dinput_param_s bt_dinput_param = {0};

TaskHandle_t bt_dinput_task_handle = NULL;

// Private vars
bool bt_dinput_connected = false;

// Private functions
// Callbacks for GAP bt events
void bt_dinput_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    const char* TAG = "bt_dinput_gap_cb";

    switch (event) 
    {
        case ESP_BT_GAP_DISC_RES_EVT:
            ESP_LOGI(TAG, "ESP_BT_GAP_DISC_RES_EVT");
            esp_log_buffer_hex(TAG, param->disc_res.bda, ESP_BD_ADDR_LEN);
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
                esp_log_buffer_hex(TAG, param->auth_cmpl.bda, ESP_BD_ADDR_LEN);
                bt_dinput_connected = true;
            } else {
                ESP_LOGI(TAG, "authentication failed, status:%d", param->auth_cmpl.stat);
            }
            break;
        }

        case ESP_BT_GAP_MODE_CHG_EVT:{
            ESP_LOGI(TAG, "power mode change: %d", param->mode_chg.mode);
            if (param->mode_chg.mode == 0)
            {

                
            }
            else
            {

            }
            break;
        }
        
        default:
            ESP_LOGI(TAG, "UNKNOWN GAP EVT: %d", event);
            break; 
    }
}

// Callbacks for HID report events
void bt_dinput_hidd_cb(esp_hidd_cb_event_t event, esp_hidd_cb_param_t *param)
{
    static const char* TAG = "bt_dinput_hidd_cb";

    switch (event) {
        case ESP_HIDD_INIT_EVT:
            if (param->init.status == ESP_HIDD_SUCCESS) {
                ESP_LOGI(TAG, "init hidd success!");
            } else {
                ESP_LOGI(TAG, "init hidd failed!");
            }
            break;
        case ESP_HIDD_DEINIT_EVT:
            break;
        case ESP_HIDD_REGISTER_APP_EVT:
            if (param->register_app.status == ESP_HIDD_SUCCESS) {
                ESP_LOGI(TAG, "Register HIDD app parameters success!");
                if(param->register_app.bd_addr == NULL)
                {
                    ESP_LOGI(TAG, "bd_addr is undefined!");
                }
            } else {
                ESP_LOGI(TAG, "Register HIDD app parameters failed!");
            }
            break;
        case ESP_HIDD_UNREGISTER_APP_EVT:
            if (param->unregister_app.status == ESP_HIDD_SUCCESS) {
                ESP_LOGI(TAG, "unregister app success!");
            } else {
                ESP_LOGI(TAG, "unregister app failed!");
            }
            break;
        case ESP_HIDD_OPEN_EVT:
            if (param->open.status == ESP_HIDD_SUCCESS) {
                if (param->open.conn_status == ESP_HIDD_CONN_STATE_CONNECTING) {
                    ESP_LOGI(TAG, "connecting...");
                } else if (param->open.conn_status == ESP_HIDD_CONN_STATE_CONNECTED) {
                    bt_dinput_connected = true;
                    ESP_LOGI(TAG, "connected to %02x:%02x:%02x:%02x:%02x:%02x", param->open.bd_addr[0],
                            param->open.bd_addr[1], param->open.bd_addr[2], param->open.bd_addr[3], param->open.bd_addr[4],
                            param->open.bd_addr[5]);
                    ESP_LOGI(TAG, "making self non-discoverable and non-connectable.");
                    esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);

                    // To Do save pairing
                    /*
                    if (!loaded_settings.)
                    {
                        dinput_savepairing(param->open.bd_addr);
                    }*/

                } else {
                    ESP_LOGI(TAG, "unknown connection status");
                    ESP_LOGI(TAG, "making self non-discoverable and non-connectable.");
                    esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
                }
            } else {
                ESP_LOGI(TAG, "open failed!");
                ESP_LOGI(TAG, "making self non-discoverable and non-connectable.");
                esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
            }
            break;
        case ESP_HIDD_CLOSE_EVT:
            ESP_LOGI(TAG, "ESP_HIDD_CLOSE_EVT");
            if (param->close.status == ESP_HIDD_SUCCESS) {
                if (param->close.conn_status == ESP_HIDD_CONN_STATE_DISCONNECTING) {
                    ESP_LOGI(TAG, "disconnecting...");
                } else if (param->close.conn_status == ESP_HIDD_CONN_STATE_DISCONNECTED) {
                    bt_dinput_connected = false;
                    vTaskDelay(3000 / portTICK_PERIOD_MS);
                    if (!bt_dinput_connected) 
                    {
                        core_ns_stop();
                        ESP_LOGI(TAG, "disconnected!");
                    }
                } else {
                    ESP_LOGI(TAG, "unknown connection status");
                }
            } else {
                ESP_LOGI(TAG, "close failed!");
                ESP_LOGI(TAG, "making self non-discoverable and non-connectable.");
                esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
            }
            break;
        case ESP_HIDD_SEND_REPORT_EVT:
            break;
        case ESP_HIDD_REPORT_ERR_EVT:
            ESP_LOGI(TAG, "ESP_HIDD_REPORT_ERR_EVT");
            break;
        case ESP_HIDD_GET_REPORT_EVT:
            ESP_LOGI(TAG, "ESP_HIDD_GET_REPORT_EVT id:0x%02x, type:%d, size:%d", param->get_report.report_id,
                    param->get_report.report_type, param->get_report.buffer_size);
            break;
        case ESP_HIDD_SET_REPORT_EVT:
            ESP_LOGI(TAG, "ESP_HIDD_SET_REPORT_EVT");
            break;
        case ESP_HIDD_SET_PROTOCOL_EVT:
            ESP_LOGI(TAG, "ESP_HIDD_SET_PROTOCOL_EVT");
            break;
        case ESP_HIDD_INTR_DATA_EVT:
            // Send interrupt data to command handler
            // To do: Handle OUT reports

            break;
        case ESP_HIDD_VC_UNPLUG_EVT:
            ESP_LOGI(TAG, "ESP_HIDD_VC_UNPLUG_EVT");
            if (param->vc_unplug.status == ESP_HIDD_SUCCESS) {
                if (param->close.conn_status == ESP_HIDD_CONN_STATE_DISCONNECTED) {
                    bt_dinput_connected = false;
                    ESP_LOGI(TAG, "disconnected!");
                    vTaskDelay(3000 / portTICK_PERIOD_MS);
                    if (!bt_dinput_connected) core_bt_dinput_stop();
                } else {
                    ESP_LOGI(TAG, "unknown connection status");
                }
            } else {
                ESP_LOGI(TAG, "close failed!");
                ESP_LOGI(TAG, "making self non-discoverable and non-connectable.");
                esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
            }
            break;
        default:
            ESP_LOGI(TAG, "UNKNOWN EVENT: %d", event);

            break;
        }
}

hoja_err_t core_bt_dinput_start(void)
{
    const char* TAG = "core_bt_dinput_start";
    esp_err_t ret;

    // Set MAC address for DInput Bluetooth
    //esp_base_mac_addr_set(loaded_settings.ns_client_bt_address);
    
    // Set up NS app parameters
    bt_dinput_param.app_param.name = "DInput Controller";
    bt_dinput_param.app_param.description = "Gamepad";
    bt_dinput_param.app_param.provider = "HHL";
    bt_dinput_param.app_param.subclass = 0x08;
    bt_dinput_param.app_param.desc_list = dinput_hid_report_descriptor;
    bt_dinput_param.app_param.desc_list_len = DINPUT_HID_REPORT_MAP_LEN;
    memset(&bt_dinput_param.both_qos, 0, sizeof(esp_hidd_qos_param_t));

    // Release BT BLE mode memory
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) 
    {
        ESP_LOGE(TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return HOJA_FAIL;
    }

    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) 
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
    
    if ((ret = esp_bt_gap_register_callback(bt_dinput_gap_cb)) != ESP_OK) 
    {
        ESP_LOGE(TAG, "GAP callback register failed: %s\n", esp_err_to_name(ret));
        return HOJA_FAIL;
    }

    esp_bt_cod_t dinput_cod;
    dinput_cod.minor = 0x2;
    dinput_cod.major = 0x5;
    dinput_cod.service = 0x400;
    esp_bt_gap_set_cod(dinput_cod, ESP_BT_SET_COD_MAJOR_MINOR);

    ESP_LOGI(TAG, "Register HID device callback");
    if ((ret = esp_bt_hid_device_register_callback(bt_dinput_hidd_cb)) != ESP_OK)
    {
        ESP_LOGE(TAG, "HID Callback register failed: %s\n", esp_err_to_name(ret));
        return HOJA_FAIL;
    }

	ESP_LOGI(TAG, "Starting HID Device");
	if ((ret = esp_bt_hid_device_init()) != ESP_OK)
    {
        ESP_LOGE(TAG, "HID device failed to start:");
        return HOJA_FAIL;
    }

    esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
    esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_NONE;
    esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));

    ESP_LOGI(TAG, "Register HID Device app");
    if ((ret = esp_bt_hid_device_register_app(&bt_dinput_param.app_param, &bt_dinput_param.both_qos, &bt_dinput_param.both_qos)) != ESP_OK)
    {
        ESP_LOGE(TAG, "HID device register app failed:");
        return HOJA_FAIL;
    }

    esp_bt_dev_set_device_name("DInput Controller");

    bt_dinput_connected = false;
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);

    hoja_event_cb(HOJA_EVT_BT, HEVT_BT_STARTED, 0x00);

    // Delay 1 seconds to see if console initiates connection
    vTaskDelay(1000/portTICK_PERIOD_MS);

    /*
    if (loaded_settings.ns_controller_paired & !ns_connected)
    {
        // Connect to paired host device if we haven't connected already
        if (esp_bt_hid_device_connect(loaded_settings.ns_host_bt_address) != ESP_OK)
        {
            ESP_LOGI(TAG, "Failed to connect to paired switch. Setting scannable and discoverable.");
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        }
    }
    else
    {
        ESP_LOGI(TAG, "Controller already connected");
    }

    vTaskDelay(1500/portTICK_PERIOD_MS);
    if (!ns_connected)
    {
        // If still not connected, return failure.
        ESP_LOGI(TAG, "Not connected to BT Device. Fall back to another mode.");
        core_bt_dinput_stop();
        return HOJA_FAIL;
    }*/

    return HOJA_OK;
}

hoja_err_t core_bt_dinput_stop()
{
    const char* TAG = "core_bt_dinput_stop";

    if (bt_dinput_task_handle != NULL)
    {
        vTaskDelete(bt_dinput_task_handle);
    }
    bt_dinput_task_handle = NULL;

    esp_bt_hid_device_disconnect();
    esp_bt_hid_device_unregister_app();
    esp_bt_hid_device_deinit();
    esp_bluedroid_disable();
                    
    ESP_LOGI(TAG, "BT DInput Core stopped OK.");
    hoja_event_cb(HOJA_EVT_BT, HEVT_BT_DISCONNECT, 0x00);

    return HOJA_OK;
}

hoja_err_t dinput_savepairing(uint8_t* host_addr)
{
    const char* TAG = "dinput_savepairing";

    /*
    if (host_addr == NULL)
    {
        ESP_LOGE(TAG, "Host address is blank.");
        return HOJA_FAIL;
    }

    ESP_LOGI(TAG, "Pairing to BT Device.");

    // Copy host address into settings memory.
    memcpy(loaded_settings.ns_host_bt_address, host_addr, sizeof(loaded_settings.ns_host_bt_address));
    loaded_settings.ns_controller_paired = true;
    
    // Save all settings
    if (hoja_settings_saveall() == HOJA_OK)
    {
        ESP_LOGI(TAG, "Pairing info saved.");
        hoja_event_cb(HOJA_EVT_BT, HEVT_BT_PAIRED, 0x00);
        return HOJA_OK;
    } 
    else
    {
        ESP_LOGE(TAG, "Failed to save pairing settings.");
        return HOJA_FAIL;
    }*/
    return HOJA_OK;
}
