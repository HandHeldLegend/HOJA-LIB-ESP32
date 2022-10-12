#ifndef UTIL_BATTERY_H
#define UTIL_BATTERY_H

#include "hoja_includes.h"

#define UTIL_BATTERY_I2C_ADDRESS 0x6A

typedef enum
{
    BATSTATUS_NOTCHARGING = 0x00,
    BATSTATUS_TRICKLEFAST = 0x01,
    BATSTATUS_CONSTANT    = 0x02,
    BATSTATUS_COMPLETED   = 0x03,
} util_battery_status_t;

typedef enum
{
    BATCABLE_UNPLUGGED    = 0,
    BATCABLE_PLUGGED      = 1,
} util_battery_plugged_t;

typedef struct
{
    union
    {
        struct 
        { 
            uint8_t plug_status : 1;
            uint8_t dummy1 : 4;
            uint8_t charge_status : 2;
            uint8_t dummy2 : 1;
        };
        uint8_t status;
    };
} util_battery_status_s;

uint8_t util_battery_getstatus();

void util_battery_write(uint8_t offset, uint8_t byte);

#endif