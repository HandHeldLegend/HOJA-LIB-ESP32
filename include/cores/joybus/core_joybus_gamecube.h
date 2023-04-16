#ifndef CORE_JOYBUS_GAMECUBE_H
#define CORE_JOYBUS_GAMECUBE_H

// Common definitions for GameCube Joybus RMT setup.

#include "hoja_includes.h"

// TX Channel Defines
#define GAMECUBE_CHANNEL_ORIGIN     RMT_CHANNEL_2
#define GAMECUBE_CHANNEL_POLL       RMT_CHANNEL_4

// TX Channel Memory
#define GAMECUBE_ORIGIN_MEM   RMTMEM.chan[GAMECUBE_CHANNEL_ORIGIN].data32
#define GAMECUBE_POLL_MEM     RMTMEM.chan[GAMECUBE_CHANNEL_POLL].data32

// Origin pointer defines
#define GAMECUBE_ORIGIN_TXSTART        RMT.conf_ch[GAMECUBE_CHANNEL_ORIGIN].conf1.tx_start
#define GAMECUBE_ORIGIN_MEMRST         RMT.conf_ch[GAMECUBE_CHANNEL_ORIGIN].conf1.mem_rd_rst
#define GAMECUBE_ORIGIN_DIVCT          RMT.conf_ch[GAMECUBE_CHANNEL_ORIGIN].conf0.div_cnt
#define GAMECUBE_ORIGIN_MEMSIZE        RMT.conf_ch[GAMECUBE_CHANNEL_ORIGIN].conf0.mem_size
#define GAMECUBE_ORIGIN_CONTMODE       RMT.conf_ch[GAMECUBE_CHANNEL_ORIGIN].conf1.tx_conti_mode
#define GAMECUBE_ORIGIN_CARRIEREN      RMT.conf_ch[GAMECUBE_CHANNEL_ORIGIN].conf0.carrier_en
#define GAMECUBE_ORIGIN_MEMOWNER       RMT.conf_ch[GAMECUBE_CHANNEL_ORIGIN].conf1.mem_owner
#define GAMECUBE_ORIGIN_REFALWAYSON    RMT.conf_ch[GAMECUBE_CHANNEL_ORIGIN].conf1.ref_always_on
#define GAMECUBE_ORIGIN_IDLEOUTEN      RMT.conf_ch[GAMECUBE_CHANNEL_ORIGIN].conf1.idle_out_en
#define GAMECUBE_ORIGIN_IDLEOUTLVL     RMT.conf_ch[GAMECUBE_CHANNEL_ORIGIN].conf1.idle_out_lv
#define GAMECUBE_ORIGIN_TXENDSTAT      RMT.int_st.ch2_tx_end
#define GAMECUBE_ORIGIN_TXENDINTENA    RMT.int_ena.ch2_tx_end
#define GAMECUBE_ORIGIN_CLEARTXINT     RMT.int_clr.ch2_tx_end

// Poll pointer defines
#define GAMECUBE_POLL_TXSTART        RMT.conf_ch[GAMECUBE_CHANNEL_POLL].conf1.tx_start
#define GAMECUBE_POLL_MEMRST         RMT.conf_ch[GAMECUBE_CHANNEL_POLL].conf1.mem_rd_rst
#define GAMECUBE_POLL_DIVCT          RMT.conf_ch[GAMECUBE_CHANNEL_POLL].conf0.div_cnt
#define GAMECUBE_POLL_MEMSIZE        RMT.conf_ch[GAMECUBE_CHANNEL_POLL].conf0.mem_size
#define GAMECUBE_POLL_CONTMODE       RMT.conf_ch[GAMECUBE_CHANNEL_POLL].conf1.tx_conti_mode
#define GAMECUBE_POLL_CARRIEREN      RMT.conf_ch[GAMECUBE_CHANNEL_POLL].conf0.carrier_en
#define GAMECUBE_POLL_MEMOWNER       RMT.conf_ch[GAMECUBE_CHANNEL_POLL].conf1.mem_owner
#define GAMECUBE_POLL_REFALWAYSON    RMT.conf_ch[GAMECUBE_CHANNEL_POLL].conf1.ref_always_on
#define GAMECUBE_POLL_IDLEOUTEN      RMT.conf_ch[GAMECUBE_CHANNEL_POLL].conf1.idle_out_en
#define GAMECUBE_POLL_IDLEOUTLVL     RMT.conf_ch[GAMECUBE_CHANNEL_POLL].conf1.idle_out_lv
#define GAMECUBE_POLL_TXENDSTAT      RMT.int_st.ch4_tx_end
#define GAMECUBE_POLL_TXENDINTENA    RMT.int_ena.ch4_tx_end
#define GAMECUBE_POLL_CLEARTXINT     RMT.int_clr.ch4_tx_end

#define GC_ORIGIN_RESPONSE_SIZE  82
#define GC_POLL_RESPONSE_SIZE    66

// Helpers to get the index for specific
// buttons and sticks
#define GC_BUTTON_A         7
#define GC_BUTTON_B         6
#define GC_BUTTON_X         5
#define GC_BUTTON_Y         4
#define GC_BUTTON_LB        9
#define GC_BUTTON_RB        10
#define GC_BUTTON_Z         11
#define GC_BUTTON_START     3
#define GC_BUTTON_DUP       12
#define GC_BUTTON_DDOWN     13
#define GC_BUTTON_DRIGHT    14
#define GC_BUTTON_DLEFT     15

#define GC_ADC_LEFTX        16
#define GC_ADC_LEFTY        24
#define GC_ADC_RIGHTX       32
#define GC_ADC_RIGHTY       40
#define GC_ADC_LT           48
#define GC_ADC_RT           56

#define GC_ADC_LEN          8

#endif