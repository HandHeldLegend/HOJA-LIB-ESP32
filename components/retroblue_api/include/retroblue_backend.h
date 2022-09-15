/** This header file defines different data types that are not 
 * unique to a specific core, but will be used in the general API. 
 * I wanted to put them here because I thought it would look less... ugly?
 * To be honest I have no idea what I'm doing and I've never made an API before.
 * Please send your thoughts and prayers :)
 */
#ifndef RETROBLUE_BACKEND_H
#define RETROBLUE_BACKEND_H

#include <assert.h>
#include "esp_log.h"
#include "esp_hidd_api.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_bt.h"
#include "esp_err.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_gap_bt_api.h"
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stddef.h>
#include "driver/spi_slave.h"
#include "driver/gpio.h"
#include "driver/rmt.h"

#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "retroblue_settings.h"
#include "retroblue_api.h"
#include "rbc_err.h"
#include "rbc_util_joybus_rmt.h"
#include "sdkconfig.h"


/** @brief This is a struct for containing all of the 
 * button input data as bits. This saves space
 * and allows for easier handoff to the various
 * controller cores in the future. 
**/
typedef struct 
{
    //Dpad
    uint8_t d_up;
    uint8_t d_down;
    uint8_t d_left;
    uint8_t d_right;
    //Buttons
    uint8_t b_up;
    uint8_t b_down;
    uint8_t b_left;
    uint8_t b_right;
    //Triggers
    uint8_t t_l;
    uint8_t t_zl;
    uint8_t t_r;
    uint8_t t_zr;

    //Special Functions
    uint8_t b_start;
    uint8_t b_select;
    uint8_t b_capture;
    uint8_t b_home;
    
    //Stick clicks
    uint8_t sb_left;
    uint8_t sb_right;

    // Bool to confirm all data is set
    uint8_t set;
    uint8_t sent;

} GamepadButtonData;

/** 
 * @brief Global struct for reading incoming button data.
 */
GamepadButtonData g_button_data;

/** 
 * @brief This data should always be provided as uint16_t to provide
 * as much data as possible. This data will likely need to be
 * scaled down to a different range and we should provide more
 * data to avoid as much stairstepping in values as possible 
 * 
 * @param           lsx  Left Stick X Data
 * @param           lsy  Left Stick Y Data
 * @param           rsx  Left Stick X Data
 * @param           rsy  Left Stick X Data
*/
typedef struct 
{
    uint16_t lsx;
    uint16_t lsy;
    uint16_t rsx;
    uint16_t rsy;
} GamepadStickData;

/**
 *  @brief Global struct for reading incoming stick data.
 */
GamepadStickData g_stick_data;

typedef enum RB_BT_EVENTS
{
    RB_EVENT_BT_READY       = 0,
    RB_EVENT_BT_CONNECTED   = 1,
    RB_EVENT_BT_PAIRED      = 2,
    RB_EVENT_BT_DISCONNECTED = 3,
    RB_EVENT_BT_SHUTDOWN    = 4
} rb_event_bt_t;

typedef enum RB_CT_EVENTS
{
    RB_EVENT_CT_COLORSET    = 0,
    RB_EVENT_CT_PLAYERSET   = 1,
    RB_EVENT_CT_RUMBLEON    = 2,
    RB_EVENT_CT_RUMBLEBRAKE = 3,
    RB_EVENT_CT_RUMBLEOFF   = 4,
    RB_EVENT_CT_BUTTONCLEAR = 5
} rb_event_ct_t;

typedef struct
{
    uint8_t selected_core;
    bool api_initialized;
    bool core_started;
    bool controller_connected;
} rb_param_s;

rb_param_s rb_params;

// Function to reset all buttons to neutral values
void rb_button_reset();

#endif
