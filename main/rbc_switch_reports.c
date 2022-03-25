#include "rbc_switch_reports.h"

uint8_t ns_input_report[362] = {};
uint16_t ns_input_report_size = 50;
uint16_t ns_report_timer;

uint8_t ns_input_frequency = INPUT_FREQUENCY_SLOW;
uint8_t ns_timeout_counter = 0;

// Clear the outgoing report data
void ns_report_clear(void)
{
    memset(ns_input_report, 0, sizeof(ns_input_report));
}

// Set the outgoing report ID (Byte 0).
void ns_report_setid(uint8_t report_id)
{
    ns_input_report[0] = report_id;
}

// Set the outgoing report acknowledgement byte (Byte 13).
void ns_report_setack(uint8_t ack)
{
    ns_input_report[13] = ack;
}

// Set the outgoing report sub command parameter (Byte 14).
void ns_report_setsubcmd(uint8_t sub_cmd)
{
    ns_input_report[14] = sub_cmd;
}

// Set the outgoing report timer parameter and increment the timer (Byte 1).
void ns_report_settimer(void)
{
    ns_input_report[1] = ns_report_timer;
    if(ns_report_timer + 1 > 255)
    {
        ns_report_timer = 0;
    }
    else ns_report_timer += 1;
}

// Set the outgoing report to include battery/connection byte (Byte 2).
void ns_report_setbattconn(void)
{
    uint8_t tmp = (ns_controller_data.battery_level_full << 4);
    tmp |= ns_controller_data.connection_info;
    ns_input_report[2] = tmp;
}

// Sets the outgoing report according to the given button mode.
void ns_report_setbuttons(ns_report_buttonmode_t button_mode)
{
    switch(button_mode)
    {
        // Sets bytes 1 - 11
        case NS_BM_SHORT:
            ns_input_report[1] = ns_input_short.buttons_first;
            ns_input_report[2] = ns_input_short.buttons_second;
            ns_input_report[3] = ns_input_short.stick_hat;

            // To-do: Sticks
            ns_input_report[4] = 0;
            ns_input_report[5] = 0;
            ns_input_report[6] = 0;
            ns_input_report[7] = 0;
            ns_input_report[8] = 0;
            ns_input_report[9] = 0;
            ns_input_report[10] = 0;
            ns_input_report[11] = 0;

            break;

        // Sets bytes 3 - 11
        case NS_BM_LONG:
            ns_input_report[3] = ns_input_long.right_buttons;
            ns_input_report[4] = ns_input_long.shared_buttons;
            ns_input_report[5] = ns_input_long.left_buttons;

            // To-do: Sticks
            ns_input_report[6] = 0;
            ns_input_report[7] = 0;
            ns_input_report[8] = 0;
            ns_input_report[9] = 0;
            ns_input_report[10] = 0;
            ns_input_report[11] = 0;
            ns_input_report[12] = 0x08;
            break;
    }
    
}

//This sets a portion of the input report in bulk. Cleaner input report setting
void ns_report_bulkset(uint8_t start_idx, uint8_t* data, uint8_t len)
{
    for(int i = 0; i < len; i++)
    {
        ns_input_report[i+start_idx] = data[i];
    }
}

// Set the input report data for the 'Get Device Info' subcommand 0x02.
void ns_report_sub_setdevinfo(void)
{
    ns_input_report[15] = ns_controller_data.fw_primary;
    ns_input_report[16] = ns_controller_data.fw_secondary;
    ns_input_report[17] = ns_controller_data.controller_type;
    ns_input_report[18] = 0x02; // Experiment with this later :)
    ns_report_bulkset(19, ns_controller_data.client_mac_address, 6); // Send client bt address Big Endian
    ns_input_report[25] = 0x01;
    ns_input_report[26] = ns_controller_data.color_set;

    ns_input_report_size += 12;
}

/**
 * @brief Set the input report data for the 'Triggers elapsed time' subcommand 0x04.
 * This will set the same elapsed time for all relevant buttons since this only seems
 * to be used in connection mode where the L/R buttons need to be pressed to connect.
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
        ns_input_report[15 + i] = upper_ms;
        ns_input_report[16 + i] = lower_ms;
    }

    ns_input_report_size += 14;
}

// 
void ns_report_sub_setshipmode(uint8_t ship_mode)
{
    if (ship_mode != 0x00 || ship_mode != 0x01) return;

    // Handle adjusting ship mode.
}

void ns_report_task_intrrtimeout(void * parameters)
{   
    const char* TAG = "ns_report_task_interrtimeout";

    while(1)
    {
        if (ns_timeout_counter > 0)
        {
            
            ns_timeout_counter = 1;
            if (ns_input_frequency != INPUT_FREQUENCY_SLOW)
            {
                ESP_LOGI(TAG, "Switching to lower frequency.");
                ns_input_frequency = INPUT_FREQUENCY_SLOW;
            }
        }
        else
        {
            if (ns_input_frequency != INPUT_FREQUENCY_FAST)
            {
                ESP_LOGI(TAG, "Switching to higher frequency");
                ns_input_frequency = INPUT_FREQUENCY_FAST;
            }
        }
        ns_timeout_counter += 1;
        vTaskDelay( 50 / portTICK_PERIOD_MS );
    }
    
}

// Set a 0x3F or short input report
void ns_report_task_sendshort()
{
    const char* TAG = "ns_report_task_sendshort";
    ESP_LOGI(TAG, "Sending short (0x3F) report");

    ns_report_clear();
    ns_report_settimer();
    ns_report_setid(0x3F);
    ns_report_setbuttons(NS_BM_SHORT);
    esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0x3F, ns_input_report_size, ns_input_report);
    
}

void ns_report_task_sendstandard(void * parameters)
{
    const char* TAG = "ns_report_task_sendstandard";
    ESP_LOGI(TAG, "Sending standard (0x30) reports on core %d\n", xPortGetCoreID());

    while(1)
    {
        ns_report_clear();
        ns_report_settimer();
        ns_report_setid(0x30);
        ns_report_setbuttons(NS_BM_LONG);
        ns_report_setbattconn();
        ns_input_report_size = 12;
        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0x3F, ns_input_report_size, ns_input_report);

        vTaskDelay(INPUT_FREQUENCY_SLOW / portTICK_PERIOD_MS);
    }
    
    
}

void ns_report_task_sendempty(void * parameters)
{
    const char* TAG = "ns_report_task_sendempty";
    ESP_LOGI(TAG, "Sending empty (0xFF) reports on core %d\n", xPortGetCoreID());

    while(1)
    {
        ns_report_clear();
        ns_input_report[0] = 0x00;
        ns_input_report[1] = 0x00;
        ns_input_report_size = 2;
        esp_bt_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, 0xA1, ns_input_report_size, ns_input_report);

        vTaskDelay(INPUT_FREQUENCY_SLOW / portTICK_PERIOD_MS);
    }
}
