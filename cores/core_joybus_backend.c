#include "core_joybus_backend.h"

volatile joybus_status_t     _joybus_status = JOYBUS_STATUS_IDLE;
volatile joybus_mode_t       _joybus_mode = JOYBUS_MODE_IDLE;

rmt_isr_handle_t    _rmt_isr_handle = NULL;
TaskHandle_t        _joybus_task_handle;
volatile uint8_t    _status_attempts = 0;
#define STATUS_FLIP 20
bool _n64_enable = true;
uint8_t _joybus_watchdog_timer = 0;

rmt_item32_t n64_poll_buffer[N64_POLL_RESPONSE_SIZE] = {
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X8, JB_RMT_0X0, 
        JB_RMT_0X8, JB_RMT_0X0,
        JB_STOP, JB_ZERO
    };

// Set up N64
// Canned response to status
static const rmt_item32_t n64_status_buffer[JB_STATUS_LEN] = {
    JB_RMT_0X0, JB_RMT_0X2,
    JB_RMT_0X0, JB_RMT_0X0,
    JB_RMT_0X0, JB_RMT_0X0,
    JB_STOP, JB_ZERO
};

/**
 * This CRC table for repeating bytes is take from
 * the cube64 project
 *  http://cia.vc/stats/project/navi-misc/cube64
 */
