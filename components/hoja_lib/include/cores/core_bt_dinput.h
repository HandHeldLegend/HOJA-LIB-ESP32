#ifndef CORE_BT_DINPUT_H
#define CORE_BT_DINPUT_H

#include "hoja_includes.h"

// Parameters for bluetooth function
typedef struct 
{
    esp_hidd_app_param_t app_param;
    esp_hidd_qos_param_t both_qos;
    SemaphoreHandle_t ns_semaphore;
} __attribute__ ((packed)) bt_dinput_param_s;

// Instance of DInput parameters
extern bt_dinput_param_s bt_dinput_param;

// Handler for vTask for report mode changes.
extern TaskHandle_t bt_dinput_task_handle;

// Start the BT DInput controller core
hoja_err_t core_bt_dinput_start(void);

// Stop the BT DInput controller core
hoja_err_t core_bt_dinput_stop(void);

hoja_err_t dinput_savepairing(uint8_t* host_addr);

#endif