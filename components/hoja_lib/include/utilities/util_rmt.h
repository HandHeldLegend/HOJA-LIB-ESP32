#ifndef UTIL_RMT_H
#define UTIL_RMT_H

#include "hoja_includes.h"

typedef struct {
    struct {
        volatile rmt_item32_t data32[SOC_RMT_MEM_WORDS_PER_CHANNEL];
    } chan[SOC_RMT_CHANNELS_PER_GROUP];
} legacy_rmt_mem_t;

// RMTMEM address is declared in <target>.peripherals.ld
extern legacy_rmt_mem_t RMTMEM;

// For ESP32 S3
#if CONFIG_IDF_TARGET_ESP32S3

#define JB_RX_CHANNEL   4

// FIFO Mem Access Bit
#define JB_RMT_FIFO     RMT.sys_conf.apb_fifo_mask

// Start transmission
// TX Channesl are CHN. RX are CHM
#define JB_TX_BEGIN     RMT.chnconf0[0].tx_start_chn
// Clock divider
#define JB_TX_CLKDIV    RMT.chnconf0[0].div_cnt_chn
// Block memory size
#define JB_TX_MEMSIZE   RMT.chnconf0[0].mem_size_chn
// Continuous Transmission Enable
#define JB_TX_CONT      RMT.chnconf0[0].tx_conti_mode_chn
// Carrier Enable
#define JB_TX_CARRIER   RMT.chnconf0[0].carrer_en_chn
// Memory owner
//#define JB_TX_MEMOWNER  UNUSED FOR TX ON ESP32S3
// Idle level
#define JB_TX_IDLELVL   RMT.chnconf0[0].idle_out_lv_chn
// Idle level enable
#define JB_TX_IDLEEN    RMT.chnconf0[0].idle_out_en_chn
// Transmission complete interrupt enable
#define JB_TX_COMPLETEISR   RMT.int_ena.ch0_tx_end_int_ena
// Memory RD Reset
#define JB_TX_RDRST     RMT.chnconf0[0].mem_rd_rst_chn
// Enable carrier
#define JB_TX_CARRIER   RMT.chnconf0[0].carrier_en_chn
// Memory access
#define JB_TX_MEM           RMTMEM.chan[0].data32

// Start receive
// TX Channesl are CHN. RX are CHM
#define JB_RX_EN        RMT.chmconf[0].conf1.rx_en_chm
// Clock divider
#define JB_RX_CLKDIV    RMT.chmconf[0].conf0.div_cnt_chm
// Block memory size
#define JB_RX_MEMSIZE   RMT.chmconf[0].conf0.mem_size_chm
// Memory owner
#define JB_RX_MEMOWNER  RMT.chmconf[0].conf1.mem_owner_chm
// Idle level
#define JB_RX_IDLETHRESH    RMT.chmconf[0].conf0.idle_thres_chm
// Receive complete interrupt enable
#define JB_RX_COMPLETEISR   RMT.int_ena.ch4_rx_end_int_ena
// Memory RD Reset
#define JB_RX_RDRST     RMT.chmconf[0].conf1.mem_wr_rst_chm
// Memory access
#define JB_RX_MEM           RMTMEM.chan[4].data32

// For ESP32 standard
#elif CONFIG_IDF_TARGET_ESP32

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
#define JB_RX_FILTERLEN     RMT.conf_ch[JB_RX_CHANNEL].conf1.rx_filter_thres
// Memory access
#define JB_RX_MEM           RMTMEM.chan[JB_RX_CHANNEL].data32

// TX Channel Defines
#define JB_TX_CHANNEL_PROBE     RMT_CHANNEL_1
#define JB_TX_CHANNEL_ORIGIN    RMT_CHANNEL_2
#define JB_TX_CHANNEL_POLL      RMT_CHANNEL_4

// TX Channel Memory
#define JB_PROBE_MEM    RMTMEM.chan[JB_TX_CHANNEL_PROBE].data32
#define JB_ORIGIN_MEM   RMTMEM.chan[JB_TX_CHANNEL_ORIGIN].data32
#define JB_POLL_MEM     RMTMEM.chan[JB_TX_CHANNEL_POLL].data32

