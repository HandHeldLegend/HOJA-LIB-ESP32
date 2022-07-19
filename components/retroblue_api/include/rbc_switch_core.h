#ifndef RBC_SWITCH_CORE_H
#define RBC_SWITCH_CORE_H

// Include any necessary includes from RetroBlue backend
#include "rbc_err.h"
#include "retroblue_backend.h"

// Library for handling commands and sub-commands
#include "rbc_switch_comms.h"

// Libaray for handling input report
// generation
#include "rbc_switch_reports.h"

// Library for handling controller input
// buttons, sticks, sixaxis
#include "rbc_switch_input.h"

// Library for handling controller
// configuration and loaded controller
// data.
#include "rbc_switch_controller.h"

// Library for handling SPI read commands
#include "rbc_switch_spi.h"

// Library for interpreting vibration data
// and translating it for the RetroBlue API
//#include "rbc_switch_vibration.h"

/* Define controller types supported */
#define NS_CONTROLLER_TYPE_JOYCON_L    0x01
#define NS_CONTROLLER_TYPE_JOYCON_R    0x02
#define NS_CONTROLLER_TYPE_PROCON      0x03
#define NS_CONTROLLER_TYPE_NESCLASSIC  0x04
#define NS_CONTROLLER_TYPE_FCCLASSIC   0x05
#define NS_CONTROLLER_TYPE_SNESCLASSIC 0x06
#define NS_CONTROLLER_TYPE_GENESIS     0x07
#define NS_CONTROLLER_TYPE_N64CLASSIC  0x08

/* Define controller global elements */
#define NS_FW_PRIMARY       0x03
#define NS_FW_SECONDARY     0x80

#define NS_COLOR_SET        true

/* Define color defaults */
#define NS_COLOR_BLACK      0x00

// Parameters for bluetooth function
struct nscore_param_s
{
    esp_hidd_app_param_t app_param;
    esp_hidd_qos_param_t both_qos;
    SemaphoreHandle_t ns_semaphore;
};

// Instance of NS Core parameters
struct nscore_param_s ns_core_param;

// HID descriptor instances
static uint8_t hid_descriptor_ns_core[213];

// Handler for vTask for report mode changes.
TaskHandle_t ns_ReportModeHandle;
// Hander for the vTask for keeping a pulse.
TaskHandle_t ns_BlankReportsHandle;

// Start the Nintendo Switch controller core
rb_err_t rbc_core_ns_start(void);

// Stop the Nintendo Switch controller core
rb_err_t rbc_core_ns_stop(void);

rb_err_t rbc_core_ns_savepairing(uint8_t* host_addr);

#endif