#ifndef UTIL_BATTERY_H
#define UTIL_BATTERY_H

#include "hoja_includes.h"

#define UTIL_BATTERY_I2C_ADDRESS 0x6A

// Each type corresponds to the battery I2C address
typedef enum
{
    BATTYPE_UNDEFINED = 0x00,
    BATTYPE_BQ25180 = 0x6A,
} util_battery_type_t;

typedef enum
{
    BATSTATUS_UNDEFINED   = 0xFF,
    BATSTATUS_NOTCHARGING = 0x00,
    BATSTATUS_TRICKLEFAST = 0x01,
    BATSTATUS_CONSTANT    = 0x02,
    BATSTATUS_COMPLETED   = 0x03,
} util_battery_status_t;

typedef enum
{
    BATCABLE_UNDEFINED    = 0xF,
    BATCABLE_UNPLUGGED    = 0x0,
    BATCABLE_PLUGGED      = 0x1,
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
} __attribute__ ((packed)) util_battery_status_s;

// PUBLIC FUNCTIONS
// -----------------
bool util_battery_external_power(void);

util_battery_status_t util_get_battery_charging_status(void);

hoja_err_t util_battery_set_type(util_battery_type_t type);

hoja_err_t util_battery_boot_status(void);

hoja_err_t util_battery_start_monitor(void);

void util_battery_stop_monitor(void);

uint8_t util_battery_is_charging(void);

bool util_battery_is_connected(void);

hoja_err_t util_battery_set_charge_rate(uint16_t rate_ma);

void util_battery_enable_ship_mode(void);
// -----------------
// -----------------

#endif