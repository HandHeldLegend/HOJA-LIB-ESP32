#ifndef HOJA_SETTINGS_H
#define HOJA_SETTINGS_H

/*This header is for defining all of the settings that get saved to
the onboard flash storage for later loading. */
#include "hoja_includes.h"

// Magic byte for determining if settings are up to date.
// "rb01" -> 0x72623031
#define SETTINGS_MAGIC 0x72623033
#define SETTINGS_NAMESPACE "storage"

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
    uint32_t magic_bytes;

    // Device will auto-boot using the last used core.
    uint8_t controller_core;

    // Controller colors
    uint8_t color_r;
    uint8_t color_g;
    uint8_t color_b;

    uint16_t sx_min;
    uint16_t sx_center;
    uint16_t sx_max;

    uint16_t sy_min;
    uint16_t sy_center;
    uint16_t sy_max;

    // Nintendo Switch Core Settings
    uint8_t ns_client_bt_address[8];
    uint8_t ns_host_bt_address[6];
    bool    ns_controller_paired;
    uint8_t ns_controller_type;

    // SNES Core Settings
    uint8_t snes_stick_dpad;

} hoja_settings_s;

extern hoja_settings_s loaded_settings;

hoja_err_t hoja_settings_init(void);

hoja_err_t hoja_settings_saveall(void);

hoja_err_t hoja_settings_default(void);


#endif