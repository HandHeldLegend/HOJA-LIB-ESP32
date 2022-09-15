#ifndef RBC_USB_CORE_H
#define RBC_USB_CORE_H

#include "driver/i2c.h"
#include "retroblue_backend.h"
#include "retroblue_api.h"

typedef enum
{
    USB_CMD_CHECKREADY = 0,
    USB_CMD_SYSTEMSET = 1,
    USB_CMD_INPUT = 2,
    USB_CMD_USBCHANGE = 3,
} USB_Core_Commands_t;

typedef enum
{
    USB_SYSTEM_START = 0,
    USB_SYSTEM_STOP = 1,
    USB_SYSTEM_RESET = 2,
} USB_System_Commands_t;

typedef enum
{
    USB_MSG_OK = 0,
    USB_MSG_FAIL = 1,
    USB_MSG_NOSTART = 2,
    USB_MSG_NOSTOP = 3,
} USB_Core_Response_Msg_t;

// I2C gamepad struct format
typedef struct
{
    union
    {
        struct
        {
            uint8_t dpad_right : 1;
            uint8_t dpad_down : 1;
            uint8_t dpad_left : 1;
            uint8_t dpad_up : 1;
            uint8_t button_y : 1;
            uint8_t button_x : 1;
            uint8_t button_b : 1;
            uint8_t button_a : 1;
        };
        uint8_t buttons_1;
    };

    union
    {
        struct
        {
            uint8_t button_minus : 1;
            uint8_t button_plus : 1;
            uint8_t button_capture : 1;
            uint8_t button_home : 1;
            uint8_t trigger_zr : 1;
            uint8_t trigger_zl : 1;
            uint8_t trigger_r : 1;
            uint8_t trigger_l : 1;
        };
        uint8_t buttons_2;
    };

    union
    {
        struct
        {
            uint8_t dummy_1 : 6;
            uint8_t stick_right : 1;
            uint8_t stick_left : 1;
        };
        uint8_t buttons_3;
    };

    uint8_t stick_left_x;
    uint8_t stick_left_y;
    uint8_t stick_right_x;
    uint8_t stick_right_y;
} i2c_input_s;

void rbc_core_usb_sendinput_task(void * parameters);

rb_err_t rbc_core_usb_start(void);

void rbc_core_usb_stop(void);

void rbc_core_usb_init(void);

#endif