#include "core_gamecube_backend.h"

#define GC_POLL_RMT_LEN    66
#define GC_PROBE_RMT_LEN   26
#define GC_ORIGIN_RMT_LEN  82

TaskHandle_t gamecube_TaskHandle = NULL;

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

    hoja_stick_cb();

    gcmd_poll_rmt[GC_BUTTON_START]  = g_button_data.b_start  ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_Y]      = g_button_data.b_left   ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_X]      = g_button_data.b_up     ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_B]      = g_button_data.b_down   ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_A]      = g_button_data.b_right  ? JB_HIGH : JB_LOW;

    gcmd_poll_rmt[GC_BUTTON_LB]     = g_button_data.t_zl     ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_RB]     = g_button_data.t_zr     ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_Z]      = (g_button_data.t_l | g_button_data.t_r)    ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_DUP]    = g_button_data.d_up     ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_DDOWN]  = g_button_data.d_down   ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_DLEFT]  = g_button_data.d_left   ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_DRIGHT] = g_button_data.d_right  ? JB_HIGH : JB_LOW;

    memcpy(JB_TX_POLL_MEM, gcmd_poll_rmt, sizeof(rmt_item32_t) * GC_POLL_RMT_LEN);

    hoja_stick_cb();

    hoja_button_reset();
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
        cmd_buffer.byte0 = 0x00;
        uint8_t tmpi = 0;

        for (uint8_t i = 0; i < 8; i++)
        {   
            // If duration 0 is less than duration 1, it's a HIGH bit. Otherwise low bit.
            // Shift the bit so it's always in the leftmost bit position.
            cmd_buffer.byte0 |= ( (JB_RX_MEM[i].duration0 < JB_RX_MEM[i].duration1) ? 1 : 0) << (7-i);
        }

        command = cmd_buffer.byte0;

        // Disable RX
        JB_RX_CONF1.rx_en = 0;
        // Reset write pointer for RX
        JB_RX_CONF1.mem_wr_rst = 1;
        
        // Clear RX bit for ch0
        RMT.int_clr.ch0_rx_end = 1;

        JB_RX_CONF1.mem_wr_rst = 0;

        // Check the command byte and respond accordingly
        switch(cmd_buffer.byte0)
        {
            // Probe Command
            case 0x00:
            default:
                gpio_matrix_out(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_OUT0_IDX + RMT_TX_CHANNEL_PROBE, 0, 0);
                JB_TX_PROBE_CONF1.mem_owner = 0;
                JB_TX_PROBE_CONF1.tx_start = 1;
                break;

            // Origin Command
            case 0x41:
            case 0x42:
                gpio_matrix_out(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_OUT0_IDX + RMT_TX_CHANNEL_ORIGIN, 0, 0);
                JB_TX_ORIGIN_CONF1.mem_owner = 0;
                JB_TX_ORIGIN_CONF1.tx_start = 1;
                gamecube_input_translate();
                break;

            // Poll Command
            case 0x40:
                gpio_matrix_out(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_OUT0_IDX + RMT_TX_CHANNEL_POLL, 0, 0);
                JB_TX_POLL_CONF1.mem_owner = 0;
                JB_TX_POLL_CONF1.tx_start = 1;
                gamecube_input_translate();
                break;
        }
        
        
    }
    // Probe response transaction end
    else if (RMT.int_st.ch1_tx_end)
    {
        // Reset TX read pointer bit
        JB_TX_PROBE_CONF1.mem_rd_rst = 1;
        JB_TX_PROBE_CONF1.mem_rd_rst = 0;
        // Clear TX end interrupt bit
        RMT.int_clr.ch1_tx_end = 1;

        // Start RX on ch0
        gpio_matrix_in(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_IN0_IDX + RMT_RX_CHANNEL, 0);
        JB_RX_CONF1.mem_owner = 1;
        JB_RX_CONF1.rx_en = 1;
    }
    // Origin response transaction end
    else if (RMT.int_st.ch2_tx_end)
    {
        // Reset TX read pointer bit
        JB_TX_ORIGIN_CONF1.mem_rd_rst = 1;
        JB_TX_ORIGIN_CONF1.mem_rd_rst = 0;
        // Clear TX end interrupt bit
        RMT.int_clr.ch2_tx_end = 1;

        // Start RX on ch0
        gpio_matrix_in(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_IN0_IDX + RMT_RX_CHANNEL, 0);
        JB_RX_CONF1.mem_owner = 1;
        JB_RX_CONF1.rx_en = 1;
    }
    // Poll response transaction end
    else if (RMT.int_st.ch4_tx_end)
    {
        // Reset TX read pointer bit
        JB_TX_POLL_CONF1.mem_rd_rst = 1;
        JB_TX_POLL_CONF1.mem_rd_rst = 0;
        // Clear TX end interrupt bit
        RMT.int_clr.ch4_tx_end = 1;

        // Start RX on ch0
        gpio_matrix_in(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_IN0_IDX + RMT_RX_CHANNEL, 0);
        JB_RX_CONF1.mem_owner = 1;
        JB_RX_CONF1.rx_en = 1;
    }
    
}

