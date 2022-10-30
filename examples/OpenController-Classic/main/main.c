#include "main.h"

// Common macros

// Clear a value from high GPIO register (GPIO 32 and higher)
// GPIO.out1_w1tc.val = (uint32_t) (GPIO_NUM-32);
// Set a value from high GPIO register (GPIO 32 and higher)
// GPIO.out1_w1ts.val = (uint32_t) (GPIO_NUM-32);

// Clear a value from low GPIO register (GPIO 0-31)
// GPIO.out_w1tc = (uint32_t) (1ULL<< GPIO_NUM);
// Set a value from low GPIO register (GPIO 0-31)
// GPIO.out_w1ts = (uint32_t) (1ULL<< GPIO_NUM);

// Scanning pins for keypad config
#define GPIO_BTN_SCANA      GPIO_NUM_5
#define GPIO_BTN_SCANB      GPIO_NUM_18
#define GPIO_BTN_SCANC      GPIO_NUM_19
#define GPIO_BTN_SCAND      GPIO_NUM_32

// Port pins for keypad config
#define GPIO_BTN_PULLA      GPIO_NUM_33  
#define GPIO_BTN_PULLB      GPIO_NUM_25
#define GPIO_BTN_PULLC      GPIO_NUM_26
#define GPIO_BTN_PULLD      GPIO_NUM_27

// Button pins (mostly uneeded but looks nicer in code)
#define GPIO_BTN_A          GPIO_BTN_SCANA
#define GPIO_BTN_B          GPIO_BTN_SCANA
#define GPIO_BTN_X          GPIO_BTN_SCANA
#define GPIO_BTN_Y          GPIO_BTN_SCANA
#define GPIO_BTN_DU         GPIO_BTN_SCAND 
#define GPIO_BTN_DL         GPIO_BTN_SCAND
#define GPIO_BTN_DD         GPIO_BTN_SCAND
#define GPIO_BTN_DR         GPIO_BTN_SCAND
#define GPIO_BTN_L          GPIO_BTN_SCANC    
#define GPIO_BTN_ZL         GPIO_BTN_SCANC
#define GPIO_BTN_R          GPIO_BTN_SCANC
#define GPIO_BTN_ZR         GPIO_BTN_SCANC
#define GPIO_BTN_START      GPIO_BTN_SCANC
#define GPIO_BTN_SELECT     GPIO_NUM_2
#define GPIO_BTN_HOME       GPIO_BTN_SCANC
#define GPIO_BTN_CAPTURE    GPIO_BTN_SCANC

// Buttons that are outside of the keypad scanning config
#define GPIO_BTN_STICKL     GPIO_NUM_22
#define GPIO_BTN_STICKR     GPIO_NUM_21
#define GPIO_BTN_SYNC       GPIO_NUM_16

// ADC channels for analog stick input
#define ADC_STICK_LX        ADC1_CHANNEL_0
#define ADC_STICK_LY        ADC1_CHANNEL_3
#define ADC_STICK_RX        ADC1_CHANNEL_6
#define ADC_STICK_RY        ADC1_CHANNEL_7

// Input pin mask creation for keypad scanning setup
#define GPIO_INPUT_PIN_MASK     ( (1ULL<<GPIO_BTN_SCANA) | (1ULL<<GPIO_BTN_SCANB) | (1ULL<<GPIO_BTN_SCANC) | (1ULL<<GPIO_BTN_SCAND) | (1ULL<<GPIO_BTN_SELECT) )
#define GPIO_INPUT_PORT_MASK    ( (1ULL<< GPIO_BTN_PULLA) | (1ULL<<GPIO_BTN_PULLB) | (1ULL<<GPIO_BTN_PULLC) | (1ULL<<GPIO_BTN_PULLD) )

