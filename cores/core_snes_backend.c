#include "cores/core_snes_backend.h"

uint16_t snes_button_buffer = 0xFFFF;
TaskHandle_t snes_TaskHandle = NULL;

spi_slave_transaction_t snes_slave_transaction;

uint8_t snes_lower = 0xFF;
uint8_t snes_upper = 0xFF;
uint8_t snes_buffer[2] = {0xFF, 0xFF};

void spi_dummy_cb(spi_slave_transaction_t *trans)
{

}

void snes_task(void * parameters)
{
    for(;;)
    {
        esp_err_t err;

        // SNES bits button order
        // B, Y, Start, Select, Up, Down, Left, Right, A, X, L, R
        // Ordered starting from bit 0 with B up to bit 11 with R.
        // SNES/NES buttons are HIGH when not pressed, LOW when pressed.
        // We subtract the button value as it's inverted. Unpressed trailing bits need to
        // remain HIGH as well.

        // Reset button output to all HIGH bits.
        snes_lower = 0xFF;
        snes_upper = 0xFF;

        // Go through each bit and set accordingly.
        snes_lower -= (hoja_button_data.button_down)  << 7U;
        snes_lower -= (hoja_button_data.button_left)  << 6U;
        snes_lower -= (hoja_button_data.button_select)<< 5U;
        snes_lower -= (hoja_button_data.button_start) << 4U;
        snes_lower -= (hoja_button_data.dpad_up)      << 3U;
        snes_lower -= (hoja_button_data.dpad_down)    << 2U;
        snes_lower -= (hoja_button_data.dpad_left)    << 1U;
        snes_lower -= (hoja_button_data.dpad_right);

        snes_upper -= (hoja_button_data.button_right) << 7U;
        snes_upper -= (hoja_button_data.button_up)    << 6U;
        snes_upper -= (hoja_button_data.trigger_l | hoja_button_data.trigger_zl)    << 5U;
        snes_upper -= (hoja_button_data.trigger_r | hoja_button_data.trigger_zr)    << 4U;

        //ESP_LOGI("SNES", "%x", (unsigned int) snes_lower);

        // Reset HOJA input buttons for next scan sequence.
        hoja_button_reset();

        snes_buffer[0] = snes_lower;
        snes_buffer[1] = snes_upper;
        snes_slave_transaction.length = 16;
        snes_slave_transaction.trans_len = 16;
        snes_slave_transaction.tx_buffer = snes_buffer;

        spi_slave_transmit(SPI2_HOST, &snes_slave_transaction, portMAX_DELAY);
        vTaskDelay(1/portTICK_PERIOD_MS);
    }
}

hoja_err_t core_snes_start()
{
    const char* TAG = "core_snes_start";

    // Set up SPI
    esp_err_t ret;

    // Configuration for the SPI bus
    spi_bus_config_t buscfg={
        .mosi_io_num    = -1,
        .miso_io_num    = (int) CONFIG_HOJA_GPIO_NS_SERIAL,
        .sclk_io_num    = (int) CONFIG_HOJA_GPIO_NS_CLOCK,
        .quadwp_io_num  = -1,
        .quadhd_io_num  = -1,
    };

    // Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg={
        .mode           = 2,
        .spics_io_num   = (int) CONFIG_HOJA_GPIO_NS_LATCH,
        .queue_size     = 1,
        .flags          = 0,
    };

    // Initialize SPI slave interface
    ret=spi_slave_initialize(SPI2_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
    assert(ret==ESP_OK);
    memset(&snes_slave_transaction, 0, sizeof(snes_slave_transaction));

    // Check if task is running, delete if so.
    if (snes_TaskHandle != NULL)
    {
        vTaskDelete(snes_TaskHandle);
        snes_TaskHandle = NULL;
    }

    // Start SNES task.
    xTaskCreatePinnedToCore(snes_task, "SNES/NES Task Loop", 2024,
                            NULL, 0, &snes_TaskHandle, 0);
    
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
    err = spi_slave_free(SPI2_HOST);
    assert(err==ESP_OK);

    ESP_LOGI(TAG, "SNES Core stopped OK.");

    return HOJA_OK;
}
