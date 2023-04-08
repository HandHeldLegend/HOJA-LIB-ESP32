#ifndef HOJA_BACKEND_H
#define HOJA_BACKEND_H

#include "hoja_includes.h"

extern hoja_button_data_s   hoja_button_data;
extern hoja_analog_data_s   hoja_analog_data;
extern hoja_button_data_s   hoja_processed_buttons;
extern button_remap_s       hoja_remaps;

void    hoja_set_battery_lvl(uint8_t level);
uint8_t hoja_get_battery_lvl();

void    hoja_set_external_power(bool plugged);
bool    hoja_get_external_power();

// Function to reset all buttons to neutral values
void hoja_button_reset(void);

// Process remap according to remap config
void hoja_button_remap_process(void);

// Enable or disable remap
void hoja_button_remap_enable(bool enable);

// Task to scan buttons periodically.
void hoja_button_task(void * parameters);

void hoja_set_battery_level(uint8_t level);

void hoja_set_external_power(bool plugged);

bool hoja_get_external_power();

#endif
