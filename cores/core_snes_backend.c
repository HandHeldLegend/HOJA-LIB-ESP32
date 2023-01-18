#include "core_snes_backend.h"

volatile uint16_t snes_button_buffer= 0x5555;
volatile bool  done = false;
TaskHandle_t snes_TaskHandle = NULL;
#define LATCH_MASK (1ULL << CONFIG_HOJA_GPIO_NS_LATCH)
#define CLOCK_MASK (1ULL << CONFIG_HOJA_GPIO_NS_CLOCK)
#define DATA_MASK   (1ULL << CONFIG_HOJA_GPIO_NS_SERIAL)

static QueueHandle_t snes_evt_queue = NULL;

// This interrupt triggers when the latch line goes high.
static void IRAM_ATTR snes_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    done = true;
}

hoja_err_t core_snes_start()
{
    const char* TAG = "core_snes_start";

    // Set up I2C
    esp_err_t ret;

    // Start SNES task.
    xTaskCreatePinnedToCore(snes_task, "SNES/NES Task Loop", 2024,
                            NULL, 0, &snes_TaskHandle, HOJA_CORE_CPU);
    
    ESP_LOGI(TAG, "SNES Core started OK.");

    return HOJA_OK;
}

hoja_err_t core_snes_stop()
{
    const char* TAG = "core_snes_stop";

    // Stop SNES task
    // Check if task is running, delete if so.
    if (snes_TaskHandle != NULL)
    {
        vTaskDelete(snes_TaskHandle);
    }
    snes_TaskHandle = NULL;
    
    // Deinitialize SPI slave interface
    esp_err_t err;
    err = spi_slave_free(VSPI_HOST);
    assert(err==ESP_OK);

    ESP_LOGI(TAG, "SNES Core stopped OK.");

    return HOJA_OK;
}

// Thanks to DarthCloud for some insight into the register workings for the SPI peripheral :)

#define SPI_LL_RST_MASK (SPI_OUT_RST | SPI_IN_RST | SPI_AHBM_RST | SPI_AHBM_FIFO_RST)
#define SPI_LL_UNUSED_INT_MASK  (SPI_INT_EN | SPI_SLV_WR_STA_DONE | SPI_SLV_RD_STA_DONE | SPI_SLV_WR_BUF_DONE | SPI_SLV_RD_BUF_DONE)
#define SNES_DELAY_US 210

