#include "util_battery.h"

util_battery_status_t util_battery_getstatus(util_battery_field_t field)
{
    const char* TAG = "util_battery_getstatus";

    if (util_i2c_status != UTIL_I2C_STATUS_AVAILABLE)
    {
        ESP_LOGE(TAG, "Must init I2C before getting battery status.");
        return BATSTATUS_EMPTY;
    }

    ESP_LOGI(TAG, "Battery utility get status.");
    esp_err_t err = ESP_OK;
    util_battery_status_t ret = BATSTATUS_EMPTY;

    // BUILD BATTERY READ COMMAND
    i2c_cmd_handle_t tmpcmd = i2c_cmd_link_create();
    uint8_t response[1] = {0};

    i2c_master_start(tmpcmd);
    i2c_master_write_byte(tmpcmd, (UTIL_BATTERY_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(tmpcmd, 0x0, true);
    i2c_master_start(tmpcmd);
    i2c_master_write_byte(tmpcmd, (UTIL_BATTERY_I2C_ADDRESS << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(tmpcmd, &response[0], I2C_MASTER_LAST_NACK);
    //i2c_master_read_byte(tmpcmd, &response[1], I2C_MASTER_LAST_NACK);
    i2c_master_stop(tmpcmd);

    // START READ
    err = i2c_master_cmd_begin(I2C_NUM_0, tmpcmd, 1000/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(tmpcmd);
    
    // CHECK IF BATTERY CHARGE STATUS READ WENT OK
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Battery Status Read: Transmit Fail.");
        ESP_LOGE(esp_err_to_name(err), "");
        return BATSTATUS_EMPTY;
    }
    
    // Parse out response data
    uint8_t chg_stat = (response[0] >> 5) & 0x3;

    if (field == BATFIELD_CHGSTAT)
    {
        switch(chg_stat)
        {
            default:
            case 0x00:
                ret = BATSTATUS_NOTCHARGING;
                break;
            case 0x01:
                ret = BATSTATUS_TRICKLEFAST;
                break;
            case 0x02:
                ret = BATSTATUS_CONSTANT;
                break;
            case 0x03:
                ret = BATSTATUS_COMPLETED;
                break;
        }
    }
    else if (field == BATFIELD_VGOOD)
    {
        if (response[0] & 0x1)
        {
            ret = BATSTATUS_VGOOD;
        }
        else 
        {
            ret = BATSTATUS_VNOTGOOD;
        }
    }

    return ret;
}