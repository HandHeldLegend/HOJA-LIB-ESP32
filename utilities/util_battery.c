#include "util_battery.h"

// Private variables
util_battery_type_t _current_battery_type = BATTYPE_UNDEFINED;
util_battery_status_t _current_battery_status = BATSTATUS_UNDEFINED;
util_battery_plugged_t _current_battery_plugged = BATCABLE_UNDEFINED;

TaskHandle_t util_battery_monitor_taskhandle = NULL;

// Private functions
hoja_err_t util_battery_get_status_byte(uint8_t *status_byte)
{
    const char* TAG = "util_battery_get_status_byte";

    // Check our current battery type
    if (_current_battery_type == BATTYPE_BQ25180)
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
    if (_current_battery_type == BATTYPE_BQ25180)
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

        if (e == HOJA_OK)
        {
            util_battery_status_s s = {0};
            s.status = status_byte;

            // Prioritize charger plug/unplug status.
            if (_current_battery_plugged != s.plug_status)
            {
                _current_battery_plugged = s.plug_status;
                switch(_current_battery_plugged)
                {
                    default:
                    case BATCABLE_UNPLUGGED:
                    hoja_set_external_power(false);
                    hoja_event_cb(HOJA_EVT_CHARGER, HEVT_CHARGER_UNPLUGGED, 0x00);
                    break;
                    case BATCABLE_PLUGGED:
                    hoja_set_external_power(true);
                    hoja_event_cb(HOJA_EVT_CHARGER, HEVT_CHARGER_PLUGGED, 0x00);
                    break;
                }
            }

            // Check if charging status changed if we're plugged in.
            if (current_battery_status != s.charge_status)
            {
                _current_battery_status = s.charge_status;
                switch(_current_battery_status)
                {
                    default:
                    case BATSTATUS_NOTCHARGING:
                    hoja_event_cb(HOJA_EVT_BATTERY, HEVT_BATTERY_NOCHARGE, 0x00);
                    break;
                    case BATSTATUS_TRICKLEFAST:
                    case BATSTATUS_CONSTANT:
                    hoja_event_cb(HOJA_EVT_BATTERY, HEVT_BATTERY_CHARGING, 0x00);
                    break;
                    case BATSTATUS_COMPLETED:
                    hoja_event_cb(HOJA_EVT_BATTERY, HEVT_BATTERY_CHARGECOMPLETE, 0x00);
                    break;
                }
            }

            // Refresh every 1 second
            vTaskDelay(1000/portTICK_PERIOD_MS);
        }
    }
}


// Public functions

util_battery_status_t util_get_battery_charging_status(void)
{
    return current_battery_status;
}

/**
 * @brief Returns a bool indicating whether external power is connected.
 * 
*/
bool util_battery_external_power(void)
{
    if (current_battery_plugged == BATCABLE_PLUGGED)
    {
        return true;
    }
    return false;
}



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
            _current_battery_type = type;
            break;
    }
    return HOJA_OK;
}

/**
 * @brief Upon boot, get the battery status and send a HOJA boot event callback.
 * 
*/
hoja_err_t util_battery_boot_status(void)
{
    const char* TAG = "util_battery_boot_status";
    ESP_LOGI(TAG, "Getting boot status.");

    if (_current_battery_type == BATTYPE_UNDEFINED)
    {
        ESP_LOGI(TAG, "Must set battery type before calling this.");
        return HOJA_FAIL;
    }

    if ((_current_battery_status != BATSTATUS_UNDEFINED) || (_current_battery_plugged != BATCABLE_UNDEFINED))
    {
        ESP_LOGI(TAG, "Boot event can only be called once per power cycle.");
        return HOJA_FAIL;
    }

    uint8_t status = 0x00;
    util_battery_status_s batstat = {0};
    hoja_err_t err = util_battery_get_status_byte(&status);
    batstat.status = status;

    _current_battery_status  = batstat.charge_status;
    _current_battery_plugged = batstat.plug_status;

    if (err == HOJA_I2C_FAIL)
    {
        hoja_event_cb(HOJA_EVT_BOOT, HEVT_BOOT_NOBATTERY, 0x00);
        return HOJA_OK;
    }

    switch(_current_battery_plugged)
    {
        default:
        case BATCABLE_PLUGGED:
            hoja_set_external_power(true);
            hoja_event_cb(HOJA_EVT_BOOT, HEVT_BOOT_PLUGGED, 0x00);
            break;
        case BATCABLE_UNPLUGGED:
            hoja_set_external_power(false);
            if (hoja_get_force_wired())
            {
                hoja_event_cb(HOJA_EVT_BOOT, HEVT_BOOT_PLUGGED, 0x00);
            }
            else
            {
                hoja_event_cb(HOJA_EVT_BOOT, HEVT_BOOT_UNPLUGGED, 0x00);
            }
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

    if (_current_battery_type == BATTYPE_UNDEFINED)
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
 * @brief Returns whether or not the battery is charging currently.
 * Value of 1 means it is charging.
 * Value of 2 means it is done charging.
*/
uint8_t util_battery_is_charging(void)
{
    switch(_current_battery_status)
    {
        default:
        case BATSTATUS_UNDEFINED:
        case BATSTATUS_NOTCHARGING:
            return 0;
            break;

        case BATSTATUS_TRICKLEFAST:
        case BATSTATUS_CONSTANT:
            return 1;
            break;

        case BATSTATUS_COMPLETED:
            return 2;
            break;
    }
}

/**
 * @brief Returns whether or not the PMIC is connected currently.
*/
bool util_battery_is_connected(void)
{
    switch (_current_battery_plugged)
    {
        default:
        case BATCABLE_UNDEFINED:
        case BATCABLE_UNPLUGGED:
            return false;
            break;

        case BATCABLE_PLUGGED:
            return true;
            break;
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

    if (_current_battery_type == BATTYPE_BQ25180)
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

    if (_current_battery_type == BATTYPE_BQ25180)
    {
        util_battery_write(0x9, 0x41);
    }
    else
    {
        ESP_LOGE(TAG, "Invalid battery type.");
    }
}
