#ifndef CORE_SWITCH_CONTROLLER_H
#define CORE_SWITCH_CONTROLLER_H

#include "cores/core_switch_backend.h"
#include "switch/core_switch_types.h"

/**
 * @brief A struct containing all of the controller information 'loaded'.
 * This acts as a one-stop area to set different controller settings that
 * get used by the rest of this core.
 */
typedef struct
{
    union 
    {
        struct
        {
            uint8_t is_charging : 1;
            uint8_t battery_level_sub : 7;
        };
        // 0-8 indicates batt level. LSB = charging is enabled
        uint8_t battery_level_full; // default 0x4
    };
    
    // 0x0 is Pro con default
    uint8_t connection_info; // default 0x0

    uint8_t controller_type_primary;    // Primary byte for controller type
    uint8_t controller_type_secondary;  // Secondary byte for controller type

} ns_controller_data_s;

// Virtual controller memory.
extern ns_controller_data_s ns_controller_data;

void ns_controller_setup_memory(void);

void ns_controller_applycalibration(void);

void ns_controller_setinputreportmode(uint8_t report_mode);

void ns_controller_setshipmode(uint8_t ship_mode);

void ns_controller_sleep_handle(ns_power_handle_t power_type);

void ns_controller_input_task_set(ns_report_mode_t report_mode_type);

#endif
