#include "hoja_frontend.h"

// Private variables
TaskHandle_t hoja_button_taskhandle = NULL;
hoja_core_t hoja_current_core = HOJA_CORE_NULL;
hoja_status_t hoja_current_status = HOJA_STATUS_IDLE;
bool _hoja_force_wired = false;

/**
 * @brief Initialize HOJA library. Register any callbacks before calling this function.
*/
hoja_err_t hoja_init()
{
    const char* TAG = "hoja_init";
	esp_err_t ret;

    if (hoja_current_status == HOJA_STATUS_INITIALIZED)
    {
        ESP_LOGE(TAG, "API is already initialized.");
        return HOJA_FAIL;
    }

    if (hoja_current_status == HOJA_STATUS_RUNNING)
    {
        ESP_LOGE(TAG, "API is running a core already.");
        return HOJA_FAIL;
    }

    bool fail = false;
    if (hoja_button_cb == NULL)
    {
        fail = true;
        ESP_LOGE(TAG, "hoja_button_cb is not registered!");
    }
    if (hoja_analog_cb == NULL)
    {
        fail = true;
        ESP_LOGE(TAG, "hoja_analog_cb is not registered!");
    }
    if (hoja_event_cb == NULL)
    {
        fail = true;
        ESP_LOGE(TAG, "hoja_event_cb is not registered!");
    }
    if (fail)
    {
        return HOJA_FAIL;
    }


    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    if (hoja_settings_init() != HOJA_OK)
    {
        ESP_LOGE(TAG, "Settings failed to load.");
        return HOJA_FAIL;
    }

    if (hoja_button_taskhandle != NULL)
    {
        vTaskDelete(hoja_button_taskhandle);
    }
    
    // We need to start the button scan task.
    xTaskCreatePinnedToCore(hoja_button_task, "HOJA Button Task", 2048, NULL, 0, &hoja_button_taskhandle, HOJA_INPUT_CPU);
    
    hoja_current_status = HOJA_STATUS_INITIALIZED;

    ESP_LOGI(TAG, "HOJA initialized.");

    hoja_event_cb(HOJA_EVT_SYSTEM, HEVT_API_INIT_OK, 0x00);

    return HOJA_OK;
}

/**
 * @brief Set the core used by HOJA.
 * @param core Type of hoja_core_t
*/
hoja_err_t hoja_set_core(hoja_core_t core)
{
    const char* TAG = "hoja_set_core";

    if (core >= HOJA_CORE_MAX)
    {
        ESP_LOGE(TAG, "Core type is invalid!");
        return HOJA_FAIL;
    }

    if (!core)
    {
        ESP_LOGE(TAG, "Core type is null!");
        return HOJA_FAIL;
    }

    if (hoja_current_status == HOJA_STATUS_RUNNING)
    {
        ESP_LOGE(TAG, "Cannot set core while another core is running!");
        return HOJA_FAIL;
    }

    hoja_current_core = core;

    ESP_LOGI(TAG, "HOJA Core set.");
    return HOJA_OK;
}

/**
 * @brief Attempt to start the core specified by hoja_set_core
*/
hoja_err_t hoja_start_core(void)
{
    char* TAG = "hoja_start_core";
    hoja_err_t err = HOJA_FAIL;

    switch(hoja_current_core)
    {
        case HOJA_CORE_NS:
            ESP_LOGI(TAG, "Attempting Nintendo Switch Core start...");
            err = core_ns_start();
            break;
        case HOJA_CORE_SNES:
            ESP_LOGI(TAG, "Attempting SNES/NES Core start...");
            err = core_snes_start();
            break;
        case HOJA_CORE_N64:
            ESP_LOGI(TAG, "Attempting N64 Core start...");
            err = core_joybus_n64_coldboot();
            break;
        case HOJA_CORE_GAMECUBE:
            ESP_LOGI(TAG, "Attempting GameCube Core start...");
            err = core_joybus_gamecube_start();
            break;
        case HOJA_CORE_USB:
            ESP_LOGI(TAG, "Attempting USB Core start...");
            err = core_usb_start();
            break;
        case HOJA_CORE_BT_DINPUT:
            ESP_LOGI(TAG, "Attempting BT DInput Core start...");
            err = core_bt_dinput_start();
            break;
        case HOJA_CORE_BT_XINPUT:
            ESP_LOGI(TAG, "Attempting BT XInput Core start...");
            err = core_bt_xinput_start();
            break;
        default:
            // No core matches!
            ESP_LOGE(TAG, "Specified core does not exist or isn't implemented yet.");
            return HOJA_FAIL;
            break;
    }

    if (err == HOJA_OK)
    {
        hoja_current_status = HOJA_STATUS_RUNNING;
    }
    return err;
}

