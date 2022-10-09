#include "core_usb_backend.h"

#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       3000

#define USB_I2C_ADDR                 0x45     /*!< Slave address of the MPU9250 sensor */

TaskHandle_t usb_TaskHandle = NULL;

hoja_err_t core_usb_start(void)
{
    const char* TAG = "core_usb_start";

    // Set up I2C peripheral.

	i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = CONFIG_HOJA_GPIO_I2C_SDA;
    conf.scl_io_num = CONFIG_HOJA_GPIO_I2C_SCL;
    // Use external pull-up resistors according to I2C specifications.
    conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
    conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
    // Max clock speed for ESP32 I2C.
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(I2C_NUM_0, &conf);
    esp_err_t err = i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start I2C Bus. Check configuration and pins.");
        return HOJA_FAIL;
    }
    ESP_LOGI(TAG, "I2C peripheral set up OK.");

    // Next we need to send the command to the I2C device
    // to start USB function. We then wait 2 seconds to see if the device
    // has read back that it is ready. This does not necessarily mean that
    // the USB device is connected, but that it is ok to send data. 
    i2c_cmd_handle_t tmpcmd = i2c_cmd_link_create();
    uint8_t tosend[2] = {USB_CMD_SYSTEMSET, USB_SYSTEM_START};

    // Build the i2c packet
    // to send the USB start command
    i2c_master_start(tmpcmd);
    i2c_master_write_byte(tmpcmd, (USB_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(tmpcmd, tosend, 2, true);
    i2c_master_stop(tmpcmd);

    // Start transmission with 1 second timeout
    err = i2c_master_cmd_begin(I2C_NUM_0, tmpcmd, 1000/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(tmpcmd);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to send I2C Start Command:");
        ESP_LOGE(esp_err_to_name(err), "");
        return HOJA_FAIL;
    }
    ESP_LOGI(TAG, "I2C sent start message OK.");

    // Delay 1.5 seconds and we'll check for status command to see
    // if we can continue.
    vTaskDelay(1500/portTICK_RATE_MS);

    // Build the next command to verify the status.
    tmpcmd = i2c_cmd_link_create();
    uint8_t response[2] = {0};
    response[0] = 0xFF;
    response[1] = 0xFF;
    i2c_master_start(tmpcmd);
    i2c_master_write_byte(tmpcmd, (USB_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(tmpcmd, &response[0], I2C_MASTER_ACK);
    i2c_master_read_byte(tmpcmd, &response[1], I2C_MASTER_LAST_NACK);
    i2c_master_stop(tmpcmd);

    // Start transmission with 1 second timeout
    err = i2c_master_cmd_begin(I2C_NUM_0, tmpcmd, 1000/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(tmpcmd);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to retrieve I2C command response:");
        ESP_LOGE(esp_err_to_name(err), "");
        return HOJA_FAIL;
    }
    ESP_LOGI(TAG, "I2C got command response OK.");

    // If we get back the correct command data and OK response, we
    // are good to go to begin sending input data for the joypad.
    if ( (response[0] == USB_CMD_SYSTEMSET) && (response[1] == USB_MSG_OK) )
    {
        ESP_LOGI(TAG, "I2C confirmation of USB device start OK.");

        // Check if task is running, delete if so.
        if (usb_TaskHandle != NULL)
        {
            vTaskDelete(usb_TaskHandle);
            usb_TaskHandle = NULL;
        }

        // Start SNES task.
        xTaskCreatePinnedToCore(usb_sendinput_task, "USB I2C Task Loop", 2024,
                                NULL, 0, &usb_TaskHandle, 1);
        
        ESP_LOGI(TAG, "USB Core started OK.");

        return HOJA_OK;
    }
    else
    {
        ESP_LOGE(TAG, "I2C system start message failure.");
        ESP_LOGE(TAG, "Returned data: %X", response[1]);
        return HOJA_FAIL;
    }

    return HOJA_FAIL;
}

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
        err = i2c_master_cmd_begin(I2C_NUM_0, tmpcmd, 1000/portTICK_PERIOD_MS);

        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to send I2C Input:");
            ESP_LOGE(esp_err_to_name(err), "");
        }

        // Scan the sticks using the callback defined by user.
        // hoja_button_cb();
        hoja_stick_cb();

        // Convert button presses to appropriate format
        i2c_input.button_a      = g_button_data.b_right;
        i2c_input.button_b      = g_button_data.b_down;
        i2c_input.button_x      = g_button_data.b_left;
        i2c_input.button_y      = g_button_data.b_up;

        i2c_input.dpad_up       = g_button_data.d_up;
        i2c_input.dpad_left     = g_button_data.d_left;
        i2c_input.dpad_down     = g_button_data.d_down;
        i2c_input.dpad_right    = g_button_data.d_right;

        i2c_input.trigger_l     = g_button_data.t_l;
        i2c_input.trigger_r     = g_button_data.t_r;
        i2c_input.trigger_zl    = g_button_data.t_zl;
        i2c_input.trigger_zr    = g_button_data.t_zr;
        
        i2c_input.stick_right   = g_button_data.sb_right;
        i2c_input.stick_left    = g_button_data.sb_left;

        i2c_input.button_home       = g_button_data.b_home;
        i2c_input.button_capture    = g_button_data.b_capture;
        i2c_input.button_plus       = g_button_data.b_start;
        i2c_input.button_minus      = g_button_data.b_select;

        i2c_input.stick_left_x      = g_stick_data.lsx >> 4;
        i2c_input.stick_left_y      = 255 - (g_stick_data.lsy >> 4);
        i2c_input.stick_right_x     = g_stick_data.rsx >> 4;
        i2c_input.stick_right_y     = 255 - (g_stick_data.rsy >> 4);

        output_buffer[1] = i2c_input.buttons_1;
        output_buffer[2] = i2c_input.buttons_2;
        output_buffer[3] = i2c_input.buttons_3;
        output_buffer[4] = i2c_input.stick_left_x;
        output_buffer[5] = i2c_input.stick_left_y;
        output_buffer[6] = i2c_input.stick_right_x;
        output_buffer[7] = i2c_input.stick_right_y;

        hoja_button_reset();
        
        // Delay 1 ms
        vTaskDelay(1/portTICK_PERIOD_MS);

    }
    
    i2c_cmd_link_delete(tmpcmd);
}