// Masks to clear all relevant bits when doing keypad scan
#define GPIO_INPUT_CLEAR0_MASK  ( (1ULL<< GPIO_BTN_PULLA) | (1ULL<<GPIO_BTN_PULLB) | (1ULL<<GPIO_BTN_PULLD) )
#define GPIO_INPUT_CLEAR1_MASK  ( (1ULL<<(GPIO_BTN_PULLC-32)) )

// Define pins for Nintendo NES/SNES wired pad latch/clock
#define PAD_PIN_CLOCK   GPIO_NUM_15
#define PAD_PIN_LATCH   GPIO_NUM_14

// Define pin for Nintendo wired serial line (GameCube and SNES)
#define PAD_PIN_SERIAL  GPIO_NUM_17

// Variables used to store register reads
uint32_t regread_low = 0;
uint32_t regread_high = 0;

// Variable to hold current color data
rgb_s led_colors[CONFIG_HOJA_RGB_COUNT] = {0};

bool getbit(uint32_t bytes, uint8_t bit)
{
    uint8_t tmp = bit;
    if (bit > 31)
    {
        tmp -= 32;
    }
    return (bytes >> tmp) & 0x1;
}

// Set up function to update inputs
// This will scan the sticks/buttons once
// at a refresh rate determined by the core.

// Used to determine delay period between each scan (microseconds)
#define US_READ  15

void button_task()
{
    // First set port D as low output
    GPIO.out_w1tc = (uint32_t) (1ULL<<GPIO_BTN_PULLD);
    ets_delay_us(US_READ);

    // Read the GPIO registers and mask the data
    regread_low = REG_READ(GPIO_IN_REG) & GPIO_INPUT_PIN_MASK;
    regread_high = REG_READ(GPIO_IN1_REG);
    
    // Grab the relevant button data
    // We OR-EQUALS because we don't want the possibility
    // of a button input getting dropped.

    // Y button
    g_button_data.b_left        |= !getbit(regread_low, GPIO_BTN_Y);
    // Dpad Down
    g_button_data.d_down        |= !getbit(regread_high, GPIO_BTN_DD);
    // L trigger
    g_button_data.t_l           |= !getbit(regread_low, GPIO_BTN_L);

    // Release port D Set port C
    GPIO.out_w1ts = (uint32_t) (1ULL<<GPIO_BTN_PULLD);
    GPIO.out_w1tc = (uint32_t) (1ULL<<GPIO_BTN_PULLC);
    //GPIO.out1_w1tc.val = (uint32_t) 0x1; //GPIO_BTN_PULLC
    ets_delay_us(US_READ);

    // Read the GPIO registers and mask the data
    regread_low = REG_READ(GPIO_IN_REG) & GPIO_INPUT_PIN_MASK;
    regread_high = REG_READ(GPIO_IN1_REG);

    // X button
    g_button_data.b_up          |= !getbit(regread_low, GPIO_BTN_X);
    // Dpad Down
    g_button_data.d_left        |= !getbit(regread_high, GPIO_BTN_DL);

    // Release port C set port B
    GPIO.out_w1ts = (uint32_t) (1ULL<<GPIO_BTN_PULLC);
    GPIO.out_w1tc = (uint32_t) (1ULL<<GPIO_BTN_PULLB);
    ets_delay_us(US_READ);

    // Read the GPIO registers and mask the data
    regread_low = REG_READ(GPIO_IN_REG) & GPIO_INPUT_PIN_MASK;
    regread_high = REG_READ(GPIO_IN1_REG);

    // B button
    g_button_data.b_down        |= !getbit(regread_low, GPIO_BTN_B);
    // Dpad Up
    g_button_data.d_up          |= !getbit(regread_high, GPIO_BTN_DU);
    // Start button
    g_button_data.b_start       |= !getbit(regread_low, GPIO_BTN_START);

    // Release port B set port A
    GPIO.out_w1ts = (uint32_t) (1ULL<<GPIO_BTN_PULLB);
    GPIO.out1_w1tc.val = (uint32_t) (1ULL << 1);
    ets_delay_us(US_READ);

    // Read the GPIO registers and mask the data
    regread_low = REG_READ(GPIO_IN_REG) & GPIO_INPUT_PIN_MASK;
    regread_high = REG_READ(GPIO_IN1_REG);
    
    // Release port A
    GPIO.out1_w1ts.val = (uint32_t) (1ULL << 1);

    // A button
    g_button_data.b_right       |= !getbit(regread_low, GPIO_BTN_A);
    // Dpad Right
    g_button_data.d_right       |= !getbit(regread_high, GPIO_BTN_DR);
    // R trigger
    g_button_data.t_r           |= !getbit(regread_low, GPIO_BTN_R);

    // Read select button (not tied to matrix)
    g_button_data.b_select      |= !getbit(regread_low, GPIO_BTN_SELECT);
}