uint8_t crc_repeating_table[256] = {
    0xFF, // 0x00
    0x14, // 0x01
    0xAC, // 0x02
    0x47, // 0x03
    0x59, // 0x04
    0xB2, // 0x05
    0x0A, // 0x06
    0xE1, // 0x07
    0x36, // 0x08
    0xDD, // 0x09
    0x65, // 0x0A
    0x8E, // 0x0B
    0x90, // 0x0C
    0x7B, // 0x0D
    0xC3, // 0x0E
    0x28, // 0x0F
    0xE8, // 0x10
    0x03, // 0x11
    0xBB, // 0x12
    0x50, // 0x13
    0x4E, // 0x14
    0xA5, // 0x15
    0x1D, // 0x16
    0xF6, // 0x17
    0x21, // 0x18
    0xCA, // 0x19
    0x72, // 0x1A
    0x99, // 0x1B
    0x87, // 0x1C
    0x6C, // 0x1D
    0xD4, // 0x1E
    0x3F, // 0x1F
    0xD1, // 0x20
    0x3A, // 0x21
    0x82, // 0x22
    0x69, // 0x23
    0x77, // 0x24
    0x9C, // 0x25
    0x24, // 0x26
    0xCF, // 0x27
    0x18, // 0x28
    0xF3, // 0x29
    0x4B, // 0x2A
    0xA0, // 0x2B
    0xBE, // 0x2C
    0x55, // 0x2D
    0xED, // 0x2E
    0x06, // 0x2F
    0xC6, // 0x30
    0x2D, // 0x31
    0x95, // 0x32
    0x7E, // 0x33
    0x60, // 0x34
    0x8B, // 0x35
    0x33, // 0x36
    0xD8, // 0x37
    0x0F, // 0x38
    0xE4, // 0x39
    0x5C, // 0x3A
    0xB7, // 0x3B
    0xA9, // 0x3C
    0x42, // 0x3D
    0xFA, // 0x3E
    0x11, // 0x3F
    0xA3, // 0x40
    0x48, // 0x41
    0xF0, // 0x42
    0x1B, // 0x43
    0x05, // 0x44
    0xEE, // 0x45
    0x56, // 0x46
    0xBD, // 0x47
    0x6A, // 0x48
    0x81, // 0x49
    0x39, // 0x4A
    0xD2, // 0x4B
    0xCC, // 0x4C
    0x27, // 0x4D
    0x9F, // 0x4E
    0x74, // 0x4F
    0xB4, // 0x50
    0x5F, // 0x51
    0xE7, // 0x52
    0x0C, // 0x53
    0x12, // 0x54
    0xF9, // 0x55
    0x41, // 0x56
    0xAA, // 0x57
    0x7D, // 0x58
    0x96, // 0x59
    0x2E, // 0x5A
    0xC5, // 0x5B
    0xDB, // 0x5C
    0x30, // 0x5D
    0x88, // 0x5E
    0x63, // 0x5F
    0x8D, // 0x60
    0x66, // 0x61
    0xDE, // 0x62
    0x35, // 0x63
    0x2B, // 0x64
    0xC0, // 0x65
    0x78, // 0x66
    0x93, // 0x67
    0x44, // 0x68
    0xAF, // 0x69
    0x17, // 0x6A
    0xFC, // 0x6B
    0xE2, // 0x6C
    0x09, // 0x6D
    0xB1, // 0x6E
    0x5A, // 0x6F
    0x9A, // 0x70
    0x71, // 0x71
    0xC9, // 0x72
    0x22, // 0x73
    0x3C, // 0x74
    0xD7, // 0x75
    0x6F, // 0x76
    0x84, // 0x77
    0x53, // 0x78
    0xB8, // 0x79
    0x00, // 0x7A
    0xEB, // 0x7B
    0xF5, // 0x7C
    0x1E, // 0x7D
    0xA6, // 0x7E
    0x4D, // 0x7F
    0x47, // 0x80
    0xAC, // 0x81
    0x14, // 0x82
    0xFF, // 0x83
    0xE1, // 0x84
    0x0A, // 0x85
    0xB2, // 0x86
    0x59, // 0x87
    0x8E, // 0x88
    0x65, // 0x89
    0xDD, // 0x8A
    0x36, // 0x8B
    0x28, // 0x8C
    0xC3, // 0x8D
    0x7B, // 0x8E
    0x90, // 0x8F
    0x50, // 0x90
    0xBB, // 0x91
    0x03, // 0x92
    0xE8, // 0x93
    0xF6, // 0x94
    0x1D, // 0x95
    0xA5, // 0x96
    0x4E, // 0x97
    0x99, // 0x98
    0x72, // 0x99
    0xCA, // 0x9A
    0x21, // 0x9B
    0x3F, // 0x9C
    0xD4, // 0x9D
    0x6C, // 0x9E
    0x87, // 0x9F
    0x69, // 0xA0
    0x82, // 0xA1
    0x3A, // 0xA2
    0xD1, // 0xA3
    0xCF, // 0xA4
    0x24, // 0xA5
    0x9C, // 0xA6
    0x77, // 0xA7
    0xA0, // 0xA8
    0x4B, // 0xA9
    0xF3, // 0xAA
    0x18, // 0xAB
    0x06, // 0xAC
    0xED, // 0xAD
    0x55, // 0xAE
    0xBE, // 0xAF
    0x7E, // 0xB0
    0x95, // 0xB1
    0x2D, // 0xB2
    0xC6, // 0xB3
    0xD8, // 0xB4
    0x33, // 0xB5
    0x8B, // 0xB6
    0x60, // 0xB7
    0xB7, // 0xB8
    0x5C, // 0xB9
    0xE4, // 0xBA
    0x0F, // 0xBB
    0x11, // 0xBC
    0xFA, // 0xBD
    0x42, // 0xBE
    0xA9, // 0xBF
    0x1B, // 0xC0
    0xF0, // 0xC1
    0x48, // 0xC2
    0xA3, // 0xC3
    0xBD, // 0xC4
    0x56, // 0xC5
    0xEE, // 0xC6
    0x05, // 0xC7
    0xD2, // 0xC8
    0x39, // 0xC9
    0x81, // 0xCA
    0x6A, // 0xCB
    0x74, // 0xCC
    0x9F, // 0xCD
    0x27, // 0xCE
    0xCC, // 0xCF
    0x0C, // 0xD0
    0xE7, // 0xD1
    0x5F, // 0xD2
    0xB4, // 0xD3
    0xAA, // 0xD4
    0x41, // 0xD5
    0xF9, // 0xD6
    0x12, // 0xD7
    0xC5, // 0xD8
    0x2E, // 0xD9
    0x96, // 0xDA
    0x7D, // 0xDB
    0x63, // 0xDC
    0x88, // 0xDD
    0x30, // 0xDE
    0xDB, // 0xDF
    0x35, // 0xE0
    0xDE, // 0xE1
    0x66, // 0xE2
    0x8D, // 0xE3
    0x93, // 0xE4
    0x78, // 0xE5
    0xC0, // 0xE6
    0x2B, // 0xE7
    0xFC, // 0xE8
    0x17, // 0xE9
    0xAF, // 0xEA
    0x44, // 0xEB
    0x5A, // 0xEC
    0xB1, // 0xED
    0x09, // 0xEE
    0xE2, // 0xEF
    0x22, // 0xF0
    0xC9, // 0xF1
    0x71, // 0xF2
    0x9A, // 0xF3
    0x84, // 0xF4
    0x6F, // 0xF5
    0xD7, // 0xF6
    0x3C, // 0xF7
    0xEB, // 0xF8
    0x00, // 0xF9
    0xB8, // 0xFA
    0x53, // 0xFB
    0x4D, // 0xFC
    0xA6, // 0xFD
    0x1E, // 0xFE
    0xF5  // 0xFF
};

