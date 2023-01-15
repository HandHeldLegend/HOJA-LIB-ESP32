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

#include "switch/core_switch_types.h"

extern ns_subcore_t _ns_subcore;

hoja_err_t core_ns_set_subcore(ns_subcore_t subcore);

// Start the Nintendo Switch controller core
hoja_err_t core_ns_start(void);

// Stop the Nintendo Switch controller core
hoja_err_t core_ns_stop(void);

hoja_err_t ns_savepairing(uint8_t* host_addr);

#endif