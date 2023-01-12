#include "hoja_settings.h"

hoja_settings_s loaded_settings = {};

hoja_err_t hoja_settings_init(void)
{
    nvs_handle_t my_handle;
    const char* TAG = "hoja_settings_init";
    esp_err_t err;

    // Check if we have the magic byte

    // Open storage
    err = nvs_open(SETTINGS_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "During HOJA load settings, NVS Open failed.");
        return HOJA_FAIL;
    }

    size_t required_size = 0;
    err = nvs_get_blob(my_handle, "hoja_settings", NULL, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return HOJA_FAIL;

    ESP_LOGI(TAG, "Required size: %d", required_size);

    if (required_size > 0)
    {
        err = nvs_get_blob(my_handle, "hoja_settings", &loaded_settings, &required_size);
        if (err != ESP_OK)
        {   
            ESP_LOGE(TAG, "Could not load settings. 0x%08X", (unsigned int) loaded_settings.magic_bytes);
            return HOJA_FAIL;
        }
        if (loaded_settings.magic_bytes != SETTINGS_MAGIC)
        {
            ESP_LOGI(TAG, "Settings magic bytes don't match. Setting to default...");
            nvs_close(my_handle);

            if (hoja_settings_default()) return HOJA_OK;
        }
        else
        {
            ESP_LOGI(TAG, "Settings loaded with magic byte 0x%08X", (unsigned int) loaded_settings.magic_bytes);
            nvs_close(my_handle);
        }
        
    }
    else
    {
        ESP_LOGI(TAG, "Settings unset. Set defaults...");
        // We need to set/save all the appropriate settings.
        nvs_close(my_handle);

        if (hoja_settings_default()) return HOJA_OK;
    }

    ESP_LOGI(TAG, "HOJA Settings initialized OK.");
    return HOJA_OK;
}

// Save all settings
hoja_err_t hoja_settings_saveall(void)
{
    const char* TAG = "hoja_settings_saveall";
    nvs_handle_t my_handle;
    esp_err_t err;
    // Open
    err = nvs_open(SETTINGS_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "During HOJA settings save, NVS storage failed to open.");
        return HOJA_FAIL;
    }

    nvs_set_blob(my_handle, "hoja_settings", &loaded_settings, sizeof(loaded_settings));

    nvs_commit(my_handle);
    nvs_close(my_handle);

    return HOJA_OK;
}

// Sets all settings to default.
hoja_err_t hoja_settings_default(void)
{
    const char* TAG = "hoja_settings_default";
    nvs_handle_t my_handle;
    esp_err_t err;
    ESP_LOGI(TAG, "Setting defaults...");

    // Open
    err = nvs_open(SETTINGS_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Error when opening NVS storage for HOJA Settings.");
        return HOJA_FAIL;
    }

    loaded_settings.magic_bytes = SETTINGS_MAGIC;

    loaded_settings.controller_mode = HOJA_CONTROLLER_MODE_RETRO;

    // Nintendo Switch Core stuff
    memset(loaded_settings.ns_host_bt_address, 0, 6);
    hoja_settings_generate_btmac(loaded_settings.ns_client_bt_address);
    loaded_settings.ns_controller_paired = false;
    loaded_settings.ns_controller_type = NS_CONTROLLER_TYPE_PROCON;

    // Dinput Core stuff
    memset(loaded_settings.dinput_host_bt_address, 0, 6);
    hoja_settings_generate_btmac(loaded_settings.dinput_client_bt_address);
    loaded_settings.dinput_controller_paired = false;

    // Xinput Core stuff
    memset(loaded_settings.xinput_host_bt_address, 0, 6);
    hoja_settings_generate_btmac(loaded_settings.xinput_client_bt_address);
    loaded_settings.xinput_controller_paired = false;

    // Default color greenish
    loaded_settings.color_r = 0x38;
    loaded_settings.color_g = 0xFF;
    loaded_settings.color_b = 0x59;

    loaded_settings.sx_min = 0xFA;
    loaded_settings.sx_center = 0x740;
    loaded_settings.sx_max = 0xF47;

    loaded_settings.sy_min = 0xFA;
    loaded_settings.sy_center = 0x740;
    loaded_settings.sy_max = 0xF47;

    loaded_settings.cx_min = 0xFA;
    loaded_settings.cx_center = 0x740;
    loaded_settings.cx_max = 0xF47;

    loaded_settings.cy_min = 0xFA;
    loaded_settings.cy_center = 0x740;
    loaded_settings.cy_max = 0xF47;

    loaded_settings.snes_stick_dpad = false;

    // Set blob
    nvs_set_blob(my_handle, "hoja_settings", &loaded_settings, sizeof(hoja_settings_s));
    nvs_commit(my_handle);
    nvs_close(my_handle);

    return HOJA_OK;
}

void hoja_settings_generate_btmac(uint8_t * out)
{
    // Generate random MAC address
    for(int i = 0; i < 8; i++)
    {
        switch(i)
        {
            case 0:
                out[i] = 0x98;
                break;
            case 1:
                out[i] = 0x41;
                break;
            case 2:
                out[i] = 0x5C;
                break;
            default:
                out[i] = esp_random()%255;
                break;
        }
    }
}