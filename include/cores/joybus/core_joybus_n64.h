#ifndef CORE_JOYBUS_N64_H
#define CORE_JOYBUS_N64_H

// Common definitions for N64 Joybus RMT setup.

#include "hoja_includes.h"

// RX and TX Channel Define
#define N64_CHANNEL         RMT_CHANNEL_0

// Channel Memory
#define N64_CHANNEL_MEM    RMTMEM.chan[N64_CHANNEL].data32

// Poll pointer defines
#define N64_CHANNEL_TXSTART        RMT.conf_ch[N64_CHANNEL].conf1.tx_start
#define N64_CHANNEL_MEMRST         RMT.conf_ch[N64_CHANNEL].conf1.mem_rd_rst
#define N64_CHANNEL_DIVCT          RMT.conf_ch[N64_CHANNEL].conf0.div_cnt
#define N64_CHANNEL_MEMSIZE        RMT.conf_ch[N64_CHANNEL].conf0.mem_size
#define N64_CHANNEL_CONTMODE       RMT.conf_ch[N64_CHANNEL].conf1.tx_conti_mode
#define N64_CHANNEL_CARRIEREN      RMT.conf_ch[N64_CHANNEL].conf0.carrier_en
#define N64_CHANNEL_MEMOWNER       RMT.conf_ch[N64_CHANNEL].conf1.mem_owner
#define N64_CHANNEL_REFALWAYSON    RMT.conf_ch[N64_CHANNEL].conf1.ref_always_on
#define N64_CHANNEL_IDLEOUTEN      RMT.conf_ch[N64_CHANNEL].conf1.idle_out_en
#define N64_CHANNEL_IDLEOUTLVL     RMT.conf_ch[N64_CHANNEL].conf1.idle_out_lv
#define N64_CHANNEL_TXENDSTAT      RMT.int_st.ch0_tx_end
#define N64_CHANNEL_TXENDINTENA    RMT.int_ena.ch0_tx_end
#define N64_CHANNEL_CLEARTXINT     RMT.int_clr.ch0_tx_end

#define N64_POLL_RESPONSE_SIZE  34
#define N64_PAK_RESPONSE_SIZE   266

// Helpers to get the index for specific
// buttons and sticks
#define N64_BUTTON_A         0
#define N64_BUTTON_B         1
#define N64_BUTTON_Z         2
#define N64_BUTTON_START     3
#define N64_BUTTON_DUP       4
#define N64_BUTTON_DDOWN     5
#define N64_BUTTON_DRIGHT    6
#define N64_BUTTON_DLEFT     7
#define N64_BUTTON_RESET     8
#define N64_BUTTON_L         10
#define N64_BUTTON_R         11
#define N64_BUTTON_CUP       12
#define N64_BUTTON_CDOWN     13
#define N64_BUTTON_CLEFT     14
#define N64_BUTTON_CRIGHT    15

#define N64_ADC_STICKX       16
#define N64_ADC_STICKY       24

#define N64_ADC_LEN          8

#endif