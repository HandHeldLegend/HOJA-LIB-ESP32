#include "core_switch_controller.h"

ns_controller_data_s ns_controller_data;

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
