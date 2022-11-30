#ifndef UTIL_GAMEPAD_H
#define UTIL_GAMEPAD_H

#include "hoja_includes.h"

typedef enum
{
    ANALOG_SCALER_GC    = 0,        // Scales values between 28 and 228 for real GameCube range
    ANALOG_SCALER_8BIT  = 1,        // Scales values from 12 bit values to 8 bit
    ANALOG_SCALER_N64   = 2,        // Scales values for N64 range (not yet implemented)
} util_analog_scaler_t;

typedef enum
{
    SOCD_NATURAL            = 0,    // Natural
    SOCD_NEUTRAL            = 1,    // Neutral
    SOCD_ABSPRIORITY        = 2,    // Absolute Priority 
    SOCD_LASTPRIORITY       = 3,    // Last Input Priority
    SOCD_LASTINPRIORITY     = 4,    // Last Input Controlled Priority
    SOCD_FIRSTINPRIORITY    = 5,    // First Input Priority
    SOCD_FIRSTINCONTROLLED  = 6     // First Input Controlled Priority
} util_socd_resolutions_t;

// PUBLIC FUNCTIONS
// -----------------

// Snapback functions are placeholder for now. Not yet implemented as of 11/29/2022
void util_snapback_set_strength(hoja_analog_axis_t axis, uint8_t strength);

void util_snapback_add_value(hoja_analog_axis_t axis, uint16_t value);

uint16_t util_snapback_get_value(hoja_analog_axis_t axis);

// Stick scaling is placeholder for now. " "
uint16_t util_analog_get_scaled(uint16_t value, util_analog_scaler_t scaler_type);

// SOCD functionality is placeholder for now. Not yet implemented as of 11/29/2022
void util_socd_set_type(util_socd_resolutions_t type);

void util_socd_apply(uint16_t *hoja_buttons_all);

// -----------------
// -----------------

#endif