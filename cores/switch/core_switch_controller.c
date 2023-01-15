#include "core_switch_controller.h"


ns_controller_data_s ns_controller_data;
ns_report_mode_t _ns_reporting_mode = NS_REPORT_MODE_IDLE;
ns_power_handle_t _ns_power_handle;
TaskHandle_t ns_bt_task_handle = NULL;

// Sets up various aspects of controller memory
// which is used by SPI emulation. 
void ns_controller_setup_memory(void)
{
    const char* TAG = "ns_controller_setup_memory";

    switch(_ns_subcore)
    {
        case NS_TYPE_UNSET:
        default:
            ESP_LOGE(TAG, "Invalid core trying to start. Defaulting to ProCon.");
        case NS_TYPE_PROCON:
            ns_controller_data.controller_type_primary = 0x03;
            ns_controller_data.controller_type_secondary = 0x02;
            break;
        case NS_TYPE_N64:
            ns_controller_data.controller_type_primary = 0x0C;
            ns_controller_data.controller_type_secondary = 0x11;
            break;
        case NS_TYPE_SNES:
            ns_controller_data.controller_type_primary = 0x0B; //11
            ns_controller_data.controller_type_secondary = 0x2; //2
            break;
        case NS_TYPE_FC:
            ns_controller_data.controller_type_primary = 0x07;
            ns_controller_data.controller_type_secondary = 0x02;
            break;
        case NS_TYPE_NES:
            ns_controller_data.controller_type_primary = 0x09;
            ns_controller_data.controller_type_secondary = 0x02;
            break;
        case NS_TYPE_GENESIS:
            ns_controller_data.controller_type_primary = 0x0D;
            ns_controller_data.controller_type_secondary = 0x02;
            break;
    }
}

// Apply stick calibration params to controller emulated memory.
void ns_controller_applycalibration(void)
{
    const char* TAG = "ns_input_stickcalibration";

    uint16_t tmp_max_x;
    uint16_t tmp_min_x;
    uint16_t tmp_max_y;
    uint16_t tmp_min_y;

    tmp_max_x = loaded_settings.sx_max - loaded_settings.sx_center;
    tmp_min_x = loaded_settings.sx_center - loaded_settings.sx_min;

    tmp_max_y = loaded_settings.sy_max - loaded_settings.sy_center;
    tmp_min_y = loaded_settings.sy_center + loaded_settings.sy_min;

    ns_input_stickcaldata.l_stick_cal[0] = 0xB2;
    ns_input_stickcaldata.l_stick_cal[1] = 0xA1;
    ns_input_stickcaldata.l_stick_cal[2] = tmp_max_x & 0xFF;
    ns_input_stickcaldata.l_stick_cal[3] = (tmp_max_x & 0xF00) >> 8;
    ns_input_stickcaldata.l_stick_cal[3] |= (tmp_max_y & 0xF) >> 4;
    ns_input_stickcaldata.l_stick_cal[4] = (tmp_max_y & 0xFF0) >> 4;
    ns_input_stickcaldata.l_stick_cal[5] = loaded_settings.sx_center & 0xFF;
    ns_input_stickcaldata.l_stick_cal[6] = (loaded_settings.sx_center & 0xF00) >> 8;
    ns_input_stickcaldata.l_stick_cal[6] |= (loaded_settings.sy_center & 0xF) << 4;
    ns_input_stickcaldata.l_stick_cal[7] = (loaded_settings.sy_center & 0xFF0) >> 4;
    ns_input_stickcaldata.l_stick_cal[8] = tmp_min_x & 0xFF;
    ns_input_stickcaldata.l_stick_cal[9] = (tmp_min_x & 0xF00) >> 8;
    ns_input_stickcaldata.l_stick_cal[9] |= (tmp_min_y & 0xF) << 4;
    ns_input_stickcaldata.l_stick_cal[10] = tmp_min_y >> 4;

    ns_input_stickcaldata.r_stick_cal[0] = 0xB2;
    ns_input_stickcaldata.r_stick_cal[1] = 0xA1;
    ns_input_stickcaldata.r_stick_cal[2] = tmp_max_x & 0xFF;
    ns_input_stickcaldata.r_stick_cal[3] = (tmp_max_x & 0xF00) >> 8;
    ns_input_stickcaldata.r_stick_cal[3] |= (tmp_max_y & 0xF) >> 4;
    ns_input_stickcaldata.r_stick_cal[4] = (tmp_max_y & 0xFF0) >> 4;
    ns_input_stickcaldata.r_stick_cal[5] = loaded_settings.sx_center & 0xFF;
    ns_input_stickcaldata.r_stick_cal[6] = (loaded_settings.sx_center & 0xF00) >> 8;
    ns_input_stickcaldata.r_stick_cal[6] |= (loaded_settings.sy_center & 0xF) << 4;
    ns_input_stickcaldata.r_stick_cal[7] = (loaded_settings.sy_center & 0xFF0) >> 4;
    ns_input_stickcaldata.r_stick_cal[8] = tmp_min_x & 0xFF;
    ns_input_stickcaldata.r_stick_cal[9] = (tmp_min_x & 0xF00) >> 8;
    ns_input_stickcaldata.r_stick_cal[9] |= (tmp_min_y & 0xF) << 4;
    ns_input_stickcaldata.r_stick_cal[10] = tmp_min_y >> 4;

    ESP_LOGI(TAG, "Nintendo Switch Core stick calibration translated.");
}

