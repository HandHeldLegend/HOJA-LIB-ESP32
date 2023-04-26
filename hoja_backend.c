#include "hoja_backend.h"

#define SLEEP_BUTTON_TIME 3000

hoja_button_data_s hoja_button_data = {};
hoja_button_data_s hoja_processed_buttons = {};
hoja_analog_data_s hoja_analog_data = {};
uint8_t _hoja_battery_level = 128;

button_remap_s _hoja_remaps = {};
hoja_dpadmode_t _hoja_dpad_mode = DPAD_MODE_STANDARD;

bool _hoja_remap_enable = false;
uint16_t sleep_timer = 0;

// Used to update the battery level. 
// This requires user responsibility to measure and
// report battery level.
void hoja_set_battery_level(uint8_t level)
{
    _hoja_battery_level = level;
}

uint8_t hoja_get_battery_level()
{
    return _hoja_battery_level;
}

void hoja_set_dpad_mode(hoja_dpadmode_t mode)
{
    _hoja_dpad_mode = mode;
}

void hoja_process_dpad()
{
    switch(_hoja_dpad_mode)
    {
        default:
        case DPAD_MODE_STANDARD:
            break;

        case DPAD_MODE_ANALOGONLY:
            hoja_analog_data.ls_x = DPAD_ANALOG_CENTER;
            hoja_analog_data.ls_y = DPAD_ANALOG_CENTER;
            hoja_analog_data.ls_x -= (hoja_processed_buttons.dpad_left) ? DPAD_DISTANCE : 0;
            hoja_analog_data.ls_x += (hoja_processed_buttons.dpad_right) ? DPAD_DISTANCE : 0;
            hoja_analog_data.ls_y -= (hoja_processed_buttons.dpad_down) ? DPAD_DISTANCE : 0;
            hoja_analog_data.ls_y += (hoja_processed_buttons.dpad_up) ? DPAD_DISTANCE : 0;
            hoja_processed_buttons.buttons_all &= 0xFFFFFFFFFFFFFFF0;
            break;
    }
}

// Resets all button data set
void hoja_button_reset()
{
    memset(&hoja_button_data, 0x00, sizeof(hoja_button_data));
}

uint64_t hoja_get_remap(void)
{
    return _hoja_remaps.val;
}

uint64_t hoja_get_remap_default(void)
{
    button_remap_s map = {
        .dpad_up    = MAPCODE_DUP,
        .dpad_down  = MAPCODE_DDOWN,
        .dpad_left  = MAPCODE_DLEFT,
        .dpad_right = MAPCODE_DRIGHT,
        
        .button_up  = MAPCODE_B_UP,
        .button_down = MAPCODE_B_DOWN,
        .button_left = MAPCODE_B_LEFT,
        .button_right = MAPCODE_B_RIGHT,

        .trigger_l = MAPCODE_T_L,
        .trigger_r = MAPCODE_T_R,
        .trigger_zl = MAPCODE_T_ZL,
        .trigger_zr = MAPCODE_T_ZR,
        
        .button_start = MAPCODE_B_START,
        .button_select = MAPCODE_B_SELECT,
        .button_stick_left = MAPCODE_B_STICKL,
        .button_stick_right = MAPCODE_B_STICKR,
    };

    return map.val;
}

// A value of 0x00 in will reset the map to defaults.
void hoja_load_remap(uint64_t button_map)
{
    if (!button_map)
    {
        _hoja_remaps.val = hoja_get_remap_default();
    }
    else
    {
        _hoja_remaps.val = button_map;
    }
}

void hoja_button_remap_process()
{
    hoja_processed_buttons.buttons_all      = hoja_button_data.buttons_all;
    hoja_processed_buttons.buttons_system   = hoja_button_data.buttons_system;

    if (!_hoja_remap_enable)
    {
        return;
    }

    hoja_processed_buttons.buttons_all = 0;

    hoja_processed_buttons.buttons_all |= hoja_button_data.dpad_up    << _hoja_remaps.dpad_up;
    hoja_processed_buttons.buttons_all |= hoja_button_data.dpad_down  << _hoja_remaps.dpad_down;
    hoja_processed_buttons.buttons_all |= hoja_button_data.dpad_left  << _hoja_remaps.dpad_left;
    hoja_processed_buttons.buttons_all |= hoja_button_data.dpad_right << _hoja_remaps.dpad_right;

    hoja_processed_buttons.buttons_all |= hoja_button_data.button_up      << _hoja_remaps.button_up;
    hoja_processed_buttons.buttons_all |= hoja_button_data.button_down    << _hoja_remaps.button_down;
    hoja_processed_buttons.buttons_all |= hoja_button_data.button_left    << _hoja_remaps.button_left;
    hoja_processed_buttons.buttons_all |= hoja_button_data.button_right   << _hoja_remaps.button_right;

    hoja_processed_buttons.buttons_all |= hoja_button_data.trigger_l      << _hoja_remaps.trigger_l;
    hoja_processed_buttons.buttons_all |= hoja_button_data.trigger_zl     << _hoja_remaps.trigger_zl;
    hoja_processed_buttons.buttons_all |= hoja_button_data.trigger_r      << _hoja_remaps.trigger_r;
    hoja_processed_buttons.buttons_all |= hoja_button_data.trigger_zr     << _hoja_remaps.trigger_zr;

    hoja_processed_buttons.buttons_all |= hoja_button_data.button_start       << _hoja_remaps.button_start;
    hoja_processed_buttons.buttons_all |= hoja_button_data.button_select      << _hoja_remaps.button_select;
    hoja_processed_buttons.buttons_all |= hoja_button_data.button_stick_left  << _hoja_remaps.button_stick_left;
    hoja_processed_buttons.buttons_all |= hoja_button_data.button_stick_right << _hoja_remaps.button_stick_right;
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

