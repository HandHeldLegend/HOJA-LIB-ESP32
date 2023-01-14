#include "core_switch_backend.h"

TaskHandle_t switch_bt_task_handle = NULL;

uint8_t ns_currentReportMode = 0xAA;
bool ns_connected = false;

// Private variables
ns_subcore_t _ns_subcore = NS_TYPE_PROCON;
ns_core_status_t _ns_status = NS_STATUS_IDLE;

// SWITCH BTC HIDD Callback
void switch_bt_hidd_cb(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    esp_hidd_event_t event = (esp_hidd_event_t)id;
    esp_hidd_event_data_t *param = (esp_hidd_event_data_t *)event_data;
    static const char *TAG = "switch_bt_hidd_cb";

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
        // Send interrupt data to command handler
        ns_comms_handle_command(param->output.data[0], param->output.length, param->output.data[0]);
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

            //TODO add NS core stop

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

// SWITCH BTC GAP Event Callback
void switch_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    const char* TAG = "switch_bt_gap_cb";
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

                // TO DO start input task

            } else {
                ESP_LOGI(TAG, "authentication failed, status:%d", param->auth_cmpl.stat);
            }
            break;
        }
        case ESP_BT_GAP_MODE_CHG_EVT:{
            // This is critical for Nintendo Switch to act upon.
            // If power mode is 0, there should be NO packets sent from the controller until
            // another power mode is initiated by the Nintendo Switch console.
            ESP_LOGI(TAG, "power mode change: %d", param->mode_chg.mode);
            if (param->mode_chg.mode == 0)
            {
                /*
                if (ns_ReportModeHandle != NULL)
                {
                    vTaskDelete(ns_ReportModeHandle);
                    ns_ReportModeHandle = NULL;
                }*/
                
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

// DEPRECEATED TO BE REMOVED START
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
                /*
                if (ns_ReportModeHandle != NULL)
                {
                    vTaskDelete(ns_ReportModeHandle);
                    ns_ReportModeHandle = NULL;
                }*/
                
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
// DEPRECEATED TO BE REMOVED END

// Switch HID report maps
const esp_hid_raw_report_map_t switch_report_maps[1] = {
    {
        .data = procon_hid_descriptor,
        .len = (uint16_t) PROCON_HID_REPORT_MAP_LEN,
    }
};

// Bluetooth App setup data
util_bt_app_params_s switch_app_params = {
    .hidd_cb            = switch_bt_hidd_cb,
    .gap_cb             = switch_bt_gap_cb,
    .bt_mode            = ESP_BT_MODE_CLASSIC_BT,
    .appearance         = ESP_HID_APPEARANCE_GAMEPAD,
};

const esp_hid_device_config_t switch_hidd_config = {
    .vendor_id  = HID_VEND_NSPRO,
    .product_id = HID_PROD_NSPRO,
    .version    = 0x0000,
    .device_name = "Pro Controller",
    .manufacturer_name = "Nintendo",
    .serial_number = "000000",
    .report_maps    = switch_report_maps,
    .report_maps_len = 1,
};

/** @brief Set the Nintendo Switch sub controller type
 * Required before starting core.
 * 
 * @param subcore Type of ns_subcore_t
*/
hoja_err_t core_ns_set_subcore(ns_subcore_t subcore)
{
    const char* TAG = "core_ns_set_subcore";

    if (_ns_status > NS_STATUS_SUBCORESET)
    {
        ESP_LOGE(TAG, "Core must be stopped before setting subcore.");
        return HOJA_FAIL;
    }

    if ( (subcore == NS_TYPE_UNSET) || (subcore == NULL) || (subcore >= NS_TYPE_MAX) )
    {
        ESP_LOGE(TAG, "Invalid subcore type.");
        return HOJA_FAIL;
    }

    ESP_LOGI(TAG, "Switch Sub-Core set.");
    _ns_subcore = subcore;
    _ns_status = NS_STATUS_SUBCORESET;
    return HOJA_OK;
}

// Attempt start of Nintendo Switch controller core
hoja_err_t core_ns_start(void)
{
    const char* TAG = "core_ns_start";
    esp_err_t ret;
    hoja_err_t err;

    // Load Controller data
    ns_controller_data.battery_level_full = 0x04;
    ns_controller_data.connection_info = 0x00;

    // SET UP CONTROLLER TYPE VARS
    ns_controller_setup_memory();

    ns_controller_data.sticks_calibrated = true;
    ns_controller_data.input_report_mode = 0xFF;

    // Convert calibration data
    ns_controller_applycalibration();

    // Set MAC address for Switch Bluetooth
    esp_base_mac_addr_set(loaded_settings.ns_client_bt_address);

    err = util_bluetooth_init(loaded_settings.ns_client_bt_address);
    err = util_bluetooth_register_app(&switch_app_params, &switch_hidd_config);

    ESP_LOGI(TAG, "Delaying 10 seconds then shutting down");
    vTaskDelay(10000/portTICK_PERIOD_MS);

    util_bluetooth_stop();

    return HOJA_OK;

    // TODO handle connecting to already paired device
}

// Stop Nintendo Switch controller core
hoja_err_t core_ns_stop()
{
    const char* TAG = "core_ns_stop";

    if (switch_bt_task_handle != NULL)
    {
        vTaskDelete(switch_bt_task_handle);
    }
    switch_bt_task_handle = NULL;

    // TODO replace with nice util BT way to stop BT.
    esp_bt_hid_device_disconnect();
    esp_bt_hid_device_unregister_app();
    esp_bt_hid_device_deinit();
    esp_bluedroid_disable();
                    
    ESP_LOGI(TAG, "Nintendo Switch Core stopped OK.");
    // TODO replace with BT stopped event. At this point disconnect already happened probably.
    hoja_event_cb(HOJA_EVT_BT, HEVT_BT_DISCONNECT, 0x00);

    return HOJA_OK;
}

// Save Nintendo Switch bluetooth pairing
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
        hoja_event_cb(HOJA_EVT_BT, HEVT_BT_PAIRED, 0x00);
        return HOJA_OK;
    } 
    else
    {
        ESP_LOGE(TAG, "Failed to save pairing settings.");
        return HOJA_FAIL;
    }
}
