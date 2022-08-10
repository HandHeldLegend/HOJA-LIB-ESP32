#include "rbc_gamecube_core.h"

#define GMC_POLL_RMT_LEN    66
#define GMC_PROBE_RMT_LEN   26
#define GMC_ORIGIN_RMT_LEN  82
const rmt_item32_t RMT_ZERO = {0};

rmt_item32_t gcmd_poll_rmt[GMC_POLL_RMT_LEN] = {
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X8, JB_RMT_0X0,
        JB_RMT_0X8, JB_RMT_0X0, 
        JB_RMT_0X8, JB_RMT_0X0,
        JB_RMT_0X8, JB_RMT_0X0, 
        JB_RMT_0X8, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_STOP, RMT_ZERO
    };

rb_err_t gc_input_translate(void)
{
    rb_stick_cb();
    rb_button_cb();
    rb_button_cb();
    rb_button_cb();
    gcmd_poll_rmt[GC_BUTTON_START] = g_button_data.b_start  ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_START] = g_button_data.b_left   ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_START] = g_button_data.b_up     ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_START] = g_button_data.b_down   ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_START] = g_button_data.b_right  ? JB_HIGH : JB_LOW;

    gcmd_poll_rmt[GC_BUTTON_START] = g_button_data.t_zl     ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_START] = g_button_data.t_zr     ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_START] = (g_button_data.t_l | g_button_data.t_r)    ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_START] = g_button_data.d_up     ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_START] = g_button_data.d_down   ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_START] = g_button_data.d_left   ? JB_HIGH : JB_LOW;
    gcmd_poll_rmt[GC_BUTTON_START] = g_button_data.d_right  ? JB_HIGH : JB_LOW;

    memcpy(JB_TX_POLL_MEM, gcmd_poll_rmt, sizeof(rmt_item32_t) * 66);

    rb_button_reset();

    return RB_OK;
}

volatile uint8_t inttt = 0;
volatile uint8_t sents = 0;

static void gamecube_core_isr(void* arg) 
{

    if (RMT.int_st.ch0_rx_end)
    {
        inttt++;
        // Disable RX
        JB_RX_CONF1.rx_en = 0;
        // Reset write pointer for RX
        JB_RX_CONF1.mem_wr_rst = 1;
        JB_RX_CONF1.mem_wr_rst = 0;
        // Clear RX bit for ch0
        RMT.int_clr.ch0_rx_end = 1;

        // Start transaction on ch2
        gpio_matrix_out(RB_PIN_SERIAL, RMT_SIG_OUT0_IDX + RMT_TX_CHANNEL_ORIGIN, 0, 0);
        JB_TX_ORIGIN_CONF1.tx_start = 1;
        
    }
    else if (RMT.int_st.ch1_tx_end)
    {
        RMT.int_clr.ch1_tx_end = 1;
    }
    else if (RMT.int_st.ch2_tx_end)
    {
        sents++;
        // Reset TX read pointer bit
        JB_TX_ORIGIN_CONF1.mem_rd_rst = 1;
        JB_TX_ORIGIN_CONF1.mem_rd_rst = 0;
        // Clear TX end interrupt bit
        RMT.int_clr.ch2_tx_end = 1;

        // Start RX on ch0
        JB_RX_CONF1.rx_en = 1;
    }
    
}

void gc_core_init(void)
{
    const char* TAG = "gc_core_init";

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
    // Enable RX got interrupt
    RMT.int_ena.ch0_rx_end = 1;
    

    rmt_item32_t gcmd_probe_rmt[10] = {
        JB_RMT_0X0, JB_RMT_0X0, 
        JB_STOP, RMT_ZERO
    };

    // Set up the canned response to origin
    rmt_item32_t gcmd_origin_rmt[GMC_ORIGIN_RMT_LEN] = {
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
        JB_STOP, RMT_ZERO
    };

    JB_TX_PROBE_CONF1.mem_rd_rst = 1;
    JB_TX_PROBE_CONF1.mem_rd_rst = 0;

    volatile uint32_t *tmp = &JB_TX_PROBE_MEM[0].val;

    memcpy(JB_TX_PROBE_MEM, gcmd_probe_rmt, sizeof(rmt_item32_t) * 10);
    memcpy(JB_TX_ORIGIN_MEM, gcmd_origin_rmt, sizeof(rmt_item32_t) * 82);

    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[RB_PIN_SERIAL], PIN_FUNC_GPIO);
    gpio_set_direction(RB_PIN_SERIAL, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_matrix_out(RB_PIN_SERIAL, RMT_SIG_OUT0_IDX + RMT_TX_CHANNEL_PROBE, 0, 0);
    gpio_matrix_in(RB_PIN_SERIAL, RMT_SIG_IN0_IDX + RMT_RX_CHANNEL, 0);



    rmt_isr_register(gamecube_core_isr, NULL, 3, NULL);

}

void gamecube_core_start()
{  
    const char* TAG = "gamecube_core_start";
    esp_err_t er = ESP_OK;
    ESP_LOGI(TAG, "GameCube Core Started.");

    gc_core_init();

    vTaskDelay(1000/portTICK_PERIOD_MS);
    
    JB_RX_CONF1.rx_en = 1;

    while(1)
    {
        ESP_LOGI(TAG, "CMDs received: %d", inttt);
        ESP_LOGI(TAG, "CMDs sent: %d", sents);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }

}