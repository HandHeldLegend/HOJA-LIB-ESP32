#ifndef RETROBLUE_API_H
#define RETROBLUE_API_H

#include "retroblue_backend.h"
#include "retroblue_settings.h"
#include "rbc_switch_core.h"
#include "rbc_snes_core.h"

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

SemaphoreHandle_t xSemaphore;

/* Functions relating to the API itself */

// Initialize the API.
// This loads any settings saved and initializes flash storage.
rb_err_t rb_api_init();

// Set the controller core
rb_err_t rb_api_setCore(uint8_t core_type);

// Starts the controller. You MUST set the core before running this.
rb_err_t rb_api_startController(void);

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