// Output this directly to the RMT address of our choosing
void n64_buffer_crc()
{
    uint8_t crc = 0;
    uint8_t byte_data = 0;

    /*
    // Process the first data byte
    byte_data = ((N64_CHANNEL_MEM[24].val & JB_BIT_MASK) ? 0x80 : 0x00) |
                ((N64_CHANNEL_MEM[25].val & JB_BIT_MASK) ? 0x40 : 0x00) |
                ((N64_CHANNEL_MEM[26].val & JB_BIT_MASK) ? 0x20 : 0x00) |
                ((N64_CHANNEL_MEM[27].val & JB_BIT_MASK) ? 0x10 : 0x00) |
                ((N64_CHANNEL_MEM[28].val & JB_BIT_MASK) ? 0x8 : 0x00) |
                ((N64_CHANNEL_MEM[29].val & JB_BIT_MASK) ? 0x4 : 0x00) |
                ((N64_CHANNEL_MEM[30].val & JB_BIT_MASK) ? 0x2 : 0x00) |
                ((N64_CHANNEL_MEM[31].val & JB_BIT_MASK) ? 0x1 : 0x00);
    */

    for (size_t i = 0; i < 8; i++) {
        byte_data <<= 1;
        if (N64_CHANNEL_MEM[24 + i].val & JB_BIT_MASK) {
            byte_data |= 1;
        }
    }

    crc = crc_repeating_table[byte_data] ^ 0xFF;

    N64_CHANNEL_MEM[0].val = (crc & 0x80) ? JB_HIGH.val : JB_LOW.val;
    N64_CHANNEL_MEM[1].val = (crc & 0x40) ? JB_HIGH.val : JB_LOW.val;
    N64_CHANNEL_MEM[2].val = (crc & 0x20) ? JB_HIGH.val : JB_LOW.val;
    N64_CHANNEL_MEM[3].val = (crc & 0x10) ? JB_HIGH.val : JB_LOW.val;
    N64_CHANNEL_MEM[4].val = (crc & 0x8) ? JB_HIGH.val : JB_LOW.val;
    N64_CHANNEL_MEM[5].val = (crc & 0x4) ? JB_HIGH.val : JB_LOW.val;
    N64_CHANNEL_MEM[6].val = (crc & 0x2) ? JB_HIGH.val : JB_LOW.val;
    N64_CHANNEL_MEM[7].val = (crc & 0x1) ? JB_HIGH.val : JB_LOW.val;
    N64_CHANNEL_MEM[8].val = JB_STOP.val;
    N64_CHANNEL_MEM[9].val = 0x00;
}

// Loads N64 PAK response into channel 0 buffer
void n64_buffer_pak(void)
{
    // Set up the canned response to controller pak reading
    rmt_item32_t n64_pak_rmt[N64_PAK_RESPONSE_SIZE] = {0};

    for(uint16_t i = 0; i < N64_PAK_RESPONSE_SIZE; i++)
    {
        n64_pak_rmt[i] = JB_LOW;
    }
    n64_pak_rmt[N64_PAK_RESPONSE_SIZE-2] = JB_STOP;
    n64_pak_rmt[N64_PAK_RESPONSE_SIZE-1] = JB_ZERO;

    // Copy canned responses into RMT memory
    memcpy(N64_CHANNEL_MEM, n64_pak_rmt, sizeof(rmt_item32_t) * N64_PAK_RESPONSE_SIZE);
}

