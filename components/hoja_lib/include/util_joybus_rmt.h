#ifndef UTIL_JOYBUS_RMT_H
#define UTIL_JOYBUS_RMT_H

#include "hoja_includes.h"

#define RMT_TX_CHANNEL_POLL     RMT_CHANNEL_4
#define RMT_TX_CHANNEL_ORIGIN   RMT_CHANNEL_2
#define RMT_TX_CHANNEL_PROBE    RMT_CHANNEL_1
#define RMT_RX_CHANNEL          RMT_CHANNEL_0

#define JB_0L_TICKS     12
#define JB_0H_TICKS     4
#define JB_1L_TICKS     4
#define JB_1H_TICKS     12
#define JB_STOP_TICKS   8

#define JB_IDLE_TICKS   32

#define JB_LOW      (rmt_item32_t) {{{ JB_0L_TICKS,      0, JB_0H_TICKS, 1 }}}
#define JB_HIGH     (rmt_item32_t) {{{ JB_1L_TICKS,      0, JB_1H_TICKS, 1 }}}
#define JB_STOP     (rmt_item32_t) {{{ JB_STOP_TICKS,    0, 0,           1 }}}

#define JB_RMT_0X0  JB_LOW, JB_LOW, JB_LOW, JB_LOW
#define JB_RMT_0X9  JB_HIGH, JB_LOW, JB_LOW, JB_HIGH
#define JB_RMT_0X3  JB_LOW, JB_LOW, JB_HIGH, JB_HIGH
#define JB_RMT_0X8  JB_HIGH, JB_LOW, JB_LOW, JB_LOW
#define JB_RMT_0XF  JB_HIGH, JB_HIGH, JB_HIGH, JB_HIGH

#define JB_ZERO     (rmt_item32_t) {{{0,0,0,0}}}

typedef struct
{
    union
    {
        struct
        {
            uint8_t byte0 : 8;
            uint8_t byte1 : 8;
            uint8_t byte2 : 8;
            uint8_t byte3 : 8;
        };
        uint32_t val;
    };
    
} joybus_rx_buffer_s;

#endif

