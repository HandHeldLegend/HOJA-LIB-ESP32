#ifndef CORE_JOYBUS_BACKEND_H
#define CORE_JOYBUS_BACKEND_H

#include "hoja_includes.h"

#include "joybus/core_joybus_gamecube.h"
#include "joybus/core_joybus_n64.h"

// Common joybus RMT definitions
typedef struct {
    struct {
        volatile rmt_item32_t data32[SOC_RMT_MEM_WORDS_PER_CHANNEL];
    } chan[SOC_RMT_CHANNELS_PER_GROUP];
} legacy_rmt_mem_t;

#define JB_BIT_MASK 0x7FF80000
#define JB_BIT_MASK_GC 0x7FF00000

// RMT REGISTER FOR RX STUFF

// RMTMEM address is declared in <target>.peripherals.ld
extern legacy_rmt_mem_t RMTMEM;

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
#define JB_RMT_0X2  JB_LOW, JB_LOW, JB_HIGH, JB_LOW
#define JB_RMT_0X5  JB_LOW, JB_HIGH, JB_LOW, JB_HIGH
#define JB_RMT_0X3  JB_LOW, JB_LOW, JB_HIGH, JB_HIGH
#define JB_RMT_0X8  JB_HIGH, JB_LOW, JB_LOW, JB_LOW
#define JB_RMT_0XF  JB_HIGH, JB_HIGH, JB_HIGH, JB_HIGH

#define JB_ZERO     (rmt_item32_t) {{{0,0,0,0}}}

#define JB_STATUS_LEN 26

// FIFO Mem Access Bit
#define JB_RMT_FIFO     RMT.apb_conf.fifo_mask

// RX Channel 0
#define JB_RX_CHANNEL   RMT_CHANNEL_0
// Start receive
// TX Channesl are CHN. RX are CHM
#define JB_RX_EN            RMT.conf_ch[JB_RX_CHANNEL].conf1.rx_en
// Clock divider
#define JB_RX_CLKDIV        RMT.conf_ch[JB_RX_CHANNEL].conf0.div_cnt
// Block memory size
#define JB_RX_MEMSIZE       RMT.conf_ch[JB_RX_CHANNEL].conf0.mem_size
// Memory owner
#define JB_RX_MEMOWNER      RMT.conf_ch[JB_RX_CHANNEL].conf1.mem_owner
// Clock reference
#define JB_RX_REFALWAYSON    RMT.conf_ch[JB_RX_CHANNEL].conf1.ref_always_on
// Idle level
#define JB_RX_IDLETHRESH    RMT.conf_ch[JB_RX_CHANNEL].conf0.idle_thres
// Receive complete interrupt enable
#define JB_RX_COMPLETEISR   RMT.int_ena.ch0_rx_end
#define JB_RX_CLEARISR      RMT.int_clr.ch0_rx_end
// Memory RD Reset
#define JB_RX_RDRST         RMT.conf_ch[JB_RX_CHANNEL].conf1.mem_rd_rst
// Memory WR Reset
#define JB_RX_WRRST         RMT.conf_ch[JB_RX_CHANNEL].conf1.mem_wr_rst
// Filter enable
#define JB_RX_FILTEREN      RMT.conf_ch[JB_RX_CHANNEL].conf1.rx_filter_en
#define JB_RX_FILTERTHRESH     RMT.conf_ch[JB_RX_CHANNEL].conf1.rx_filter_thres
// Memory access
#define JB_RX_MEM           RMTMEM.chan[JB_RX_CHANNEL].data32

// TX Channel for status
#define JB_STATUS_CHANNEL   RMT_CHANNEL_1
#define JB_STATUS_MEM       RMTMEM.chan[JB_STATUS_CHANNEL].data32
// Status pointer defines
#define JB_STATUS_TXSTART        RMT.conf_ch[JB_STATUS_CHANNEL].conf1.tx_start
#define JB_STATUS_MEMRST         RMT.conf_ch[JB_STATUS_CHANNEL].conf1.mem_rd_rst
#define JB_STATUS_DIVCT          RMT.conf_ch[JB_STATUS_CHANNEL].conf0.div_cnt
#define JB_STATUS_MEMSIZE        RMT.conf_ch[JB_STATUS_CHANNEL].conf0.mem_size
#define JB_STATUS_CONTMODE       RMT.conf_ch[JB_STATUS_CHANNEL].conf1.tx_conti_mode
#define JB_STATUS_CARRIEREN      RMT.conf_ch[JB_STATUS_CHANNEL].conf0.carrier_en
#define JB_STATUS_MEMOWNER       RMT.conf_ch[JB_STATUS_CHANNEL].conf1.mem_owner
#define JB_STATUS_REFALWAYSON    RMT.conf_ch[JB_STATUS_CHANNEL].conf1.ref_always_on
#define JB_STATUS_IDLEOUTEN      RMT.conf_ch[JB_STATUS_CHANNEL].conf1.idle_out_en
#define JB_STATUS_IDLEOUTLVL     RMT.conf_ch[JB_STATUS_CHANNEL].conf1.idle_out_lv
#define JB_STATUS_TXENDSTAT      RMT.int_st.ch1_tx_end
#define JB_STATUS_TXENDINTENA    RMT.int_ena.ch1_tx_end
#define JB_STATUS_CLEARTXINT     RMT.int_clr.ch1_tx_end


//GAMECUBE STUFF
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
// GAMECUBE END

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

} __attribute__ ((packed)) joybus_rx_buffer_s;

typedef enum
{
    JOYBUS_STATUS_IDLE,
    JOYBUS_STATUS_INITALIZED,
    JOYBUS_STATUS_RUNNING_N64,
    JOYBUS_STATUS_RUNNING_GAMECUBE,
} joybus_status_t;

typedef enum
{
    JOYBUS_MODE_IDLE,
    JOYBUS_MODE_GAMECUBE,
    JOYBUS_MODE_N64
} joybus_mode_t;

void joybus_all_deinit(bool reset);

static void gamecube_rmt_isr(void* arg);
static void n64_rmt_isr(void* arg);

void joybus_general_init(void);

hoja_err_t core_joybus_n64_coldboot(void);

hoja_err_t core_joybus_n64_start(void);

hoja_err_t core_joybus_gamecube_start(void);

void core_joybus_stop(void);

#endif
