#ifndef CORE_USB_BACKEND_H
#define CORE_USB_BACKEND_H

#include "hoja_includes.h"

typedef enum
{
    USB_STATUS_IDLE,
    USB_STATUS_STARTED,
    USB_STATUS_CONNECTED,
} usb_status_t;

typedef enum
{
    USB_CMD_CHECKREADY  = 0,
    USB_CMD_SYSTEMSET   = 1,
    USB_CMD_INPUT       = 2,
    USB_CMD_USBCHANGE   = 3,
} usb_core_commands_t;

typedef enum
{
    USB_SYSTEM_START    = 0,
    USB_SYSTEM_STOP     = 1,
    USB_SYSTEM_RESET    = 2,
} usb_system_commands_t;

typedef enum
{
    USB_MSG_OK      = 0,
    USB_MSG_FAIL    = 1,
    USB_MSG_NOSTART = 2,
    USB_MSG_NOSTOP  = 3,
} usb_core_respons_msg_t;

typedef enum
{
    USB_SUBCORE_DINPUT  = 0,
    USB_SUBCORE_NS      = 1,
    USB_SUBCORE_XINPUT  = 2,
    USB_SUBCORE_GCINPUT = 3,
    USB_SUBCORE_MAX
} usb_subcore_t;

// USB gamepad struct format
typedef struct
{
    // Copy all hoja_button_data to here.
    uint16_t    buttons_all;
    uint8_t     buttons_system;
    uint8_t     ls_x;
    uint8_t     ls_y;
    uint8_t     rs_x;
    uint8_t     rs_y;
    uint8_t     lt_a;
    uint8_t     rt_a;
} __attribute__ ((packed)) usb_input_s;

hoja_err_t core_usb_set_subcore(usb_subcore_t subcore);

hoja_err_t core_usb_stop(void);

// Exposed functions
hoja_err_t core_usb_start(void);

// Private functions
void usb_sendinput_task(void * parameters);

void usb_init(void);

#endif