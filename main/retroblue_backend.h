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
#include <math.h>
#include "esp_timer.h"
#include <byteswap.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "retroblue_settings.h"
#include "retroblue_api.h"

/* CORE TYPES */
#define CORE_NINTENDOSWITCH 0

/** @brief This is a struct for containing all of the 
 * button input data as bits. This saves space
 * and allows for easier handoff to the various
 * controller cores in the future. 
 * @param       dpad_up
 * @param       dpad_down
 * @param       dpad_left
 * @param       dpad_right
 * 
 * @param       button_up  Triangle, NS X, etc.
 * @param       button_down Cross, NS B, etc.
 * @param       button_left
 * @param       button_right
 * 
 * @param       trigger_lu L1, L, LT
 * @param       trigger_ll L2, ZL, LB
 * @param       trigger_ru R1, R, RT
 * @param       trigger_rl R2, ZR, RB
 * 
 * @param       special_start Plus, Options, Menu
 * @param       special_select Minus, Select
 * @param       special_left Capture, Share
 * @param       special_right Home, Xbox, PS
 * 
 * @param       stick_b_left Left stick click
 * @param       stick_b_right Right stick click
 **/

typedef struct 
{
    //Dpad
    uint8_t dpad_up;
    uint8_t dpad_down;
    uint8_t dpad_left;
    uint8_t dpad_right;
    //Buttons
    uint8_t button_up;
    uint8_t button_down;
    uint8_t button_left;
    uint8_t button_right;
    //Triggers
    uint8_t trigger_lu;
    uint8_t trigger_ll;
    uint8_t trigger_ru;
    uint8_t trigger_rl;

    //Special Functions
    uint8_t special_start;
    uint8_t special_select;
    uint8_t special_left;
    uint8_t special_right;
    
    //Stick clicks
    uint8_t stick_b_left;
    uint8_t stick_b_right;
    uint8_t empty1;
    uint8_t empty2;
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

#endif
