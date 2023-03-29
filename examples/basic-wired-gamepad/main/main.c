#include <stdio.h>
#include "hoja_includes.h"

/**
 *  This demo is designed to demonstrate how straightforward it is to get a controller
 *  up and running with minimal programming effort. Some foreknowledge of ESP-IDF is
 *  useful to resolve issues with compilation. The CMake system is not as automated
 *  as other IDEs such as Arduino, but the performance gains are worth your time!
 * 
 *  This gamepad is a standard layout similar to the SNES buttons. No analog sticks
 *  used in this example (More to come on that eventually)
*/

// We must define our GPIO. Doing our own
// defines in this way makes the code easier to read and change later on.
#define GPIO_BTN_A          GPIO_NUM_4
#define GPIO_BTN_B          GPIO_NUM_5
#define GPIO_BTN_X          GPIO_NUM_13
#define GPIO_BTN_Y          GPIO_NUM_14
#define GPIO_BTN_DPAD_U     GPIO_NUM_15 
#define GPIO_BTN_DPAD_L     GPIO_NUM_16
#define GPIO_BTN_DPAD_D     GPIO_NUM_17
#define GPIO_BTN_DPAD_R     GPIO_NUM_18
#define GPIO_BTN_L          GPIO_NUM_19
#define GPIO_BTN_R          GPIO_NUM_20
#define GPIO_BTN_START      GPIO_NUM_21
#define GPIO_BTN_SELECT     GPIO_NUM_22

// We want to do another define for easy setup of our GPIO pins.
// We will use pull-up configuration so we can simply pull each
// pin to ground to count the button as 'pressed'.
#define GPIO_INPUT_PIN_MASK     ( (1ULL << GPIO_BTN_A)|(1ULL << GPIO_BTN_B)|(1ULL << GPIO_BTN_X)|(1ULL << GPIO_BTN_Y)|(1ULL << GPIO_BTN_DPAD_U)|(1ULL << GPIO_BTN_DPAD_L)|(1ULL << GPIO_BTN_DPAD_D)|(1ULL << GPIO_BTN_DPAD_R)|(1ULL << GPIO_BTN_L)|(1ULL << GPIO_BTN_R)|(1ULL << GPIO_BTN_START)|(1ULL << GPIO_BTN_SELECT))


// We must define our own callback method
// that will be registered with HOJA. When HOJA is initialized
// this function will be repeatedly called, thus scanning the buttons.
// This allows flexibility to scan buttons as you see fit, and you don't
// really have to worry about anything else :)
void local_button_cb()
{
    // First we scan the GPIO. We can do this with register reads
    // though this isn't quite the place to learn how to use this
    // to its fullest. Since our highest GPIO used is 31, we do not
    // need to worry about reading the higher GPIO registers. 

    uint32_t register_read_low = REG_READ(GPIO_IN_REG);

    // If we wanted to read GPIO 32 and upwards,
    // we could do this
    // uint32_t register_read_high = REG_READ(GPIO_IN1_REG);

    // From here, we can read out the button data.
    // We have a pointer to the API's internal button
    // structure to set the buttons.

    // Please note that it's good practice here to use OR equals when setting the buttons.
    // This will allow the buttons to be scanned many times in between controller updates to
    // the host/console device. The buttons states are automatically reset by the API.

    // There are some utilities provided by this API that can be useful. In this demo
    // we use the util_getbit demo function which simply returns the bit value from
    // a 32 bit unsigned int. The result is cleaner code that is easier to read :)
    hoja_button_data.dpad_down      |= !util_getbit(register_read_low, GPIO_BTN_DPAD_D);
    hoja_button_data.dpad_left      |= !util_getbit(register_read_low, GPIO_BTN_DPAD_L);
    hoja_button_data.dpad_right     |= !util_getbit(register_read_low, GPIO_BTN_DPAD_R);
    hoja_button_data.dpad_up        |= !util_getbit(register_read_low, GPIO_BTN_DPAD_U);

    // ABXY buttons are not labelled as such in the HOJA backend.
    // This is because this core supports many controller types ranging from N64 to XInput.
    // The position of the buttons is more important than the lettering here. 
    hoja_button_data.button_right   |= !util_getbit(register_read_low, GPIO_BTN_A);
    hoja_button_data.button_down    |= !util_getbit(register_read_low, GPIO_BTN_B);
    hoja_button_data.button_up      |= !util_getbit(register_read_low, GPIO_BTN_X);
    hoja_button_data.button_left    |= !util_getbit(register_read_low, GPIO_BTN_Y);

    // Intellisense in VS Code with the ESP-IDF extension is very useful if you are
    // unsure of some types and want to quickly get more information on a function
    // or unknown type. Simply right click and select "Go to definition"!
    hoja_button_data.trigger_l      |= !util_getbit(register_read_low, GPIO_BTN_L);
    hoja_button_data.trigger_r      |= !util_getbit(register_read_low, GPIO_BTN_R);

    hoja_button_data.button_select  |= !util_getbit(register_read_low, GPIO_BTN_SELECT);
    hoja_button_data.button_start   |= !util_getbit(register_read_low, GPIO_BTN_START);

    // There is a special button which is button_sleep.
    // This is can be set by any button of your choosing as shown.
    // This will send an event to the HOJA event callback system.
    // More documentation on the event callback system will be available
    // at a later time. See the examples folder for the latest updates.

    // The sleep button callback event will execute when you press and hold the select button 
    // for about 3 seconds. Useful for putting the controller to sleep on a battery etc. 
    hoja_button_data.button_sleep   |= !util_getbit(register_read_low, GPIO_BTN_SELECT);
}

