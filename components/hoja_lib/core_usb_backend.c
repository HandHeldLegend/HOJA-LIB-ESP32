#include "core_usb_backend.h"

#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          200000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       3000

#define USB_I2C_ADDR                 0x45     /*!< Slave address of the MPU9250 sensor */

TaskHandle_t usb_TaskHandle = NULL;

hoja_err_t core_usb_stop(void)
{
    const char* TAG = "core_usb_start";

    // Conditional function define if EFM8UB1 Companion Option is ENABLED
    #if CONFIG_HOJA_EFM8UB1_ENABLE
    ESP_LOGI(TAG, "Core Stop: USB");
    esp_err_t err = ESP_OK;

    if (util_i2c_status != UTIL_I2C_STATUS_AVAILABLE)
    {
        ESP_LOGE(TAG, "Cannot stop USB core. Required I2C utility initialize first.");
        return HOJA_I2C_NOTINIT;
    }
    ESP_LOGI(TAG, "USB Core okay to stop as I2C is set up.");

    // BUILD USB START COMMAND
    i2c_cmd_handle_t tmpcmd = i2c_cmd_link_create();
    uint8_t tosend[2] = {USB_CMD_SYSTEMSET, USB_SYSTEM_STOP};
    i2c_master_start(tmpcmd);
    i2c_master_write_byte(tmpcmd, (USB_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(tmpcmd, tosend, 2, true);
    i2c_master_stop(tmpcmd);

    // TRANSMIT USB START COMMAND
    err = i2c_master_cmd_begin(I2C_NUM_0, tmpcmd, 1000/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(tmpcmd);

    // CHECK IF USB START COMMAND TRANSMITTED OK
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "USB Core Command: USB Service Stop: Transmit Fail.");
        ESP_LOGE(esp_err_to_name(err), "");
        return HOJA_I2C_FAIL;
    }
    ESP_LOGI(TAG, "USB Core Command: USB Service Stop: Transmit OK.");

    vTaskDelete(usb_TaskHandle);
    usb_TaskHandle = NULL;

    return HOJA_OK;
    
    // If EFM8UB1 Companion is not enabled...
    #else
    ESP_LOGE(TAG, "USB not enabled. Enable EFM8UB1 Companion in SDK settings.");
    return HOJA_FAIL;
    #endif
}

hoja_err_t core_usb_start(void)
{
    const char* TAG = "core_usb_start";

    // Conditional function define if EFM8UB1 Companion Option is ENABLED
    #if CONFIG_HOJA_EFM8UB1_ENABLE
    ESP_LOGI(TAG, "Core Start: USB EFM8UB1 Mode");
    esp_err_t err = ESP_OK;

    if (util_i2c_status != UTIL_I2C_STATUS_AVAILABLE)
    {
        ESP_LOGE(TAG, "Cannot start USB core. Required I2C utility initialize first.");
        return HOJA_FAIL;
    }
    ESP_LOGI(TAG, "USB Core okay to start as I2C is set up.");

    // BUILD USB START COMMAND
    i2c_cmd_handle_t tmpcmd = i2c_cmd_link_create();
    uint8_t tosend[2] = {USB_CMD_SYSTEMSET, USB_SYSTEM_START};
    i2c_master_start(tmpcmd);
    i2c_master_write_byte(tmpcmd, (USB_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(tmpcmd, tosend, 2, true);
    i2c_master_stop(tmpcmd);

    // TRANSMIT USB START COMMAND
    err = i2c_master_cmd_begin(I2C_NUM_0, tmpcmd, 1000/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(tmpcmd);

    // CHECK IF USB START COMMAND TRANSMITTED OK
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "USB Core Command: USB Service Start: Transmit Fail.");
        ESP_LOGE(esp_err_to_name(err), "");
        return HOJA_FAIL;
    }
    ESP_LOGI(TAG, "USB Core Command: USB Service Start: Transmit OK.");

    // DELAY 500MS BEFORE CHECKING STATUS
    vTaskDelay(500/portTICK_PERIOD_MS);

    // READ USB START COMMAND RESPONSE
    tmpcmd = i2c_cmd_link_create();
    uint8_t response[2] = {0};
    response[0] = 0xFF;
    i2c_master_start(tmpcmd);
    i2c_master_write_byte(tmpcmd, (USB_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(tmpcmd, &response[0], I2C_MASTER_LAST_NACK);
    i2c_master_stop(tmpcmd);

    // START READ
    err = i2c_master_cmd_begin(I2C_NUM_0, tmpcmd, 1000/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(tmpcmd);

    // CHECK IF USB START READ WENT OK
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "USB Core Read: Transmit Fail.");
        ESP_LOGE(esp_err_to_name(err), "");
        return HOJA_FAIL;
    }
    ESP_LOGI(TAG, "USB Core Read: Transmit OK.");

    // CHECK USB START RESPONSE STATUS
    if ( response[0] == USB_MSG_OK )
    {

        ESP_LOGI(TAG, "USB Core Status: USB Service Started OK.");

        tosend[0] = USB_CMD_CHECKREADY;

        // BUILD USB STATUS GET COMMAND
        tmpcmd = i2c_cmd_link_create();
        i2c_master_start(tmpcmd);
        i2c_master_write_byte(tmpcmd, (USB_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write(tmpcmd, tosend, 1, true);
        i2c_master_stop(tmpcmd);    

        // TRANSMIT USB STATUS COMMAND
        err = i2c_master_cmd_begin(I2C_NUM_0, tmpcmd, 1000/portTICK_PERIOD_MS);
        i2c_cmd_link_delete(tmpcmd);

        // CHECK IF USB STATUS COMMAND WENT OK
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "USB Core Command: Get USB Serivce Status: Transmit Fail.");
            ESP_LOGE(esp_err_to_name(err), "");
            return HOJA_FAIL;
        }
        ESP_LOGI(TAG, "USB Core Command: Get USB Service Status: Transmit OK.");

        // READ USB STATUS
        tmpcmd = i2c_cmd_link_create();
        uint8_t response[2] = {0};
        response[0] = 0xFF;
        response[1] = 0xFF;
        i2c_master_start(tmpcmd);
        i2c_master_write_byte(tmpcmd, (USB_I2C_ADDR << 1) | I2C_MASTER_READ, true);
        i2c_master_read_byte(tmpcmd, &response[0], I2C_MASTER_ACK);
        i2c_master_read_byte(tmpcmd, &response[1], I2C_MASTER_LAST_NACK);
        i2c_master_stop(tmpcmd);

        // DELAY 500MS BEFORE CHECKING STATUS
        vTaskDelay(500/portTICK_PERIOD_MS);

        // Start transmission with 1 second timeout
        err = i2c_master_cmd_begin(I2C_NUM_0, tmpcmd, 1000/portTICK_PERIOD_MS);
        i2c_cmd_link_delete(tmpcmd);

        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "USB Core Read: Transmit Fail.");
            ESP_LOGE(esp_err_to_name(err), "");
            return HOJA_I2C_FAIL;
        }
        ESP_LOGI(TAG, "USB Core Read: Transmit OK.");

        if ( (response[0] == USB_CMD_CHECKREADY) && (response[1] == USB_MSG_OK) )
        {
            ESP_LOGI(TAG, "USB Core Command: Get Bus Status: Bus OK.");

            // Check if task is running, delete if so.
            if (usb_TaskHandle != NULL)
            {
                vTaskDelete(usb_TaskHandle);
                usb_TaskHandle = NULL;
            }

            // Start USB Send Input I2C task.
            xTaskCreatePinnedToCore(usb_sendinput_task, "USB I2C Task Loop", 2024,
                                    NULL, 0, &usb_TaskHandle, 0);
            
            ESP_LOGI(TAG, "USB Core started OK.");

            hoja_event_cb(HOJA_EVT_USB, HOJA_USB_CONNECTED, 0x00);

            return HOJA_OK;

        }   
        else
        {
            ESP_LOGE(TAG, "USB Core Command: Get Bus Status: Bus Unplugged.");
            return HOJA_USB_NODET;
        }

    }
    else
    {
        ESP_LOGE(TAG, "I2C system start message failure.");
        ESP_LOGE(TAG, "Returned data: %X", response[1]);
        return HOJA_FAIL;
    }

    return HOJA_FAIL;
    #else
    ESP_LOGE(TAG, "Other USB Modes not implemented currently. Enable EFM8UB1 Companion in SDK settings.");
    return HOJA_FAIL;
    #endif
}

#if CONFIG_HOJA_EFM8UB1_ENABLE
void usb_sendinput_task(void * parameters)
{
    // Set up our variables for conversion
    const char* TAG = "usb_sendinput_task";
    i2c_input_s i2c_input = {};
    uint8_t output_buffer[8] = {};
    output_buffer[0] = USB_CMD_INPUT;
    esp_err_t err = ESP_OK;

    // Set up and send data over I2C
    i2c_cmd_handle_t tmpcmd = i2c_cmd_link_create();

    // Build the i2c packet
    // to send the USB start command
    i2c_master_start(tmpcmd);
    i2c_master_write_byte(tmpcmd, (USB_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(tmpcmd, output_buffer, 8, true);
    i2c_master_stop(tmpcmd);

    while(1)
    {
        // Start transmission with 1 second timeout
        err = i2c_master_cmd_begin(I2C_NUM_0, tmpcmd, portMAX_DELAY);

        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to send I2C Input to USB:");
            ESP_LOGE(esp_err_to_name(err), "");
        }

        // Scan the analogs using the callback defined by user.
        hoja_analog_cb(&hoja_analog_data);

        // Convert button presses to appropriate format
        i2c_input.button_a      = hoja_button_data.button_right;
        i2c_input.button_b      = hoja_button_data.button_down;
        i2c_input.button_x      = hoja_button_data.button_left;
        i2c_input.button_y      = hoja_button_data.button_up;

        i2c_input.dpad_up       = hoja_button_data.dpad_up;
        i2c_input.dpad_left     = hoja_button_data.dpad_left;
        i2c_input.dpad_down     = hoja_button_data.dpad_down;
        i2c_input.dpad_right    = hoja_button_data.dpad_right;

        i2c_input.trigger_l     = hoja_button_data.trigger_l;
        i2c_input.trigger_r     = hoja_button_data.trigger_r;
        i2c_input.trigger_zl    = hoja_button_data.trigger_zl;
        i2c_input.trigger_zr    = hoja_button_data.trigger_zr;
        
        i2c_input.stick_right   = hoja_button_data.button_stick_right;
        i2c_input.stick_left    = hoja_button_data.button_stick_left;

        i2c_input.button_home       = hoja_button_data.button_home;
        i2c_input.button_capture    = hoja_button_data.button_capture;
        i2c_input.button_plus       = hoja_button_data.button_start;
        i2c_input.button_minus      = hoja_button_data.button_select;

        i2c_input.stick_left_x      = hoja_analog_data.ls_x >> 4;
        i2c_input.stick_left_y      = 255 - (hoja_analog_data.ls_y >> 4);
        i2c_input.stick_right_x     = hoja_analog_data.rs_x >> 4;
        i2c_input.stick_right_y     = 255 - (hoja_analog_data.rs_y >> 4);

        output_buffer[1] = i2c_input.buttons_1;
        output_buffer[2] = i2c_input.buttons_2;
        output_buffer[3] = i2c_input.buttons_3;
        output_buffer[4] = i2c_input.stick_left_x;
        output_buffer[5] = i2c_input.stick_left_y;
        output_buffer[6] = i2c_input.stick_right_x;
        output_buffer[7] = i2c_input.stick_right_y;

        hoja_button_reset();

        vTaskDelay(1/portTICK_PERIOD_MS);

    }

    i2c_cmd_link_delete(tmpcmd);
}
#endif
