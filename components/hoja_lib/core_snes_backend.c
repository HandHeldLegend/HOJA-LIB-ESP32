#include "core_snes_backend.h"

uint16_t snes_button_buffer = 0xFFFF;
TaskHandle_t snes_TaskHandle = NULL;

// Set up var for receive buffer.
WORD_ALIGNED_ATTR char snes_recvbuf[129]="";
spi_slave_transaction_t snes_slave_transaction;

void spi_dummy_cb(spi_slave_transaction_t *trans)
{

}

hoja_err_t core_snes_start()
{
    const char* TAG = "core_snes_start";

    // Set up SPI
    esp_err_t ret;

    if ( (HOJA_PIN_SERIAL == 0xFF) | (HOJA_PIN_CLOCK == 0xFF) | (HOJA_PIN_LATCH == 0xFF) )
    {
        ESP_LOGE(TAG, "All three GPIO pins must be set for this function (HOJA_PIN_SERIAL, HOJA_PIN_LATCH, HOJA_PIN_CLOCK).");
        return HOJA_FAIL;
    }

    //Configuration for the SPI bus
    spi_bus_config_t buscfg={
        .mosi_io_num    = -1,
        .miso_io_num    = (int) HOJA_PIN_SERIAL,
        .sclk_io_num    = (int) HOJA_PIN_CLOCK,
        .quadwp_io_num  = -1,
        .quadhd_io_num  = -1,
    };

    //Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg={
        .mode           = 2,
        .spics_io_num   = (int) HOJA_PIN_LATCH,
        .queue_size     = 1,
        .flags          = 0,
        .post_setup_cb  = spi_dummy_cb,
        .post_trans_cb  = spi_dummy_cb
    };

    //Initialize SPI slave interface
    ret=spi_slave_initialize(HSPI_HOST, &buscfg, &slvcfg, 0);
    assert(ret==ESP_OK);

    memset(snes_recvbuf, 0, 33);
    memset(&snes_slave_transaction, 0, sizeof(snes_slave_transaction));

    // Check if task is running, delete if so.
    if (snes_TaskHandle != NULL)
    {
        vTaskDelete(snes_TaskHandle);
        snes_TaskHandle = NULL;
    }

    // Start SNES task.
    xTaskCreatePinnedToCore(snes_task, "SNES/NES Task Loop", 2024,
                            NULL, 0, &snes_TaskHandle, 1);
    
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
    err = spi_slave_free(HSPI_HOST);
    assert(err==ESP_OK);

    ESP_LOGI(TAG, "SNES Core stopped OK.");

    return HOJA_OK;
}

void snes_task(void * parameters)
{
    for(;;)
    {
        esp_err_t err;

        // Scan the buttons using the callback defined by user.
        hoja_button_cb();

        // SNES bits button order
        // B, Y, Start, Select, Up, Down, Left, Right, A, X, L, R
        // Ordered starting from bit 0 with B up to bit 11 with R.
        // SNES/NES buttons are HIGH when not pressed, LOW when pressed.
        // We subtract the button value as it's inverted. Unpressed trailing bits need to
        // remain HIGH as well.

        // Reset button output to all HIGH bits.
        snes_button_buffer = 0xFFFF;

        // Simultaneous dpad direction erasure
        if (g_button_data.d_down && g_button_data.d_up)
        {
            g_button_data.d_down = 0;
            g_button_data.d_up = 0;
        }
        if (g_button_data.d_left && g_button_data.d_right)
        {
            g_button_data.d_left = 0;
            g_button_data.d_right = 0;
        }

        // Go through each bit and set accordingly.
        snes_button_buffer -= (g_button_data.b_down     << 15U  );
        snes_button_buffer -= (g_button_data.b_left     << 14U  );
        snes_button_buffer -= (g_button_data.b_select   << 13U  );
        snes_button_buffer -= (g_button_data.b_start    << 12U  );
        snes_button_buffer -= (g_button_data.d_up       << 11U  );
        snes_button_buffer -= (g_button_data.d_down     << 10U  );
        snes_button_buffer -= (g_button_data.d_left     << 9U   );
        snes_button_buffer -= (g_button_data.d_right    << 8U   );
        snes_button_buffer -= (g_button_data.b_right    << 7U   );
        snes_button_buffer -= (g_button_data.b_up       << 6U   );
        snes_button_buffer -= (g_button_data.t_l        << 5U   );
        snes_button_buffer -= (g_button_data.t_r        << 4U   );

        // Reset HOJA input buttons for next scan sequence.
        hoja_button_reset();

        memset(snes_recvbuf, 0xA5, 129);
        snes_slave_transaction.length = 15;
        uint8_t tmplow = (snes_button_buffer >> 8);
        uint8_t tmphigh = (snes_button_buffer & 0xFF);
        uint8_t tmp[2] = {tmplow, tmphigh};
        snes_slave_transaction.tx_buffer = tmp;

        err = spi_slave_transmit(HSPI_HOST, &snes_slave_transaction, portMAX_DELAY);
        vTaskDelay(3/portTICK_PERIOD_MS);
    }
}
