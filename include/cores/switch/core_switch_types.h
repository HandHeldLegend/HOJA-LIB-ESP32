#ifndef CORE_SWITCH_TYPES_H
#define CORE_SWITCH_TYPES_H

/**
 *  @brief NS Core Controller Types
 */
typedef enum
{
    NS_TYPE_UNSET,
    NS_TYPE_PROCON,
    NS_TYPE_N64,
    NS_TYPE_JOYCON_L,
    NS_TYPE_JOYCON_R,
    NS_TYPE_SNES,
    NS_TYPE_NES,
    NS_TYPE_FC,
    NS_TYPE_GENESIS,
    NS_TYPE_MAX,
} ns_subcore_t;

/**
 * @brief NS Core Report mode enums
*/
typedef enum
{
    NS_REPORT_MODE_IDLE,
    NS_REPORT_MODE_SIMPLE,
    NS_REPORT_MODE_FULL,
    NS_REPORT_MODE_MAX,
} ns_report_mode_t;

/**
 * @brief NS Core Status
*/
typedef enum
{
    NS_STATUS_IDLE,
    NS_STATUS_SUBCORESET,
    NS_STATUS_RUNNING,
} ns_core_status_t;

// Input report IDs

// Input report short
#define NS_REPORT_SHORT 0x3F
// Input report full
#define NS_REPORT_FULL  0x30
// Input report with subcmd reply
#define NS_REPORT_SUBCMD 0x21

/*
#define COMM_RID_MCUFWUP          0x23
#define COMM_RID_MCUMODE          0x31
#define COMM_RID_UNKNOWN          0x32
#define COMM_RID_UNKNOWN2         0x33
*/

// Output report IDs

// Output report ID types

// Subcmd with rumble Output report
#define COMM_RUMBLE_SUB     0x01
// NFC/IR firmware update report
#define COMM_NFCIRFWUPDATE  0x03
// Rumble data only report
#define COMM_RUMBLE_ONLY    0x10
// Request for NFC or IR report
#define COMM_NFCIR_REQUEST  0x11

// Sub-command types
#define SUBC_GET_CONTROLLERSTATE    0x00
#define SUBC_BLUETOOTH_PAIR         0x01
#define SUBC_GET_DEVINFO            0x02
#define SUBC_SET_INPUTMODE          0x03
#define SUBC_GET_TRIGGER_ET         0x04
#define SUBC_GET_PAGELIST_STATE     0x05
#define SUBC_SET_HCI_STATE          0x06
#define SUBC_RESET_PAIRING          0x07
#define SUBC_SET_SHIPMODE           0x08
#define SUBC_READ_SPI               0x10
#define SUBC_WRITE_SPI              0x11
#define SUBC_ERASE_SPI              0x12
#define SUBC_RESET_MCU              0x20
#define SUBC_SET_MCUCONFIG          0x21
#define SUBC_SET_MCUSTATE           0x22
#define SUBC_SET_MCUUNKNOWN         0x24
#define SUBC_RESET_MCUUNKNOWN       0x25
#define SUBC_SET_UNKNOWN2           0x28
#define SUBC_GET_NFCIRDATA          0x29
#define SUBC_SET_GPIOPIN            0x2A
#define SUBC_GET_NFCIRDATA2         0x2B
#define SUBC_SET_PLAYER             0x30
#define SUBC_GET_PLAYER             0x31
#define SUBC_SET_HOMELED            0x38
#define SUBC_ENABLE_IMU             0x40
#define SUBC_SET_IMUSENSITIVITY     0x41
#define SUBC_WRITE_IMUREGISTER      0x42
#define SUBC_READ_IMUREGISTER       0x43
#define SUBC_ENABLE_VIBRATION       0x48
#define SUBC_GET_VOLTAGEREG         0x50
#define SUBC_SETGPIOPIN2            0x51
#define SUBC_GETGPIOPIN             0x52

// Pair command types
#define PAIR_GETBTADDR      0x01
#define PAIR_GETLTK         0x02
#define PAIR_SAVEINFO       0x03

/* Define controller global elements */
#define NS_FW_PRIMARY       0x03
#define NS_FW_SECONDARY     0x80

#endif