void n64_translate_input(void)
{

    hoja_button_remap_process();
    hoja_analog_cb();

    n64_poll_buffer[N64_BUTTON_START]  = hoja_processed_buttons.button_start  ? JB_HIGH : JB_LOW;
    n64_poll_buffer[N64_BUTTON_CDOWN]  = hoja_processed_buttons.button_left   ? JB_HIGH : JB_LOW;
    n64_poll_buffer[N64_BUTTON_CUP]    = hoja_processed_buttons.button_up     ? JB_HIGH : JB_LOW;
    n64_poll_buffer[N64_BUTTON_B]      = hoja_processed_buttons.button_down   ? JB_HIGH : JB_LOW;
    n64_poll_buffer[N64_BUTTON_A]      = hoja_processed_buttons.button_right  ? JB_HIGH : JB_LOW;

    n64_poll_buffer[N64_BUTTON_Z]     = hoja_processed_buttons.trigger_zl     ? JB_HIGH : JB_LOW;
    n64_poll_buffer[N64_BUTTON_R]     = hoja_processed_buttons.trigger_zr     ? JB_HIGH : JB_LOW;
    n64_poll_buffer[N64_BUTTON_CLEFT] = hoja_processed_buttons.trigger_l      ? JB_HIGH : JB_LOW;
    n64_poll_buffer[N64_BUTTON_CRIGHT]= hoja_processed_buttons.trigger_r      ? JB_HIGH : JB_LOW;
    n64_poll_buffer[N64_BUTTON_DUP]    = hoja_processed_buttons.dpad_up     ? JB_HIGH : JB_LOW;
    n64_poll_buffer[N64_BUTTON_DDOWN]  = hoja_processed_buttons.dpad_down   ? JB_HIGH : JB_LOW;
    n64_poll_buffer[N64_BUTTON_DLEFT]  = hoja_processed_buttons.dpad_left   ? JB_HIGH : JB_LOW;
    n64_poll_buffer[N64_BUTTON_DRIGHT] = hoja_processed_buttons.dpad_right  ? JB_HIGH : JB_LOW;
    hoja_analog_data.ls_x >>= 4;
    hoja_analog_data.ls_y >>= 4;
    hoja_analog_data.rs_x >>= 4;
    hoja_analog_data.rs_y >>= 4;

    // Copy analog data
    // Nice clean code to do it :)
    for(int i = 7; i >= 0; i--)
    {
        n64_poll_buffer[N64_ADC_STICKX + i] = (hoja_analog_data.ls_x & 1) ? JB_HIGH : JB_LOW;
        hoja_analog_data.ls_x >>= 1;

        n64_poll_buffer[N64_ADC_STICKY + i] = (hoja_analog_data.ls_y & 1) ? JB_HIGH : JB_LOW;
        hoja_analog_data.ls_y >>= 1;
    }
}

void gamecube_translate_input(void)
{
    rmt_item32_t gc_poll_tmp[GC_POLL_RESPONSE_SIZE] = {
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X8, JB_RMT_0X0,
        JB_RMT_0X8, JB_RMT_0X0, 
        JB_RMT_0X8, JB_RMT_0X0,
        JB_RMT_0X8, JB_RMT_0X0, 
        JB_RMT_0X8, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_STOP, JB_ZERO
    };

    hoja_button_remap_process();
    hoja_analog_cb();

    gc_poll_tmp[GC_BUTTON_START]  = hoja_processed_buttons.button_start  ? JB_HIGH : JB_LOW;
    gc_poll_tmp[GC_BUTTON_Y]      = hoja_processed_buttons.button_left   ? JB_HIGH : JB_LOW;
    gc_poll_tmp[GC_BUTTON_X]      = hoja_processed_buttons.button_up     ? JB_HIGH : JB_LOW;
    gc_poll_tmp[GC_BUTTON_B]      = hoja_processed_buttons.button_down   ? JB_HIGH : JB_LOW;
    gc_poll_tmp[GC_BUTTON_A]      = hoja_processed_buttons.button_right  ? JB_HIGH : JB_LOW;

    gc_poll_tmp[GC_BUTTON_LB]     = hoja_processed_buttons.trigger_zl     ? JB_HIGH : JB_LOW;
    gc_poll_tmp[GC_BUTTON_RB]     = hoja_processed_buttons.trigger_zr     ? JB_HIGH : JB_LOW;
    gc_poll_tmp[GC_BUTTON_Z]      = (hoja_processed_buttons.trigger_l | hoja_processed_buttons.trigger_r)    ? JB_HIGH : JB_LOW;
    gc_poll_tmp[GC_BUTTON_DUP]    = hoja_processed_buttons.dpad_up     ? JB_HIGH : JB_LOW;
    gc_poll_tmp[GC_BUTTON_DDOWN]  = hoja_processed_buttons.dpad_down   ? JB_HIGH : JB_LOW;
    gc_poll_tmp[GC_BUTTON_DLEFT]  = hoja_processed_buttons.dpad_left   ? JB_HIGH : JB_LOW;
    gc_poll_tmp[GC_BUTTON_DRIGHT] = hoja_processed_buttons.dpad_right  ? JB_HIGH : JB_LOW;
    hoja_analog_data.ls_x >>= 4;
    hoja_analog_data.ls_y >>= 4;
    hoja_analog_data.rs_x >>= 4;
    hoja_analog_data.rs_y >>= 4;

    // Copy analog data
    // Nice clean code to do it :)
    for(int i = 7; i >= 0; i--)
    {
        gc_poll_tmp[GC_ADC_LEFTX + i] = (hoja_analog_data.ls_x & 1) ? JB_HIGH : JB_LOW;
        hoja_analog_data.ls_x >>= 1;

        gc_poll_tmp[GC_ADC_LEFTY + i] = (hoja_analog_data.ls_y & 1) ? JB_HIGH : JB_LOW;
        hoja_analog_data.ls_y >>= 1;

        gc_poll_tmp[GC_ADC_RIGHTX + i] = (hoja_analog_data.rs_x & 1) ? JB_HIGH : JB_LOW;
        hoja_analog_data.rs_x >>= 1;

        gc_poll_tmp[GC_ADC_RIGHTY + i] = (hoja_analog_data.rs_y & 1) ? JB_HIGH : JB_LOW;
        hoja_analog_data.rs_y >>= 1;
    }

    memcpy(GAMECUBE_POLL_MEM, gc_poll_tmp, sizeof(rmt_item32_t) * GC_POLL_RESPONSE_SIZE);
}

