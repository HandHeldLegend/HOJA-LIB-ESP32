#ifndef RBC_SWITCH_INPUT_H
#define RBC_SWITCH_INPUT_H

#include "rbc_switch_core.h"

/**
 * @brief A struct containing unions which are usful
 * for setting individual button bits, as well as
 * accessing full report bytes for later sending.
 */
typedef struct
{
    union
    {
        struct
        {
            uint8_t none    : 2;
            uint8_t sr      : 1;
            uint8_t sl      : 1;
            uint8_t up      : 1;
            uint8_t left    : 1;
            uint8_t right   : 1;
            uint8_t down    : 1;  
        };
        uint8_t buttons_first;
    };
    union
    {
        struct{
            uint8_t zl_zr   : 1;
            uint8_t l_r     : 1;
            uint8_t capture : 1;
            uint8_t home    : 1;
            uint8_t rs_btn : 1;
            uint8_t ls_btn : 1;
            uint8_t plus    : 1;
            uint8_t minus   : 1;
        };
        uint8_t buttons_second;
    };
    uint8_t stick_hat;
} ns_input_short_s;

typedef struct
{
    union
    {
        struct
        {
            uint8_t zr_btn      : 1;
            uint8_t r_btn       : 1;
            uint8_t sl_r_btn    : 1;
            uint8_t sr_r_btn    : 1;
            uint8_t a_btn       : 1;
            uint8_t b_btn       : 1;
            uint8_t x_btn       : 1;
            uint8_t y_btn       : 1;
        };
        uint8_t right_buttons;
    };
    union
    {
        struct
        {
            uint8_t charge_grip_active : 1;
            uint8_t none        : 1;
            uint8_t cap_btn     : 1;
            uint8_t home_btn    : 1;
            uint8_t ls_btn      : 1;
            uint8_t rs_btn      : 1;
            uint8_t plus_btn    : 1;
            uint8_t minus_btn   : 1;
        };
        uint8_t shared_buttons;
    };
    union
    {
        struct
        {
            uint8_t zl_btn      : 1;
            uint8_t l_btn       : 1;
            uint8_t sl_l_btn    : 1;
            uint8_t sr_l_btn    : 1;
            uint8_t left_btn    : 1;
            uint8_t right_btn   : 1;
            uint8_t up_btn      : 1;
            uint8_t down_btn    : 1;
        };
        uint8_t left_buttons;
    };
} ns_input_long_s;

ns_input_short_s ns_input_short;
ns_input_long_s ns_input_long;

#endif
