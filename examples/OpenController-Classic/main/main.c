#include "main.h"

// Main file for Open Controller Classic Edition example code. 

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

// ADC channel for battery voltage reading
#define ADC_BATTERY_LVL     ADC1_CHANNEL_0

// Variables used to store register reads
uint32_t regread_low = 0;
uint32_t regread_high = 0;

// Variable to hold current color data
rgb_s led_colors[CONFIG_HOJA_RGB_COUNT] = {0};

void read_battery_voltage(void * parameters)
{
    // Set up ADC
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC_BATTERY_LVL, ADC_ATTEN_DB_11));

    for(;;)
    {
        // Read ADC and convert to voltage values
        int out = 0;
        out = adc1_get_raw(ADC_BATTERY_LVL);
        float raw = (float) out;
        float voltage = ((raw * 3.3) / 1950);
        ESP_LOGI("BAT LVL:", "%f", voltage);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

bool getbit(uint32_t bytes, uint8_t bit)
{
    uint8_t tmp = bit;
    if (bit > 31)
    {
        tmp -= 32;
    }
    return (bytes >> tmp) & 0x1;
}

uint32_t sleep_check_timer = 0;
// Sleep mode should check the charge level every 30 seconds or so. 
void enter_sleep()
{
    ESP_LOGI("enter_sleep", "Entering sleep mode");
    util_battery_write(0x9, 0x41);
    esp_deep_sleep_start();
}

// Set up function to update inputs
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

    if (g_button_data.b_select)
    {
        sleep_check_timer+=1;
    }
    else
    {
        sleep_check_timer = 0;
    }

    if (sleep_check_timer >= 250)
    {
        enter_sleep();
    }
}

// Check to see if we should enable 'retro mode (Wired SNES/NES/GameCube)'
bool retro_mode_check()
{
    // Scan all buttons
    button_task();

    // Check if start is pressed
    if (g_button_data.b_start)
    {
        hoja_button_reset();
        return true;
    }
    hoja_button_reset();
    return false;
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
TaskHandle_t battery_voltage_handle = NULL;
TaskHandle_t retro_mode_handle      = NULL;

util_battery_status_s battery_status = {0};

// Task that loops checking what retro mode we're going to be in
void retro_mode_loop(void * params)
{
    ESP_LOGI("retro_mode_loop", "Waiting for retro mode recognition...");
    for(;;)
    {
        util_wire_det_t detected_type = wired_detect();

        switch(detected_type)
        {
            default:
            case DETECT_NONE:
                vTaskDelay(500/portTICK_PERIOD_MS);
                break;
            case DETECT_JOYBUS:
                rgb_setall(COLOR_PURPLE, led_colors);
                rgb_show();
                vTaskDelay(500/portTICK_PERIOD_MS);
                core_gamecube_start();
                vTaskDelete(retro_mode_handle);
                break;
            case DETECT_SNES:
                rgb_setall(COLOR_ORANGE, led_colors);
                rgb_show();
                vTaskDelay(500/portTICK_PERIOD_MS);
                core_snes_start();
                vTaskDelete(retro_mode_handle);
                break;
        }
    }
}

bool battery_check_standby = false;
bool retro_mode_enabled = false;

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
            if (t.plug_status != battery_status.plug_status && !retro_mode_enabled)
            {
                ESP_LOGI(TAG, "Plug status changed. Rebooting...");
                
                // Send restart command to battery utility
                core_usb_stop();
                esp_restart();
            }
            else if (retro_mode_enabled)
            {
                retro_mode_enabled = false;
            }

            rgb_setbrightness(25);
            // If the status has changed, it's dirty and we
            // need to inspect!
            battery_status.status = t.status;
            
        }

        if (battery_check_standby)
        {
            if (t.charge_status == BATSTATUS_NOTCHARGING)
            {
                rgb_setall(COLOR_RED, led_colors);
            }
            else if (t.charge_status == BATSTATUS_TRICKLEFAST)
            {
                rgb_setall(COLOR_ORANGE, led_colors);
            }
            else if (t.charge_status == BATSTATUS_CONSTANT)
            {
                rgb_setall(COLOR_PURPLE, led_colors);
            }
            else if (t.charge_status == BATSTATUS_COMPLETED)
            {
                rgb_setall(COLOR_GREEN, led_colors);
            }
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
    util_rgb_init(led_colors, UTIL_RGB_MODE_GRB);

    rgb_setbrightness(25);
    boot_anim();
    rgb_setbrightness(25);

    vTaskDelay(100/portTICK_PERIOD_MS);

    util_i2c_initialize();
    battery_status.status = 0;
    // Get plugged status
    battery_status.status = util_battery_getstatus();

    // Check if we should boot into retro mode
    if (retro_mode_check())
    {
        retro_mode_enabled = true;
        rgb_setall(COLOR_RED, led_colors);
        rgb_show();
        // Start battery voltage read task.
        xTaskCreatePinnedToCore(retro_mode_loop, "Retro Mode Checker", 2024, NULL, 0, &retro_mode_handle, 1);
    }
    else
    {
        // Not retro mode, let's do other checks
        // If we're plugged in, attempt USB startup
        if (battery_status.plug_status == BATCABLE_PLUGGED)
        {
            // Set battery charge rate to 200mA
            util_battery_write(0x4, 0x2F);

            // If cable is plugged in on boot, we can
            // check what we're plugged in to. Try USB first?
            hoja_err_t herr = core_usb_start();
            
            if (herr != HOJA_OK)
            {
                // If USB core is not ready, stop it.
                core_usb_stop();
                // Enter a standby charging mode
                battery_check_standby = true;
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
            if (core_ns_start() != HOJA_OK)
            {
                // Prevent edge cases by checking usb status again before trying sleep.
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
                }
                // Enter a sleep mode
                enter_sleep();
            }
        }
    }

    // Start battery monitor task.
    xTaskCreatePinnedToCore(battery_check_task, "Battery Monitor", 2024, NULL, 0, &battery_monitor_handle, 1);

    // Start battery voltage read task.
    xTaskCreatePinnedToCore(read_battery_voltage, "Battery Voltage Checker", 2024, NULL, 0, &battery_voltage_handle, 1);
    
}
