#include "core_switch_reports.h"

uint8_t ns_input_report_id = 0x00;
uint8_t ns_input_report[NS_INPUT_REPORT_BUFFERSIZE] = {};
uint16_t ns_input_report_size = 50;
uint16_t ns_report_timer;

// Clear the outgoing report data
void ns_report_clear(void)
{
    memset(ns_input_report, 0, sizeof(ns_input_report));
}

// Set the outgoing report ID (Byte 0).
void ns_report_setid(uint8_t report_id)
{
    ns_input_report_id = report_id;
}

// Set the outgoing report acknowledgement byte (Byte 13).
void ns_report_setack(uint8_t ack)
{
    ns_input_report[12] = ack;
}

// Set the outgoing report sub command parameter (Byte 14).
void ns_report_setsubcmd(uint8_t sub_cmd)
{
    ns_input_report[13] = sub_cmd;
}

// Set the outgoing report timer parameter and increment the timer (Byte 1).
void ns_report_settimer(void)
{
    ns_input_report[0] = ns_report_timer;
    if(ns_report_timer + 1 > 255)
    {
        ns_report_timer = 0;
    }
    else ns_report_timer += 1;
}

// Set the outgoing report to include battery/connection byte (Byte 2).
void ns_report_setbattconn(void)
{
    uint8_t tmp_bat = (ns_controller_data.battery_level_full << 4);
    tmp_bat |= ns_controller_data.connection_info;
    ns_input_report[1] = tmp_bat;
}

// Sets the input report for full mode.
void ns_report_setinputreport_full(ns_input_long_s *ns_input_long)
{
    ns_input_report[2] = ns_input_long->right_buttons;
    ns_input_report[3] = ns_input_long->shared_buttons;
    ns_input_report[4] = ns_input_long->left_buttons;

    // Set sticks directly from hoja_analog_data
    // Saves cycles :)
    ns_input_report[5] = (ns_input_long->ls_x & 0xFF);
    ns_input_report[6] = (ns_input_long->ls_x & 0xF00) >> 8;
    //ns_input_report[7] |= (g_stick_data.lsy & 0xF) << 4;
    ns_input_report[7] = (ns_input_long->ls_y & 0xFF0) >> 4;
    ns_input_report[8] = (ns_input_long->rs_x & 0xFF);
    ns_input_report[9] = (ns_input_long->rs_x & 0xF00) >> 8;
    ns_input_report[10] = (ns_input_long->rs_y & 0xFF0) >> 4;
    ns_input_report[11] = 0x08;
}

// Sets the input report for short mode.
void ns_report_setinputreport_short(ns_input_short_s *ns_input_short)
{
    ns_input_report[0] = ns_input_short->buttons_first;
    ns_input_report[1] = ns_input_short->buttons_second;
    ns_input_report[2] = 0x8; //ns_input_short.stick_hat;

    // To-do: Sticks
    ns_input_report[3] = ns_input_short->l_stick[0];
    ns_input_report[4] = ns_input_short->l_stick[1];
    ns_input_report[5] = ns_input_short->l_stick[2];
    ns_input_report[6] = ns_input_short->l_stick[3];
    ns_input_report[7] = 0;
    ns_input_report[8] = 0;
    ns_input_report[9] = 0;
    ns_input_report[10] = 0;
}

//This sets a portion of the input report in bulk. Cleaner input report setting
void ns_report_bulkset(uint8_t start_idx, uint8_t* data, uint8_t len)
{
    memcpy(&ns_input_report[start_idx], data, len);
}

// Set the input report data for the 'Get Device Info' subcommand 0x02.
void ns_report_sub_setdevinfo(void)
{
    ns_input_report[14] = NS_FW_PRIMARY;
    ns_input_report[15] = NS_FW_SECONDARY;

    ns_input_report[16] = ns_controller_data.controller_type_primary;
    ns_input_report[17] = ns_controller_data.controller_type_secondary;
    
    ns_report_bulkset(18, loaded_settings.ns_client_bt_address, 6); // Send client bt address Big Endian
    ns_input_report[24] = 0x00; //0x1C; // Default 0x01
    ns_input_report[25] = 0x02; //NS_COLOR_SET; 

    ns_input_report_size += 12;
}

/**
 * @brief Set the input report data for the 'Triggers elapsed time' subcommand 0x04.
 * This will set the same elapsed time for all relevant buttons since this only seems
 * to be used in connection mode where the L/R buttons need to be pressed to connect.
 * 
 * Some technical notes for you nerds out there-- This is used to help set up which player number a controller is.
 * A controller on Nintendo switch is 'connected' but then nothing happens until the console knows when the L/R
 * (Or ZL/ZR or SL/SR) buttons have been pressed last. It seems a bit convoluted to me but whatever!
 * 
 * @param[in] time_10_ms Time of milisecconds divided by 10. Example: 1000ms/10 = 100. 
 */
