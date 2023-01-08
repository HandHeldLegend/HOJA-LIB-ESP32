#ifndef HOJA_SETTINGS_H
#define HOJA_SETTINGS_H

/*This header is for defining all of the settings that get saved to
the onboard flash storage for later loading. */
#include "hoja_includes.h"

// Magic byte for determining if settings are up to date.
// "rb01" -> 0x72623031
#define SETTINGS_MAGIC 0x72623037
#define SETTINGS_NAMESPACE "hoja_settings"

typedef enum
{
    HOJA_CONTROLLER_MODE_NS,
    HOJA_CONTROLLER_MODE_RETRO,
    HOJA_CONTROLLER_MODE_XINPUT,
    HOJA_CONTROLLER_MODE_DINPUT,
} hoja_controller_mode_t;

/**
 * @brief This is a struct which contains all of the
 * elements of the settings. We can then neatly save this
 * and retrieve it as one object instead of scattering everything
 * around.
 * 
 */
typedef struct
{
    // Universal settings for HOJA library
    //------------------------------------
    // Magic byte to determine version changes
    uint32_t magic_bytes;
    //------------------------------------
    //------------------------------------

    // Save controller mode so we can boot
    // into the same mode each time automatically.
    hoja_controller_mode_t controller_mode;

    // Stick calibration
    //------------------------------------
    // Left stick
    uint16_t sx_min;
    uint16_t sx_center;
    uint16_t sx_max;

    uint16_t sy_min;
    uint16_t sy_center;
    uint16_t sy_max;

    // Right stick
    uint16_t cx_min;
    uint16_t cx_center;
    uint16_t cx_max;

    uint16_t cy_min;
    uint16_t cy_center;
    uint16_t cy_max;
    //------------------------------------
    //------------------------------------

    // Controller color
    //------------------------------------
    uint8_t color_r;
    uint8_t color_g;
    uint8_t color_b;
    //------------------------------------
    //------------------------------------

    // Nintendo Switch Core Settings
    //------------------------------------
    uint8_t ns_client_bt_address[8];
    uint8_t ns_host_bt_address[6];
    bool    ns_controller_paired;
    uint8_t ns_controller_type;
    //------------------------------------
    //------------------------------------

    // Bluetooth DInput Core Settings
    //------------------------------------
    uint8_t dinput_client_bt_address[8];
    uint8_t dinput_host_bt_address[6];
    bool    dinput_controller_paired;
    //------------------------------------
    //------------------------------------

    // Bluetooth XInput Core Settings
    uint8_t xinput_client_bt_address[8];
    uint8_t xinput_host_bt_address[6];
    bool    xinput_controller_paired;
    //------------------------------------
    //------------------------------------

    // SNES Core Settings
    //------------------------------------
    bool snes_stick_dpad;
    //------------------------------------
    //------------------------------------

} __attribute__ ((packed)) hoja_settings_s;

extern hoja_settings_s loaded_settings;

hoja_err_t hoja_settings_init(void);

// PUBLIC FUNCTIONS
// -----------------

hoja_err_t hoja_settings_saveall(void);

hoja_err_t hoja_settings_default(void);

void hoja_settings_generate_btmac(uint8_t * out);

// -----------------
// -----------------


#endif