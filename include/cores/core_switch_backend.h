#ifndef CORE_SWITCH_BACKEND_H
#define CORE_SWITCH_BACKEND_H

// Include any necessary includes from HOJA backend
#include "hoja_includes.h"

// Nintendo Switch Core Includes
// Library for handling commands and sub-commands
#include "switch/core_switch_comms.h"

// Libaray for handling input report
// generation
#include "switch/core_switch_reports.h"

// Library for handling controller input
// buttons, sticks, sixaxis
#include "switch/core_switch_input.h"

// Library for handling controller
// configuration and loaded controller
// data.
#include "switch/core_switch_controller.h"

// Library for handling SPI read command emulation
#include "switch/core_switch_spi.h"

// Library for vibration emulation (todo)
#include "switch/core_switch_vibration.h"

// Library for motion controls
#include "switch/core_switch_imu.h"

// Library for interpreting vibration data
// and translating it for the RetroBlue API
//#include "rbc_switch_vibration.h"

/**
 *  @brief NS Core Controller Types
 */
typedef enum
{
    NS_TYPE_UNSET,
    NS_TYPE_PROCON,
    NS_TYPE_N64,
    NS_TYPE_JOYCON_L,
    NS_TYPE_JOYCON_R,
    NS_TYPE_SNES,
    NS_TYPE_NES,
    NS_TYPE_FC,
    NS_TYPE_GENESIS,
    NS_TYPE_MAX,
} ns_subcore_t;

extern ns_subcore_t _ns_subcore;

/**
 * @brief NS Core Status
*/
typedef enum
{
    NS_STATUS_IDLE,
    NS_STATUS_SUBCORESET,
    NS_STATUS_RUNNING,
} ns_core_status_t;

/* Define controller global elements */
#define NS_FW_PRIMARY       0x03
#define NS_FW_SECONDARY     0x80

#define NS_COLOR_SET        true

/* Define color defaults */
#define NS_COLOR_BLACK      0x00

// Parameters for bluetooth function
typedef struct 
{
    esp_hidd_app_param_t app_param;
    esp_hidd_qos_param_t both_qos;
    SemaphoreHandle_t ns_semaphore;
} nscore_param_s;

// Instance of NS Core parameters
extern nscore_param_s ns_core_param;

// Handler for vTask for report mode changes.
extern TaskHandle_t ns_ReportModeHandle;

extern uint8_t ns_currentReportMode;

hoja_err_t core_ns_set_subcore(ns_subcore_t subcore);

// Start the Nintendo Switch controller core
hoja_err_t core_ns_start(void);

// Stop the Nintendo Switch controller core
hoja_err_t core_ns_stop(void);

hoja_err_t ns_savepairing(uint8_t* host_addr);

#endif