void joybus_gamecube_init(void)
{
    // Origin Channel
    GAMECUBE_ORIGIN_DIVCT          = 20;
    GAMECUBE_ORIGIN_MEMSIZE        = 2;
    GAMECUBE_ORIGIN_CONTMODE       = 0;
    GAMECUBE_ORIGIN_CARRIEREN      = 0;
    GAMECUBE_ORIGIN_MEMOWNER       = RMT_MEM_OWNER_TX;
    GAMECUBE_ORIGIN_REFALWAYSON    = 1;
    GAMECUBE_ORIGIN_IDLEOUTEN      = 1;
    GAMECUBE_ORIGIN_IDLEOUTLVL     = RMT_IDLE_LEVEL_HIGH;
    GAMECUBE_ORIGIN_TXENDINTENA    = 1;

    // Poll Channel
    GAMECUBE_POLL_DIVCT          = 20;
    GAMECUBE_POLL_MEMSIZE        = 2;
    GAMECUBE_POLL_CONTMODE       = 0;
    GAMECUBE_POLL_CARRIEREN      = 0;
    GAMECUBE_POLL_MEMOWNER       = RMT_MEM_OWNER_TX;
    GAMECUBE_POLL_REFALWAYSON    = 1;
    GAMECUBE_POLL_IDLEOUTEN      = 1;
    GAMECUBE_POLL_IDLEOUTLVL     = RMT_IDLE_LEVEL_HIGH;
    GAMECUBE_POLL_TXENDINTENA    = 1;

    // Set up probe response
    static rmt_item32_t gc_status_rmt[JB_STATUS_LEN] = {
        JB_RMT_0X0, JB_RMT_0X9,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X3,
        JB_STOP, JB_ZERO
    };

    // Set up the canned response to origin
    static rmt_item32_t gc_origin_rmt[GC_ORIGIN_RESPONSE_SIZE] = {
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X8, JB_RMT_0X0,
        JB_RMT_0X8, JB_RMT_0X0, 
        JB_RMT_0X8, JB_RMT_0X0,
        JB_RMT_0X8, JB_RMT_0X0, 
        JB_RMT_0X8, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_STOP, JB_ZERO
    };

    // Copy canned responses into RMT memory
    memcpy(JB_STATUS_MEM, gc_status_rmt, sizeof(rmt_item32_t) * JB_STATUS_LEN);
    memcpy(GAMECUBE_ORIGIN_MEM, gc_origin_rmt, sizeof(rmt_item32_t) * GC_ORIGIN_RESPONSE_SIZE);

    // Start RX on ch0
    gpio_matrix_in(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_IN0_IDX + JB_RX_CHANNEL, 0);
    JB_RX_MEMOWNER  = 1;
    // Reset write/read pointer for RX
    JB_RX_WRRST = 1;
    JB_RX_RDRST = 1;
    JB_RX_EN = 1;
}

void joybus_n64_init(void)
{
    JB_RX_IDLETHRESH    = 16; // 3.5us idle

    // N64 has to share a lot of
    // memory, so we use the same channel for RX/TX
    N64_CHANNEL_DIVCT          = 20;
    N64_CHANNEL_MEMSIZE        = 8;
    N64_CHANNEL_CONTMODE       = 0;
    N64_CHANNEL_CARRIEREN      = 0;

    // Do not enable memory owner for TX until we're ready to send.
    //N64_CHANNEL_MEMOWNER       = RMT_MEM_OWNER_TX;

    N64_CHANNEL_REFALWAYSON    = 1;
    N64_CHANNEL_IDLEOUTEN      = 1;
    N64_CHANNEL_IDLEOUTLVL     = RMT_IDLE_LEVEL_HIGH;
    N64_CHANNEL_TXENDINTENA    = 1;

    gpio_matrix_out(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_OUT0_IDX + N64_CHANNEL, 0, 0);
    // Start RX on ch0
    gpio_matrix_in(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_IN0_IDX + JB_RX_CHANNEL, 0);
    JB_RX_MEMOWNER  = RMT_MEM_OWNER_RX;
    // Reset write/read pointer for RX
    JB_RX_WRRST = 1;
    JB_RX_RDRST = 1;
    JB_RX_EN = 1;
}


