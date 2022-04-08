#include "retroblue_settings.h"

RetroBlueSettings loaded_settings = {};

void rb_settings_init(void)
{
    nvs_handle_t my_handle;
    const char* TAG = "rb_settings_init";
    esp_err_t err;

    // Check if we have the magic byte

    // Open storage
    err = nvs_open(SETTINGS_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return;

    size_t required_size = 0;
    err = nvs_get_blob(my_handle, "rb_settings", NULL, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return;

    ESP_LOGI(TAG, "Required size: %d", required_size);

    if (required_size > 0)
    {
        err = nvs_get_blob(my_handle, "rb_settings", &loaded_settings, &required_size);
        if (err != ESP_OK)
        {   
            ESP_LOGE(TAG, "Could not load settings. 0x%08X", loaded_settings.magic_bytes);
            return;
        }
        if (loaded_settings.magic_bytes != SETTINGS_MAGIC)
        {
            ESP_LOGI(TAG, "Settings magic bytes don't match.");
            nvs_close(my_handle);
            rb_settings_default();
        }
        else
        {
            ESP_LOGI(TAG, "Settings loaded with magic byte 0x%08X", loaded_settings.magic_bytes);
            nvs_close(my_handle);
        }
        
    }
    else
    {
        ESP_LOGI(TAG, "Settings unset. Set defaults...");
        // We need to set/save all the appropriate settings.
        nvs_close(my_handle);
        rb_settings_default();
    }

}

// Save all settings
void rb_settings_saveall(void)
{
    nvs_handle_t my_handle;
    esp_err_t err;
    // Open
    err = nvs_open(SETTINGS_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return;

    nvs_set_blob(my_handle, "rb_settings", &loaded_settings, sizeof(loaded_settings));

    nvs_commit(my_handle);
    nvs_close(my_handle);
}

// Sets all settings to default.
void rb_settings_default(void)
{
    const char* TAG = "rb_set_defaults";
    nvs_handle_t my_handle;
    esp_err_t err;
    ESP_LOGI(TAG, "Setting defaults...");

    // Open
    err = nvs_open(SETTINGS_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return;

    loaded_settings.magic_bytes = SETTINGS_MAGIC;
    loaded_settings.controller_core = 0;

    // Generate NS core address
    for(int i = 0; i < 8; i++)
    {
        switch(i)
        {
            case 0:
                loaded_settings.ns_client_bt_address[i] = 0x98;
                break;
            case 1:
                loaded_settings.ns_client_bt_address[i] = 0x41;
                break;
            case 2:
                loaded_settings.ns_client_bt_address[i] = 0x5C;
                break;
            default:
                loaded_settings.ns_client_bt_address[i] = esp_random()%255;
                break;
        }
        
    }
    memset(loaded_settings.ns_host_bt_address, 0, 6);
    loaded_settings.ns_controller_paired = false;

    loaded_settings.sx_min = 0xFA;
    loaded_settings.sx_center = 0x740;
    loaded_settings.sx_max = 0xF47;

    loaded_settings.sy_min = 0xFA;
    loaded_settings.sy_center = 0x740;
    loaded_settings.sy_max = 0xF47;

    // Set default stick settings

    // Set blob
    nvs_set_blob(my_handle, "rb_settings", &loaded_settings, sizeof(loaded_settings));

    nvs_commit(my_handle);
    nvs_close(my_handle);
}