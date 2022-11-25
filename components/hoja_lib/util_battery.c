#include "util_battery.h"

uint8_t current_battery_status = 0x00;

uint8_t util_battery_getstatus()
{
    const char* TAG = "util_battery_getstatus";
    uint8_t ret = current_battery_status;

    if (util_i2c_status != UTIL_I2C_STATUS_AVAILABLE)
    {
        ESP_LOGE(TAG, "Must init I2C before getting battery status.");
        return ret;
    }

    esp_err_t err = ESP_OK;
    

    // BUILD BATTERY READ COMMAND
    i2c_cmd_handle_t tmpcmd = i2c_cmd_link_create();
    uint8_t response[1] = {0};

    i2c_master_start(tmpcmd);
    i2c_master_write_byte(tmpcmd, (UTIL_BATTERY_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(tmpcmd, 0x0, true);
    i2c_master_start(tmpcmd);
    i2c_master_write_byte(tmpcmd, (UTIL_BATTERY_I2C_ADDRESS << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(tmpcmd, &response[0], I2C_MASTER_LAST_NACK);
    i2c_master_stop(tmpcmd);

    // START READ
    err = i2c_master_cmd_begin(I2C_NUM_0, tmpcmd, 1000/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(tmpcmd);
    
    // CHECK IF BATTERY CHARGE STATUS READ WENT OK
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Battery Status Read: Transmit Fail.");
        ESP_LOGE(esp_err_to_name(err), "");
        return ret;
    }
    ret = response[0];
    current_battery_status = response[0];

    ESP_LOGI(TAG, "Battery Status: %X", ret);
    return ret;
}

void util_battery_write(uint8_t offset, uint8_t byte)
{
    const char* TAG = "util_battery_write";
    uint8_t ret = 0x00;

    if (util_i2c_status != UTIL_I2C_STATUS_AVAILABLE)
    {
        ESP_LOGE(TAG, "Must init I2C before writing battery register.");
        return;
    }

    uint8_t buffer[2] = {offset, byte};

    esp_err_t err = ESP_OK;
    // Set up and send data over I2C
    i2c_cmd_handle_t tmpcmd = i2c_cmd_link_create();

    // Build the i2c packet
    // to send the USB start command
    i2c_master_start(tmpcmd);
    i2c_master_write_byte(tmpcmd, (UTIL_BATTERY_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(tmpcmd, buffer, 2, true);
    i2c_master_stop(tmpcmd);

    err = i2c_master_cmd_begin(I2C_NUM_0, tmpcmd, portMAX_DELAY);

    i2c_cmd_link_delete(tmpcmd);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to send I2C command to battery:");
        ESP_LOGE(esp_err_to_name(err), "");
    }
}
