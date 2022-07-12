#include "rbc_switch_input.h"

ns_input_short_s ns_input_short = {};
ns_input_long_s ns_input_long = {};
ns_input_stickcaldata_s ns_input_stickcaldata = {};

// Translate the global input from RBC Core to the Switch
// native core input type for reporting.
rb_err_t ns_input_translate(uint8_t button_mode)
{
    const char* TAG = "ns_input_translate";

    if (button_mode != NS_BM_SHORT && button_mode != NS_BM_LONG) 
    {
        ESP_LOGE(TAG, "No button mode defined.");
        return RB_FAIL;
    }

    // We perform the translation based on the button mode passed as a parameter.
    switch(button_mode)
    {
        case NS_BM_SHORT:
            // Clear the data first
            memset(&ns_input_short, 0, sizeof(ns_input_short_s));

            // Short mode stick data set
            ns_input_short.l_stick[0] = g_stick_data.lsx & 0xFF;
            ns_input_short.l_stick[1] = (g_stick_data.lsx & 0xF00) >> 8;
            ns_input_short.l_stick[2] = g_stick_data.lsy & 0xFF;
            ns_input_short.l_stick[3] = (g_stick_data.lsy & 0xF00) >> 8;


            ns_input_short.b_right = g_button_data.b_right;
            ns_input_short.b_down = g_button_data.b_down;
            break;

        case NS_BM_LONG:
            //memset(&ns_input_long, 0, sizeof(ns_input_long_s));

            // Long mode stick data set
            ns_input_long.l_stick[0] = g_stick_data.lsx & 0xFF;
            ns_input_long.l_stick[1] = g_stick_data.lsx >> 8;
            ns_input_long.l_stick[1] |= (g_stick_data.lsy & 0xF) << 4;
            ns_input_long.l_stick[2] = g_stick_data.lsy >> 4;

            ns_input_long.r_stick[0] = g_stick_data.rsx & 0xFF;
            ns_input_long.r_stick[1] = g_stick_data.rsx >> 8;
            ns_input_long.r_stick[1] |= (g_stick_data.rsy & 0xF) << 4;
            ns_input_long.r_stick[2] = g_stick_data.rsy >> 4;

            ns_input_long.b_plus = g_button_data.b_start;
            ns_input_long.b_minus = g_button_data.b_select;
            ns_input_long.b_home = g_button_data.b_home;
            ns_input_long.b_capture = g_button_data.b_capture;

            ns_input_long.d_down = g_button_data.d_down;
            ns_input_long.d_up = g_button_data.d_up;
            ns_input_long.d_left = g_button_data.d_left;
            ns_input_long.d_right = g_button_data.d_right;

            ns_input_long.b_a = g_button_data.b_right;
            ns_input_long.b_b = g_button_data.b_down;
            ns_input_long.b_x = g_button_data.b_up;
            ns_input_long.b_y = g_button_data.b_left;

            ns_input_long.t_zl = g_button_data.t_zl;
            ns_input_long.t_zr = g_button_data.t_zr;
            ns_input_long.t_l = g_button_data.t_l;
            ns_input_long.t_r = g_button_data.t_r;    
            break;
        default:
            ESP_LOGE(TAG, "Input type invalid!");
            return RB_FAIL;
    }

    return RB_OK;
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
