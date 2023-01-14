#ifndef CORE_SWITCH_INPUT_H
#define CORE_SWITCH_INPUT_H

#include "core_switch_backend.h"

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
            uint8_t b_down      : 1;
            uint8_t b_right     : 1;
            uint8_t b_left      : 1;
            uint8_t b_up        : 1;
            uint8_t t_sl        : 1;
            uint8_t t_sr        : 1;
            uint8_t none        : 2;   
        };
        uint8_t buttons_first;
    };
    union
    {
        struct
        {
            uint8_t b_minus     : 1;
            uint8_t b_plus      : 1;
            uint8_t sb_left     : 1;
            uint8_t sb_right    : 1;
            uint8_t b_home      : 1;
            uint8_t b_capture   : 1;
            uint8_t t_lr        : 1;
            uint8_t t_zlzr      : 1;
        };
        uint8_t buttons_second;
    };
    uint8_t stick_hat;

    // Stick analog values
    uint8_t l_stick[4];
    uint8_t r_stick[4];

} __attribute__ ((packed)) ns_input_short_s;

typedef struct
{
    union
    {
        struct
        {
            // Y and C-Up (N64)
            uint8_t b_y       : 1;
            
            
            // X and C-Left (N64)
            uint8_t b_x       : 1;
            
            uint8_t b_b       : 1;
            uint8_t b_a       : 1;
            uint8_t t_r_sr    : 1;
            uint8_t t_r_sl    : 1;
            uint8_t t_r       : 1;

            // ZR and C-Down (N64)
            uint8_t t_zr      : 1;
        };
        uint8_t right_buttons;
    };
    union
    {
        struct
        {
            // Minus and C-Right (N64)
            uint8_t b_minus     : 1; 
            
            // Plus and Start
            uint8_t b_plus      : 1;
            
            uint8_t sb_right    : 1;
            uint8_t sb_left     : 1;
            uint8_t b_home      : 1;
            uint8_t b_capture   : 1;
            uint8_t none        : 1;
            uint8_t charge_grip_active : 1;
        };
        uint8_t shared_buttons;
    };
    union
    {
        struct
        {
            uint8_t d_down    : 1;
            uint8_t d_up      : 1;
            uint8_t d_right   : 1;
            uint8_t d_left    : 1;
            uint8_t t_l_sr    : 1;
            uint8_t t_l_sl    : 1;
            uint8_t t_l       : 1;

            // ZL and Z (N64)
            uint8_t t_zl      : 1;
            
        };
        uint8_t left_buttons;
    };

    uint8_t l_stick[3];
    uint8_t r_stick[3];

} __attribute__ ((packed)) ns_input_long_s;

typedef struct
{
    uint8_t l_stick_cal[11];
    uint8_t r_stick_cal[11];
} __attribute__ ((packed)) ns_input_stickcaldata_s;

extern ns_input_short_s ns_input_short;
extern ns_input_long_s ns_input_long;
extern ns_input_stickcaldata_s ns_input_stickcaldata;

void ns_input_translate_full(void);

void ns_input_translate_short(void);

#endif
