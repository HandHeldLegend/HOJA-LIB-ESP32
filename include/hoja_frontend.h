#ifndef HOJA_FRONTEND_H
#define HOJA_FRONTEND_H

#include "hoja_includes.h"

extern hoja_status_t hoja_current_status;

// This struct type is used to
// set up HOJA API before runtime.
// This should be passed through with hoja_api_init()
// Different settings are implemented here which are
// not meant to change during runtime and should
// be set before compilation

// PUBLIC FUNCTIONS
// -----------------

hoja_err_t hoja_init(void);

hoja_err_t hoja_set_core(hoja_core_t core);

hoja_err_t hoja_start_core(void);

hoja_err_t hoja_stop_core(void);

hoja_err_t hoja_set_color(rgb_s color);

hoja_err_t hoja_set_analog_calibration(hoja_analog_axis_t axis, uint16_t min, uint16_t max, uint16_t center);

hoja_err_t hoja_reset_pairing(hoja_core_t core);

// Core-specific functions.

// NINTENDO SWITCH
// ----------------

hoja_err_t hoja_ns_set_type(hoja_ns_controller_t controller);

// ----------------
// -----------------

// Vars for callbacks
typedef void (*hoja_button_callback_t) (hoja_button_data_s *);
typedef void (*hoja_analog_callback_t) (hoja_analog_data_s *);
typedef void (*hoja_event_callback_t) (hoja_event_type_t, uint8_t, uint8_t);
extern hoja_button_callback_t hoja_button_cb;
extern hoja_analog_callback_t hoja_analog_cb;
extern hoja_event_callback_t hoja_event_cb;

// Function for button callback must be as follows (function name can change)
// Ex: void local_button_cb(hoja_button_data_s *button_data)
hoja_err_t hoja_register_button_callback(hoja_button_callback_t func);

// Ex: void local_analog_cb(hoja_analog_data_s *analog_data)
hoja_err_t hoja_register_analog_callback(hoja_analog_callback_t func);

// Ex: void local_event_cb(hoja_event_type_t type, uint8_t evt, uint8_t param)
hoja_err_t hoja_register_event_callback(hoja_event_callback_t func);

#endif
