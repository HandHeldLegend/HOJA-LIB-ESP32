#ifndef CORE_BT_DINPUT_H
#define CORE_BT_DINPUT_H

#include "hoja_includes.h"

#define DI_HID_LEN 9

#define DI_INPUT_REPORT_ID 0x01

// Input structure for DInput USB gamepad Data
typedef struct
{
    union
    {
        struct
        {
            uint8_t button_y    : 1;
            uint8_t button_b    : 1;
            uint8_t button_a    : 1;
            uint8_t button_x    : 1;
            uint8_t trigger_l   : 1;
            uint8_t trigger_r   : 1;
            uint8_t trigger_zl  : 1;
            uint8_t trigger_zr  : 1;
        };
        uint8_t buttons_1;
    };

    union
    {
        struct
        {
            uint8_t button_minus  : 1;
            uint8_t button_plus   : 1;
            uint8_t stick_left    : 1;
            uint8_t stick_right   : 1;
            uint8_t button_home   : 1;
            uint8_t button_capture: 1;
            uint8_t dummy_1       : 2;
        }; 
        uint8_t buttons_2;
    };

  uint8_t dpad_hat;
  uint8_t stick_left_x;
  uint8_t stick_left_y;
  uint8_t stick_right_x;
  uint8_t stick_right_y;
  uint8_t analog_trigger_l;
  uint8_t analog_trigger_r;
} __attribute__ ((packed)) di_input_s;

// Handler for vTask for report mode changes.
extern TaskHandle_t dinput_bt_task_handle;

void dinput_start_task(void);

void dinput_stop_task(void);

// Start the BT DInput controller core
hoja_err_t core_bt_dinput_start(void);

#endif