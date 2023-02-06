#ifndef CORE_USB_BACKEND_H
#define CORE_USB_BACKEND_H

#include "hoja_includes.h"

typedef enum
{
    USB_STATUS_IDLE,
    USB_STATUS_INITIALIZED,
} usb_status_t;

typedef enum
{
  I2C_CMD_IDLE = 0,    // No command
  I2C_CMD_STATUS = 1,  // Returns the current USB status and gamepadStatus
  I2C_CMD_SETMODE, // Start, stop, or reset USB function
  I2C_CMD_SETUSB,  // Set the current USB input mode
  I2C_CMD_INPUT,   // Sends an Input report over USB
  I2C_CMD_CONFIG,  // Sends a configuration report over USB

} ui2c_cmd_t;

typedef enum
{
  I2C_RETURN_CMD_IDLE,  // No return data
  I2C_RETURN_CMD_STATUS,// Return status data of USB and Gamepad
  I2C_RETURN_CMD_RUMBLE,// Return any rumble status if it's changed
  I2C_RETURN_CMD_CONFIG,// Return any configuration request
} ui2c_return_cmd_t;

typedef enum
{
  I2C_CONFIG_SAVERESET,
  I2C_CONFIG_GETALL,
  I2C_CONFIG_ANALOG_CALIBRATION,
  I2C_CONFIG_TRIGGER_SENSITIVITY,
  I2C_CONFIG_TRIGGER_MODE,
  I2C_CONFIG_CALIBRATE_SET,
} ui2c_config_t;

// System set commands
typedef enum
{
  I2C_SETMODE_START = 0,
  I2C_SETMODE_STOP  = 1,
  I2C_SETMODE_RESET = 2
} ui2c_setmode_t;

// I2C Response Status Messages
typedef enum
{
  I2C_OK       = 0,  // Message and command executed OK
  I2C_FAIL     = 1,  // Message and command did not execute OK
} ui2c_response_t;

typedef enum
{
  GAMEPAD_STATUS_IDLE,
  GAMEPAD_STATUS_INITIALIZED,
} ui2c_gamepad_status_t;

typedef enum
{
  USB_STATUS_NOBUS,
  USB_STATUS_BUSOK,
} ui2c_usb_status_t;

typedef enum
{
  USB_SUBCORE_IDLE    = 0,
  USB_SUBCORE_DINPUT  = 1,
  USB_SUBCORE_NS      = 2,
  USB_SUBCORE_XINPUT  = 3,
  USB_SUBCORE_GCINPUT = 4,
  USB_SUBCORE_MAX,
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

// Exposed functions

hoja_err_t core_usb_set_subcore(usb_subcore_t subcore);

void core_usb_stop(void);

hoja_err_t core_usb_start(void);

// Private functions
void usb_sendinput_task(void * parameters);

#endif