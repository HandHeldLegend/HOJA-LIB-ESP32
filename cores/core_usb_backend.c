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

ui2c_gamepad_status_t   _ui2c_gamepad_status    = GAMEPAD_STATUS_IDLE;
ui2c_usb_status_t       _ui2c_usb_status        = USB_STATUS_NOBUS;

QueueHandle_t _usb_xQueue;

typedef struct
{
    uint8_t usb_data[10];
} usb_core_message_s;

void usb_cmd_start()
{
    uint8_t out[10] = {I2C_SYSCMD_SETMODE, I2C_SETMODE_START};
    usb_core_message_s msg_out = {0};
    memcpy(&msg_out, out, 10);
    xQueueSend(_usb_xQueue, &msg_out, (TickType_t) 0);
}

void usb_cmd_stop()
{
    uint8_t out[10] = {I2C_SYSCMD_SETMODE, I2C_SETMODE_STOP};
    usb_core_message_s msg_out = {0};
    memcpy(&msg_out, out, 10);
    xQueueSend(_usb_xQueue, &msg_out, (TickType_t) 0);
}

void usb_cmd_set_subcore()
{
    uint8_t out[10] = {I2C_SYSCMD_SETUSB, core_usb_subcore};
    usb_core_message_s msg_out = {0};
    memcpy(&msg_out, out, 10);
    xQueueSend(_usb_xQueue, &msg_out, (TickType_t) 0);
}

void usb_handle_cmd(uint8_t *data)
{
    const char* TAG = "usb_handle_cmd";

    ui2c_return_msg_s msg = {
        .val = data[0],
    };

    if (msg.rumble)
    {
        // Perform rumble callback
    }

    if (msg.config_request)
    {
        // USB Host has requested config data
    }
    else if (msg.config_write)
    {
        // USB Host is requesting to write a config command
    }

    if (msg.gamepad_status != _ui2c_gamepad_status)
    {
        _ui2c_gamepad_status = msg.gamepad_status;
        ESP_LOGI(TAG, "New USB Gamepad Status: %x", _ui2c_gamepad_status);
        if (_ui2c_gamepad_status == GAMEPAD_STATUS_INITIALIZED)
        {
            ESP_LOGI(TAG, "USB Gamepad Mode Initialized.");
        }
    }
    
    if (msg.usb_status != _ui2c_usb_status)
    {
        _ui2c_usb_status = msg.usb_status;
        ESP_LOGI(TAG, "New USB Status: %x", _ui2c_usb_status);

        if ((_ui2c_usb_status == USB_STATUS_BUSOK) && (_ui2c_gamepad_status == GAMEPAD_STATUS_INITIALIZED))
        {
            ESP_LOGI(TAG, "USB Plugged and running OK.");
        }
        else
        {
            ESP_LOGI(TAG, "USB detached.");
            //usb_cmd_stop();
        }
    }

}

// Perform send/receive over i2c
void usb_txrx_data(i2c_cmd_handle_t *cmd_handle)
{
    const char* TAG = "usb_txrx_data";

    // TRANSMIT I2C
    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, *cmd_handle, portMAX_DELAY);

    // CHECK IF TRANSMITTED OK
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "USB failed to send I2C tx.");
        ESP_LOGE(esp_err_to_name(err), "");
    }
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

void usb_loop_task(void * parameters)
{
    const char* TAG = "usb_loop_task";

    uint8_t out[10] = {0};
    uint8_t in[10] = {0};
    usb_input_s usb_input = {0};

    usb_core_message_s usb_core_message = {0};

    // BUILD USB START COMMAND
    i2c_cmd_handle_t tmpcmd = i2c_cmd_link_create();
    i2c_master_start(tmpcmd);
    i2c_master_write_byte(tmpcmd, (USB_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(tmpcmd, out, 10, true);
    i2c_master_start(tmpcmd);
    i2c_master_write_byte(tmpcmd, (USB_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(tmpcmd, in, 10, I2C_MASTER_LAST_NACK); 
    i2c_master_stop(tmpcmd);

    for(;;)
    {
        if (xQueueReceive(_usb_xQueue, &(usb_core_message), (TickType_t) 0))
        {
            ESP_LOGI(TAG, "Event got: %x", usb_core_message.usb_data[0]);
            // Config event received
            memcpy(out, &usb_core_message, 10);
            usb_txrx_data(&tmpcmd);
            usb_handle_cmd(in);
            vTaskDelay(500/portTICK_PERIOD_MS);
        }
        else
        {
            vTaskDelay((TickType_t) 1);
        }

        // Send normal input
        // Scan the analogs using the callback defined by user.
        hoja_analog_cb();
        hoja_button_remap_process();

        usb_input.buttons_all    = hoja_processed_buttons.buttons_all;
        usb_input.buttons_system = hoja_processed_buttons.buttons_system;
        usb_input.ls_x           = (hoja_analog_data.ls_x >> 4);
        usb_input.ls_y           = (hoja_analog_data.ls_y >> 4);
        usb_input.rs_x           = (hoja_analog_data.rs_x >> 4);
        usb_input.rs_y           = (hoja_analog_data.rs_y >> 4);
        usb_input.lt_a           = (hoja_analog_data.lt_a >> 4);
        usb_input.rt_a           = (hoja_analog_data.rt_a >> 4);

        out[0] = I2C_SYSCMD_INPUT;
        memcpy(&out[1], &usb_input, 9);
        usb_txrx_data(&tmpcmd);
        usb_handle_cmd(in);
    }

    i2c_cmd_link_delete(tmpcmd);
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

    _usb_xQueue = xQueueCreate(8, sizeof(usb_core_message_s *));

    if (_usb_xQueue == 0)
    {
        ESP_LOGI("usb_loop_task", "Failed to create event queue");
    }

    // Start USB task.
    xTaskCreatePinnedToCore(usb_loop_task, "USB Task Loop", 2024,
                            NULL, 0, &usb_TaskHandle, HOJA_CORE_CPU);

    
    usb_cmd_set_subcore();
    usb_cmd_start();

    return HOJA_OK;
}


