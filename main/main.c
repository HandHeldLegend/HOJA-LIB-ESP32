#include "main.h"

#define GPIO_BTN_A  16
#define GPIO_BTN_B  15
// Input pin mask creation
#define GPIO_INPUT_PIN_SEL ((1ULL<<GPIO_BTN_A) | (1ULL<<GPIO_BTN_B))

#define BTN_DEBOUNCE_COUNT 6

button_s btn_a = {};
button_s btn_b = {};

bool buttons_changed = false;
uint32_t regread = 0;
uint32_t counter = 0;
GamepadButtonData gp_buttondata = {};

TaskHandle_t ButtonTaskHandle = NULL;

static int adc_raw;

bool getbit(uint32_t bytes, uint8_t bit)
{
    return (bytes >> bit) & 0x1;
}

static void button_task(void* arg)
{
    const char* TAG = "button_task";

    for(;;)
    {
        // Read the GPIO register once to save time :)
        // We mask it with our pin select to erase any unwanted data.
        uint32_t regtmp = REG_READ(GPIO_IN_REG) & GPIO_INPUT_PIN_SEL;
        gp_buttondata.button_right = !getbit(regread, GPIO_BTN_A);
        gp_buttondata.button_down = !getbit(regread, GPIO_BTN_B);

        if (regread != regtmp)
        {
            counter ++;
            ESP_LOGI(TAG, "Buttons changed: %d", counter);
        }

        regread = regtmp;

        vTaskDelay( 16 / portTICK_PERIOD_MS );
    }

    
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

    btn_a.gpio_num = GPIO_BTN_A;
    btn_b.gpio_num = GPIO_BTN_B;

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    xTaskCreate(button_task, "button_task", 2048, NULL, 1, ButtonTaskHandle);

    err = rb_api_init();
    err = rb_api_setCore(CORE_NINTENDOSWITCH);
    //err = rb_api_startController();

}
