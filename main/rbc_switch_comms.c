#include "rbc_switch_comms.h"

/**
 * @brief Handles incoming commands and sends the data through
 * to the appropriate function handlers.
 */
void ns_comms_handle_command(uint8_t report_id, uint16_t len, uint8_t* p_data)
{
    const char* TAG = "ns_comms_handle_command";

    // Clear the report
    ns_report_clear();
    // Set report timer
    ns_report_settimer();
    // Set battery/connection byte
    ns_report_setbattconn();
    // Set full report response buttons
    ns_report_setbuttons(NS_BM_LONG); 

    switch(p_data[0])
    {
        // Rumble and a sub-command.
        case COMM_RUMBLE_SUB:
            ESP_LOGI(TAG, "Sub-command with rumble data received.");

            // Set input report ID
            ns_report_setid(COMM_RID_SUBCMDSTANDARD);

            // Extract rumble data
            //vibration_data_s vibration_data = rbc_vibration_parse(p_data);

            // Handle rumble data
            //rbc_vibration_handle(vibration_data);

            // Pass off to the sub-command handler.
            ns_comms_handle_subcommand(p_data[10], len, p_data);

            // Send input report
            esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0x21, ns_input_report_size, ns_input_report);
            break;

        // Just Rumble data
        case COMM_RUMBLE_ONLY:
            //ESP_LOGI(TAG, "Only rumble data received.");

            // Set input report ID
            ns_report_setid(COMM_RID_STANDARDFULL);
            // Extract rumble data
            //vibration_data_s vibration_data = rbc_vibration_parse(p_data);

            // Handle rumble data
            //rbc_vibration_handle(vibration_data);

            break;

        // UNIMPLEMENTED, RETURN STANDARD FULL REPORT
        // NFC IR MCU firmware update
        case COMM_NFCIRFWUPDATE:
            ESP_LOGI(TAG, "NFC/IR FW Update packet received.");
        // Requesting NFC or IR data to be sent when it's ready.
        case COMM_NFCIR_REQUEST:
            ESP_LOGI(TAG, "NFC/IR Data request received.");
        // Unrecognized command
        default:
            ESP_LOGI(TAG, "Unrecognized request received.");
            // Set input report ID
            ns_report_setid(COMM_RID_STANDARDFULL);

            break;
    }

}

// Handle 0x01 Output reports requiring sub-command response
void ns_comms_handle_subcommand(uint8_t command, uint16_t len, uint8_t* p_data)
{
    const char* TAG = "ns_comms_handle_subcommand";

    // Set defaults for all outgoing responses
    // to sub-commands.
    ns_report_setsubcmd(command);  
    ns_input_report_size = SUB_C_RESPONSE_LENGTH_DEFAULT;

    switch(command)
    {

        case SUBC_GET_CONTROLLERSTATE:
            ESP_LOGI(TAG, "SUBC - Get controller state.");
            ns_report_setack(0x80);
            
            break;

        case SUBC_GET_DEVINFO:
            ESP_LOGI(TAG, "SUBC - Get device info.");

            ns_report_setack(0x82);
            // Set battery data
            // Set connection info
            ns_report_sub_setdevinfo();
            break;

        case SUBC_SET_INPUTMODE:
            ESP_LOGI(TAG, "SUBC - Set input mode.");
            ns_report_setack(0x80);

            // Set the cycling input report mode.
            // This task gets pinned and sends repeatedly at a given frequency.
            ns_controller_setinputreportmode(p_data[SUB_C_DATA_IDX]);
            break;
        
        case SUBC_GET_TRIGGER_ET:
            ESP_LOGI(TAG, "SUBC - Get trigger elapsed time.");
            ns_report_setack(0x83);
            // 1000ms / 10 = value of 100.
            ns_report_sub_triggertime(100);
            break;

        case SUBC_SET_SHIPMODE:
            ESP_LOGI(TAG, "SUBC - Set ship mode.");
            ns_report_setack(0x80);
            //ns_controller_setshipmode(p_data[SUB_C_DATA_IDX]);
            break;

        case SUBC_READ_SPI:
            ESP_LOGI(TAG, "SUBC - SPI read command: %d, %d", p_data[12], p_data[11]);
            ns_report_setack(0x90);

            ns_spi_readfromaddress(p_data[12], p_data[11], p_data[15]);
            break;

        case SUBC_ENABLE_IMU:
            ESP_LOGI(TAG, "SUBC - Enable IMU (Sixaxis).");
            ns_report_setack(0x80);
            // Enable Sixaxis
            break;

        case SUBC_ENABLE_VIBRATION:
            ns_report_setack(0x80);

            if(p_data[11])
            {
                ESP_LOGI(TAG, "SUBC - Enable fast frequency");
                ns_input_frequency = INPUT_FREQUENCY_FAST;
            }
            else 
            {
                ESP_LOGI(TAG, "SUBC - Enable slow frequency");
                ns_input_frequency = INPUT_FREQUENCY_SLOW;
            }

            // TO-DO - Enable vibration
            break;

        case SUBC_SET_PLAYER:
            ESP_LOGI(TAG, "SUBC - Set player number/lights.");
            ns_report_setack(0x80);
            // TO-DO - Set player number
            break;

        case SUBC_SET_MCUCONFIG:
            ESP_LOGI(TAG, "SUBC - Set MCU configuration.");
            ns_report_setack(0x80);
            break;
        
        default:
            ESP_LOGI(TAG, "SUBC - Unrecognized subcommand.");
            ns_report_setack(0x80);
            break;

    }
}
