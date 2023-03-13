#include "core_gamecube_backend.h"

#define GC_POLL_RMT_LEN    66
#define GC_PROBE_RMT_LEN   26
#define GC_ORIGIN_RMT_LEN  82

rmt_isr_handle_t gc_rmt_isr_handle;

void gamecube_input_translate(void)
{
    rmt_item32_t gcmd_poll_rmt[GC_POLL_RMT_LEN] = {
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

    gcmd_poll_rmt[GC_BUTTON_START]  = hoja_processed_buttons.button_start  ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_Y]      = hoja_processed_buttons.button_left   ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_X]      = hoja_processed_buttons.button_up     ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_B]      = hoja_processed_buttons.button_down   ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_A]      = hoja_processed_buttons.button_right  ? JB_HIGH : JB_LOW;

    gcmd_poll_rmt[GC_BUTTON_LB]     = hoja_processed_buttons.trigger_zl     ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_RB]     = hoja_processed_buttons.trigger_zr     ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_Z]      = (hoja_processed_buttons.trigger_l | hoja_processed_buttons.trigger_r)    ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_DUP]    = hoja_processed_buttons.dpad_up     ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_DDOWN]  = hoja_processed_buttons.dpad_down   ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_DLEFT]  = hoja_processed_buttons.dpad_left   ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_DRIGHT] = hoja_processed_buttons.dpad_right  ? JB_HIGH : JB_LOW;
    hoja_analog_data.ls_x >>= 4;
    hoja_analog_data.ls_y >>= 4;
    hoja_analog_data.rs_x >>= 4;
    hoja_analog_data.rs_y >>= 4;

    // Copy analog data
    // Nice clean code to do it :)
    for(int i = 7; i >= 0; i--)
    {
        gcmd_poll_rmt[GC_ADC_LEFTX + i] = (hoja_analog_data.ls_x & 1) ? JB_HIGH : JB_LOW;
        hoja_analog_data.ls_x >>= 1;

        gcmd_poll_rmt[GC_ADC_LEFTY + i] = (hoja_analog_data.ls_y & 1) ? JB_HIGH : JB_LOW;
        hoja_analog_data.ls_y >>= 1;

        gcmd_poll_rmt[GC_ADC_RIGHTX + i] = (hoja_analog_data.rs_x & 1) ? JB_HIGH : JB_LOW;
        hoja_analog_data.rs_x >>= 1;

        gcmd_poll_rmt[GC_ADC_RIGHTY + i] = (hoja_analog_data.rs_y & 1) ? JB_HIGH : JB_LOW;
        hoja_analog_data.rs_y >>= 1;
    }

    memcpy(JB_POLL_MEM, gcmd_poll_rmt, sizeof(rmt_item32_t) * GC_POLL_RMT_LEN);

    //hoja_button_reset();
}

volatile uint32_t raw = 0x00;
volatile uint32_t command = 0xFF;
volatile uint32_t tmpgot = 0x00;

static void gamecube_rmt_isr(void* arg) 
{
    // At the end of a received transaction
    // on channel 0
    if (RMT.int_st.ch0_rx_end)
    {
        // Set up command byte placeholder
        joybus_rx_buffer_s cmd_buffer = {0};
        cmd_buffer.val = 0x00;

        for (uint8_t i = 0; i < 8; i++)
        {   
            // If duration 0 is less than duration 1, it's a HIGH bit. Otherwise low bit.
            // Shift the bit so it's always in the leftmost bit position.
            cmd_buffer.val |= ( (JB_RX_MEM[i].duration0 < JB_RX_MEM[i].duration1) ? 1 : 0) << (7-i);
        }

        command = cmd_buffer.val;

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
            // Probe Command
            case 0x00:
            default:
                gpio_matrix_out(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_OUT0_IDX + JB_TX_CHANNEL_PROBE, 0, 0);
                JB_PROBE_MEMOWNER = 0;
                JB_PROBE_TXSTART = 1;
                break;

            // Origin Command
            case 0x41:
            case 0x42:
                gpio_matrix_out(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_OUT0_IDX + JB_TX_CHANNEL_ORIGIN, 0, 0);
                JB_ORIGIN_MEMOWNER = 0;
                JB_ORIGIN_TXSTART = 1;
                gamecube_input_translate();
                break;

            // Poll Command
            case 0x40:
                gpio_matrix_out(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_OUT0_IDX + JB_TX_CHANNEL_POLL, 0, 0);
                JB_POLL_MEMOWNER = 0;
                JB_POLL_TXSTART = 1;
                gamecube_input_translate();
                break;
        }
        
        
    }
    // Probe response transaction end
    else if (JB_PROBE_TXENDSTAT)
    {
        // Reset TX read pointer bit
        JB_PROBE_MEMRST     = 1;
        JB_PROBE_MEMRST     = 0;
        // Clear TX end interrupt bit
        JB_PROBE_CLEARTXINT  = 1;

        // Start RX on ch0
        gpio_matrix_in(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_IN0_IDX + JB_RX_CHANNEL, 0);
        JB_RX_MEMOWNER  = 1;
        // Reset write/read pointer for RX
        JB_RX_WRRST = 1;
        JB_RX_RDRST = 1;
        JB_RX_EN        = 1;
    }
    // Origin response transaction end
    else if (JB_ORIGIN_TXENDSTAT)
    {
        // Reset TX read pointer bit
        JB_ORIGIN_MEMRST     = 1;
        JB_ORIGIN_MEMRST     = 0;
        // Clear TX end interrupt bit
        JB_ORIGIN_CLEARTXINT  = 1;

        // Start RX on ch0
        gpio_matrix_in(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_IN0_IDX + JB_RX_CHANNEL, 0);
        JB_RX_MEMOWNER  = 1;
        // Reset write/read pointer for RX
        JB_RX_WRRST = 1;
        JB_RX_RDRST = 1;
        JB_RX_EN        = 1;
    }
    // Poll response transaction end
    else if (JB_POLL_TXENDSTAT)
    {
        // Reset TX read pointer bit
        JB_POLL_MEMRST     = 1;
        JB_POLL_MEMRST     = 0;
        // Clear TX end interrupt bit
        JB_POLL_CLEARTXINT  = 1;

        // Start RX on ch0
        gpio_matrix_in(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_IN0_IDX + JB_RX_CHANNEL, 0);
        JB_RX_MEMOWNER  = 1;
        // Reset write/read pointer for RX
        JB_RX_WRRST = 1;
        JB_RX_RDRST = 1;
        JB_RX_EN        = 1;
    }
    
}

