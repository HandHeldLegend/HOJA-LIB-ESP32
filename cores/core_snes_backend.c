#include "core_snes_backend.h"

volatile uint16_t snes_button_buffer= 0x5555;
volatile bool  done = false;
TaskHandle_t snes_TaskHandle = NULL;
#define LATCH_MASK (1ULL << CONFIG_HOJA_GPIO_NS_LATCH)
#define CLOCK_MASK (1ULL << CONFIG_HOJA_GPIO_NS_CLOCK)
#define DATA_MASK   (1ULL << CONFIG_HOJA_GPIO_NS_SERIAL)

#define VIRTUAL_HIGH 0x38
#define VIRTUAL_LOW 0x30

static QueueHandle_t snes_evt_queue = NULL;

// This interrupt triggers when the latch line goes high or low.
static void IRAM_ATTR snes_isr_handler(void* arg)
{
    
    //uint32_t gpio_num = (uint32_t) arg;
    
    // Toggle done so we can reset our SPI transaction for next
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

void core_snes_stop()
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
}

// Thanks to DarthCloud for some insight into the register workings for the SPI peripheral :)

#define SPI_LL_RST_MASK (SPI_OUT_RST | SPI_IN_RST | SPI_AHBM_RST | SPI_AHBM_FIFO_RST)
#define SPI_LL_UNUSED_INT_MASK  (SPI_INT_EN | SPI_SLV_WR_STA_DONE | SPI_SLV_RD_STA_DONE | SPI_SLV_WR_BUF_DONE | SPI_SLV_RD_BUF_DONE)
#define SNES_DELAY_US 250

void snes_task(void * parameters)
{
    const char* TAG = "snes_task";

    //zero-initialize the config structure.
    gpio_config_t io_conf = {};

    // Interrupt on rising edge since we can use that to our advantage
    // Start 
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
    // Set data output to be enabled with bit
    GPIO.func_out_sel_cfg[CONFIG_HOJA_GPIO_NS_SERIAL].oen_sel = 1;
    GPIO.enable_w1ts = DATA_MASK;

    // CS latch input setup
    gpio_matrix_in(CONFIG_HOJA_GPIO_NS_LATCH, HSPICS0_IN_IDX, false);

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

    SPI2.slv_wrbuf_dlen.bit_len = 32 - 1;
    SPI2.slv_rdbuf_dlen.bit_len = 32 - 1;
    SPI2.slv_rd_bit.val = 32-1;

    SPI2.user.usr_miso = 1;
    SPI2.user.usr_mosi = 0;

    SPI2.slave.trans_inten = 0;
    SPI2.slave.trans_done = 0;

    SPI2.data_buf[0] = 0x0000FFFF;
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
        // First, delay x microseconds to ensure the SPI transaction is completed.
        {
            ets_delay_us(SNES_DELAY_US);
            // At this point, the transction should be over. Load our next transaction.
            
            // Set data GPIO to low
            GPIO.out_w1tc = DATA_MASK;
            GPIO.func_out_sel_cfg[CONFIG_HOJA_GPIO_NS_SERIAL].func_sel = 256;

            // Disable data out
            //GPIO.enable_w1tc = DATA_MASK;

            // Reset and start
            SPI2.slave.trans_done = 0;
            SPI2.slave.sync_reset = 1;
            SPI2.slave.sync_reset = 0;

            // CS latch input setup
            // 0x38 is high, 0x30 is low
            // Next, set the input to the internal latch line as high
            GPIO.func_in_sel_cfg[HSPICS0_IN_IDX].sig_in_sel = 1;
            GPIO.func_in_sel_cfg[HSPICS0_IN_IDX].func_sel = VIRTUAL_HIGH;

            // Set clock as virtual input and high
            //GPIO.func_in_sel_cfg[HSPICLK_IN_IDX].sig_in_sel = 1;
            GPIO.func_in_sel_cfg[HSPICLK_IN_IDX].func_sel = VIRTUAL_HIGH;

            // Set up our SPI TX
            // SNES bits button order
            // B, Y, Start, Select, Up, Down, Left, Right, A, X, L, R
            // Ordered starting from bit 0 with B up to bit 11 with R.
            // SNES/NES buttons are HIGH when not pressed, LOW when pressed.
            // We subtract the button value as it's inverted. Unpressed trailing bits need to
            // remain HIGH as well.

            // Process conversion of button data
            hoja_button_remap_process();

            // Go through each bit and set accordingly.
            snes_button_buffer |= (hoja_processed_buttons.button_down << 7U);
            snes_button_buffer |= (hoja_processed_buttons.button_left     << 6U  );
            snes_button_buffer |= (hoja_processed_buttons.button_select   << 5U  );
            snes_button_buffer |= (hoja_processed_buttons.button_start    << 4U  );
            snes_button_buffer |= (hoja_processed_buttons.dpad_up       << 3U  );
            snes_button_buffer |= (hoja_processed_buttons.dpad_down     << 2U  );
            snes_button_buffer |= (hoja_processed_buttons.dpad_left     << 1U   );
            snes_button_buffer |= (hoja_processed_buttons.dpad_right);

            snes_button_buffer |= (hoja_processed_buttons.button_right    << 15U   );
            snes_button_buffer |= (hoja_processed_buttons.button_up       << 14U   );
            snes_button_buffer |= (hoja_processed_buttons.trigger_l        << 13U   );
            snes_button_buffer |= (hoja_processed_buttons.trigger_r        << 12U   );

            // The data is set up all weird?
            // Byte order 0xF0 0xF0
            //  Bits 8-13  <||   ||
            //               |   ||
            //       14-17<--|   ||
            //                   ||
            //             0-3<--||
            //                    |
            //              4-7<--|

            // Set last bit as low to hold the line low on idle
            //snes_button_buffer |= (1 << 8U);

            SPI2.data_buf[0] = (0x0000FFFF) & ~(snes_button_buffer);
            //SPI2.data_buf[0] |= 0xFFFF0000;
            snes_button_buffer = 0;

            SPI2.cmd.usr = 1;

            // Set CS virtual low
            GPIO.func_in_sel_cfg[HSPICS0_IN_IDX].func_sel = VIRTUAL_LOW;

            // wait 50us
            ets_delay_us(50);

            // Pulse clock 16x
            for (uint8_t i = 0; i < 16; i++)
            {
                GPIO.func_in_sel_cfg[HSPICLK_IN_IDX].func_sel = VIRTUAL_LOW;
                ets_delay_us(6);

                GPIO.func_in_sel_cfg[HSPICLK_IN_IDX].func_sel = VIRTUAL_HIGH;
                ets_delay_us(6);
            }

            // Set CLK back to normal
            GPIO.func_in_sel_cfg[HSPICLK_IN_IDX].func_sel = CONFIG_HOJA_GPIO_NS_CLOCK;
            GPIO.func_in_sel_cfg[HSPICLK_IN_IDX].sig_in_sel = 1;

            // Set clock source to latch pin
            GPIO.func_in_sel_cfg[HSPICS0_IN_IDX].func_sel = CONFIG_HOJA_GPIO_NS_LATCH;
            GPIO.func_in_sel_cfg[HSPICS0_IN_IDX].sig_in_sel = 1;

            // Set GPIO data to SPI output source
            GPIO.func_out_sel_cfg[CONFIG_HOJA_GPIO_NS_SERIAL].func_sel = HSPIQ_OUT_IDX;

            //GPIO.enable_w1ts = DATA_MASK;

            done = false;
        }
        vTaskDelay(0.2/portTICK_PERIOD_MS);
    }
}