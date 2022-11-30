#include "core_switch_backend.h"

nscore_param_s ns_core_param = {0};

// This is an adjusted HID descriptor to only
// use the necessary HID reports
static uint8_t hid_descriptor_ns_core[134] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)

    0x85, 0x21,        //   Report ID (33)
    0x06, 0x01, 0xFF,  //   Usage Page (Vendor Defined 0xFF01)
    0x09, 0x21,        //   Usage (0x21)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x00,        //   Logical Maximum (0)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x30,        //   Report Count (48)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x85, 0x30,        //   Report ID (48)
    0x09, 0x30,        //   Usage (0x30)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x00,        //   Logical Maximum (0)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x30,        //   Report Count (48)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x85, 0x3F,        //   Report ID (63)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x10,        //   Usage Maximum (0x10)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x10,        //   Report Count (16)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position) --100
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x39,        //   Usage (Hat switch)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x07,        //   Logical Maximum (7)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x33,        //   Usage (Rx)
    0x09, 0x34,        //   Usage (Ry)
    0x15, 0x00,        //   Logical Minimum (0)
    0x27, 0xFF, 0xFF, 0x00, 0x00,  //   Logical Maximum (65534)
    0x75, 0x10,        //   Report Size (16)
    0x95, 0x04,        //   Report Count (4)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x85, 0x01,        //   Report ID (1)
    0x06, 0x01, 0xFF,  //   Usage Page (Vendor Defined 0xFF01)
    0x09, 0x01,        //   Usage (0x01)
    0x15, 0x00,        //   Logical Minimum (0)
    0x27, 0xFF, 0xFF, 0x00, 0x00,  //   Logical Maximum (65534) - 154
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x30,        //   Report Count (48)
    0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)

    0x85, 0x10,        //   Report ID (16)
    0x09, 0x10,        //   Usage (0x10)
    0x15, 0x00,        //   Logical Minimum (0)
    0x27, 0xFF, 0xFF, 0x00, 0x00,  //   Logical Maximum (65534)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x30,        //   Report Count (48)
    0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)

    0xC0
};

//uint8_t hid_descriptor_ns_core_len = 213;

TaskHandle_t ns_ReportModeHandle = NULL;
uint8_t ns_currentReportMode = 0xAA;
bool ns_connected = false;

// Callbacks for GAP bt events
void ns_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    const char* TAG = "ns_bt_gap_cb";
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
                ns_connected = true;
            } else {
                ESP_LOGI(TAG, "authentication failed, status:%d", param->auth_cmpl.stat);
            }
            break;
        }
        // This is critical for Nintendo Switch to act upon.
        // If power mode is 0, there should be NO packets sent from the controller until
        // another power mode is initiated by the Nintendo Switch console.
        case ESP_BT_GAP_MODE_CHG_EVT:{
            ESP_LOGI(TAG, "power mode change: %d", param->mode_chg.mode);
            if (param->mode_chg.mode == 0)
            {
                if (ns_ReportModeHandle != NULL)
                {
                    vTaskDelete(ns_ReportModeHandle);
                    ns_ReportModeHandle = NULL;
                }
                
            }
            else
            {
                if (ns_currentReportMode == 0xAA)
                {
                    ns_controller_setinputreportmode(0x30);
                }
                else
                {
                    ns_controller_setinputreportmode(ns_currentReportMode);
                }
                
            }
            break;
        }
        
        default:
            ESP_LOGI(TAG, "UNKNOWN GAP EVT: %d", event);
            break; 
    }
}