// Probe pointer defines
#define JB_PROBE_TXSTART        RMT.conf_ch[JB_TX_CHANNEL_PROBE].conf1.tx_start
#define JB_PROBE_MEMRST         RMT.conf_ch[JB_TX_CHANNEL_PROBE].conf1.mem_rd_rst
#define JB_PROBE_DIVCT          RMT.conf_ch[JB_TX_CHANNEL_PROBE].conf0.div_cnt
#define JB_PROBE_MEMSIZE        RMT.conf_ch[JB_TX_CHANNEL_PROBE].conf0.mem_size
#define JB_PROBE_CONTMODE       RMT.conf_ch[JB_TX_CHANNEL_PROBE].conf1.tx_conti_mode
#define JB_PROBE_CARRIEREN      RMT.conf_ch[JB_TX_CHANNEL_PROBE].conf0.carrier_en
#define JB_PROBE_MEMOWNER       RMT.conf_ch[JB_TX_CHANNEL_PROBE].conf1.mem_owner
#define JB_PROBE_REFALWAYSON    RMT.conf_ch[JB_TX_CHANNEL_PROBE].conf1.ref_always_on
#define JB_PROBE_IDLEOUTEN      RMT.conf_ch[JB_TX_CHANNEL_PROBE].conf1.idle_out_en
#define JB_PROBE_IDLEOUTLVL     RMT.conf_ch[JB_TX_CHANNEL_PROBE].conf1.idle_out_lv
#define JB_PROBE_TXENDSTAT      RMT.int_st.ch1_tx_end
#define JB_PROBE_TXENDINTENA    RMT.int_ena.ch1_tx_end
#define JB_PROBE_CLEARTXINT     RMT.int_clr.ch1_tx_end

// Origin pointer defines
#define JB_ORIGIN_TXSTART        RMT.conf_ch[JB_TX_CHANNEL_ORIGIN].conf1.tx_start
#define JB_ORIGIN_MEMRST         RMT.conf_ch[JB_TX_CHANNEL_ORIGIN].conf1.mem_rd_rst
#define JB_ORIGIN_DIVCT          RMT.conf_ch[JB_TX_CHANNEL_ORIGIN].conf0.div_cnt
#define JB_ORIGIN_MEMSIZE        RMT.conf_ch[JB_TX_CHANNEL_ORIGIN].conf0.mem_size
#define JB_ORIGIN_CONTMODE       RMT.conf_ch[JB_TX_CHANNEL_ORIGIN].conf1.tx_conti_mode
#define JB_ORIGIN_CARRIEREN      RMT.conf_ch[JB_TX_CHANNEL_ORIGIN].conf0.carrier_en
#define JB_ORIGIN_MEMOWNER       RMT.conf_ch[JB_TX_CHANNEL_ORIGIN].conf1.mem_owner
#define JB_ORIGIN_REFALWAYSON    RMT.conf_ch[JB_TX_CHANNEL_ORIGIN].conf1.ref_always_on
#define JB_ORIGIN_IDLEOUTEN      RMT.conf_ch[JB_TX_CHANNEL_ORIGIN].conf1.idle_out_en
#define JB_ORIGIN_IDLEOUTLVL     RMT.conf_ch[JB_TX_CHANNEL_ORIGIN].conf1.idle_out_lv
#define JB_ORIGIN_TXENDSTAT      RMT.int_st.ch2_tx_end
#define JB_ORIGIN_TXENDINTENA    RMT.int_ena.ch2_tx_end
#define JB_ORIGIN_CLEARTXINT     RMT.int_clr.ch2_tx_end

// Poll pointer defines
#define JB_POLL_TXSTART        RMT.conf_ch[JB_TX_CHANNEL_POLL].conf1.tx_start
#define JB_POLL_MEMRST         RMT.conf_ch[JB_TX_CHANNEL_POLL].conf1.mem_rd_rst
#define JB_POLL_DIVCT          RMT.conf_ch[JB_TX_CHANNEL_POLL].conf0.div_cnt
#define JB_POLL_MEMSIZE        RMT.conf_ch[JB_TX_CHANNEL_POLL].conf0.mem_size
#define JB_POLL_CONTMODE       RMT.conf_ch[JB_TX_CHANNEL_POLL].conf1.tx_conti_mode
#define JB_POLL_CARRIEREN      RMT.conf_ch[JB_TX_CHANNEL_POLL].conf0.carrier_en
#define JB_POLL_MEMOWNER       RMT.conf_ch[JB_TX_CHANNEL_POLL].conf1.mem_owner
#define JB_POLL_REFALWAYSON    RMT.conf_ch[JB_TX_CHANNEL_POLL].conf1.ref_always_on
#define JB_POLL_IDLEOUTEN      RMT.conf_ch[JB_TX_CHANNEL_POLL].conf1.idle_out_en
#define JB_POLL_IDLEOUTLVL     RMT.conf_ch[JB_TX_CHANNEL_POLL].conf1.idle_out_lv
#define JB_POLL_TXENDSTAT      RMT.int_st.ch4_tx_end
#define JB_POLL_TXENDINTENA    RMT.int_ena.ch4_tx_end
#define JB_POLL_CLEARTXINT     RMT.int_clr.ch4_tx_end

#endif

#endif