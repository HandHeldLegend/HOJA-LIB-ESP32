#include "rbc_switch_input.h"

ns_input_short_s ns_input_short = {};
ns_input_long_s ns_input_long = {};
ns_input_stickcaldata_s ns_input_stickcaldata = {};

// Translate the global input from RBC Core to the Switch
// native core input type for reporting.
void ns_input_grabinput(uint8_t button_mode)
{
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


            ns_input_short.right = g_button_data.button_right;
            ns_input_short.down = g_button_data.button_down;
            break;

        case NS_BM_LONG:
            memset(&ns_input_long, 0, sizeof(ns_input_long_s));

            // Long mode stick data set
            ns_input_long.l_stick[0] = g_stick_data.lsx & 0xFF;
            ns_input_long.l_stick[1] = g_stick_data.lsx >> 8;
            ns_input_long.l_stick[1] |= (g_stick_data.lsy & 0xF) << 4;
            ns_input_long.l_stick[2] = g_stick_data.lsy >> 4;

            ns_input_long.a_btn = g_button_data.button_right;
            ns_input_long.b_btn = g_button_data.button_down;

            break;
    }
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

    ESP_LOGI(TAG, "L Stick Settings: \nMin:%d\nCenter%d:\nMax:%d", loaded_settings.sx_min, loaded_settings.sx_center, loaded_settings.sx_max);

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
}
