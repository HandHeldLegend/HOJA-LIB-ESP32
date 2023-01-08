#ifndef HOJA_TYPES_H
#define HOJA_TYPES_H

/**
 * Error handling
 */
typedef enum
{
    HOJA_OK = 0,
    HOJA_FAIL,
    HOJA_USB_NODET,         // USB Core not detected on attempt.
    HOJA_I2C_NOTINIT,       // I2C not initialized.
    HOJA_I2C_FAIL,          // I2C communication failed.
    HOJA_BATTERY_NOTSET,    // Battery type needs to be set.
} hoja_err_t;

/** 
 * @brief This data should always be provided as uint16_t to provide
 * as much data as possible. This data will likely need to be
 * scaled down to a different range and we should provide more
 * data to avoid as much stairstepping in values as possible. ONLY 12 BIT VALS.
 * 
 * @param           ls_x  Left Stick X Data
 * @param           ls_y  Left Stick Y Data
 * @param           rs_x  Left Stick X Data
 * @param           rs_y  Left Stick X Data
 * @param           lt_a  Left trigger Analog Data
 * @param           rt_a  Right trigger Analog Data
*/
typedef struct 
{
    uint16_t ls_x;
    uint16_t ls_y;
    uint16_t rs_x;
    uint16_t rs_y;
    uint16_t lt_a;
    uint16_t rt_a;
} __attribute__ ((packed)) hoja_analog_data_s;

/** @brief This is a struct for containing all of the 
 * button input data as bits. This saves space
 * and allows for easier handoff to the various
 * controller cores in the future. 
**/
typedef struct 
{
    union
    {
        struct
        {
            // D-Pad
            uint8_t dpad_up             : 1;
            uint8_t dpad_down           : 1;
            uint8_t dpad_left           : 1;
            uint8_t dpad_right          : 1;
            // Buttons
            uint8_t button_up           : 1;
            uint8_t button_down         : 1;
            uint8_t button_left         : 1;
            uint8_t button_right        : 1;
            // Triggers
            uint8_t trigger_l           : 1;
            uint8_t trigger_zl          : 1;
            uint8_t trigger_r           : 1;
            uint8_t trigger_zr          : 1;
            // Side Buttons
            uint8_t side_button_left    : 1;
            uint8_t side_button_right   : 1;

            // Special Functions
            uint8_t button_start        : 1;
            uint8_t button_select       : 1;
            
            // Stick clicks
            uint8_t button_stick_left   : 1;
            uint8_t button_stick_right  : 1;
        };
        uint16_t buttons_all;
    };

    union
    {
        struct
        {
            // Menu buttons (Not remappable by API)
            uint8_t button_capture  : 1;
            uint8_t button_home     : 1;
            uint8_t padding         : 6;
        };
        uint8_t buttons_system;
    };

    // Button for sleeping the controller
    uint8_t button_sleep;

} __attribute__ ((packed)) hoja_button_data_s;


/**
 *  @brief Type to determine what event is being sent
 */
typedef enum
{
    HOJA_EVT_BT,
    HOJA_EVT_SYSTEM,
    HOJA_EVT_USB,
    HOJA_EVT_GC,
    HOJA_EVT_NS,
    HOJA_EVT_INPUT,
    HOJA_EVT_WIRED,
    HOJA_EVT_BOOT,
    HOJA_EVT_BATTERY,
    HOJA_EVT_CHARGER,
} hoja_event_type_t;

/**
 *  @brief Bluetooth events
 */
typedef enum
{
    HEVT_BT_STARTED     = 0,
    HEVT_BT_CONNECTING  = 1,
    HEVT_BT_PAIRED      = 2,
    HEVT_BT_DISCONNECT  = 3,
} hoja_bt_event_t;

/**
 *  @brief Nintendo Switch Core events
 */
typedef enum
{
    HEVT_NS_RUMBLE,
    HEVT_NS_PLAYERSET,
} hoja_ns_event_t;

/**
 *  @brief USB Core(s) events
 */
typedef enum
{
    HEVT_USB_CONNECTED,
    HEVT_USB_DISCONNECTED,
} hoja_usb_event_t;

/**
 *  @brief GameCube Core events
 */
typedef enum
{
    HEVT_GC_RUMBLE
} hoja_gc_event_t;

/**
 *  @brief System events
 */
typedef enum
{
    HEVT_API_INIT_OK,
    HEVT_API_SHUTDOWN,
    HEVT_API_REBOOT,
} hoja_system_event_t;

/**
 *  @brief Wired utility detect events
 */
typedef enum
{
    HEVT_WIRED_NO_DETECT     = 0,
    HEVT_WIRED_SNES_DETECT   = 1,
    HEVT_WIRED_JOYBUS_DETECT = 2,
} hoja_wired_event_t;

typedef enum
{
    HEVT_BOOT_PLUGGED,
    HEVT_BOOT_UNPLUGGED,
    HEVT_BOOT_NOBATTERY,
} hoja_boot_event_t;

typedef enum
{
    HEVT_CHARGER_PLUGGED,
    HEVT_CHARGER_UNPLUGGED,
} hoja_charger_event_t;

/**
 *  @brief Battery utility events
 */
typedef enum
{
    HEVT_BATTERY_CHARGING,
    HEVT_BATTERY_CHARGECOMPLETE,
    HEVT_BATTERY_LVLCHANGE,
    HEVT_BATTERY_NOCHARGE,
} hoja_battery_event_t;

/**
 *  @brief Analog axis types
 */
typedef enum
{
    HOJA_ANALOG_LEFT_X      = 0,
    HOJA_ANALOG_LEFT_Y      = 1,
    HOJA_ANALOG_RIGHT_X     = 2,
    HOJA_ANALOG_RIGHT_Y     = 3,
    HOJA_ANALOG_TRIGGER_L   = 4,
    HOJA_ANALOG_TRIGGER_R   = 5,
} hoja_analog_axis_t;

/**
 *  @brief HOJA Core Types
 */
typedef enum
{
    HOJA_CORE_NULL,
    HOJA_CORE_NS,
    HOJA_CORE_SNES,
    HOJA_CORE_N64,
    HOJA_CORE_GC,
    HOJA_CORE_USB,
    HOJA_CORE_BT_DINPUT,
    HOJA_CORE_BT_XINPUT,
    HOJA_CORE_MAX,
} hoja_core_t;

/**
 *  @brief HOJA System Status
 */
typedef enum
{
    HOJA_STATUS_IDLE        = 0, // HOJA not loaded. Needs initialization.
    HOJA_STATUS_INITIALIZED = 1, // API has been initialized
    HOJA_STATUS_RUNNING     = 2, // Core is running
} hoja_status_t;

/**
 *  @brief NS Core Controller Types
 */
typedef enum
{
    NS_TYPE_PROCON      = 0,
    NS_TYPE_JOYCON_L    = 1,
    NS_TYPE_JOYCON_R    = 2,
    NS_TYPE_SNES        = 3,
    NS_TYPE_NES         = 4,
    NS_TYPE_FC          = 5,
    NS_TYPE_GENESIS     = 6,
} hoja_ns_controller_t;

/**
 *  @brief RGB Struct Type Definition
 */
typedef struct
{
    union
    {
        struct
        {
            uint8_t blue;
            uint8_t green;
            uint8_t red;
            uint8_t dummy;
        };
        uint32_t rgb;
    };
    
} __attribute__ ((packed)) rgb_s;

#endif