#ifndef UTIL_WIRED_DETECT_H
#define UTIL_WIRED_DETECT_H

#include "hoja_includes.h"

typedef enum
{
    DETECT_NONE     = 0,
    DETECT_JOYBUS   = 1,
    DETECT_SNES     = 2,
} util_wire_det_t;

// We want to fire an interrupt
// at 8 pulses received.
#define PULSE_MAX_WIRED 32
#define PULSE_MIN_POSS  4
#define PULSE_MAX_POSS  64

// For the detection algorithm. We can monitor
// the appropriate NS pins and count pulses.
// We can have a timeout set so that if we do not
// encounter the appropriate amount of pulses
typedef struct {
    int unit;
    uint32_t status;

} pcnt_evt_t;

// PUBLIC FUNCTIONS
// -----------------
void util_wired_detect(void);

hoja_err_t util_wired_detect_loop(void);
// -----------------
// -----------------

#endif