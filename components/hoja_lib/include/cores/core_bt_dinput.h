#ifndef CORE_BT_DINPUT_H
#define CORE_BT_DINPUT_H

#include "hoja_includes.h"

// Handler for vTask for report mode changes.
extern TaskHandle_t bt_dinput_task_handle;

// Start the BT DInput controller core
hoja_err_t core_bt_dinput_start(void);

#endif