// Separate task to read sticks.
// This is essential to have as a separate component as ADC scans typically take more time and this is only
// scanned once between each polling interval. This varies from core to core.
void stick_task()
{
    const char* TAG = "stick_task";
    // read stick 1 and 2

    /*
    g_stick_data.lsx = (uint16_t) adc1_get_raw(ADC_STICK_LX);
    g_stick_data.rsx = (uint16_t) adc1_get_raw(ADC_STICK_RX);
    g_stick_data.rsy = (uint16_t) adc1_get_raw(ADC_STICK_RY);
    */

    g_stick_data.lsx = 2048;
    g_stick_data.lsy = 2048;
    g_stick_data.rsx = 2048;
    g_stick_data.rsy = 2048;

    return;
}

void boot_anim()
{
    int back_forth = 0;
    bool toggle = false;
    bool colorflip = false;
    uint8_t color_idx = 0;
    uint8_t color_last_idx = 0;
    rgb_s colors[6] = {COLOR_RED, COLOR_ORANGE, COLOR_YELLOW, COLOR_GREEN, COLOR_BLUE, COLOR_PURPLE};
    for(int i = 0; i < 12; i++)
    {
        memset(led_colors, 0x00, sizeof(led_colors));
        led_colors[back_forth] = colors[color_idx];
        
        if (!toggle)
        {
            if (back_forth > 0)
            {
                led_colors[back_forth-1] = colors[color_last_idx];
                color_last_idx = color_idx;
            }
            back_forth += 1;
            if (back_forth == CONFIG_HOJA_RGB_COUNT)
            {
                toggle = true;
                back_forth = CONFIG_HOJA_RGB_COUNT-1;
            }
        }
        else
        {
            if (back_forth < CONFIG_HOJA_RGB_COUNT-1)
            {
                led_colors[back_forth+1] = colors[color_last_idx];
                color_last_idx = color_idx;
            }
            back_forth -= 1;
            if (back_forth == -1)
            {
                toggle = false;
                back_forth = 0;
            }
        }

        if (!colorflip)
        {
            if (color_idx + 1 > 5)
            {
                colorflip = true;
                color_idx = 5;
            }
            else
            {
                color_idx += 1;
            }
        }
        else
        {
            if (color_idx - 1 < 0)
            {
                colorflip = false;
                color_idx = 0;
            }
            else
            {
                color_idx -= 1;
            }
        }

        rgb_show();
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
    rgb_setall(COLOR_BLACK, led_colors);
    rgb_show();
}

TaskHandle_t battery_monitor_handle = NULL;

util_battery_status_s battery_status = {0};

void battery_check_task(void * parameters)
{
    const char* TAG = "battery_check_task";

    for(;;)
    {
        util_battery_status_s t = {0};
        t.status = util_battery_getstatus();
        if (t.status != battery_status.status)
        {
            // Check if our plugged status has changed.
            if (t.plug_status != battery_status.plug_status)
            {
                ESP_LOGI(TAG, "Plug status changed. Rebooting...");
                
                // Send restart command to battery utility
                core_usb_stop();
                esp_restart();
            }

            rgb_setbrightness(25);
            // If the status has changed, it's dirty and we
            // need to inspect!
            if (t.plug_status == BATCABLE_PLUGGED)
            {
                if (t.charge_status == BATSTATUS_NOTCHARGING)
                {
                    rgb_setall(COLOR_YELLOW, led_colors);
                }
                else if (t.charge_status == BATSTATUS_TRICKLEFAST)
                {
                    rgb_setall(COLOR_GREEN, led_colors);
                }
                else if (t.charge_status == BATSTATUS_CONSTANT)
                {
                    rgb_setall(COLOR_TEAL, led_colors);
                }
                else if (t.charge_status == BATSTATUS_COMPLETED)
                {
                    rgb_setall(COLOR_BLUE, led_colors);
                }
                
            }
            else
            {
                rgb_setall(COLOR_RED, led_colors);
            }
            battery_status.status = t.status;
            rgb_show();
        }
        vTaskDelay(500/portTICK_PERIOD_MS);
    } 
}

void app_main()
{
    const char* TAG = "app_main";

    #if CONFIG_USB_COMPANION_SETTINGS
        ESP_LOGI(TAG, "USB I2C Enabled.");
    #else
        ESP_LOGI(TAG, "USB I2C Disabled.");
    #endif

    hoja_err_t err;

    // Set up ADC
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC_STICK_LX, ADC_ATTEN_DB_11));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC_STICK_LY, ADC_ATTEN_DB_11));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC_STICK_RX, ADC_ATTEN_DB_11));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC_STICK_RY, ADC_ATTEN_DB_11));


    // IO configuration we can reuse
    gpio_config_t io_conf = {};

    // Set up IO pins for scanning button matrix
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_MASK;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = GPIO_INPUT_PORT_MASK;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    GPIO.out_w1ts = GPIO_INPUT_CLEAR0_MASK;
    GPIO.out1_w1ts.val = (uint32_t) 0x3;

    hoja_api_regbuttoncallback(button_task);
    hoja_api_regstickcallback(stick_task);

    hoja_api_init();
    util_i2c_initialize();
    util_rgb_init(led_colors, UTIL_RGB_MODE_GRB);

    rgb_setbrightness(25);
    boot_anim();
    rgb_setbrightness(25);

    vTaskDelay(100/portTICK_PERIOD_MS);
    // Get plugged status
    battery_status.status = util_battery_getstatus();

    util_battery_write(0xA, 0xE0);

    if (battery_status.plug_status == BATCABLE_PLUGGED)
    {
        // If cable is plugged in on boot, we can
        // check what we're plugged in to. Try USB first?
        hoja_err_t herr = core_usb_start();

        if (herr != HOJA_OK)
        {
            // If we're here the USB core did not start because of some reason. Check other valid methods.
            util_wire_det_t detected_type = wired_detect();

            switch(detected_type)
            {
                default:
                case DETECT_NONE:
                    break;
                case DETECT_JOYBUS:
                    rgb_setall(COLOR_PURPLE, led_colors);
                    rgb_show();
                    vTaskDelay(500/portTICK_PERIOD_MS);
                    core_gamecube_start();
                    break;
                case DETECT_SNES:
                    rgb_setall(COLOR_ORANGE, led_colors);
                    rgb_show();
                    vTaskDelay(500/portTICK_PERIOD_MS);
                    core_snes_start();
                    break;
            }
        }
        else
        {
            // If we're here USB Core started OK.
            rgb_s col = {.rgb = 0xf4aef5};
            rgb_setall(col, led_colors);
            rgb_show();
            vTaskDelay(500/portTICK_PERIOD_MS);
        }

    }
    else
    {
        // If we're here, start our wireless mode of choice.
        rgb_setall(COLOR_BLUE, led_colors);
        rgb_show();
        vTaskDelay(500/portTICK_PERIOD_MS);
        core_ns_start();
    }

    // Start battery monitor task.
    xTaskCreatePinnedToCore(battery_check_task, "Battery Monitor", 2024, NULL, 0, battery_monitor_handle, 1);
    
}
