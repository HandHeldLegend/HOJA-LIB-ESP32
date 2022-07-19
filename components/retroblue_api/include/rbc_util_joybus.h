#ifndef RBC_UTIL_JOYBUS_H
#define RBC_UTIL_JOYBUS_H

#include "retroblue_backend.h"

#define JB_OUT_ZERO_LOW_NS      3000
#define JB_OUT_ZERO_HIGH_NS     1000
#define JB_OUT_ONE_LOW_NS       1000
#define JB_OUT_ONE_HIGH_NS      3000
#define JB_OUT_STOP_NS          2000

uint32_t jb_0l_ticks;
uint32_t jb_0h_ticks;
uint32_t jb_1l_ticks;
uint32_t jb_1h_ticks;

void rbc_joybus_init(rmt_config_t *config);

void rbc_joybus_translate(uint8_t *data, uint8_t size, uint16_t *out_size);

#endif