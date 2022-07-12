#ifndef RBC_SWITCH_COMMS_H
#define RBC_SWITCH_COMMS_H

#include "rbc_switch_core.h"

typedef uint8_t ns_core_subcmd_t;

// Sub-command repsponse start index
#define SUB_C_DATA_IDX 11

// Sub-command response minimum full input report length
#define SUB_C_RESPONSE_LENGTH_DEFAULT 15

// Output report ID types
#define COMM_RUMBLE_SUB     0x01
#define COMM_NFCIRFWUPDATE  0x03
#define COMM_RUMBLE_ONLY    0x10
#define COMM_NFCIR_REQUEST  0x11

// Input report ID types
#define COMM_RID_SIMPLE           0x3F
#define COMM_RID_SUBCMDSTANDARD   0x21
#define COMM_RID_MCUFWUP          0x23
#define COMM_RID_STANDARDFULL     0x30
#define COMM_RID_MCUMODE          0x31
#define COMM_RID_UNKNOWN          0x32
#define COMM_RID_UNKNOWN2         0x33

void ns_comms_handle_command(uint8_t command, uint16_t len, uint8_t* p_data);

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

void ns_comms_handle_subcommand(uint8_t command, uint16_t len, uint8_t* p_data);

// Pair command types
#define PAIR_GETBTADDR      0x01
#define PAIR_GETLTK         0x02
#define PAIR_SAVEINFO       0x03

void ns_comms_handle_paircommand(uint8_t command, uint16_t len, uint8_t* p_data);

void ns_comms_handle_mcucommand(uint8_t command, uint16_t len, uint8_t* p_data);

#endif
