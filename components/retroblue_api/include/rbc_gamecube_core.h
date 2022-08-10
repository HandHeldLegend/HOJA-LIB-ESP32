#ifndef RBC_GAMECUBE_CORE_H
#define RBC_GAMECUBE_CORE_H

#include "retroblue_backend.h"
#include "retroblue_api.h"
#include "rbc_util_joybus_rmt.h"
#include <hal/clk_gate_ll.h>
#include <hal/rmt_ll.h>

#define RBC_GAMECUBE_CONTROLLER_RESOLUTION_HZ   250000
#define RBC_GAMECUBE_CONSOLE_RESOLUTION_HZ      200000

// Defines for some parameters relating to GCC communication.
#define RBC_GAMECUBE_ID_UPPER   0x09
#define RBC_GAMECUBE_ID_LOWER   0x03

#define RBC_GC_CMD_PROBE        0x00
#define RBC_GC_CMD_ORIGIN       0x41
#define RBC_GC_CMD_POLL         0x40

#define GC_PROBE_RESPONSE_LEN   3
#define GC_ORIGIN_RESPONSE_LEN  10
#define GC_POLL_RESPONSE_LEN    8

// Helpers to get the index for specific
// buttons and sticks
#define GC_BUTTON_A         7
#define GC_BUTTON_B         6
#define GC_BUTTON_X         5
#define GC_BUTTON_Y         4
#define GC_BUTTON_LB        8
#define GC_BUTTON_RB        9
#define GC_BUTTON_Z         10
#define GC_BUTTON_START     3
#define GC_BUTTON_DUP       11
#define GC_BUTTON_DDOWN     12
#define GC_BUTTON_DRIGHT    13
#define GC_BUTTON_DLEFT     14

#define GC_ADC_LEFTX        23
#define GC_ADC_LEFTY        31
#define GC_ADC_RIGHTX       39
#define GC_ADC_RIGHTY       47
#define GC_ADC_LT           55
#define GC_ADC_RT           63

#define GC_ADC_LEN          8

rb_err_t gc_input_translate(void);

rb_err_t gc_respond_command(uint8_t cmd, uint8_t rumble);

void gamecube_core_start(void);

#endif
