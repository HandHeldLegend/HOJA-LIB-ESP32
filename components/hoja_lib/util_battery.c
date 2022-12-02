#include "util_battery.h"

// Private variables
util_battery_type_t current_battery_type = BATTYPE_UNDEFINED;
util_battery_status_t current_battery_status = BATSTATUS_UNDEFINED;
util_battery_plugged_t current_battery_plugged = BATCABLE_UNDEFINED;

TaskHandle_t util_battery_monitor_taskhandle = NULL;

// Private functions
hoja_err_t util_battery_get_status_byte(uint8_t *status_byte)
{
    const char* TAG = "util_battery_get_status_byte";

    // Check our current battery type
    if (current_battery_type == BATTYPE_BQ25180)
    {
        if (util_i2c_status != UTIL_I2C_STATUS_AVAILABLE)
        {
            ESP_LOGE(TAG, "Must init I2C before getting battery status.");
            return HOJA_I2C_NOTINIT;
        }

        esp_err_t err = ESP_OK;
        

        // BUILD BATTERY READ COMMAND
        i2c_cmd_handle_t tmpcmd = i2c_cmd_link_create();
        uint8_t response[1] = {0};

        i2c_master_start(tmpcmd);
        i2c_master_write_byte(tmpcmd, (BATTYPE_BQ25180 << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(tmpcmd, 0x0, true);
        i2c_master_start(tmpcmd);
        i2c_master_write_byte(tmpcmd, (BATTYPE_BQ25180 << 1) | I2C_MASTER_READ, true);
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
            return HOJA_I2C_FAIL;
        }
        *status_byte = response[0];
    }
    else
    {
        return HOJA_BATTERY_NOTSET;
    }
    
    return HOJA_OK;
}

hoja_err_t util_battery_write(uint8_t offset, uint8_t byte)
{
    const char* TAG = "util_battery_write";
    uint8_t ret = 0x00;

    // Write to battery PMIC for BQ25180
    if (current_battery_type == BATTYPE_BQ25180)
    {
        if (util_i2c_status != UTIL_I2C_STATUS_AVAILABLE)
        {
            ESP_LOGE(TAG, "Must init I2C before writing battery register.");
            return HOJA_I2C_NOTINIT;
        }

        uint8_t buffer[2] = {offset, byte};

        esp_err_t err = ESP_OK;
        // Set up and send data over I2C
        i2c_cmd_handle_t tmpcmd = i2c_cmd_link_create();

        // Build the i2c packet
        // to send the USB start command
        i2c_master_start(tmpcmd);
        i2c_master_write_byte(tmpcmd, (BATTYPE_BQ25180 << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write(tmpcmd, buffer, 2, true);
        i2c_master_stop(tmpcmd);

        err = i2c_master_cmd_begin(I2C_NUM_0, tmpcmd, portMAX_DELAY);

        i2c_cmd_link_delete(tmpcmd);

        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to send I2C command to battery:");
            ESP_LOGE(esp_err_to_name(err), "");
            return HOJA_I2C_FAIL;
        }
    }
    else
    {
        ESP_LOGE(TAG, "Invalid battery type.");
        return HOJA_BATTERY_NOTSET;
    }
    

    return HOJA_OK;
}

void util_battery_monitor_task(void * params)
{
    const char* TAG = "util_battery_monitor_task";
    uint8_t status_byte = 0x00;

    ESP_LOGI(TAG, "Starting monitor task loop...");
    for(;;)
    {
        
        hoja_err_t e = util_battery_get_status_byte(&status_byte);

        if (e != HOJA_OK)
        {
            if (e == HOJA_I2C_FAIL || e == HOJA_I2C_NOTINIT)
            {
                hoja_event_cb(HOJA_EVT_BATTERY, BATTERY_NO_COMMUNICATION, false);
            }
        }
        else
        {
            util_battery_status_s s = {0};
            s.status = status_byte;
            bool stat_change = true;

            if (current_battery_status == BATSTATUS_UNDEFINED || current_battery_plugged == BATCABLE_UNDEFINED)
            {
                stat_change = false;
            }

            if (current_battery_plugged != s.plug_status)
            {
                current_battery_plugged = s.plug_status;
                switch(current_battery_plugged)
                {
                    default:
                    case BATCABLE_UNPLUGGED:
                    hoja_event_cb(HOJA_EVT_BATTERY, BATTERY_CHARGER_DISCONNECT, stat_change);
                    break;
                    case BATCABLE_PLUGGED:
                    hoja_event_cb(HOJA_EVT_BATTERY, BATTERY_CHARGER_PLUGGED, stat_change);
                    break;
                }
            }
            else if ( (current_battery_status != s.charge_status) && (current_battery_plugged == BATCABLE_PLUGGED) )
            {
                current_battery_status = s.charge_status;
                switch(current_battery_status)
                {
                    default:
                    case BATSTATUS_NOTCHARGING:
                    hoja_event_cb(HOJA_EVT_BATTERY, BATTERY_NOT_CHARGING, stat_change);
                    break;
                    case BATSTATUS_TRICKLEFAST:
                    case BATSTATUS_CONSTANT:
                    hoja_event_cb(HOJA_EVT_BATTERY, BATTERY_CHARGING_PROGRESS, stat_change);
                    break;
                    case BATSTATUS_COMPLETED:
                    hoja_event_cb(HOJA_EVT_BATTERY, BATTERY_CHARGING_COMPLETE, stat_change);
                    break;
                }
            }

            // To-Do, perform a measurement of battery voltage and call back if threshold change is met.

            // Refresh every 1 second
            vTaskDelay(1000/portTICK_PERIOD_MS);
        }
    }
}


// Public functions

/**
 * @brief Set the type of battery so the API knows how to interface with
 * the battery PMIC.
 * @param type Type of util_battery_type_t
*/
hoja_err_t util_battery_set_type(util_battery_type_t type)
{
    if (type == NULL)
    {
        ESP_LOGE("util_battery_settype", "No battery type set.");
        return HOJA_BATTERY_NOTSET;
    }
    switch(type)
    {
        default:
            ESP_LOGE("util_battery_settype", "Invalid battery type.");
            return HOJA_FAIL;
            break;
        case BATTYPE_BQ25180:
            current_battery_type = type;
            break;
    }
    return HOJA_OK;
}

/**
 * @brief Start the battery monitor. This will send events to hoja_event_cb when
 * changes occur in status. Battery status will update every 1 second.
*/
hoja_err_t util_battery_start_monitor(void)
{
    const char* TAG = "util_battery_star_monitor";

    if (util_battery_monitor_taskhandle != NULL)
    {
        vTaskDelete(util_battery_monitor_taskhandle);
        util_battery_monitor_taskhandle = NULL;
    }

    if (current_battery_type == BATTYPE_UNDEFINED)
    {
        ESP_LOGE(TAG, "Must set battery type (util_battery_set_type) before starting monitor!");
        return HOJA_FAIL;
    }

    ESP_LOGI(TAG, "Battery monitor STARTED.");
    xTaskCreatePinnedToCore(util_battery_monitor_task, "Battery Monitor Task", 3000, NULL, 2, &util_battery_monitor_taskhandle, 1);

    return HOJA_OK;
}

/**
 * @brief Stop the battery monitor task.
*/
void util_battery_stop_monitor(void)
{
    const char* TAG = "util_battery_stop_monitor";

    if (util_battery_monitor_taskhandle != NULL)
    {
        ESP_LOGI(TAG, "battery monitor STOPPED.");
        vTaskDelete(util_battery_monitor_taskhandle);
        util_battery_monitor_taskhandle = NULL;
    }
}

/**
 * @brief Set the PMIC charge rate in mA
 * 
 * @param rate_ma Rate of charging in mA
 */
hoja_err_t util_battery_set_charge_rate(uint16_t rate_ma)
{
    const char* TAG = "util_battery_set_charge_rate";

    if (current_battery_type == BATTYPE_BQ25180)
    {
        // Default is 0x5
        uint8_t code = 0x5;
        uint16_t rate = rate_ma;
        if (rate_ma <= 35)
        {
            // CODE + 5 = rate
            code = rate - 5;
        }
        else
        {
            if (rate > 350)
            {
                ESP_LOGE(TAG, "Capped charging rate at 350mA!");
                rate = 350;
            }
            // 40 + ((CODE-31)*10) = rate
            code = ((rate - 40)/10) + 31;
        }

        // Mask off the code to ensure charging isn't disabled
        code &= 0x7F;

        // Set battery charge rate to 200mA
        hoja_err_t e = util_battery_write(0x4, code);
    }
    else
    {
        ESP_LOGE(TAG, "Invalid battery type.");
        return HOJA_BATTERY_NOTSET;
    }

    return HOJA_OK;
}

/**
 *  @brief Trigger PMIC Ship Mode. It will cut power to output from PMIC until it is
 *  awakened by plugging in a charger or holding down the wake button.
 */
void util_battery_enable_ship_mode(void)
{
    const char* TAG = "util_battery_enable_ship_mode";

    if (current_battery_type == BATTYPE_BQ25180)
    {
        util_battery_write(0x9, 0x41);
    }
    else
    {
        ESP_LOGE(TAG, "Invalid battery type.");
    }
}