void gamecube_init(void)
{
    const char* TAG = "gamecube_init";

    periph_ll_enable_clk_clear_rst(PERIPH_RMT_MODULE);

    JB_RMT_FIFO = RMT_DATA_MODE_MEM;

    // set up RMT peripherial register stuff
    // for receive channel
    JB_RX_CLKDIV        = 10; // 0.125us increments.
    JB_RX_IDLETHRESH    = 35; // 4us idle
    JB_RX_MEMSIZE       = 1;
    JB_RX_FILTEREN      = 0;
    JB_RX_MEMOWNER      = 1;
    JB_RX_RDRST         = 1;
    JB_RX_COMPLETEISR   = 1;

    // For ESP32 S3
    #if CONFIG_IDF_TARGET_ESP32S3
    #elif CONFIG_IDF_TARGET_ESP32
    RMT.conf_ch[JB_RX_CHANNEL].conf1.ref_always_on = 1;
    #endif

    // set up RMT peripheral register stuff
    // for transaction channels :)
    // Probe Channel
    JB_PROBE_DIVCT          = 20;
    JB_PROBE_MEMSIZE        = 1;
    JB_PROBE_CONTMODE       = 0;
    JB_PROBE_CARRIEREN      = 0;
    JB_PROBE_MEMOWNER       = RMT_MEM_OWNER_TX;
    JB_PROBE_REFALWAYSON    = 1;
    JB_PROBE_IDLEOUTEN      = 1;
    JB_PROBE_IDLEOUTLVL     = RMT_IDLE_LEVEL_HIGH;
    JB_PROBE_TXENDINTENA    = 1;


    // Origin Channel
    JB_ORIGIN_DIVCT          = 20;
    JB_ORIGIN_MEMSIZE        = 2;
    JB_ORIGIN_CONTMODE       = 0;
    JB_ORIGIN_CARRIEREN      = 0;
    JB_ORIGIN_MEMOWNER       = RMT_MEM_OWNER_TX;
    JB_ORIGIN_REFALWAYSON    = 1;
    JB_ORIGIN_IDLEOUTEN      = 1;
    JB_ORIGIN_IDLEOUTLVL     = RMT_IDLE_LEVEL_HIGH;
    JB_ORIGIN_TXENDINTENA    = 1;

    // Poll Channel
    JB_POLL_DIVCT          = 20;
    JB_POLL_MEMSIZE        = 2;
    JB_POLL_CONTMODE       = 0;
    JB_POLL_CARRIEREN      = 0;
    JB_POLL_MEMOWNER       = RMT_MEM_OWNER_TX;
    JB_POLL_REFALWAYSON    = 1;
    JB_POLL_IDLEOUTEN      = 1;
    JB_POLL_IDLEOUTLVL     = RMT_IDLE_LEVEL_HIGH;
    JB_POLL_TXENDINTENA    = 1;
    
    rmt_item32_t gcmd_probe_rmt[GC_PROBE_RMT_LEN] = {
        JB_RMT_0X0, JB_RMT_0X9,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X3,
        JB_STOP, JB_ZERO
    };

    // Set up the canned response to origin
    rmt_item32_t gcmd_origin_rmt[GC_ORIGIN_RMT_LEN] = {
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

    memcpy(JB_PROBE_MEM, gcmd_probe_rmt, sizeof(rmt_item32_t) * GC_PROBE_RMT_LEN);
    memcpy(JB_ORIGIN_MEM, gcmd_origin_rmt, sizeof(rmt_item32_t) * GC_ORIGIN_RMT_LEN);

    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[CONFIG_HOJA_GPIO_NS_SERIAL], PIN_FUNC_GPIO);
    gpio_set_direction(CONFIG_HOJA_GPIO_NS_SERIAL, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_matrix_out(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_OUT0_IDX + RMT_TX_CHANNEL_PROBE, 0, 0);
    gpio_matrix_in(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_IN0_IDX + RMT_RX_CHANNEL, 0);

    rmt_isr_register(gamecube_rmt_isr, NULL, 3, &gc_rmt_isr_handle);
}

hoja_err_t core_gamecube_start()
{  
    const char* TAG = "core_gamecube_start";
    esp_err_t er = ESP_OK;
    ESP_LOGI(TAG, "GameCube Core Started.");
    
    gamecube_init();

    vTaskDelay(200/portTICK_PERIOD_MS);
    
    JB_RX_EN = 1;

    return HOJA_OK;
}

void core_gamecube_stop()
{
    const char* TAG = "core_gamecube_stop";
    periph_ll_enable_clk_clear_rst(PERIPH_RMT_MODULE);

    rmt_isr_deregister(gc_rmt_isr_handle);
}
