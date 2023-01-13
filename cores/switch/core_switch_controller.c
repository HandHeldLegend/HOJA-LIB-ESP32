#include "core_switch_controller.h"

ns_controller_data_s ns_controller_data;

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

void ns_controller_setinputreportmode(uint8_t report_mode)
{
    char* TAG = "ns_controller_setinputreportmode";

    ESP_LOGI(TAG, "Switching to input mode: %04x", report_mode);
    ns_currentReportMode = report_mode;
    switch(report_mode)
    {
        // Blank report mode
        case 0xFF:
            ESP_LOGI(TAG, "Starting short report mode.");
            if (ns_ReportModeHandle != NULL)
            {
                vTaskDelete(ns_ReportModeHandle);
                ns_ReportModeHandle = NULL;
            }
            xTaskCreatePinnedToCore(ns_report_task_sendempty, 
                                    "Blank Send Task", 2048,
                                    NULL, 0, &ns_ReportModeHandle, 0);
            break;

        // Standard
        case 0x30:
            ESP_LOGI(TAG, "Starting standard report mode.");
            if (ns_ReportModeHandle != NULL)
            {
                vTaskDelete(ns_ReportModeHandle);
                ns_ReportModeHandle = NULL;
            }

            // ns_report_task_sendstandard
            xTaskCreatePinnedToCore(ns_report_task_sendstandard, 
                                "Standard Send Task", 2048,
                                NULL, 0, &ns_ReportModeHandle, 0);

            break;
        // NFC/IR
        case 0x31:  
            break;
        // SimpleHID. Data pushes only on button press/release
        case 0x3F:
            ESP_LOGI(TAG, "Starting short report mode.");
            if (ns_ReportModeHandle != NULL)
            {
                vTaskDelete(ns_ReportModeHandle);
                ns_ReportModeHandle = NULL;
            }
            xTaskCreatePinnedToCore(ns_report_task_sendshort, 
                                    "Short (0x3F) Send Task", 2048,
                                    NULL, 0, &ns_ReportModeHandle, 0);
            break;
            ns_controller_data.input_report_mode = report_mode;
            break;

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
