#include "util_i2c.h"

util_i2c_status_t util_i2c_status = UTIL_I2C_STATUS_DISABLED;

hoja_err_t util_i2c_initialize(void)
{
    const char* TAG = "util_i2c_initialize";

    #if CONIFG_HOJA_I2C_ENABLE
    ESP_LOGI(TAG, "I2C Utility Start.");
    esp_err_t err = ESP_OK;

    if (util_i2c_status == UTIL_I2C_STATUS_AVAILABLE)
    {
        ESP_LOGI(TAG, "I2C Utility attempted initialize when already started!");
        err = i2c_driver_delete(I2C_NUM_0);

        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to delete I2C driver that's already running. Fail return.");
            return HOJA_FAIL;
        }
        ESP_LOGI(TAG, "Stopped running I2C utility to start it fresh.");
    }

    // Set up I2C peripheral.
	i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = CONFIG_HOJA_GPIO_I2C_SDA;
    conf.scl_io_num = CONFIG_HOJA_GPIO_I2C_SCL;
    // Use external pull-up resistors according to I2C specifications.
    conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
    conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
    // Max clock speed for ESP32 I2C.
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(I2C_NUM_0, &conf);
    err = i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start I2C Bus. Check configuration and pins.");
        util_i2c_status = UTIL_I2C_STATUS_DISABLED;
        return HOJA_FAIL;
    }
    ESP_LOGI(TAG, "I2C peripheral set up OK.");
    util_i2c_status = UTIL_I2C_STATUS_AVAILABLE;
    return HOJA_OK;
    #else

    ESP_LOGE(TAG, "I2C utility is disabled. Enable it in SDK settings for HOJA component.");
    return HOJA_FAIL;
    
    #endif
}