void flip_mode()
{
    joybus_all_deinit(false);
    if (!_n64_enable)
    {
        _n64_enable = true;
        joybus_general_init();
        joybus_n64_init();
    }
    else
    {
        _n64_enable = false;
        joybus_general_init();
        joybus_gamecube_init();
    }
}

// Interrupt function for joybus core
static void joybus_isr(void* arg)
{
    // At the end of a received transaction
    // on channel 0
    if (RMT.int_st.ch0_rx_end && _n64_enable)
    {
        uint8_t cmd_buffer = 0;
        
        // Process the first data byte
        for (size_t i = 0; i < 8; i++) {
            cmd_buffer <<= 1;
            if (JB_RX_MEM[0 + i].val & JB_BIT_MASK) {
                cmd_buffer |= 1;
            }
        }

        // Disable RX
        JB_RX_EN = 0;
        // Reset write pointer for RX
        JB_RX_RDRST = 1;
        // Clear RX bit for ch0
        JB_RX_CLEARISR = 1;
        JB_RX_RDRST = 0;

        if (cmd_buffer == 0x03)
        {
            N64_CHANNEL_MEMOWNER = RMT_MEM_OWNER_TX;
            n64_buffer_crc();
            N64_CHANNEL_TXSTART = 1;
        }
        else if (cmd_buffer == 0x01)
        {
            _joybus_watchdog_timer = 0;
            N64_CHANNEL_MEMOWNER = RMT_MEM_OWNER_TX;
            memcpy(N64_CHANNEL_MEM, n64_poll_buffer, sizeof(rmt_item32_t) * N64_POLL_RESPONSE_SIZE);
            N64_CHANNEL_TXSTART = 1;
            _joybus_status = JOYBUS_STATUS_RUNNING_N64;
            n64_translate_input();
        }
        else if (cmd_buffer == 0x02)
        {
            N64_CHANNEL_MEMOWNER = RMT_MEM_OWNER_TX;
            N64_CHANNEL_TXSTART = 1;
        }
        else
        {
            N64_CHANNEL_MEMOWNER = RMT_MEM_OWNER_TX;
            // Copy into RMT memory
            memcpy(&N64_CHANNEL_MEM[0].val, n64_status_buffer, sizeof(rmt_item32_t) * JB_STATUS_LEN);
            N64_CHANNEL_TXSTART = 1;
        }
    }
    
    else if (RMT.int_st.ch0_rx_end)
    {

        // Process the first data byte
        uint8_t cmd_buffer = ((JB_RX_MEM[0].duration0 < JB_RX_MEM[0].duration1) << 7) |
            ((JB_RX_MEM[1].duration0 < JB_RX_MEM[1].duration1) << 6) |
            ((JB_RX_MEM[2].duration0 < JB_RX_MEM[2].duration1) << 5) |
            ((JB_RX_MEM[3].duration0 < JB_RX_MEM[3].duration1) << 4) |
            ((JB_RX_MEM[4].duration0 < JB_RX_MEM[4].duration1) << 3) |
            ((JB_RX_MEM[5].duration0 < JB_RX_MEM[5].duration1) << 2) |
            ((JB_RX_MEM[6].duration0 < JB_RX_MEM[6].duration1) << 1) |
            ((JB_RX_MEM[7].duration0 < JB_RX_MEM[7].duration1) << 0);

        // Disable RX
        JB_RX_EN = 0;
        // Reset write pointer for RX
        JB_RX_RDRST = 1;
        // Clear RX bit for ch0
        JB_RX_CLEARISR = 1;
        JB_RX_RDRST = 0;

        // Check the command byte and respond accordingly
        switch(cmd_buffer)
        {
            // Probe command (N64 and GameCube use the same thing...)
            default:
            case 0xFF:
            case 0x00:
                gpio_matrix_out(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_OUT0_IDX + JB_STATUS_CHANNEL, 0, 0);
                JB_STATUS_MEMOWNER  = 0;
                JB_STATUS_TXSTART   = 1;
                break;
            case 0x41:
            case 0x42:
                gpio_matrix_out(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_OUT0_IDX + GAMECUBE_CHANNEL_ORIGIN, 0, 0);
                GAMECUBE_ORIGIN_MEMOWNER    = 0;
                GAMECUBE_POLL_MEMOWNER      = 0;
                GAMECUBE_ORIGIN_TXSTART     = 1;
                gamecube_translate_input();
                break;
            case 0x40:
                _joybus_watchdog_timer = 0;
                gpio_matrix_out(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_OUT0_IDX + GAMECUBE_CHANNEL_POLL, 0, 0);
                GAMECUBE_POLL_MEMOWNER  = 0;
                GAMECUBE_POLL_TXSTART   = 1;
                gamecube_translate_input();
                break;
        }

    }
    else if (N64_CHANNEL_TXENDSTAT)
    {
        // Reset TX read pointer bit
        N64_CHANNEL_MEMRST     = 1;
        N64_CHANNEL_MEMRST     = 0;
        // Clear TX end interrupt bit
        N64_CHANNEL_CLEARTXINT  = 1;

        // Start RX on ch0
        gpio_matrix_in(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_IN0_IDX + JB_RX_CHANNEL, 0);
        JB_RX_MEMOWNER  = RMT_MEM_OWNER_RX;
        // Reset write/read pointer for RX
        JB_RX_WRRST = 1;
        JB_RX_RDRST = 1;
        JB_RX_EN    = 1;
    }
    // Probe response transaction end
    else if (JB_STATUS_TXENDSTAT)
    {
        // Reset TX read pointer bit
        JB_STATUS_MEMRST     = 1;
        JB_STATUS_MEMRST     = 0;
        // Clear TX end interrupt bit
        JB_STATUS_CLEARTXINT  = 1;

        // Start RX on ch0
        gpio_matrix_in(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_IN0_IDX + JB_RX_CHANNEL, 0);
        JB_RX_MEMOWNER  = 1;
        // Reset write/read pointer for RX
        JB_RX_WRRST = 1;
        JB_RX_RDRST = 1;
        JB_RX_EN        = 1;
    }
    // GC ORIGIN TX STATUS ISR
    // SAME FOR N64 POLL
    else if (GAMECUBE_ORIGIN_TXENDSTAT)
    {
        // Reset TX read pointer bit
        GAMECUBE_ORIGIN_MEMRST     = 1;
        GAMECUBE_ORIGIN_MEMRST     = 0;
        // Clear TX end interrupt bit
        GAMECUBE_ORIGIN_CLEARTXINT  = 1;

        // Start RX on ch0
        gpio_matrix_in(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_IN0_IDX + JB_RX_CHANNEL, 0);
        JB_RX_MEMOWNER  = 1;
        // Reset write/read pointer for RX
        JB_RX_WRRST = 1;
        JB_RX_RDRST = 1;
        JB_RX_EN        = 1;
    }
    else if (GAMECUBE_POLL_TXENDSTAT)
    {
        GAMECUBE_POLL_MEMRST = 1;
        GAMECUBE_POLL_MEMRST = 0;
        GAMECUBE_POLL_CLEARTXINT = 1;

        // Start RX on ch0
        gpio_matrix_in(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_IN0_IDX + JB_RX_CHANNEL, 0);
        JB_RX_MEMOWNER  = 1;
        // Reset write/read pointer for RX
        JB_RX_WRRST = 1;
        JB_RX_RDRST = 1;
        JB_RX_EN        = 1;
        _joybus_mode = JOYBUS_MODE_GAMECUBE;
    }
}

