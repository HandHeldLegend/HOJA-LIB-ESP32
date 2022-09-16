#ifndef CORE_SNES_BACKEND_H
#define CORE_SNES_BACKEND_H

#include "hoja_includes.h"

// Button Buffer used to store and send button data
uint16_t snes_button_buffer;

// Exposed functions
//--------------------

// Start the SNES core
hoja_err_t core_snes_start(void);

// Stop the SNES core
hoja_err_t core_snes_stop(void);

// End exposed functions
//--------------------


// Private functions
//--------------------

// Looping SNES button task
void snes_task(void * parameters);

#endif