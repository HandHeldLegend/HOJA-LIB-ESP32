#ifndef HOJA_BACKEND_H
#define HOJA_BACKEND_H

#include "hoja_includes.h"

extern hoja_button_data_s hoja_button_data;
extern hoja_analog_data_s hoja_analog_data;

// Function to reset all buttons to neutral values
void hoja_button_reset();

// Task to scan buttons periodically.
void hoja_button_task(void * parameters);

#endif
