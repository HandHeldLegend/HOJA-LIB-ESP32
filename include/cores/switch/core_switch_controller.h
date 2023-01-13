#ifndef CORE_SWITCH_CONTROLLER_H
#define CORE_SWITCH_CONTROLLER_H

#include "cores/core_switch_backend.h"

/**
 * @brief A struct containing all of the controller information 'loaded'.
 * This acts as a one-stop area to set different controller settings that
 * get used by the rest of this core.
 */
typedef struct
{
    uint8_t player_number;

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

    

    uint8_t sticks_calibrated; //default false;

    /**
     * @brief Input Report Mode
     * @param 0x00 Active polling for NFC/IR camera data.
     * @param 0x01 Same as 0x00.
     * @param 0x02 Same as 0x00 for NFC/IR modes.
     * @param 0x03 Same as 0x00 for IR modes.
     * @param 0x23 MCU updata state report mode.
     * @param 0x30 Standard full report mode. 60hz or 120hz if Pro Controller
     * @param 0x31 NFC/IR data push mode. Pushes at 60hz.
     * @param 0x3F Simple HID mode. Pushes 'short' button reports with each button press/release.
     * @param 0xFF Blank report mode. Use this when you want the console to respond :)
     */
    uint8_t input_report_mode; //default 0x3F

} ns_controller_data_s;

extern ns_controller_data_s ns_controller_data;

void ns_controller_setup_memory(void);

void ns_controller_setinputreportmode(uint8_t report_mode);

void ns_controller_setshipmode(uint8_t ship_mode);

#endif
