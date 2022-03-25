#ifndef RBC_SWITCH_CONTROLLER_H
#define RBC_SWITCH_CONTROLLER_H

#include "rbc_switch_core.h"


/* Define controller types supported */
#define CONTROLLER_TYPE_JOYCON_L    0x01
#define CONTROLLER_TYPE_JOYCON_R    0x02
#define CONTROLLER_TYPE_PROCON      0x03
#define CONTROLLER_TYPE_NESCLASSIC  0x04
#define CONTROLLER_TYPE_SNESCLASSIC 0x05
#define CONTROLLER_TYPE_GENESIS     0x06
#define CONTROLLER_TYPE_N64CLASSIC  0x07

/**
 * @brief A struct containing all of the controller information 'loaded'.
 * This acts as a one-stop area to set different controller settings that
 * get used by the rest of this core.
 */
typedef struct
{
    // Firmware version
    union
    {
        struct
        {
            // Primary Firmware version --> XX.YY
            uint8_t fw_primary : 8;
            // Secondary Firmware version XX.YY <--
            uint8_t fw_secondary : 8;
        };
        // Full 16 bit firmware version combined. Made for easy setting :)
        uint16_t fw_full;
    };

    // Controller Bluetooth Mac Address
    uint8_t client_mac_address[6];

    // Paired device Bluetooth Mac Address
    uint8_t host_mac_address[6];

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

    /**
     * @brief Controller type
     * @param 0x01 Joy-Con (L)
     * @param 0x02 Joy-Con (R)
     * @param 0x03 Pro Controller
     */
    uint8_t controller_type; //default CONTROLLER_TYPE_PROCON

    /**
     * @brief If set to false, no color has been loaded
     * and the color will be set to default colors.
     */
    uint8_t color_set; //default false;

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

    struct 
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    } body_color;
    struct 
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    } buttons_color;
    struct 
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    } l_grip_color;
    struct
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    } r_grip_color;

} ns_controller_data_s;

ns_controller_data_s ns_controller_data;

void ns_controller_setinputreportmode(uint8_t report_mode);

void ns_controller_setshipmode(uint8_t ship_mode);

#endif
