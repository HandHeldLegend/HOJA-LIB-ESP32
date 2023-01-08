#include "core_switch_input.h"

ns_input_short_s ns_input_short = {};
ns_input_long_s ns_input_long = {};
ns_input_stickcaldata_s ns_input_stickcaldata = {};

// Translate the global input from RBC Core to the Switch
// native core input type for reporting.
hoja_err_t ns_input_translate(uint8_t button_mode)
{
    const char* TAG = "ns_input_translate";

    if (button_mode != NS_BM_SHORT && button_mode != NS_BM_LONG) 
    {
        ESP_LOGE(TAG, "No button mode defined.");
        return HOJA_FAIL;
    }

    // We perform the translation based on the button mode passed as a parameter.
    switch(button_mode)
    {
        case NS_BM_SHORT:
            // Clear the data first
            memset(&ns_input_short, 0, sizeof(ns_input_short_s));

            // Short mode stick data set
            ns_input_short.l_stick[0] = hoja_analog_data.ls_x & 0xFF;
            ns_input_short.l_stick[1] = (hoja_analog_data.ls_x & 0xF00) >> 8;
            ns_input_short.l_stick[2] = hoja_analog_data.ls_y & 0xFF;
            ns_input_short.l_stick[3] = (hoja_analog_data.ls_y & 0xF00) >> 8;


            ns_input_short.b_right = hoja_button_data.button_right;
            ns_input_short.b_down = hoja_button_data.button_down;
            break;

        case NS_BM_LONG:

            // Long mode stick data set
            ns_input_long.l_stick[0] = hoja_analog_data.ls_x & 0xFF;
            ns_input_long.l_stick[1] = hoja_analog_data.ls_x >> 8;
            ns_input_long.l_stick[1] |= (hoja_analog_data.ls_y & 0xF) << 4;
            ns_input_long.l_stick[2] = hoja_analog_data.ls_y >> 4;

            ns_input_long.r_stick[0] = hoja_analog_data.rs_x & 0xFF;
            ns_input_long.r_stick[1] = hoja_analog_data.rs_x >> 8;
            ns_input_long.r_stick[1] |= (hoja_analog_data.rs_y & 0xF) << 4;
            ns_input_long.r_stick[2] = hoja_analog_data.rs_y >> 4;

            ns_input_long.b_plus = hoja_button_data.button_start;
            ns_input_long.b_minus = hoja_button_data.button_select;
            ns_input_long.b_home = hoja_button_data.button_home;
            ns_input_long.b_capture = hoja_button_data.button_capture;

            ns_input_long.d_down    = hoja_button_data.dpad_down;
            ns_input_long.d_up      = hoja_button_data.dpad_up;
            ns_input_long.d_left    = hoja_button_data.dpad_left;
            ns_input_long.d_right   = hoja_button_data.dpad_right;

            ns_input_long.b_a       = hoja_button_data.button_right;
            ns_input_long.b_b       = hoja_button_data.button_down;
            ns_input_long.b_x       = hoja_button_data.button_up;
            ns_input_long.b_y       = hoja_button_data.button_left;

            ns_input_long.t_zl      = hoja_button_data.trigger_zl;
            ns_input_long.t_zr      = hoja_button_data.trigger_zr;
            ns_input_long.t_l       = hoja_button_data.trigger_l;
            ns_input_long.t_r       = hoja_button_data.trigger_r;

            ns_input_long.sb_left   = hoja_button_data.side_button_left;
            ns_input_long.sb_right  = hoja_button_data.side_button_right;
            break;
        default:
            ESP_LOGE(TAG, "Input type invalid!");
            return HOJA_FAIL;
    }

    return HOJA_OK;
}

// Translate the global values for RBC stick calibration
// to values for Nintendo Switch
void ns_input_stickcalibration()
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
