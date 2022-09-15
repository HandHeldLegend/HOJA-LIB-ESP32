#ifndef RETROBLUE_API_H
#define RETROBLUE_API_H

#include "retroblue_backend.h"
#include "retroblue_settings.h"
#include "rbc_switch_core.h"
#include "rbc_snes_core.h"
#include "rbc_gamecube_core.h"
#include "rbc_usb_core.h"

/* CORE TYPES */
#define CORE_NINTENDOSWITCH 0
#define CORE_SNES           1
#define CORE_GAMECUBE       2
#define CORE_N64            3
#define CORE_GENERICHID     4

// This last core type is for the wifi
// based configuration tool (WIP)
#define CORE_WIFISETUP      0xFF

// Pin needs to be defined to set the GPIO pin for SNES/NES function.
uint8_t RB_PIN_LATCH;
// Pin needs to be defined to set the GPIO pin for SNES/NES function.
uint8_t RB_PIN_CLOCK;
// Pin needs to be defined to set the GPIO pin for SNES/NES/GameCube function.
uint8_t RB_PIN_SERIAL;

uint8_t RB_PIN_I2C_SDA;

uint8_t RB_PIN_I2C_SCL;

SemaphoreHandle_t xSemaphore;

/* Functions relating to the API itself */

// Initialize the API.
// This loads any settings saved and initializes flash storage.
rb_err_t rb_api_init();

// Set the controller core
rb_err_t rb_api_setCore(uint8_t core_type);

// Starts the controller. You MUST set the core before running this.
rb_err_t rb_api_startController(void);

// Stops the controller. 
rb_err_t rb_api_stopController(void);

// Controller configuration options.
// Set the controller color
rb_err_t rb_api_setColor(uint8_t red, uint8_t green, uint8_t blue);

// Sets the analog stick values for calibration purposes
rb_err_t rb_api_setStickCalibration(uint16_t sx_min, uint16_t sx_center, uint16_t sx_max,
                                    uint16_t sy_min, uint16_t sy_center, uint16_t sy_max);

// Resets the current pairing for the current core
rb_err_t rb_api_resetPairing();

// Core-specific options.

// NINTENDO SWITCH
// ----------------
// Set the Nintendo Switch controller type
rb_err_t rb_api_setNSControllerType(uint8_t ns_controller_type);

// ----------------

// NES/SNES
// ----------------
// Enable analog stick as D-Pad input
rb_err_t rb_api_setSNESStickPad(uint8_t enable);

// ----------------

// Vars for callbacks
typedef void (*input_update_callback) (void);
input_update_callback rb_button_cb;
input_update_callback rb_stick_cb;

// Function to register callback function for setting buttons
// This is called by the core at an appropriate rate to help
// avoid weird timing issues.
rb_err_t rb_register_button_callback(input_update_callback func);
// Function to register callback function for setting sticks
rb_err_t rb_register_stick_callback(input_update_callback func);

#endif
