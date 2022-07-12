#include "retroblue_api.h"


uint8_t RB_PIN_LATCH = 0xFF;
uint8_t RB_PIN_CLOCK = 0xFF;
uint8_t RB_PIN_SERIAL = 0xFF;

SemaphoreHandle_t xSemaphore;

rb_err_t rb_api_init()
{
    const char* TAG = "rb_api_init";
	esp_err_t ret;

    xSemaphore = xSemaphoreCreateMutex();

    if (rb_params.api_initialized)
    {
        ESP_LOGE(TAG, "API is already running.");
        return RB_FAIL;
    }

    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    if (!rb_settings_init())
    {
        ESP_LOGE(TAG, "Settings failed to load.");
        return RB_FAIL;
    }

    
    rb_params.api_initialized = true;

    ESP_LOGI(TAG, "Retroblue API initialized.");

    return RB_OK;
}

rb_err_t rb_api_setCore(uint8_t core_type)
{
    const char* TAG = "rb_api_setCore";

    if (!rb_params.api_initialized)
    {
        ESP_LOGE(TAG, "Initialize API before setting the core.");
        return RB_FAIL;
    }

    if (rb_params.core_started)
    {
        ESP_LOGE(TAG, "Core already started. Core must be stopped before changing.");
        return RB_FAIL;
    }

    rb_params.selected_core = core_type;

    ESP_LOGI(TAG, "RetroBlue core set.");
    return RB_OK;
}

rb_err_t rb_api_startController(void)
{
    char* TAG = "rb_api_startController";

    if (!rb_params.api_initialized)
    {
        ESP_LOGE(TAG, "Initialize API and set core before starting the controller.");
        return RB_FAIL;
    }

    switch(rb_params.selected_core)
    {
        case CORE_NINTENDOSWITCH:
            ESP_LOGI(TAG, "Attempting Nintendo Switch Core start...");
            rbc_core_ns_start();
            break;
        case CORE_SNES:
            ESP_LOGI(TAG, "Attempting SNES/NES Core start...");
            rbc_core_snes_start();
            break;
        default:
            // No core matches!
            ESP_LOGI(TAG, "Specified core does not exist.");
            return RB_FAIL;
            break;
    }

    return RB_OK;
}

// Pointer to hold callback
input_update_callback rb_button_cb;
input_update_callback rb_stick_cb;

// Function to register callback function for setting buttons
rb_err_t rb_register_button_callback(input_update_callback func)
{
    if (func == NULL) return RB_FAIL;

    rb_button_cb = func;

    return RB_OK;
}

// Function to register callback function for setting sticks
rb_err_t rb_register_stick_callback(input_update_callback func)
{
    if (func == NULL) return RB_FAIL;

    rb_stick_cb = func;

    return RB_OK;
}

