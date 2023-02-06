#include "core_usb_backend.h"

#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          200000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       3000

#define USB_I2C_ADDR                 0x45     /*!< Slave address of the MPU9250 sensor */

TaskHandle_t usb_TaskHandle = NULL;
usb_status_t core_usb_status = USB_STATUS_IDLE;
usb_subcore_t core_usb_subcore = USB_SUBCORE_IDLE;

void usb_handle_cmd(uint8_t *data)
{
    const char* TAG = "usb_handle_cmd";

    switch(data[0])
    {
        default:
        case I2C_RETURN_CMD_IDLE:
            break;

        case I2C_RETURN_CMD_STATUS:
            ESP_LOGI(TAG, "Bus status: %x", data[1]);
            ESP_LOGI(TAG, "Gamepad status: %x", data[2]);
            break;

        case I2C_RETURN_CMD_RUMBLE:
            break;

        case I2C_RETURN_CMD_CONFIG:
            break;
    }
}

void usb_txrx_data(uint8_t *outgoing, uint8_t *incoming)
{
    const char* TAG = "usb_txrx_data";

    // BUILD USB START COMMAND
    i2c_cmd_handle_t tmpcmd = i2c_cmd_link_create();
    i2c_master_start(tmpcmd);
    i2c_master_write_byte(tmpcmd, (USB_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(tmpcmd, outgoing, 10, true);
    i2c_master_start(tmpcmd);
    i2c_master_write_byte(tmpcmd, (USB_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(tmpcmd, incoming, 10, I2C_MASTER_LAST_NACK); 
    i2c_master_stop(tmpcmd);

    // TRANSMIT USB START COMMAND
    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, tmpcmd, 1000/portTICK_PERIOD_MS);

    // CHECK IF USB START COMMAND TRANSMITTED OK
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "USB failed to send I2C tx.");
        ESP_LOGE(esp_err_to_name(err), "");
    }

    i2c_cmd_link_delete(tmpcmd);
}

hoja_err_t core_usb_set_subcore(usb_subcore_t subcore)
{
    const char* TAG = "core_usb_set_subcore";
    if(subcore >= USB_SUBCORE_MAX)
    {
        ESP_LOGE(TAG, "Invalid subcore setting.");
        return HOJA_FAIL;
    }

    ESP_LOGI(TAG, "USB SubCore Set.");
    core_usb_subcore = subcore;
    return HOJA_OK;
}

void core_usb_stop(void)
{
    const char* TAG = "core_usb_start";
}

hoja_err_t core_usb_start(void)
{
    const char* TAG = "core_usb_start";

    if (core_usb_status != USB_STATUS_IDLE)
    {
        ESP_LOGE(TAG, "USB core is already running.");
        return HOJA_FAIL;
    }

    // Conditional function define if EFM8UB1 Companion Option is ENABLED
    ESP_LOGI(TAG, "Core Start: USB EFM8UB1 Mode");
    esp_err_t err = ESP_OK;

    if (util_i2c_status != UTIL_I2C_STATUS_AVAILABLE)
    {
        ESP_LOGE(TAG, "Cannot start USB core. Required I2C utility initialize first.");
        return HOJA_FAIL;
    }

    uint8_t out[10] = {0};
    uint8_t in[10] = {0};

    out[0] = I2C_CMD_STATUS;
    ESP_LOGI(TAG, "SENDING: %x", (unsigned int) out[0]);
    usb_txrx_data(out, in);
    ESP_LOGI(TAG, "RETURNED: %x", (unsigned int) in[0]);

    usb_handle_cmd(in);

    out[0] = I2C_CMD_SETUSB;
    out[1] = USB_SUBCORE_XINPUT;
    usb_txrx_data(out, in);

    out[0] = I2C_CMD_STATUS;
    ESP_LOGI(TAG, "SENDING: %x", (unsigned int) out[0]);
    usb_txrx_data(out, in);
    ESP_LOGI(TAG, "RETURNED: %x", (unsigned int) in[0]);

    usb_handle_cmd(in);

    out[0] = I2C_CMD_SETMODE;
    out[1] = I2C_SETMODE_START;
    usb_txrx_data(out, in);

    out[0] = I2C_CMD_STATUS;
    ESP_LOGI(TAG, "SENDING: %x", (unsigned int) out[0]);
    usb_txrx_data(out, in);
    ESP_LOGI(TAG, "RETURNED: %x", (unsigned int) in[0]);

    usb_handle_cmd(in);

    return HOJA_OK;
}

void usb_idle_task(void * parameters)
{

}

#if CONFIG_HOJA_EFM8UB1_ENABLE
void usb_sendinput_task(void * parameters)
{
    // Set up our variables for conversion
    const char* TAG = "usb_sendinput_task";
    usb_input_s usb_input = {};
    uint8_t output_buffer[10] = {};
    output_buffer[0] = I2C_CMD_INPUT;
    esp_err_t err = ESP_OK;

    // Set up and send data over I2C
    i2c_cmd_handle_t tmpcmd = i2c_cmd_link_create();

    // Build the i2c packet
    // to send the USB start command
    i2c_master_start(tmpcmd);
    i2c_master_write_byte(tmpcmd, (USB_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(tmpcmd, output_buffer, 10, true);
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
        hoja_analog_cb();

        usb_input.buttons_all    = hoja_button_data.buttons_all;
        usb_input.buttons_system = hoja_button_data.buttons_system;
        usb_input.ls_x           = (hoja_analog_data.ls_x >> 4);
        usb_input.ls_y           = (hoja_analog_data.ls_y >> 4);
        usb_input.rs_x           = (hoja_analog_data.rs_x >> 4);
        usb_input.rs_y           = (hoja_analog_data.rs_y >> 4);
        usb_input.lt_a           = (hoja_analog_data.lt_a >> 4);
        usb_input.rt_a           = (hoja_analog_data.rt_a >> 4);

        memcpy(&output_buffer[1], &usb_input, 9);

        hoja_button_reset();

        vTaskDelay(1/portTICK_PERIOD_MS);

    }

    i2c_cmd_link_delete(tmpcmd);
}
#endif