// Callbacks for HID report events
void ns_bt_hidd_cb(esp_hidd_cb_event_t event, esp_hidd_cb_param_t *param)
{
    static const char* TAG = "ns_bt_hidd_cb";

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
                    ns_connected = true;
                    ESP_LOGI(TAG, "connected to %02x:%02x:%02x:%02x:%02x:%02x", param->open.bd_addr[0],
                            param->open.bd_addr[1], param->open.bd_addr[2], param->open.bd_addr[3], param->open.bd_addr[4],
                            param->open.bd_addr[5]);
                    ESP_LOGI(TAG, "making self non-discoverable and non-connectable.");
                    esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);

                    if (!loaded_settings.ns_controller_paired)
                    {
                        ns_savepairing(param->open.bd_addr);
                    }

                    ESP_LOGI(TAG, "Starting task short input mode...");
                    ns_controller_setinputreportmode(0xFF);

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
                    ns_connected = false;
                    vTaskDelay(3000 / portTICK_PERIOD_MS);
                    if (!ns_connected) 
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
            ns_comms_handle_command(param->intr_data.report_id, param->intr_data.len, param->intr_data.data);
            break;
        case ESP_HIDD_VC_UNPLUG_EVT:
            ESP_LOGI(TAG, "ESP_HIDD_VC_UNPLUG_EVT");
            if (param->vc_unplug.status == ESP_HIDD_SUCCESS) {
                if (param->close.conn_status == ESP_HIDD_CONN_STATE_DISCONNECTED) {
                    ns_connected = false;
                    ESP_LOGI(TAG, "disconnected!");
                    vTaskDelay(3000 / portTICK_PERIOD_MS);
                    if (!ns_connected) core_ns_stop();
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

hoja_err_t core_ns_start(void)
{
    const char* TAG = "core_ns_start";
    esp_err_t ret;

    // Load Controller data
    ns_controller_data.battery_level_full = 0x04;
    ns_controller_data.connection_info = 0x00;

    loaded_settings.ns_controller_type = NS_CONTROLLER_TYPE_SNESCLASSIC;

    // SET UP CONTROLLER TYPE VARS
    switch(loaded_settings.ns_controller_type)
    {
        case NS_CONTROLLER_TYPE_PROCON:
        default:
            ns_controller_data.controller_type_primary = 0x03;
            ns_controller_data.controller_type_secondary = 0x02;
            break;
        case NS_CONTROLLER_TYPE_N64CLASSIC:
            ns_controller_data.controller_type_primary = 0x0C;
            ns_controller_data.controller_type_secondary = 0x11;
            break;
        case NS_CONTROLLER_TYPE_SNESCLASSIC:
            ns_controller_data.controller_type_primary = 0x0B; //11
            ns_controller_data.controller_type_secondary = 0x2; //2
            break;
        case NS_CONTROLLER_TYPE_FCCLASSIC:
            ns_controller_data.controller_type_primary = 0x07;
            ns_controller_data.controller_type_secondary = 0x02;
            break;
        case NS_CONTROLLER_TYPE_NESCLASSIC:
            ns_controller_data.controller_type_primary = 0x09;
            ns_controller_data.controller_type_secondary = 0x02;
            break;
        case NS_CONTROLLER_TYPE_GENESIS:
            ns_controller_data.controller_type_primary = 0x0D;
            ns_controller_data.controller_type_secondary = 0x02;
            break;
    }

    ns_controller_data.sticks_calibrated = true;
    ns_controller_data.input_report_mode = 0xFF;

    // Convert calibration data
    ns_input_stickcalibration();

    // Set MAC address for Switch Bluetooth
    esp_base_mac_addr_set(loaded_settings.ns_client_bt_address);
    
    // Set up NS app parameters
    ns_core_param.app_param.name = "Pro Controller";
    ns_core_param.app_param.description = "Gamepad";
    ns_core_param.app_param.provider = "Nintendo";
    ns_core_param.app_param.subclass = 0x08;
    ns_core_param.app_param.desc_list = hid_descriptor_ns_core;
    ns_core_param.app_param.desc_list_len = sizeof(hid_descriptor_ns_core);
    ESP_LOGI(TAG, "Length of desc: %d", sizeof(hid_descriptor_ns_core));
    memset(&ns_core_param.both_qos, 0, sizeof(esp_hidd_qos_param_t));

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
    
    if ((ret = esp_bt_gap_register_callback(ns_bt_gap_cb)) != ESP_OK) 
    {
        ESP_LOGE(TAG, "GAP callback register failed: %s\n", esp_err_to_name(ret));
        return HOJA_FAIL;
    }

    esp_bt_cod_t ns_cod;
    ns_cod.minor = 0x2;
    ns_cod.major = 0x5;
    ns_cod.service = 0x400;
    esp_bt_gap_set_cod(ns_cod, ESP_BT_SET_COD_MAJOR_MINOR);

    ESP_LOGI(TAG, "Register HID device callback");
    if ((ret = esp_bt_hid_device_register_callback(ns_bt_hidd_cb)) != ESP_OK)
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
    if ((ret = esp_bt_hid_device_register_app(&ns_core_param.app_param, &ns_core_param.both_qos, &ns_core_param.both_qos)) != ESP_OK)
    {
        ESP_LOGE(TAG, "HID device register app failed:");
        return HOJA_FAIL;
    }

    esp_bt_dev_set_device_name("Pro Controller");

    ns_connected = false;
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);

    hoja_event_cb(HOJA_EVT_BT, HOJA_BT_STARTED, 0x00);

    // Delay 1 seconds to see if console initiates connection
    vTaskDelay(1000/portTICK_PERIOD_MS);

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
        ESP_LOGI(TAG, "Not connected to Switch. Fall back to another mode.");
        core_ns_stop();
        return HOJA_FAIL;
    }

    return HOJA_OK;
}

hoja_err_t core_ns_stop()
{
    const char* TAG = "rbc_core_ns_stop";

    ns_input_frequency = INPUT_FREQUENCY_SLOW;
    if (ns_ReportModeHandle != NULL)
    {
        vTaskDelete(ns_ReportModeHandle);
    }
    ns_ReportModeHandle = NULL;

    esp_bt_hid_device_disconnect();
    esp_bt_hid_device_unregister_app();
    esp_bt_hid_device_deinit();
    esp_bluedroid_disable();
                    
    ESP_LOGI(TAG, "Nintendo Switch Core stopped OK.");
    hoja_event_cb(HOJA_EVT_BT, HOJA_BT_DISCONNECT, 0x00);

    return HOJA_OK;
}

hoja_err_t ns_savepairing(uint8_t* host_addr)
{
    const char* TAG = "ns_savepairing";

    if (host_addr == NULL)
    {
        ESP_LOGE(TAG, "Host address is blank.");
        return HOJA_FAIL;
    }

    ESP_LOGI(TAG, "Pairing to Nintendo Switch.");

    // Copy host address into settings memory.
    memcpy(loaded_settings.ns_host_bt_address, host_addr, sizeof(loaded_settings.ns_host_bt_address));
    loaded_settings.ns_controller_paired = true;
    
    // Save all settings
    if (hoja_settings_saveall() == HOJA_OK)
    {
        ESP_LOGI(TAG, "Pairing info saved.");
        hoja_event_cb(HOJA_EVT_BT, HOJA_BT_PAIRED, 0x00);
        return HOJA_OK;
    } 
    else
    {
        ESP_LOGE(TAG, "Failed to save pairing settings.");
        return HOJA_FAIL;
    }
}
