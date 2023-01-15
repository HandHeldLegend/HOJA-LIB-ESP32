#include "core_switch_input.h"

ns_input_stickcaldata_s ns_input_stickcaldata = {};

// Translate the global input from RBC Core to the Switch
// native core input type for reporting.
void ns_input_translate_full(ns_input_long_s *ns_input_long)
{
    ns_input_long->ls_x     = hoja_analog_data.ls_x;
    ns_input_long->ls_y     = hoja_analog_data.ls_y;
    ns_input_long->rs_x     = hoja_analog_data.rs_x;
    ns_input_long->rs_y     = hoja_analog_data.rs_y;

    ns_input_long->d_down    = hoja_button_data.dpad_down;
    ns_input_long->d_up      = hoja_button_data.dpad_up;
    ns_input_long->d_left    = hoja_button_data.dpad_left;
    ns_input_long->d_right   = hoja_button_data.dpad_right;

    ns_input_long->sb_left   = hoja_button_data.button_stick_left;
    ns_input_long->sb_right  = hoja_button_data.button_stick_right;

    if (_ns_subcore == NS_TYPE_N64)
    {
        ns_input_long->b_a       = hoja_button_data.button_down;
        ns_input_long->b_b       = hoja_button_data.button_left;
        ns_input_long->b_y      = hoja_button_data.button_up;
        ns_input_long->t_zr    = hoja_button_data.button_right;
        ns_input_long->b_x    = hoja_button_data.trigger_l;
        ns_input_long->b_minus   = hoja_button_data.trigger_r;
        ns_input_long->t_zl       = hoja_button_data.trigger_zl;
        ns_input_long->t_r      = hoja_button_data.trigger_zr;
        ns_input_long->t_l       = hoja_button_data.button_select;
    }
    else
    {
        ns_input_long->b_a       = hoja_button_data.button_right;
        ns_input_long->b_b       = hoja_button_data.button_down;
        ns_input_long->b_x       = hoja_button_data.button_up;
        ns_input_long->b_y       = hoja_button_data.button_left;
        ns_input_long->b_plus    = hoja_button_data.button_start;
        ns_input_long->b_minus   = hoja_button_data.button_select;
        ns_input_long->b_home    = hoja_button_data.button_home;
        ns_input_long->b_capture = hoja_button_data.button_capture;
        ns_input_long->t_l       = hoja_button_data.trigger_l;
        ns_input_long->t_zl      = hoja_button_data.trigger_zl;
        ns_input_long->t_r       = hoja_button_data.trigger_r;
        ns_input_long->t_zr      = hoja_button_data.trigger_zr;
    }
}

void ns_input_translate_short(ns_input_short_s *ns_input_short)
{
    // TODO short mode implementation (maybe not needed?)
    

    // Short mode stick data set
    ns_input_short->l_stick[0] = hoja_analog_data.ls_x & 0xFF;
    ns_input_short->l_stick[1] = (hoja_analog_data.ls_x & 0xF00) >> 8;
    ns_input_short->l_stick[2] = hoja_analog_data.ls_y & 0xFF;
    ns_input_short->l_stick[3] = (hoja_analog_data.ls_y & 0xF00) >> 8;
    ns_input_short->b_right = hoja_button_data.button_right;
    ns_input_short->b_down = hoja_button_data.button_down;
}

// Compare two input reports of short type. Return true if there's a difference
bool ns_input_compare_short(ns_input_short_s *one, ns_input_short_s *two)
{
    // TODO implement comparison for short mode.
    return true;
}

// Compare two input reports of full type. Return true if there's a difference
bool ns_input_compare_full(ns_input_long_s *one, ns_input_long_s *two)
{
    bool ret = false;
    ret |= one->right_buttons   != two->right_buttons;
    ret |= one->shared_buttons  != two->shared_buttons;
    ret |= one->left_buttons    != two->left_buttons;
    ret |= one->ls_x            != two->ls_x;
    ret |= one->ls_y            != two->ls_y;
    ret |= one->rs_x            != two->rs_x;
    ret |= one->rs_y            != two->rs_y;

    return ret;
}