void joybus_general_init(void)
{
    // Set up RMT peripheral first
    periph_ll_enable_clk_clear_rst(PERIPH_RMT_MODULE);

    JB_RMT_FIFO = RMT_DATA_MODE_MEM;

    // set up RMT peripherial register stuff
    // for receive channel
    JB_RX_CLKDIV        = 10; // 0.125us increments.
    JB_RX_IDLETHRESH    = 35; // 4us idle
    JB_RX_MEMSIZE       = 1;
    JB_RX_FILTEREN      = 1;
    JB_RX_FILTERTHRESH  = 1;
    JB_RX_MEMOWNER      = 1;
    JB_RX_RDRST         = 1;
    JB_RX_COMPLETEISR   = 1;

    // For ESP32 S3
    #if CONFIG_IDF_TARGET_ESP32S3
    #elif CONFIG_IDF_TARGET_ESP32
    RMT.conf_ch[JB_RX_CHANNEL].conf1.ref_always_on = 1;
    #endif
    
    // Status Channel
    JB_STATUS_DIVCT          = 20;
    JB_STATUS_MEMSIZE        = 1;
    JB_STATUS_CONTMODE       = 0;
    JB_STATUS_CARRIEREN      = 0;
    JB_STATUS_MEMOWNER       = RMT_MEM_OWNER_TX;
    JB_STATUS_REFALWAYSON    = 1;
    JB_STATUS_IDLEOUTEN      = 1;
    JB_STATUS_IDLEOUTLVL     = RMT_IDLE_LEVEL_HIGH;
    JB_STATUS_TXENDINTENA    = 1;

    // Select pin function for I/O joybus line
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[CONFIG_HOJA_GPIO_NS_SERIAL], PIN_FUNC_GPIO);
    // Set input direction to input AND output simultaneously (open drain)
    gpio_set_direction(CONFIG_HOJA_GPIO_NS_SERIAL, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_matrix_in(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_IN0_IDX + JB_RX_CHANNEL, 0);
    // Set output/input GPIO to peripheral
    gpio_matrix_out(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_OUT0_IDX + JB_STATUS_CHANNEL, 0, 0);

    if (_rmt_isr_handle == NULL)
    {
        rmt_isr_register(joybus_isr, NULL, 3, &_rmt_isr_handle);
    }
}

