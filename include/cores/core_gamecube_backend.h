#ifndef CORE_GAMECUBE_BACKEND_H
#define CORE_GAMECUBE_BACKEND_H

#include "hoja_includes.h"

// Refresh rates for GameCube JoyBus Comms in Hz
#define GAMECUBE_CONTROLLER_RATE_HZ   250000
#define GAMECUBE_CONSOLE_RATE_HZ      200000

// Defines for some parameters relating to GCC communication.
#define GAMECUBE_ID_UPPER   0x09
#define GAMECUBE_ID_LOWER   0x03

typedef enum
{
    GC_CMD_PROBE    = 0x00,
    GC_CMD_ORIGIN   = 0x41,
    GC_CMD_POLL     = 0x40,
} GameCube_Cmd_t;

#define GC_PROBE_RESPONSE_SIZE   3
#define GC_ORIGIN_RESPONSE_SIZE  10
#define GC_POLL_RESPONSE_SIZE    8

// Helpers to get the index for specific
// buttons and sticks
#define GC_BUTTON_A         7
#define GC_BUTTON_B         6
#define GC_BUTTON_X         5
#define GC_BUTTON_Y         4
#define GC_BUTTON_LB        9
#define GC_BUTTON_RB        10
#define GC_BUTTON_Z         11
#define GC_BUTTON_START     3
#define GC_BUTTON_DUP       12
#define GC_BUTTON_DDOWN     13
#define GC_BUTTON_DRIGHT    14
#define GC_BUTTON_DLEFT     15

#define GC_ADC_LEFTX        16
#define GC_ADC_LEFTY        24
#define GC_ADC_RIGHTX       32
#define GC_ADC_RIGHTY       40
#define GC_ADC_LT           48
#define GC_ADC_RT           56

#define GC_ADC_LEN          8

void gamecube_input_translate(void);

hoja_err_t gamecube_respond_command(uint8_t cmd, uint8_t rumble);

hoja_err_t core_gamecube_start(void);

hoja_err_t core_gamecube_stop(void);

#endif