void gamecube_init(void)
{
    const char* TAG = "gamecube_init";

    periph_ll_enable_clk_clear_rst(PERIPH_RMT_MODULE);

    RMT.apb_conf.fifo_mask = RMT_DATA_MODE_MEM;

    // set up RMT peripherial register stuff
    // for receive channel
    JB_RX_CONF0.div_cnt = 10; // 0.125us increments.
    JB_RX_CONF0.idle_thres = 35; // 4us idle
    JB_RX_CONF0.mem_size = 1;
    JB_RX_CONF0.carrier_en = 0;

    JB_RX_CONF1.idle_out_lv = RMT_IDLE_LEVEL_HIGH;
    JB_RX_CONF1.idle_out_en = 1;

    JB_RX_CONF1.rx_filter_thres = 0;
    JB_RX_CONF1.rx_filter_en = 0;
    JB_RX_CONF1.mem_owner = RMT_MEM_OWNER_RX;
    JB_RX_CONF1.mem_rd_rst = 1;
    JB_RX_CONF1.mem_rd_rst = 0;
    JB_RX_CONF1.mem_wr_rst = 1;
    JB_RX_CONF1.mem_wr_rst = 0;
    JB_RX_CONF1.ref_always_on = RMT_BASECLK_APB;

    // set up RMT peripheral register stuff
    // for transaction channels :)
    JB_TX_PROBE_CONF0.div_cnt   = 20; // 0.25 us increments
    JB_TX_ORIGIN_CONF0.div_cnt  = 20;
    JB_TX_POLL_CONF0.div_cnt    = 20;
    
    JB_TX_PROBE_CONF0.mem_size  = 1;
    JB_TX_ORIGIN_CONF0.mem_size = 2;
    JB_TX_POLL_CONF0.mem_size   = 2;

    JB_TX_PROBE_CONF1.tx_conti_mode     = 0;
    JB_TX_ORIGIN_CONF1.tx_conti_mode    = 0;
    JB_TX_POLL_CONF1.tx_conti_mode      = 0;

    JB_TX_PROBE_CONF0.carrier_en    = 0;
    JB_TX_ORIGIN_CONF0.carrier_en   = 0;
    JB_TX_POLL_CONF0.carrier_en     = 0;

    JB_TX_ORIGIN_CONF1.mem_owner    = RMT_MEM_OWNER_TX;
    JB_TX_POLL_CONF1.mem_owner      = RMT_MEM_OWNER_TX;
    JB_TX_PROBE_CONF1.mem_owner     = RMT_MEM_OWNER_TX;

    JB_TX_PROBE_CONF1.ref_always_on     = RMT_BASECLK_APB;
    JB_TX_ORIGIN_CONF1.ref_always_on    = RMT_BASECLK_APB;
    JB_TX_POLL_CONF1.ref_always_on      = RMT_BASECLK_APB;

    JB_TX_PROBE_CONF1.idle_out_lv   = RMT_IDLE_LEVEL_HIGH;
    JB_TX_ORIGIN_CONF1.idle_out_lv  = RMT_IDLE_LEVEL_HIGH;
    JB_TX_POLL_CONF1.idle_out_lv    = RMT_IDLE_LEVEL_HIGH;

    JB_TX_PROBE_CONF1.idle_out_en   = 1;
    JB_TX_ORIGIN_CONF1.idle_out_en  = 1;
    JB_TX_POLL_CONF1.idle_out_en    = 1;

    // Enable transaction complete interrupts
    RMT.int_ena.ch1_tx_end = 1;
    RMT.int_ena.ch2_tx_end = 1;
    RMT.int_ena.ch4_tx_end = 1;
    // Enable RX got interrupt
    RMT.int_ena.ch0_rx_end = 1;
    

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

    JB_TX_PROBE_CONF1.mem_rd_rst = 1;
    JB_TX_PROBE_CONF1.mem_rd_rst = 0;

    memcpy(JB_TX_PROBE_MEM, gcmd_probe_rmt, sizeof(rmt_item32_t) * GC_PROBE_RMT_LEN);
    memcpy(JB_TX_ORIGIN_MEM, gcmd_origin_rmt, sizeof(rmt_item32_t) * GC_ORIGIN_RMT_LEN);

    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[CONFIG_HOJA_GPIO_NS_SERIAL], PIN_FUNC_GPIO);
    gpio_set_direction(CONFIG_HOJA_GPIO_NS_SERIAL, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_matrix_out(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_OUT0_IDX + RMT_TX_CHANNEL_PROBE, 0, 0);
    gpio_matrix_in(CONFIG_HOJA_GPIO_NS_SERIAL, RMT_SIG_IN0_IDX + RMT_RX_CHANNEL, 0);

    rmt_isr_register(gamecube_rmt_isr, NULL, 3, NULL);

}

hoja_err_t core_gamecube_start()
{  
    const char* TAG = "core_gamecube_start";
    esp_err_t er = ESP_OK;
    ESP_LOGI(TAG, "GameCube Core Started.");

    gamecube_init();

    vTaskDelay(200/portTICK_PERIOD_MS);
    
    JB_TX_PROBE_CONF1.tx_start = 1;
    JB_RX_CONF1.rx_en = 1;

    return HOJA_OK;
}