/**
 * @brief Attempt to stop the core that is currently running.
*/
void hoja_stop_core(void)
{
    const char* TAG = "hoja_stop_core";

    if (hoja_current_status != HOJA_STATUS_RUNNING)
    {
        ESP_LOGE(TAG, "Core needs to be running before you can stop it!");
        return;
    }

    switch(hoja_current_core)
    {
        case HOJA_CORE_NS:
            ESP_LOGI(TAG, "Attempting Nintendo Switch Core stop...");
            core_ns_stop();
            break;
        case HOJA_CORE_SNES:
            ESP_LOGI(TAG, "Attempting SNES/NES Core stop...");
            core_snes_stop();
            break;
        case HOJA_CORE_N64:
        case HOJA_CORE_GAMECUBE:
            ESP_LOGI(TAG, "Attempting Joybus Core stop...");
            core_joybus_stop();
            break;

        case HOJA_CORE_BT_DINPUT:
            ESP_LOGI(TAG, "Attempting BT Dinput Core stop...");
            core_bt_dinput_stop();
            break;

        case HOJA_CORE_BT_XINPUT:
            ESP_LOGI(TAG, "Attempting BT Xinput Core stop...");
            core_bt_xinput_stop();
            break;

        case HOJA_CORE_USB:
            ESP_LOGI(TAG, "Attempting USB Core stop...");
            core_usb_stop();
            break;
        default:
            // No core matches!
            ESP_LOGE(TAG, "Specified core does not exist or isn't implemented yet.");
            return;
            break;
    }

    hoja_current_status = HOJA_STATUS_INITIALIZED;
}

/**
 * @brief Is called when button task is initiated by the API.
 * 
 * @param *button_data Pointer to global button_data variable. Gets passed to callback function
 * for modification of button inputs. Recommended to perform OR EQUALS on button_data members. Type of hoja_button_data_s
*/
hoja_button_callback_t hoja_button_cb = NULL;

/**
 * @brief Is called when analog reading is initiated by the API.
 * 
 * @param *analog_data Pointer to global analog_data variable. Gets passed to callback function
 * for modification of analog inputs. Recommended to perform EQUALS on analog_data members. Type of hoja_analog_data_s
*/
hoja_analog_callback_t hoja_analog_cb = NULL;

/**
 * @brief Is called when HOJA event is triggered by the API.
 * 
 * @param type Type of hoja_event_type_t
 * @param evt Event that occurred. See hoja_types.h for details on types for each category.
 * @param param uint8_t type of data related to the event that occurred.
*/
hoja_event_callback_t hoja_event_cb = NULL;

/**
 * @brief This will force the HOJA api to override the boot event for unplugged.
 * This allows you to boot into wired modes even when you are in a wireless mode.
*/
void hoja_set_force_wired(bool enable)
{
    _hoja_force_wired = enable;
}

/**
 * @brief return the force wired status.
*/
bool hoja_get_force_wired(void)
{
    return _hoja_force_wired;
}

/**
 * @brief Register HOJA callback function for button input.
*/
hoja_err_t hoja_register_button_callback(hoja_button_callback_t func)
{
    if (func == NULL) return HOJA_FAIL;

    hoja_button_cb = func;

    return HOJA_OK;
}

/**
 * @brief Register HOJA callback function for analog input.
*/
hoja_err_t hoja_register_analog_callback(hoja_analog_callback_t func)
{
    if (func == NULL) return HOJA_FAIL;

    hoja_analog_cb = func;

    return HOJA_OK;
}

/**
 * @brief Register HOJA callback function for system, core, and utility events.
*/
hoja_err_t hoja_register_event_callback(hoja_event_callback_t func)
{
    if (func == NULL) return HOJA_FAIL;

    hoja_event_cb = func;

    return HOJA_OK;
}
