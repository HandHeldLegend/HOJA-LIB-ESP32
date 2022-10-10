#ifndef UTIL_BATTERY_H
#define UTIL_BATTERY_H

#include "hoja_includes.h"

#define UTIL_BATTERY_I2C_ADDRESS 0x6A

typedef enum
{
    BATFIELD_VGOOD = 0,
    BATFIELD_CHGSTAT = 1,
} util_battery_field_t;

typedef enum
{
    BATSTATUS_EMPTY       = 0,
    BATSTATUS_NOTCHARGING = 1,
    BATSTATUS_TRICKLEFAST = 2,
    BATSTATUS_CONSTANT    = 3,
    BATSTATUS_COMPLETED   = 4,
    BATSTATUS_VNOTGOOD    = 5,
    BATSTATUS_VGOOD       = 6,
} util_battery_status_t;

util_battery_status_t util_battery_getstatus(util_battery_field_t field);

#endif