void ns_report_sub_triggertime(uint16_t time_10_ms)
{
    uint8_t upper_ms = 0xFF & time_10_ms;
    uint8_t lower_ms = (0xFF00 & time_10_ms) >> 8;

    // Set all button groups
    // L - 15, 16
    // R - 17, 18
    // ZL - 19, 20
    // ZR - 21, 22
    // SL - 23, 24
    // SR - 25, 26
    // Home - 27, 28

    for(uint8_t i = 0; i < 14; i+=2)
    {
        ns_input_report[14 + i] = upper_ms;
        ns_input_report[15 + i] = lower_ms;
    }

    ns_input_report_size += 14;
}

// Handle sub command report for set ship mode
void ns_report_sub_setshipmode(uint8_t ship_mode)
{
    if (ship_mode != 0x00 || ship_mode != 0x01) return;

    // Handle adjusting ship mode.
    hoja_event_cb(HOJA_EVT_SYSTEM, HEVT_API_SHUTDOWN, 0x00);
}

// Length of HID report for short inputs.
#define NS_REPORT_SHORT_LEN 12

// Task used to send short or simple inputs.
// Only sends input when data is changed! SOOPER.
void ns_report_task_sendshort(void * parameters)
{
    const char* TAG = "ns_report_task_sendshort";
    ESP_LOGI(TAG, "Sending short (0x3F) reports on core %d\n", xPortGetCoreID());
    ns_input_short_s ns_input_short = {0};
    ns_input_short_s ns_input_short_last = {0};

    for(;;)
    {
        ns_report_clear();

        // TODO remove and replace with defines for all report IDs.
        // ns_report_setid(NS_REPORT_SHORT);

        ns_input_report_size = 12;
        // Update buttons and analog
        ns_input_translate_short(&ns_input_short);
        ns_report_setinputreport_short(&ns_input_short);

        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, NS_REPORT_SHORT, NS_REPORT_SHORT_LEN, ns_input_report);
        
        vTaskDelay(16 / portTICK_PERIOD_MS); 
    }
}

// Length of HID report for standard or full inputs.
#define NS_REPORT_STANDARD_LEN  13

// Task used to send standard or "full" inputs.
// Only sends input when there is a change to the 
// data! Very nice.
void ns_report_task_sendstandard(void * parameters)
{
    const char* TAG = "ns_report_task_sendstandard";
    ESP_LOGI(TAG, "Sending standard (0x30) reports on core %d\n", xPortGetCoreID());

    ns_input_long_s ns_input_long   = {0};
    ns_input_long_s ns_input_long_last   = {0};

    for(;;)
    {
        // Check the sticks once
        hoja_analog_cb();
        hoja_button_remap_process();
        ns_input_translate_full(&ns_input_long);

        // Only send an input update if our comparison is different.
        // Saves power and processing :)
        if (ns_input_compare_full(&ns_input_long, &ns_input_long_last))
        {
            ns_report_clear();
        
            // Report ID is const now. REMOVE
            //ns_report_setid(NS_REPORT_FULL);
            
            ns_report_setinputreport_full(&ns_input_long);

            ns_report_settimer();
            ns_report_setbattconn();
            //ns_input_report_size = 13;
            ns_input_report[12] = 0x70;

            esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, NS_REPORT_FULL, NS_REPORT_STANDARD_LEN, ns_input_report);

            // Copy data into "last" buffer for future comparison
            memcpy(&ns_input_long_last, &ns_input_long, sizeof(ns_input_long_s));
        }
        
        // Reset HOJA buttons
        hoja_button_reset();
        vTaskDelay(8 / portTICK_PERIOD_MS);
    }
}

// Send a stream of empty reports
// Intended to get the Nintendo Switch console
// to respond to us because the controller gets lonely when
// it pairs initially :(
void ns_report_task_sendempty(void * parameters)
{
    const char* TAG = "ns_report_task_sendempty";
    ESP_LOGI(TAG, "Sending empty reports on core %d\n", xPortGetCoreID());
    uint8_t tmp[2] = {ns_input_report[0], 0x00};

    for(;;)
    {   
        // Set report timer
        ns_report_settimer();
        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xA1, 2, tmp);
        vTaskDelay(8 / portTICK_PERIOD_MS);
    }
}
