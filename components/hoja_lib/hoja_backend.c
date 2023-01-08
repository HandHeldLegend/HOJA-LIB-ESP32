#include "hoja_backend.h"

#define SLEEP_BUTTON_TIME 3000

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
                ESP_LOGI(TAG, "Sleep triggered by select button!");
                // Send shutdown event if sleep timer is confirmed overflow.
                hoja_event_cb(HOJA_EVT_SYSTEM, HEVT_API_SHUTDOWN, 0x00);
                sleep_timer = 0;
            }
        }
        else
        {
            sleep_timer = 0;
        }

        // If we don't have an active core, reset the buttons because
        // otherwise they won't reset!
        if (hoja_current_status <= HOJA_STATUS_INITIALIZED)
        {
            hoja_button_reset();
        }
        vTaskDelay(0.5/portTICK_PERIOD_MS);
    }
}
