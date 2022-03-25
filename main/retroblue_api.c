#include "retroblue_api.h"

rb_err_t rb_api_init()
{
    const char* TAG = "rb_api_init";
	esp_err_t ret;

    if (rb_params.api_initialized)
    {
        return RB_ERR_RUNNING;
    }

    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    rb_settings_init();

    ESP_LOGI(TAG, "Retroblue API initialized.");
    rb_params.api_initialized = true;

    return RB_OK;
}

rb_err_t rb_api_setCore(uint8_t core_type)
{
    const char* TAG = "rb_api_setCore";

    if (!rb_params.api_initialized)
    {
        ESP_LOGE(TAG, "Initialize API before setting the core.");
        return RB_ERR_RUNNING;
    }

    if (rb_params.core_started)
    {
        ESP_LOGE(TAG, "Core already started. Core must be stopped before changing.");
        return RB_ERR_RUNNING;
    }

    rb_params.selected_core = core_type;

    return RB_OK;
}

rb_err_t rb_api_startController(void)
{
    char* TAG = "rb_api_startController";

    if (!rb_params.api_initialized)
    {
        ESP_LOGE(TAG, "Initialize API and set core before starting the controller.");
        return RB_ERR_RUNNING;
    }

    switch(rb_params.selected_core)
    {
        case CORE_NINTENDOSWITCH:
            ESP_LOGI(TAG, "Nintendo Switch Core starting...");
            rbc_core_ns_start();
            break;

        default:
            // No core matches!
            ESP_LOGE(TAG, "Core unspecified error.");
            return RB_ERR_UNSPECIFIEDCORE;
            break;
    }

    return RB_OK;
}

