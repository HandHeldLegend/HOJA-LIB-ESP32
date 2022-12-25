#ifndef CORE_BT_XINPUT_H
#define CORE_BT_XINPUT_H

#include "hoja_includes.h"

#define XI_HID_LEN 16

/**
 * Xbox One BT Format
 * 
 * Byte 0 - 3 : Stick Left
 * Byte 4 - 7 : Stick right
 * Byte 8 - 9   : Left Trigger
 * Byte 10 - 11 : Right Trigger
 * 
 * Byte 11 :
 * 0 : 
 * 1 : 
 * 2 : 
 * 3 : 
 * 4 : 
 * 5 : 
 * 6 : 
 * 7 : 
 * 
 * Byte 12 : DPAD VALS
 * up   : 0x1
 * down : 0x5
 * left : 0x7
 * right: 0x3
 * 
 * down-left    : 0x6
 * down-right   : 0x4
 * up-right     : 0x2
 * up-left      : 0x8
 * 
 * center   : 0x00
 * 
 * Byte 13 :
 * 0 : A 
 * 1 : B
 * 2 : 
 * 3 : X
 * 4 : X
 * 5 : 
 * 6 : L bumper
 * 7 : R bumper
 * 
 * Byte 14 :
 * 0 : 
 * 1 : 
 * 2 : Back
 * 3 : Pause
 * 4 : Guide
 * 5 : L Stick button
 * 6 : R Stick button
 * 7 : 
 * 
 * Byte 15 :
 * 0 : 
 * 1 : 
 * 2 : 
 * 3 : 
 * 4 : 
 * 5 : 
 * 6 : 
 * 7 : 
 * 
*/

typedef struct
{
    uint16_t stick_left_x   : 16;
    uint16_t stick_left_y   : 16;
    uint16_t stick_right_x  : 16;
    uint16_t stick_right_y  : 16;

    uint16_t analog_trigger_l : 16;
    uint16_t analog_trigger_r : 16;

    uint8_t dpad_hat        : 4;
    uint8_t dpad_padding    : 4;

    union
    {
        struct
        {
            uint8_t button_a    : 1;    
            uint8_t button_b    : 1;    
            uint8_t padding_1   : 1;
            uint8_t button_x    : 1;
            uint8_t button_y    : 1;
            uint8_t padding_2   : 1;
            uint8_t bumper_l    : 1;
            uint8_t bumper_r    : 1;
        } __attribute__ ((packed)); 
        uint8_t buttons_1 : 8;
    };

    union
    {
        struct
        {
            uint8_t padding_3       : 2;
            uint8_t button_back     : 1; 
            uint8_t button_menu     : 1; 
            uint8_t button_guide    : 1; 
            uint8_t button_stick_l  : 1; 
            uint8_t button_stick_r      : 1; 
            uint8_t padding_4           : 1;  
        } __attribute__ ((packed));
        uint8_t buttons_2 : 8;
    };

    uint8_t buttons_blank;

} __attribute__ ((packed)) xi_input_s;

// Handler for vTask for report mode changes.
extern TaskHandle_t bt_xinput_task_handle;

void xinput_start_task(void);

void xinput_stop_task(void);

// Start the XInput controller core
hoja_err_t core_bt_xinput_start(void);

#endif