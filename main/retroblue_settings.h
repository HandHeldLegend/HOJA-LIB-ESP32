#ifndef RETROBLUE_SETTINGS_H
#define RETROBLUE_SETTINGS_H

/*This header is for defining all of the settings that get saved to
the onboard flash storage for later loading. */
#include "retroblue_backend.h"

/**
 * @brief This is a struct which contains all of the
 * elements of the settings. We can then neatly save this
 * and retrieve it as one object instead of scattering everything
 * around.
 * 
 * @param       controller_core See retroblue.api.h for definitions
 */
typedef struct
{
    uint8_t controller_core;
    uint8_t client_bt_address[6];
    uint8_t host_bt_address[6];
} RetroBlueSettings;

RetroBlueSettings loaded_settings;

void rb_settings_init(void);

#endif