#ifndef RBC_SNES_CORE_H
#define RBC_SNES_CORE_H

#include "retroblue_api.h"
#include "retroblue_backend.h"
#include "rbc_err.h"

// Button Buffer used to store and send button data
uint16_t snes_button_buffer;

// Start the SNES core
rb_err_t rbc_core_snes_start(void);

// Stop the SNES core
rb_err_t rbc_core_snes_stop(void);

// Looping SNES button task
void snes_task(void * parameters);

#endif