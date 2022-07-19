#include "rbc_gamecube_core.h"

joybus_pad_t *pad_instance = NULL;
RingbufHandle_t ringbuffer = NULL;

void gamecube_core_start()
{  
    const char* TAG = "gamecube_core_start";
    esp_err_t er = ESP_OK;

    gpio_config_t tmp_conf;
    tmp_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    tmp_conf.mode = GPIO_MODE_INPUT_OUTPUT_OD;
    tmp_conf.pin_bit_mask = (1ULL << RB_PIN_SERIAL);
    gpio_config(&tmp_conf);

    // Set up transmission
    pad_instance = joybus_pad_init(RMT_TX_CHANNEL, RB_PIN_SERIAL);
    uint8_t cmd = 0x00;
    uint8_t data[2] = {0x00, 0x01};

    // Set up receipt
    rmt_config_t rmt_rx_config = {
      .rmt_mode = RMT_MODE_RX,
      .channel = RMT_RX_CHANNEL,
      .gpio_num = RB_PIN_SERIAL,
      .clk_div = 25,
      .mem_block_num = 1,
      .rx_config.filter_en = true,
      .rx_config.filter_ticks_thresh = 1,
      .rx_config.idle_threshold = 15,
    };

    er = rmt_config(&rmt_rx_config);

    er = rmt_driver_install(RMT_RX_CHANNEL, 1024, 0);

    er = rmt_get_ringbuf_handle(RMT_RX_CHANNEL, &ringbuffer);

    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[RB_PIN_SERIAL], PIN_FUNC_GPIO);
    gpio_set_direction(RB_PIN_SERIAL, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_matrix_out(RB_PIN_SERIAL, RMT_SIG_OUT0_IDX + RMT_TX_CHANNEL, 0, 0);
    gpio_matrix_in(RB_PIN_SERIAL, RMT_SIG_IN0_IDX + RMT_RX_CHANNEL, 0);

    er = rmt_rx_start(RMT_RX_CHANNEL, true);

    // Read loop
    size_t length = 0;
    rmt_item32_t *items = NULL;

    uint8_t counter = 0x00;

    while(1)
    {
        counter ++;
        uint8_t out = 0;
        //pad_instance->send_command(pad_instance, cmd, data, 2);
        //vTaskDelay(1000/portTICK_PERIOD_MS);
        items = (rmt_item32_t *)xRingbufferReceive(ringbuffer, &length, portMAX_DELAY);
        rmt_rx_stop(RMT_RX_CHANNEL);
        if (NULL != items)
        {  
            out = 0;
            // Convert length from number of bytes to number of entries
            length /= sizeof(rmt_item32_t);
            if (length >=1)
            {
                for (uint8_t i = 0; i < length-2; i++)
                {
                    out |= (items[i].duration0 < items[i].duration1);
                    out <<= 1;
                }
                
            }
            pad_instance->send_command(pad_instance, cmd, data, 2);
            vRingbufferReturnItem(ringbuffer, (void *) items);
        }
        //vTaskDelay(1);
        rmt_rx_start(RMT_RX_CHANNEL, true);
        if (counter == 255)
        {
            ESP_LOGI(TAG, "cmd: %X, len: %d", out, length);
            counter = 0x00;
        }
    
    }

}