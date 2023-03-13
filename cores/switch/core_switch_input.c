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

    ns_input_long->d_down    = hoja_processed_buttons.dpad_down;
    ns_input_long->d_up      = hoja_processed_buttons.dpad_up;
    ns_input_long->d_left    = hoja_processed_buttons.dpad_left;
    ns_input_long->d_right   = hoja_processed_buttons.dpad_right;

    ns_input_long->sb_left   = hoja_processed_buttons.button_stick_left;
    ns_input_long->sb_right  = hoja_processed_buttons.button_stick_right;

    if (_ns_subcore == NS_TYPE_N64)
    {
        ns_input_long->b_a       = hoja_processed_buttons.button_down;
        ns_input_long->b_b       = hoja_processed_buttons.button_left;
        ns_input_long->b_y      = hoja_processed_buttons.button_up;
        ns_input_long->t_zr    = hoja_processed_buttons.button_right;
        ns_input_long->b_x    = hoja_processed_buttons.trigger_l;
        ns_input_long->b_minus   = hoja_processed_buttons.trigger_r;
        ns_input_long->t_zl       = hoja_processed_buttons.trigger_zl;
        ns_input_long->t_r      = hoja_processed_buttons.trigger_zr;
        ns_input_long->t_l       = hoja_processed_buttons.button_select;
    }
    else
    {
        ns_input_long->b_a       = hoja_processed_buttons.button_right;
        ns_input_long->b_b       = hoja_processed_buttons.button_down;
        ns_input_long->b_x       = hoja_processed_buttons.button_up;
        ns_input_long->b_y       = hoja_processed_buttons.button_left;
        ns_input_long->b_minus   = hoja_processed_buttons.button_select;
        ns_input_long->t_l       = hoja_processed_buttons.trigger_l;
        ns_input_long->t_zl      = hoja_processed_buttons.trigger_zl;
        ns_input_long->t_r       = hoja_processed_buttons.trigger_r;
        ns_input_long->t_zr      = hoja_processed_buttons.trigger_zr;
    }
    
    ns_input_long->b_plus    = hoja_processed_buttons.button_start;
    ns_input_long->b_home    = hoja_processed_buttons.button_home;
    ns_input_long->b_capture = hoja_processed_buttons.button_capture;
}

void ns_input_translate_short(ns_input_short_s *ns_input_short)
{
    // TODO short mode implementation (maybe not needed?)
    

    // Short mode stick data set
    ns_input_short->l_stick[0] = hoja_analog_data.ls_x & 0xFF;
    ns_input_short->l_stick[1] = (hoja_analog_data.ls_x & 0xF00) >> 8;
    ns_input_short->l_stick[2] = hoja_analog_data.ls_y & 0xFF;
    ns_input_short->l_stick[3] = (hoja_analog_data.ls_y & 0xF00) >> 8;
    ns_input_short->b_right     = hoja_processed_buttons.button_right;
    ns_input_short->b_down      = hoja_processed_buttons.button_down;
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
