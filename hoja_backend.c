#include "hoja_backend.h"

#define SLEEP_BUTTON_TIME 3000

hoja_button_data_s hoja_button_data = {};
hoja_button_data_s hoja_processed_buttons = {};
hoja_analog_data_s hoja_analog_data = {};
uint8_t _hoja_battery_level = 128;
bool _hoja_external_power = false;

button_remap_s hoja_remaps = {};

bool _hoja_remap_enable = false;
uint16_t sleep_timer = 0;

// Used to update the battery level. 
// This requires user responsibility to measure and
// report battery level.
void hoja_set_battery_lvl(uint8_t level)
{
    
    _hoja_battery_level = level;
}

void hoja_set_external_power(bool plugged)
{
    _hoja_external_power = plugged;
}

bool hoja_get_external_power()
{
    return _hoja_external_power;
}

uint8_t hoja_get_battery_lvl()
{
    return _hoja_battery_level;
}

// Resets all button data set
void hoja_button_reset()
{
    memset(&hoja_button_data, 0x00, sizeof(hoja_button_data));
}

void hoja_button_remap_process()
{
    memcpy(&hoja_processed_buttons, &hoja_button_data, sizeof(hoja_button_data_s));

    if (!_hoja_remap_enable)
    {
        return;
    }

    hoja_processed_buttons.buttons_all = 0;

    hoja_processed_buttons.buttons_all |= hoja_button_data.dpad_up    << hoja_remaps.dpad_up;
    hoja_processed_buttons.buttons_all |= hoja_button_data.dpad_down  << hoja_remaps.dpad_down;
    hoja_processed_buttons.buttons_all |= hoja_button_data.dpad_left  << hoja_remaps.dpad_left;
    hoja_processed_buttons.buttons_all |= hoja_button_data.dpad_right << hoja_remaps.dpad_right;

    hoja_processed_buttons.buttons_all |= hoja_button_data.button_up      << hoja_remaps.button_up;
    hoja_processed_buttons.buttons_all |= hoja_button_data.button_down    << hoja_remaps.button_down;
    hoja_processed_buttons.buttons_all |= hoja_button_data.button_left    << hoja_remaps.button_left;
    hoja_processed_buttons.buttons_all |= hoja_button_data.button_right   << hoja_remaps.button_right;

    hoja_processed_buttons.buttons_all |= hoja_button_data.trigger_l      << hoja_remaps.trigger_l;
    hoja_processed_buttons.buttons_all |= hoja_button_data.trigger_zl     << hoja_remaps.trigger_zl;
    hoja_processed_buttons.buttons_all |= hoja_button_data.trigger_r      << hoja_remaps.trigger_r;
    hoja_processed_buttons.buttons_all |= hoja_button_data.trigger_zr     << hoja_remaps.trigger_zr;

    hoja_processed_buttons.buttons_all |= hoja_button_data.button_start       << hoja_remaps.button_start;
    hoja_processed_buttons.buttons_all |= hoja_button_data.button_select      << hoja_remaps.button_select;
    hoja_processed_buttons.buttons_all |= hoja_button_data.button_stick_left  << hoja_remaps.button_stick_left;
    hoja_processed_buttons.buttons_all |= hoja_button_data.button_stick_right << hoja_remaps.button_stick_right;
}

void hoja_button_remap_enable(bool enable)
{
    _hoja_remap_enable = enable;
}

void hoja_button_task(void * parameters)
{
    const char* TAG = "hoja_button_task";
    ESP_LOGI(TAG, "Starting task...");
    // Buttons update at a 2000hz rate.
    for(;;)
    {
        hoja_button_cb();
        
        if (hoja_button_data.button_sleep)
        {
            sleep_timer += 1;

            if (sleep_timer >= SLEEP_BUTTON_TIME && hoja_button_data.button_pair)
            {
                // Call function to enable pairing mode
                // As of now this only exists in Nintendo Switch bluetooth mode
                if (hoja_current_core == HOJA_CORE_NS && hoja_current_status == HOJA_STATUS_RUNNING)
                {
                    ns_startpairing();
                }
            }
            else if (sleep_timer >= SLEEP_BUTTON_TIME)
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

void hoja_set_battery_level(uint8_t level)
{
    _hoja_battery_level = level;
}

uint8_t hoja_get_battery_level()
{
    return _hoja_battery_level;
}