void snes_task(void * parameters)
{
    const char* TAG = "snes_task";

    //zero-initialize the config structure.
    gpio_config_t io_conf = {};

    // Interrupt on falling edge of actual
    // CS line because this will improve compatibility
    // between adapters and real console.
    io_conf.intr_type = GPIO_INTR_NEGEDGE;

    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = LATCH_MASK;
    //disable pull-down mode
    io_conf.pull_down_en = 0;

    io_conf.pull_up_en = 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_INTR_DISABLE;

    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = CLOCK_MASK;
    //disable pull-down mode
    io_conf.pull_down_en = 0;

    io_conf.pull_up_en = 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_INTR_DISABLE;

    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = DATA_MASK;
    //disable pull-down mode
    io_conf.pull_down_en = 0;

    io_conf.pull_up_en = 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    // Connect GPIO matrix

    // Enable input to peripheral from GPIO matrix
    // Set data output
    gpio_matrix_out(CONFIG_HOJA_GPIO_NS_SERIAL, HSPIQ_OUT_IDX, false, false);

    // CS latch input setup
    // Next, set the input to the internal latch line as high
    GPIO.func_in_sel_cfg[HSPICS0_IN_IDX].sig_in_sel = 1;
    GPIO.func_in_sel_cfg[HSPICS0_IN_IDX].func_sel = 0x38;

    // CLOCK input setup
    gpio_matrix_in(CONFIG_HOJA_GPIO_NS_CLOCK, HSPICLK_IN_IDX, false);

    // Set up SPI with custom stuff
    // Clear all first
    periph_ll_enable_clk_clear_rst(PERIPH_HSPI_MODULE);

    SPI2.clock.val = 0;
    SPI2.user.val = 0;
    SPI2.ctrl.val = 0;
    SPI2.slave.wr_rd_buf_en = 0; //no sure if needed
    SPI2.user.doutdin = 1; //we only support full duplex
    SPI2.user.sio = 0;
    SPI2.slave.slave_mode = 1;
    SPI2.dma_conf.val |= SPI_LL_RST_MASK;
    SPI2.dma_out_link.start = 0;
    SPI2.dma_in_link.start = 0;
    SPI2.dma_conf.val &= ~SPI_LL_RST_MASK;
    SPI2.slave.sync_reset = 1;
    SPI2.slave.sync_reset = 0;

    //use all 64 bytes of the buffer
    SPI2.user.usr_miso_highpart = 0;
    SPI2.user.usr_mosi_highpart = 0;

    //Disable unneeded ints
    SPI2.slave.val &= ~SPI_LL_UNUSED_INT_MASK;

    SPI2.ctrl.wr_bit_order = 0;
    SPI2.ctrl.rd_bit_order = 0;

    /* Set Mode 2 no DMA*/
    SPI2.pin.ck_idle_edge = 0;
    SPI2.user.ck_out_edge = 0;
    SPI2.user.ck_i_edge = 1;
    SPI2.ctrl2.miso_delay_mode = 0;
    SPI2.ctrl2.miso_delay_num = 0;
    SPI2.ctrl2.mosi_delay_mode = 1;
    SPI2.ctrl2.mosi_delay_num = 2;

    SPI2.ctrl2.setup_time = 0;
    SPI2.ctrl2.hold_time = 0;

    SPI2.slv_wrbuf_dlen.bit_len = 0;
    SPI2.slv_rdbuf_dlen.bit_len = 16 - 1;

    SPI2.user.usr_miso = 1;
    SPI2.user.usr_mosi = 0;

    SPI2.slave.trans_inten = 0;
    SPI2.slave.trans_done = 0;

    SPI2.data_buf[0] = 0xFFFFFFFF;
    SPI2.data_buf[1] = 0xFFFFFFFF;

    uint8_t flip = 0;
    esp_err_t err;
    uint32_t io_num;

    //create a queue to handle gpio event from isr
    snes_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    // Register GPIO interrupt handler
    //install gpio isr service
    gpio_install_isr_service(0);

    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(CONFIG_HOJA_GPIO_NS_LATCH, snes_isr_handler, (void*) CONFIG_HOJA_GPIO_NS_LATCH);

    // 0 or LOW is high on the output - inverted on the data line.
    // 1 or HIGH means low on the output.

    for(;;)
    {
        if (done)
        // What do we do when an interrupt is received and this is unblocked?
        // First, delay 200 microseconds
        {
            ets_delay_us(SNES_DELAY_US);
            SPI2.slave.trans_done = 0;
            SPI2.slave.sync_reset = 1;

            // Reconnect Latch line to GPIO to pull it up
            GPIO.func_in_sel_cfg[HSPICS0_IN_IDX].func_sel = 0x38;
            
            SPI2.slave.sync_reset = 0;

            // Set up our SPI TX
            // SNES bits button order
            // B, Y, Start, Select, Up, Down, Left, Right, A, X, L, R
            // Ordered starting from bit 0 with B up to bit 11 with R.
            // SNES/NES buttons are HIGH when not pressed, LOW when pressed.
            // We subtract the button value as it's inverted. Unpressed trailing bits need to
            // remain HIGH as well.

            // Go through each bit and set accordingly.
            snes_button_buffer |= (hoja_button_data.button_down << 7U);
            snes_button_buffer |= (hoja_button_data.button_left     << 6U  );
            snes_button_buffer |= (hoja_button_data.button_select   << 5U  );
            snes_button_buffer |= (hoja_button_data.button_start    << 4U  );
            snes_button_buffer |= (hoja_button_data.dpad_up       << 3U  );
            snes_button_buffer |= (hoja_button_data.dpad_down     << 2U  );
            snes_button_buffer |= (hoja_button_data.dpad_left     << 1U   );
            snes_button_buffer |= (hoja_button_data.dpad_right);

            snes_button_buffer |= (hoja_button_data.button_right    << 15U   );
            snes_button_buffer |= (hoja_button_data.button_up       << 14U   );
            snes_button_buffer |= (hoja_button_data.trigger_l        << 13U   );
            snes_button_buffer |= (hoja_button_data.trigger_r        << 12U   );

            // The data is set up all weird?
            // Byte order 0xF0 0xF0
            //  Bits 8-13  <||   ||
            //               |   ||
            //       14-17<--|   ||
            //                   ||
            //             0-3<--||
            //                    |
            //              4-7<--|
            SPI2.data_buf[0] = (0xFFFFFFFF) & ~(snes_button_buffer);
            snes_button_buffer = 0;

            // OK the transaction.
            SPI2.cmd.usr = 0;

            // Next, set the input to the internal latch line as low
            GPIO.func_in_sel_cfg[HSPICS0_IN_IDX].func_sel = 0x30;
            
            // OK the transaction.
            SPI2.cmd.usr = 1;

            // Reset HOJA input buttons for next scan sequence.
            // hoja_button_reset();
            done = false;
        }
        vTaskDelay(0.2/portTICK_PERIOD_MS);
    }
}