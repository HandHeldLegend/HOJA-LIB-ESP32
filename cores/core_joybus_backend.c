#include "core_joybus_backend.h"

joybus_status_t     _joybus_status = JOYBUS_STATUS_IDLE;
volatile joybus_mode_t       _joybus_mode = JOYBUS_MODE_IDLE;

rmt_isr_handle_t    _rmt_isr_handle = NULL;
TaskHandle_t        _joybus_task_handle;
volatile uint8_t    _status_attempts = 0;
#define STATUS_FLIP 20
bool _n64_enable = true;
volatile uint8_t _joybus_watchdog_timer = 0;

rmt_item32_t n64_poll_buffer[N64_POLL_RESPONSE_SIZE] = {
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X8, JB_RMT_0X0, 
        JB_RMT_0X8, JB_RMT_0X0,
        JB_STOP, JB_ZERO
    };

// Set up N64
// Canned response to status
rmt_item32_t n64_status_buffer[JB_STATUS_LEN] = {
    JB_RMT_0X0, JB_RMT_0X2,
    JB_RMT_0X0, JB_RMT_0X0,
    JB_RMT_0X0, JB_RMT_0X0,
    JB_STOP, JB_ZERO
};

// CRC table provided from https://github.com/darthcloud/BlueRetro/blob/master/main/wired/nsi.c
// licensed under the Apache License 2.0: https://github.com/darthcloud/BlueRetro/blob/master/LICENSE
static const uint8_t nsi_crc_table[256] = {
    0x8F, 0x85, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0xC2, 0x61, 0xF2, 0x79, 0xFE, 0x7F,
    0xFD, 0xBC, 0x5E, 0x2F, 0xD5, 0xA8, 0x54, 0x2A, 0x15, 0xC8, 0x64, 0x32, 0x19, 0xCE, 0x67, 0xF1,
    0xBA, 0x5D, 0xEC, 0x76, 0x3B, 0xDF, 0xAD, 0x94, 0x4A, 0x25, 0xD0, 0x68, 0x34, 0x1A, 0x0D, 0xC4,
    0x62, 0x31, 0xDA, 0x6D, 0xF4, 0x7A, 0x3D, 0xDC, 0x6E, 0x37, 0xD9, 0xAE, 0x57, 0xE9, 0xB6, 0x5B,
    0xEF, 0xB5, 0x98, 0x4C, 0x26, 0x13, 0xCB, 0xA7, 0x91, 0x8A, 0x45, 0xE0, 0x70, 0x38, 0x1C, 0x0E,
    0x07, 0xC1, 0xA2, 0x51, 0xEA, 0x75, 0xF8, 0x7C, 0x3E, 0x1F, 0xCD, 0xA4, 0x52, 0x29, 0xD6, 0x6B,
    0xF7, 0xB9, 0x9E, 0x4F, 0xE5, 0xB0, 0x58, 0x2C, 0x16, 0x0B, 0xC7, 0xA1, 0x92, 0x49, 0xE6, 0x73,
    0xFB, 0xBF, 0x9D, 0x8C, 0x46, 0x23, 0xD3, 0xAB, 0x97, 0x89, 0x86, 0x43, 0xE3, 0xB3, 0x9B, 0x8F,
    0x85, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0xC2, 0x61, 0xF2, 0x79, 0xFE, 0x7F, 0xFD,
    0xBC, 0x5E, 0x2F, 0xD5, 0xA8, 0x54, 0x2A, 0x15, 0xC8, 0x64, 0x32, 0x19, 0xCE, 0x67, 0xF1, 0xBA,
    0x5D, 0xEC, 0x76, 0x3B, 0xDF, 0xAD, 0x94, 0x4A, 0x25, 0xD0, 0x68, 0x34, 0x1A, 0x0D, 0xC4, 0x62,
    0x31, 0xDA, 0x6D, 0xF4, 0x7A, 0x3D, 0xDC, 0x6E, 0x37, 0xD9, 0xAE, 0x57, 0xE9, 0xB6, 0x5B, 0xEF,
    0xB5, 0x98, 0x4C, 0x26, 0x13, 0xCB, 0xA7, 0x91, 0x8A, 0x45, 0xE0, 0x70, 0x38, 0x1C, 0x0E, 0x07,
    0xC1, 0xA2, 0x51, 0xEA, 0x75, 0xF8, 0x7C, 0x3E, 0x1F, 0xCD, 0xA4, 0x52, 0x29, 0xD6, 0x6B, 0xF7,
    0xB9, 0x9E, 0x4F, 0xE5, 0xB0, 0x58, 0x2C, 0x16, 0x0B, 0xC7, 0xA1, 0x92, 0x49, 0xE6, 0x73, 0xFB,
    0xBF, 0x9D, 0x8C, 0x46, 0x23, 0xD3, 0xAB, 0x97, 0x89, 0x86, 0x43, 0xE3, 0xB3, 0x9B, 0x8F, 0x85,
};

