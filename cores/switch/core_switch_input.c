#include "core_switch_input.h"

ns_input_short_s ns_input_short = {};
ns_input_long_s ns_input_long = {};
ns_input_stickcaldata_s ns_input_stickcaldata = {};

// Translate the global input from RBC Core to the Switch
// native core input type for reporting.
void ns_input_translate_full(void)
{
    // Long mode stick data set
    ns_input_long.l_stick[0] = hoja_analog_data.ls_x & 0xFF;
    ns_input_long.l_stick[1] = hoja_analog_data.ls_x >> 8;
    ns_input_long.l_stick[1] |= (hoja_analog_data.ls_y & 0xF) << 4;
    ns_input_long.l_stick[2] = hoja_analog_data.ls_y >> 4;

    ns_input_long.r_stick[0] = hoja_analog_data.rs_x & 0xFF;
    ns_input_long.r_stick[1] = hoja_analog_data.rs_x >> 8;
    ns_input_long.r_stick[1] |= (hoja_analog_data.rs_y & 0xF) << 4;
    ns_input_long.r_stick[2] = hoja_analog_data.rs_y >> 4;

    ns_input_long.d_down    = hoja_button_data.dpad_down;
    ns_input_long.d_up      = hoja_button_data.dpad_up;
    ns_input_long.d_left    = hoja_button_data.dpad_left;
    ns_input_long.d_right   = hoja_button_data.dpad_right;

    ns_input_long.sb_left   = hoja_button_data.button_stick_left;
    ns_input_long.sb_right  = hoja_button_data.button_stick_right;

    if (_ns_subcore == NS_TYPE_N64)
    {
        ns_input_long.b_a       = hoja_button_data.button_down;
        ns_input_long.b_b       = hoja_button_data.button_left;

        ns_input_long.b_y      = hoja_button_data.button_up;
        ns_input_long.t_zr    = hoja_button_data.button_right;
        
        ns_input_long.b_x    = hoja_button_data.trigger_l;
        ns_input_long.b_minus   = hoja_button_data.trigger_r;

        ns_input_long.t_zl       = hoja_button_data.trigger_zl;
        ns_input_long.t_r      = hoja_button_data.trigger_zr;

        ns_input_long.t_l       = hoja_button_data.button_select;
    }
    else
    {
        ns_input_long.b_a       = hoja_button_data.button_right;
        ns_input_long.b_b       = hoja_button_data.button_down;
        ns_input_long.b_x       = hoja_button_data.button_up;
        ns_input_long.b_y       = hoja_button_data.button_left;

        ns_input_long.b_plus    = hoja_button_data.button_start;
        ns_input_long.b_minus   = hoja_button_data.button_select;

        ns_input_long.b_home    = hoja_button_data.button_home;
        ns_input_long.b_capture = hoja_button_data.button_capture;

        ns_input_long.t_l       = hoja_button_data.trigger_l;
        ns_input_long.t_zl      = hoja_button_data.trigger_zl;
        
        ns_input_long.t_r       = hoja_button_data.trigger_r;
        ns_input_long.t_zr      = hoja_button_data.trigger_zr;
    }
}

void ns_input_translate_short(void)
{
    // Clear the data first
    memset(&ns_input_short, 0, sizeof(ns_input_short_s));

    // TODO short mode implementation (maybe not needed?)

    // Short mode stick data set
    ns_input_short.l_stick[0] = hoja_analog_data.ls_x & 0xFF;
    ns_input_short.l_stick[1] = (hoja_analog_data.ls_x & 0xF00) >> 8;
    ns_input_short.l_stick[2] = hoja_analog_data.ls_y & 0xFF;
    ns_input_short.l_stick[3] = (hoja_analog_data.ls_y & 0xF00) >> 8;

    ns_input_short.b_right = hoja_button_data.button_right;
    ns_input_short.b_down = hoja_button_data.button_down;
}
