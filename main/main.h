#ifndef MAIN_H
#define MAIN_H

#include "retroblue_api.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#define BTN_STATE_OPEN      0
#define BTN_STATE_PRESSED   1
#define BTN_STATE_HELD      2
#define BTN_STATE_RELEASED  3

typedef struct
{
    int gpio_num;
    uint16_t debounce;
    bool pressed;
} button_s;

bool debounce(uint16_t* debounce, bool pressed, int gpio_num);

#endif
