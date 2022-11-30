#include "hoja_backend.h"

#define SLEEP_BUTTON_TIME 6000

hoja_button_data_s hoja_button_data = {};
hoja_analog_data_s hoja_analog_data = {};
uint16_t sleep_timer = 0;

// Resets all button data set
void hoja_button_reset()
{
    memset(&hoja_button_data, 0x00, sizeof(hoja_button_data));
}

void hoja_button_task(void * parameters)
{
    const char* TAG = "hoja_button_task";
    vTaskDelay(2000/portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "Starting task...");
    // Buttons update at a 2000hz rate.
    for(;;)
    {
        hoja_button_cb(&hoja_button_data);
        if (hoja_button_data.button_sleep)
        {
            sleep_timer += 1;
            if (sleep_timer >= SLEEP_BUTTON_TIME)
            {
                // Send shutdown event if sleep timer is confirmed overflow.
                hoja_event_cb(HOJA_EVT_SYSTEM, HOJA_SHUTDOWN, 0x00);
            }
        }
        else
        {
            sleep_timer = 0;
        }
        vTaskDelay(0.5/portTICK_PERIOD_MS);
    }
}
