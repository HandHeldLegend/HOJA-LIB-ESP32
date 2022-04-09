#include "main.h"

#define GPIO_BTN_A  16
#define GPIO_BTN_B  15
// Input pin mask creation
#define GPIO_INPUT_PIN_SEL ((1ULL<<GPIO_BTN_A) | (1ULL<<GPIO_BTN_B))

#define BTN_DEBOUNCE_COUNT 6

bool buttons_changed = false;
uint32_t regread = 0;
uint32_t counter = 0;

TaskHandle_t ButtonTaskHandle = NULL;

static int adc_raw;

bool getbit(uint32_t bytes, uint8_t bit)
{
    return (bytes >> bit) & 0x1;
}

// Set up function to update inputs
// This will scan the sticks/buttons once
// at a refresh rate determined by the core.
void button_task()
{
    // Read the GPIO register once to save time :)
    // We mask it with our pin select to erase any unwanted data.
    regread = REG_READ(GPIO_IN_REG) & GPIO_INPUT_PIN_SEL;
    g_button_data.button_right = !getbit(regread, GPIO_BTN_A);
    g_button_data.button_down = !getbit(regread, GPIO_BTN_B);

    // read stick 1
    g_stick_data.lsx = (uint16_t) adc1_get_raw(ADC1_CHANNEL_0);
    g_stick_data.lsy = loaded_settings.sx_center;
}

void app_main()
{
    const char* TAG = "app_main";

    rb_err_t err;

    // Use GPIO 16 and 17 for buttons
    // Use ADC channel 0 for test pot

    // Set up ADC
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11));

    // Set up GPIO
    gpio_config_t io_conf = {};

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
 
    // Register input callback
    rb_register_input_callback(button_task);

    err = rb_api_init();
    err = rb_api_setCore(CORE_NINTENDOSWITCH);
    err = rb_api_startController();

}