static portMUX_TYPE periph_spinlock = portMUX_INITIALIZER_UNLOCKED;

void joybus_all_deinit(bool reset)
{
    JB_RX_EN = 0;
    JB_STATUS_TXSTART = 0;
    N64_CHANNEL_TXSTART = 0;
    GAMECUBE_POLL_TXSTART = 0;
    GAMECUBE_ORIGIN_TXSTART = 0;
    _joybus_mode = JOYBUS_MODE_IDLE;
    
    if (_rmt_isr_handle != NULL)
    {
        rmt_isr_deregister(_rmt_isr_handle);
        _rmt_isr_handle = NULL;
    }

    if (reset)
    {
        portENTER_CRITICAL_SAFE(&periph_spinlock);
        periph_ll_disable_clk_set_rst(PERIPH_RMT_MODULE);
        portEXIT_CRITICAL_SAFE(&periph_spinlock);
    }  
}

void joybus_watchdog(void * param)
{
    joybus_mode_t current_mode = JOYBUS_MODE_IDLE;
    for(;;)
    {
        _joybus_watchdog_timer += 1;
        vTaskDelay(30/portTICK_PERIOD_MS);

        if (_joybus_watchdog_timer >= 35)
        {
            hoja_event_cb(HOJA_EVT_WIRED, HEVT_WIRED_DISCONNECT, 0);
            vTaskDelete(_joybus_task_handle);
        }
    }
}

void joybus_n64_coldboot_task(void * param)
{
    const char* TAG = "joybus_n64_coldboot_task";
    vTaskDelay(1500/portTICK_PERIOD_MS);
    if (_joybus_status == JOYBUS_STATUS_RUNNING_N64)
    {
        ESP_LOGI(TAG, "N64 was detected.");
        hoja_current_core = HOJA_CORE_N64;
        hoja_event_cb(HOJA_EVT_WIRED, HEVT_WIRED_N64_DETECT, 0);
    }
    else
    {
        ESP_LOGI(TAG, "N64 was not detected.");
        hoja_current_core = HOJA_CORE_NULL;
        hoja_current_status = HOJA_STATUS_INITIALIZED;
        hoja_event_cb(HOJA_EVT_WIRED, HEVT_WIRED_N64_DECONFIRMED, 0);
    }

    vTaskDelete(_joybus_task_handle);
    _joybus_task_handle = NULL;
}

//-----------------//
//-----------------//

// PUBLIC FUNCTIONS

// This function returns whether or not N64 console
// is detected. This is because the N64 needs the controller
// to be available on the console power. We start
// right away to ensure N64 core is running on boot. We can shut down if
// nothing is detected as N64.
hoja_err_t core_joybus_n64_coldboot(void)
{
    _joybus_mode = JOYBUS_MODE_N64;
    
    joybus_general_init();
    joybus_n64_init();

    if (_joybus_task_handle == NULL)
    {
        xTaskCreatePinnedToCore(joybus_n64_coldboot_task, "Joybus N64 Boot", 2048, NULL, 4, &_joybus_task_handle, HOJA_INPUT_CPU);
    }
    else
    {
        return HOJA_FAIL;
    }
    return HOJA_OK;
}

// Initializes Joybus Utility
hoja_err_t core_joybus_gamecube_start(void)
{
    const char* TAG = "util_joybus_init";

    joybus_all_deinit(false);
    joybus_general_init();
    joybus_gamecube_init();

    xTaskCreatePinnedToCore(joybus_watchdog, "Joybus Watchdog", 2048, NULL, 4, &_joybus_task_handle, HOJA_INPUT_CPU);

    return HOJA_OK;
}

void core_joybus_stop(void)
{
    const char* TAG = "core_joybus_stop";
    joybus_all_deinit(true);

    if(_joybus_task_handle != NULL)
    {
        vTaskDelete(_joybus_task_handle);
        _joybus_task_handle = NULL;
    }

    if (_rmt_isr_handle != NULL)
    {
        rmt_isr_deregister(_rmt_isr_handle);
        _rmt_isr_handle = NULL;
    }
}