// This is what the callback function looks like to read the analog stick data.
// This is read once per poll across each controller core. The analog values MUST be
// 12 bit values. If you are getting 8 bit readings, simply bitshift them to the proper resolution.
void local_analog_cb()
{
    // In this example, this function isn't used.
    // hoja_analog_data.ls_x = 2048; Center value example for an axis.
}

// The event system callback function is needed, even if you do not use it.
void local_event_cb(hoja_event_type_t type, uint8_t evt, uint8_t param)
{
    const char* TAG = "local_event_cb";

    // This will call when the select button is held.
    if (type == HOJA_EVT_SYSTEM && evt == HEVT_API_SHUTDOWN)
    {
        ESP_LOGI(TAG, "Sleep event triggered!");
    }
}

void app_main(void)
{
    // Set up a tag character array for logging
    const char* TAG = "app_main";

    // Set up IO configuration
    gpio_config_t io_conf = {0};

    // Set up IO pins for getting buttons

    // We do not use interrupts here.
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // Apply our previously defined pin mask
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_MASK;
    // Set pins to INPUT mode.
    io_conf.mode = GPIO_MODE_INPUT;
    // Enable internal pull-up resistors on pins.
    // If you have issues, see ESP32 documentation.
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    // Finalize configuration and register it.
    gpio_config(&io_conf);

    // These register functions MUST be called before
    // you try to initialize the API.
    // Pass the functions you've created as parameters.
    hoja_register_button_callback(local_button_cb);
    hoja_register_analog_callback(local_analog_cb);
    hoja_register_event_callback(local_event_cb);

    // We use hoja_err_t for error checking
    // with this API.
    // This helps to differentiate it from other ESP-only functions.
    hoja_err_t err;

    // Attempt initialization.
    err = hoja_init();

    // Check if we initialized Ok
    if (err != HOJA_OK)
    {   
        // If we failed, log this to the console.
        ESP_LOGE(TAG, "Failed to initialize HOJA.");
    }
    else
    {
        // If we got here, HOJA initialized OK.

        // For this example, we will start the XINPUT bluetooth gamepad core.
        // Once the controller turns on, you should see an XINPUT device in your
        // bluetooth pairing menu.
        hoja_set_core(HOJA_CORE_BT_XINPUT);

        // Attempt to start the core.
        hoja_start_core();
    }
}