#include "cores/core_snes_backend.h"

uint16_t snes_button_buffer = 0xFFFF;
TaskHandle_t snes_TaskHandle = NULL;



uint8_t snes_lower = 0xFF;
uint8_t snes_upper = 0xFF;

void spi_dummy_cb(spi_slave_transaction_t *trans)
{

}

void snes_task(void * parameters)
{
    esp_err_t err;
    for(;;)
    {
        // SNES bits button order
        // B, Y, Start, Select, Up, Down, Left, Right, A, X, L, R
        // Ordered starting from bit 0 with B up to bit 11 with R.
        // SNES/NES buttons are HIGH when not pressed, LOW when pressed.
        // We subtract the button value as it's inverted. Unpressed trailing bits need to
        // remain HIGH as well.

        // Reset button output to all HIGH bits.
        snes_lower = 0;
        snes_upper = 0;

        // Go through each bit and set accordingly.
        snes_lower |= (hoja_button_data.button_down)  << 7U;
        snes_lower |= (hoja_button_data.button_left)  << 6U;
        snes_lower |= (hoja_button_data.button_select)<< 5U;
        snes_lower |= (hoja_button_data.button_start) << 4U;
        snes_lower |= (hoja_button_data.dpad_up)      << 3U;
        snes_lower |= (hoja_button_data.dpad_down)    << 2U;
        snes_lower |= (hoja_button_data.dpad_left)    << 1U;
        snes_lower |= (hoja_button_data.dpad_right);

        snes_upper |= (hoja_button_data.button_right) << 7U;
        snes_upper |= (hoja_button_data.button_up)    << 6U;
        snes_upper |= (hoja_button_data.trigger_l | hoja_button_data.trigger_zl)    << 5U;
        snes_upper |= (hoja_button_data.trigger_r | hoja_button_data.trigger_zr)    << 4U;

        //snes_upper |= 1;

        //ESP_LOGI("SNES", "%x", (unsigned int) snes_lower);

        // Reset HOJA input buttons for next scan sequence.
        hoja_button_reset();

        WORD_ALIGNED_ATTR uint8_t snes_buffer[2] = {snes_lower, snes_upper};

        spi_slave_transaction_t snes_slave_transaction = {
            .tx_buffer  = snes_buffer,
            .length     = 8 * sizeof(snes_buffer),
        };

        err = spi_slave_transmit(HSPI_HOST, &snes_slave_transaction, portMAX_DELAY);
        if (err != ESP_OK)
        {
            ESP_LOGE("snes_task", "Fail to send SPI");
        }
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
        .max_transfer_sz = 4,
    };

    // Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg={
        .mode           = 2,
        .spics_io_num   = (int) CONFIG_HOJA_GPIO_NS_LATCH,
        .queue_size     = 1,
        .flags          = 0,
    };

    // Initialize SPI slave interface
    ret=spi_slave_initialize(HSPI_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
    assert(ret==ESP_OK);

    gpio_matrix_in(CONFIG_HOJA_GPIO_NS_CLOCK, HSPICLK_IN_IDX, true);
    

    //gpio_set_direction(CONFIG_HOJA_GPIO_NS_SERIAL, GPIO_MODE_OUTPUT);
    gpio_set_level(CONFIG_HOJA_GPIO_NS_SERIAL, 0);
    esp_rom_gpio_connect_out_signal(CONFIG_HOJA_GPIO_NS_SERIAL, HSPIQ_OUT_IDX, true, false);
    esp_rom_gpio_connect_in_signal(CONFIG_HOJA_GPIO_NS_SERIAL, HSPIQ_IN_IDX, true);
    

    // Check if task is running, delete if so.
    if (snes_TaskHandle != NULL)
    {
        vTaskDelete(snes_TaskHandle);
        snes_TaskHandle = NULL;
    }

    // Start SNES task.
    xTaskCreatePinnedToCore(snes_task, "SNES/NES Task Loop", 4048,
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
    err = spi_slave_free(SPI2_HOST);
    assert(err==ESP_OK);

    ESP_LOGI(TAG, "SNES Core stopped OK.");

    return HOJA_OK;
}
