#include "core_switch_backend.h"

TaskHandle_t switch_bt_task_handle = NULL;

uint8_t ns_currentReportMode = 0xAA;
uint8_t ns_hostAddress[6] = {0};
bool    ns_connected = false;

// Private variables
ns_subcore_t _ns_subcore = NS_TYPE_PROCON;
ns_core_status_t _ns_status = NS_STATUS_IDLE;

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
                memcpy(ns_hostAddress, param->auth_cmpl.bda, ESP_BD_ADDR_LEN);
                ns_controller_input_task_set(NS_REPORT_MODE_BLANK);
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
                ns_controller_sleep_handle(NS_POWER_SLEEP);   
            }
            else
            {
                ns_controller_sleep_handle(NS_POWER_AWAKE);    
            }
            break;
        }
        
        default:
            ESP_LOGI(TAG, "UNKNOWN GAP EVT: %d", event);
            break; 
    }
}

// Callbacks for HID report events
void switch_bt_hidd_cb(esp_hidd_cb_event_t event, esp_hidd_cb_param_t *param)
{
    const char* TAG = "ns_bt_hidd_cb";

    switch (event) {
        case ESP_HIDD_INIT_EVT:
            if (param->init.status == ESP_HIDD_SUCCESS) {
                //ESP_LOGI(TAG, "init hidd success!");
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

                    ESP_LOGI(TAG, "connected to %02x:%02x:%02x:%02x:%02x:%02x", param->open.bd_addr[0],
                            param->open.bd_addr[1], param->open.bd_addr[2], param->open.bd_addr[3], param->open.bd_addr[4],
                            param->open.bd_addr[5]);
                    ESP_LOGI(TAG, "making self non-discoverable and non-connectable.");
                    esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);

                    hoja_event_cb(HOJA_EVT_BT, HEVT_BT_CONNECTED, 0x00);
                    ns_controller_input_task_set(NS_REPORT_MODE_SIMPLE);

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
                    ESP_LOGI(TAG, "disconnected!");
                    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
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
                    ESP_LOGI(TAG, "disconnected!");
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


// Switch HID report maps
esp_hid_raw_report_map_t switch_report_maps[1] = {
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

esp_hid_device_config_t switch_hidd_config = {
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

    // Convert calibration data
    ns_controller_applycalibration();

    // Set MAC address for Switch Bluetooth
    esp_base_mac_addr_set(loaded_settings.ns_client_bt_address);

    err = util_bluetooth_init(loaded_settings.ns_client_bt_address);

    // If we are already paired, attempt connection
    if (loaded_settings.ns_controller_paired)
    {
        ESP_LOGI(TAG, "NS Paired, attempting to connect...");
        err = util_bluetooth_register_app(&switch_app_params, &switch_hidd_config, false);
        if (err == HOJA_OK)
        {
            vTaskDelay(1500/portTICK_PERIOD_MS);
            util_bluetooth_connect(loaded_settings.ns_host_bt_address);
            vTaskDelay(3000/portTICK_PERIOD_MS);
            if (!ns_connected)
            {
                ESP_LOGI(TAG, "No Switch connection, enter pairing mode...");
                return ns_startpairing();
            }
        }
        
    }
    else
    {
        // Not paired, await pairing connection
        ESP_LOGI(TAG, "NS not Paired, put into advertise mode...");
        err = util_bluetooth_register_app(&switch_app_params, &switch_hidd_config, true);
    }

    return HOJA_OK;
}

// Stop Nintendo Switch controller core
void core_ns_stop(void)
{
    const char* TAG = "core_ns_stop";
    ns_connected = false;
    ns_controller_input_task_set(NS_REPORT_MODE_IDLE);
    util_bluetooth_deinit();
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
        return HOJA_OK;
    } 
    else
    {
        ESP_LOGE(TAG, "Failed to save pairing settings.");
        return HOJA_FAIL;
    }
}

// This function is used when you are already paired and/or connected
// and you wish to pair to another Nintendo Switch console
hoja_err_t ns_startpairing(void)
{
    // First, stop the NS core.
    core_ns_stop();

    // Set the setting of paired to false.
    loaded_settings.ns_controller_paired = false;

    // Start the NS core again
    // If a new pairing is done on this session
    // it will save to memory.
    core_ns_start();

    return HOJA_OK;
}