// Output this directly to the RMT address of our choosing
void n64_buffer_crc(uint32_t *input, uint32_t *output)
{
    uint8_t bit_len = 32*8;
    uint8_t crc = 0xFF;
    // Make a pointer to our CRC table
    const uint8_t *crc_table = nsi_crc_table;
    // Set pointer at address
    volatile uint32_t *item_ptr = input;

    // Loop, incrementing the bit we are processing
    for (uint8_t i = 0; i < bit_len; ++i) {

        for (uint8_t l = 0; l<8; l++)
        {
            rmt_item32_t item = {
                .val = *item_ptr,
            };
            if (item.duration0 < item.duration1) {
                // If the bit is ONE
                // we XOR EQUALS the crc with the iteration
                // in the CRC table
                crc ^= *crc_table;
            }
            
            // Increment the pointers
            ++crc_table;
            ++item_ptr;
        }
    }

    // Set our item pointer address to the output
    item_ptr = output + 7;
    // At this point our CRC is calculated
    // Write it to the correct memory address
    for(int i = 7; i >= 0; i--)
    {
        *item_ptr = (crc & 1) ? JB_HIGH.val : JB_LOW.val;
        crc >>= 1;
        item_ptr--;
    }
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
    rmt_item32_t gc_status_rmt[JB_STATUS_LEN] = {
        JB_RMT_0X0, JB_RMT_0X9,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X3,
        JB_STOP, JB_ZERO
    };

    // Set up the canned response to origin
    rmt_item32_t gc_origin_rmt[GC_ORIGIN_RESPONSE_SIZE] = {
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
    _n64_enable = !_n64_enable;

    if (_n64_enable)
    {
        joybus_general_init();
        joybus_n64_init();
    }
    else
    {
        joybus_general_init();
        joybus_gamecube_init();
    }
}

// Interrupt function for joybus core
static void joybus_isr(void* arg)
{
    // At the end of a received transaction
    // on channel 0
    if (RMT.int_st.ch0_rx_end)
    {
        // Set up command byte placeholder
        joybus_rx_buffer_s cmd_buffer = {0};

        for (uint8_t i = 0; i < 8; i++)
        {   
            // If duration 0 is less than duration 1, it's a HIGH bit. Otherwise low bit.
            // Shift the bit so it's always in the leftmost bit position.
            cmd_buffer.val |= ( (JB_RX_MEM[i].duration0 < JB_RX_MEM[i].duration1) ? 1 : 0) << (7-i);
        }

        // Disable RX
        JB_RX_EN = 0;
        // Reset write pointer for RX
        JB_RX_RDRST = 1;
        // Clear RX bit for ch0
        JB_RX_CLEARISR = 1;
        JB_RX_RDRST = 0;

        // Check the command byte and respond accordingly
        switch(cmd_buffer.byte0)
        {
            // Probe command (N64 and GameCube use the same thing...)
            default:
            case 0xFF:
            case 0x00:
                if (_n64_enable)
                {
                    N64_CHANNEL_MEMOWNER = RMT_MEM_OWNER_TX;
                    // Copy into RMT memory
                    memcpy(N64_CHANNEL_MEM, n64_status_buffer, sizeof(rmt_item32_t) * JB_STATUS_LEN);
                    gpio_matrix_out(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_OUT0_IDX + N64_CHANNEL, 0, 0);
                    N64_CHANNEL_TXSTART = 1;
                }
                else 
                {
                    gpio_matrix_out(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_OUT0_IDX + JB_STATUS_CHANNEL, 0, 0);
                    JB_STATUS_MEMOWNER  = 0;
                    JB_STATUS_TXSTART   = 1;
                }
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

            case 0x01:
                _joybus_watchdog_timer = 0;
                gpio_matrix_out(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_OUT0_IDX + N64_CHANNEL, 0, 0);
                N64_CHANNEL_MEMOWNER = RMT_MEM_OWNER_TX;
                memcpy(N64_CHANNEL_MEM, n64_poll_buffer, sizeof(rmt_item32_t) * N64_POLL_RESPONSE_SIZE);
                N64_CHANNEL_TXSTART = 1;
                _joybus_mode = JOYBUS_MODE_N64;
                n64_translate_input();
                break;
            case 0x02:
                gpio_matrix_out(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_OUT0_IDX + N64_CHANNEL, 0, 0);
                N64_CHANNEL_MEMOWNER = RMT_MEM_OWNER_TX;
                N64_CHANNEL_TXSTART = 1;
                break;
            case 0x03:
                N64_CHANNEL_MEMOWNER = RMT_MEM_OWNER_TX;
                n64_buffer_crc(&N64_CHANNEL_MEM[2].val, &N64_CHANNEL_MEM[0].val);
                N64_CHANNEL_MEM[8].val = JB_STOP.val;
                N64_CHANNEL_MEM[9].val = JB_ZERO.val;
                gpio_matrix_out(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_OUT0_IDX + N64_CHANNEL, 0, 0);
                N64_CHANNEL_TXSTART = 1;
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
        JB_RX_EN        = 1;
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

void joybus_all_deinit(void)
{
    // Disable RX
    JB_RX_MEMOWNER  = 1;
    // Reset write/read pointer for RX
    JB_RX_WRRST = 1;
    JB_RX_RDRST = 1;
    JB_RX_EN = 0;
    // Clear RMT peripheral
    periph_ll_enable_clk_clear_rst(PERIPH_RMT_MODULE);
    if (_rmt_isr_handle != NULL)
    {
        rmt_isr_deregister(_rmt_isr_handle);
    }
}

void joybus_watchdog(void * param)
{
    joybus_mode_t current_mode = JOYBUS_MODE_IDLE;
    for(;;)
    {
        _joybus_watchdog_timer += 1;
        vTaskDelay(30/portTICK_PERIOD_MS);
        if (_joybus_mode != current_mode)
        {
            _joybus_watchdog_timer = 0;
            if (_joybus_mode == JOYBUS_MODE_GAMECUBE)
            {
                hoja_event_cb(HOJA_EVT_WIRED, HEVT_WIRED_GAMECUBE_DETECT, 0);
            }
            else if (_joybus_mode == JOYBUS_MODE_N64)
            {
                hoja_event_cb(HOJA_EVT_WIRED, HEVT_WIRED_N64_DETECT, 0);
            }
            current_mode = _joybus_mode;
        }

        if (current_mode == JOYBUS_MODE_IDLE)
        {
            if (_joybus_watchdog_timer >= 35)
            {
                _joybus_watchdog_timer = 0;
                flip_mode();
            }
        }
        else
        {
            if (_joybus_watchdog_timer >= 100)
            {
                _joybus_watchdog_timer = 0;
                hoja_event_cb(HOJA_EVT_WIRED, HEVT_WIRED_DISCONNECT, 0);
            }
        }
        
    }
}

//-----------------//
//-----------------//

// PUBLIC FUNCTIONS

// Initializes Joybus Utility
hoja_err_t core_joybus_start(void)
{
    const char* TAG = "util_joybus_init";

    joybus_general_init();
    joybus_n64_init();

    //xTaskCreatePinnedToCore(joybus_watchdog, "Joybus Watchdog", 2048, NULL, 2, &_joybus_task_handle, HOJA_CORE_CPU);

    return HOJA_OK;
}

void core_joybus_stop(void)
{
    const char* TAG = "core_gamecube_stop";
    vTaskDelete(_joybus_task_handle);
    _joybus_task_handle = NULL;
    periph_ll_enable_clk_clear_rst(PERIPH_RMT_MODULE);

    rmt_isr_deregister(_rmt_isr_handle);
}

