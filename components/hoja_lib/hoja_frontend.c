#include "hoja_frontend.h"

uint8_t HOJA_PIN_LATCH = 0xFF;
uint8_t HOJA_PIN_CLOCK = 0xFF;
uint8_t HOJA_PIN_SERIAL = 0xFF;

SemaphoreHandle_t xSemaphore;

hoja_err_t hoja_api_init()
{
    const char* TAG = "hoja_api_init";
	esp_err_t ret;

    xSemaphore = xSemaphoreCreateMutex();

    if (hoja_params.api_initialized)
    {
        ESP_LOGE(TAG, "API is already running.");
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

    
    hoja_params.api_initialized = true;

    ESP_LOGI(TAG, "HOJA initialized.");

    return HOJA_OK;
}

hoja_err_t hoja_api_setcore(uint8_t core_type)
{
    const char* TAG = "hoja_api_setcore";

    if (!hoja_params.api_initialized)
    {
        ESP_LOGE(TAG, "Initialize API before setting the core.");
        return HOJA_FAIL;
    }

    if (hoja_params.core_started)
    {
        ESP_LOGE(TAG, "Core already started. Core must be stopped before changing.");
        return HOJA_FAIL;
    }

    hoja_params.selected_core = core_type;

    ESP_LOGI(TAG, "HOJA Core set.");
    return HOJA_OK;
}

hoja_err_t hoja_api_startcore(void)
{
    char* TAG = "hoja_api_startcore";

    if (!hoja_params.api_initialized)
    {
        ESP_LOGE(TAG, "Initialize API and set core before starting the controller.");
        return HOJA_FAIL;
    }

    switch(hoja_params.selected_core)
    {
        case CORE_NINTENDOSWITCH:
            ESP_LOGI(TAG, "Attempting Nintendo Switch Core start...");
            core_ns_start();
            break;
        case CORE_SNES:
            ESP_LOGI(TAG, "Attempting SNES/NES Core start...");
            core_snes_start();
            break;
        default:
            // No core matches!
            ESP_LOGI(TAG, "Specified core does not exist.");
            return HOJA_FAIL;
            break;
    }

    return HOJA_OK;
}

// Pointer to hold callback
hoja_callback_t hoja_button_cb;
hoja_callback_t hoja_stick_cb;
hoja_callback_t hoja_rgb_cb;

// Function to register callback function for setting buttons
hoja_err_t hoja_api_regbuttoncallback(hoja_callback_t func)
{
    if (func == NULL) return HOJA_FAIL;

    hoja_button_cb = func;

    return HOJA_OK;
}

// Function to register callback function for setting sticks
hoja_err_t hoja_api_regstickcallback(hoja_callback_t func)
{
    if (func == NULL) return HOJA_FAIL;

    hoja_stick_cb = func;

    return HOJA_OK;
}

hoja_err_t hoja_api_regrgbcallback(hoja_callback_t func)
{
    if (func == NULL) return HOJA_FAIL;

    hoja_rgb_cb = func;

    return HOJA_OK;
}