void ns_controller_setinputreportmode(uint8_t report_mode)
{
    char* TAG = "ns_controller_setinputreportmode";

    ESP_LOGI(TAG, "Switching to input mode: %04x", report_mode);
    switch(report_mode)
    {
        // Standard
        case 0x30:
            ESP_LOGI(TAG, "Starting standard report mode.");
            ns_controller_input_task_set(NS_REPORT_MODE_FULL);
            break;

        // SimpleHID. Data pushes only on button press/release
        case 0x3F:
            ESP_LOGI(TAG, "Starting short report mode.");
            ns_controller_input_task_set(NS_REPORT_MODE_SIMPLE);
            break;

        // NFC/IR
        case 0x31:
        case 0x00 ... 0x03:
        default:
            // ERROR
            break;
    }
}

void ns_controller_setshipmode(uint8_t ship_mode)
{
    return;
}

void ns_controller_sleep_handle(ns_power_handle_t power_type)
{
    const char* TAG = "ns_controller_sleep_handle";
    switch(power_type)
    {
        default:
        case NS_POWER_AWAKE:
            ESP_LOGI(TAG, "Controller set to awake.");
            _ns_power_handle = NS_POWER_AWAKE;
            ns_controller_input_task_set(_ns_reporting_mode);
            break;

        case NS_POWER_SLEEP:
            ESP_LOGI(TAG, "Controller set to sleep.");
            _ns_power_handle = NS_POWER_SLEEP;
            if (ns_bt_task_handle != NULL)
            {
                vTaskDelete(ns_bt_task_handle);
                ns_bt_task_handle = NULL;
            }
            break;
    }
}

void ns_controller_input_task_set(ns_report_mode_t report_mode_type)
{
    const char* TAG = "ns_controller_input_task_set";
    switch(report_mode_type)
    {
        default:
        case NS_REPORT_MODE_IDLE:
            ESP_LOGI(TAG, "Start input IDLE task...");
            // Just stop all tasks and clear report mode internal.
            if (ns_bt_task_handle != NULL)
            {
                vTaskDelete(ns_bt_task_handle);
                ns_bt_task_handle = NULL;
            }
            _ns_reporting_mode = NS_REPORT_MODE_IDLE;
            break;

        case NS_REPORT_MODE_BLANK:
            ESP_LOGI(TAG, "Start input BLANK task...");
            if (ns_bt_task_handle != NULL)
            {
                vTaskDelete(ns_bt_task_handle);
                ns_bt_task_handle = NULL;
            }

            _ns_reporting_mode = NS_REPORT_MODE_BLANK;
            xTaskCreatePinnedToCore(ns_report_task_sendempty, 
                                "Blank Send Task", 2048,
                                NULL, 0, &ns_bt_task_handle, 0);
            break;

        case NS_REPORT_MODE_FULL:
            ESP_LOGI(TAG, "Start input FULL task...");
            if (ns_bt_task_handle != NULL)
            {
                vTaskDelete(ns_bt_task_handle);
                ns_bt_task_handle = NULL;
            }
            _ns_reporting_mode = NS_REPORT_MODE_FULL;
            xTaskCreatePinnedToCore(ns_report_task_sendstandard, 
                            "Standard Send Task", 2048,
                            NULL, 0, &ns_bt_task_handle, 0);
            break;

        case NS_REPORT_MODE_SIMPLE:
            ESP_LOGI(TAG, "Start input SIMPLE task...");
            if (ns_bt_task_handle != NULL)
            {
                vTaskDelete(ns_bt_task_handle);
                ns_bt_task_handle = NULL;
            }

            // Set the internal reporting mode.
            _ns_reporting_mode = NS_REPORT_MODE_SIMPLE;
            xTaskCreatePinnedToCore(ns_report_task_sendshort, 
                            "Standard Send Task", 2048,
                            NULL, 0, &ns_bt_task_handle, 0);

            break;
    }
}
