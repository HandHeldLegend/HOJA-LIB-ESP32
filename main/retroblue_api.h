#ifndef RETROBLUE_API_H
#define RETROBLUE_API_H

#include "retroblue_backend.h"
#include "retroblue_settings.h"
#include "rbc_switch_core.h"
#include "rbc_err.h"

SemaphoreHandle_t xSemaphore;

/* Functions relating to the API itself */

// Initialize the API.
// This loads any settings saved and initializes flash storage.
rb_err_t rb_api_init();

// Set the controller core
rb_err_t rb_api_setCore(uint8_t core_type);

// Starts the controller. You MUST set the core before running this.
rb_err_t rb_api_startController(void);

/**
 * @brief       This function is called to init callbacks with RetroBlue Bluetooth Events.
 *
 * @param[in]   callback:   pointer to the init callback function.
 */
//rb_err_t rb_api_bt_event_register_callback(rb_bt_cb_t *callback);

/**
 * @brief       This function is called to init callbacks with RetroBlue Controller Events.
 *
 * @param[in]   callback:   pointer to the init callback function.
 */
//rb_err_t rb_api_ct_event_register_callback(rb_ct_cb_t *callback);

#endif
