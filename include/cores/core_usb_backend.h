#ifndef CORE_USB_BACKEND_H
#define CORE_USB_BACKEND_H

#include "hoja_includes.h"

typedef struct
{
  union
  {
    struct
    {
      uint8_t rumble          : 1; // Rumble is enabled or disabled
      uint8_t gamepad_status  : 1; // Gamepad is initialized or not
      uint8_t usb_status      : 1; // USB is attached or not
      uint8_t config_request  : 1; // Configuration request submitted
      uint8_t config_write    : 1; // Configuration write submitted
      uint8_t padding         : 3; // Padding (unused)
    };
    uint8_t val;
  };
} __attribute__ ((packed)) ui2c_return_msg_s;

typedef enum
{
    USB_STATUS_IDLE,
    USB_STATUS_INITIALIZED,
} usb_status_t;

typedef enum
{
  I2C_SYSCMD_IDLE = 0,  // No command
  I2C_SYSCMD_SETUSB,    // Set the current USB input mode
  I2C_SYSCMD_SETMODE,   // Start, stop, or reset USB function
  I2C_SYSCMD_INPUT,     // Standard input
  I2C_SYSCMD_CONFIG,    // Sends a configuration report over USB

} ui2c_syscmd_t;

typedef enum
{
  USB_SUBCORE_IDLE    = 0,
  USB_SUBCORE_DINPUT  = 1,
  USB_SUBCORE_NS      = 2,
  USB_SUBCORE_XINPUT  = 3,
  USB_SUBCORE_GCINPUT = 4,
  USB_SUBCORE_MAX,
} usb_subcore_t;

typedef enum
{
  I2C_CONFIGCMD_SAVERESET,
  I2C_CONFIGCMD_GETALL,
  I2C_CONFIGCMD_ANALOG_CALIBRATION,
  I2C_CONFIGCMD_TRIGGER_SENSITIVITY,
  I2C_CONFIGCMD_TRIGGER_MODE,
  I2C_CONFIGCMD_CALIBRATE_SET,
} ui2c_configcmd_t;

// System set commands
typedef enum
{
  I2C_SETMODE_STOP,
  I2C_SETMODE_START,
  I2C_SETMODE_RESET,
} ui2c_setmode_t;

typedef enum
{
  GAMEPAD_STATUS_IDLE = 0,
  GAMEPAD_STATUS_INITIALIZED = 1,
} ui2c_gamepad_status_t;

typedef enum
{
  USB_STATUS_NOBUS = 0,
  USB_STATUS_BUSOK = 1,
} ui2c_usb_status_t;

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