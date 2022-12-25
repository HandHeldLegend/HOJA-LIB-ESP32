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

uint8_t local_bluetooth_mode = HOJA_CORE_NS;
uint8_t local_retro_mode = false;
uint8_t local_cable_plugged = false;

// Variables used to store register reads
uint32_t regread_low = 0;
uint32_t regread_high = 0;
bool     local_start_pressed = false;

// Variable to hold current color data
rgb_s led_colors[CONFIG_HOJA_RGB_COUNT] = {0};

// LED boot animation
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

// Removed for now
/*
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
*/

// Reboot system properly.
void enter_reboot()
{
    util_battery_set_charge_rate(35);
    esp_restart();
}

// Sleep mode should check the charge level every 30 seconds or so. 
void enter_sleep()
{
    rgb_setall(COLOR_BLACK, led_colors);
    rgb_show();

    util_battery_enable_ship_mode();
}

// Set up function to update inputs
// Used to determine delay period between each scan (microseconds)
#define US_READ  15
void local_button_cb(hoja_button_data_s *button_data)
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
    button_data->button_left    |= !util_getbit(regread_low, GPIO_BTN_Y);
    // Dpad Down
    button_data->dpad_down      |= !util_getbit(regread_high, GPIO_BTN_DD);
    // L trigger
    button_data->trigger_l      |= !util_getbit(regread_low, GPIO_BTN_L);

    // Release port D Set port C
    GPIO.out_w1ts = (uint32_t) (1ULL<<GPIO_BTN_PULLD);
    GPIO.out_w1tc = (uint32_t) (1ULL<<GPIO_BTN_PULLC);
    //GPIO.out1_w1tc.val = (uint32_t) 0x1; //GPIO_BTN_PULLC
    ets_delay_us(US_READ);

    // Read the GPIO registers and mask the data
    regread_low = REG_READ(GPIO_IN_REG) & GPIO_INPUT_PIN_MASK;
    regread_high = REG_READ(GPIO_IN1_REG);

    // X button
    button_data->button_up      |= !util_getbit(regread_low, GPIO_BTN_X);
    // Dpad Down
    button_data->dpad_left      |= !util_getbit(regread_high, GPIO_BTN_DL);

    // Release port C set port B
    GPIO.out_w1ts = (uint32_t) (1ULL<<GPIO_BTN_PULLC);
    GPIO.out_w1tc = (uint32_t) (1ULL<<GPIO_BTN_PULLB);
    ets_delay_us(US_READ);

    // Read the GPIO registers and mask the data
    regread_low = REG_READ(GPIO_IN_REG) & GPIO_INPUT_PIN_MASK;
    regread_high = REG_READ(GPIO_IN1_REG);

    // B button
    button_data->button_down    |= !util_getbit(regread_low, GPIO_BTN_B);
    // Dpad Up
    button_data->dpad_up        |= !util_getbit(regread_high, GPIO_BTN_DU);
    // Start button
    button_data->button_start   |= !util_getbit(regread_low, GPIO_BTN_START);
    if (button_data->button_start)
    {
        local_start_pressed = true;
    }
    else
    {
        local_start_pressed = false;
    }

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
    button_data->button_right   |= !util_getbit(regread_low, GPIO_BTN_A);
    // Dpad Right
    button_data->dpad_right     |= !util_getbit(regread_high, GPIO_BTN_DR);
    // R trigger
    button_data->trigger_r      |= !util_getbit(regread_low, GPIO_BTN_R);

    // Read select button (not tied to matrix)
    button_data->button_select  |= !util_getbit(regread_low, GPIO_BTN_SELECT);

    // Tie the select button to our sleep button.
    if (button_data->button_select)
    {
        button_data->button_sleep = 1;
    }
    else
    {
        button_data->button_sleep = 0;
    }
}

// Separate task to read sticks.
// This is essential to have as a separate component as ADC scans typically take more time and this is only
// scanned once between each polling interval. This varies from core to core.
void local_analog_cb(hoja_analog_data_s *analog_data)
{
    const char* TAG = "stick_task";
    // read stick 1 and 2

    /*
    analog_data->ls_x = (uint16_t) adc1_get_raw(ADC_STICK_LX);
    analog_data->rs_x = (uint16_t) adc1_get_raw(ADC_STICK_RX);
    analog_data->rs_y = (uint16_t) adc1_get_raw(ADC_STICK_RY);
    */

    analog_data->ls_x = 2048;
    analog_data->ls_y = 2048;
    analog_data->rs_x = 2048;
    analog_data->rs_y = 2048;

    analog_data->lt_a = 0;
    analog_data->rt_a = 0;
}

// Handle System events
void local_system_evt(hoja_system_event_t evt)
{
    const char* TAG = "local_system_evt";
    switch(evt)
    {
        esp_err_t err = ESP_OK;

        case HEVT_API_INIT_OK:
            ESP_LOGI(TAG, "HOJA initialized OK callback.");

            // Play boot animation.
            boot_anim();

            local_button_cb(&hoja_button_data);

            // Check to see what buttons are being held. Adjust state accordingly.
            if (hoja_button_data.button_start)
            {
                if (loaded_settings.controller_mode != HOJA_CONTROLLER_MODE_RETRO)
                {
                    loaded_settings.controller_mode = HOJA_CONTROLLER_MODE_RETRO;
                    hoja_settings_saveall();
                }
            }

            // Get boot mode and it will perform a callback.
            err = util_battery_boot_status();
            if (err != HOJA_OK)
            {
                ESP_LOGE(TAG, "Issue when getting boot battery status.");
            }

            break;

        case HEVT_API_SHUTDOWN:
            if (!local_cable_plugged)
            {
                enter_sleep();
            }
            else
            {
                enter_reboot();
            }
            break;
        case HEVT_API_REBOOT:
            enter_reboot();
            break;
    }
}

void local_bt_evt(hoja_bt_event_t evt)
{

}

void local_usb_evt(hoja_usb_event_t evt)
{

}

void local_gc_evt(hoja_gc_event_t evt, uint8_t param)
{

}

void local_ns_evt(hoja_ns_event_t evt, uint8_t param)
{

}

void local_wired_evt(hoja_wired_event_t evt)
{
    const char* TAG = "local_wired_evt";
    hoja_err_t err = HOJA_OK;

    switch(evt)
    {
        default:
        case HEVT_WIRED_NO_DETECT:
            err = HOJA_FAIL;
            break;
        
        case HEVT_WIRED_SNES_DETECT:
            hoja_set_core(HOJA_CORE_SNES);
            rgb_setall(COLOR_YELLOW, led_colors);
            err = hoja_start_core();

            break;

        case HEVT_WIRED_JOYBUS_DETECT:
            hoja_set_core(HOJA_CORE_GC);
            rgb_setall(COLOR_PURPLE, led_colors);
            err = hoja_start_core();

            break;
    }

    if (err != HOJA_OK)
    {
        ESP_LOGE(TAG, "Failed to start retro core.");
        enter_sleep();
    }
    else
    {
        ESP_LOGI(TAG, "Started retro core OK.");
        rgb_show();
    }
}

void local_battery_evt(hoja_battery_event_t evt, uint8_t param)
{
    const char* TAG = "local_battery_evt";
    hoja_err_t err = HOJA_OK;

    switch(evt)
    {
        case HEVT_BATTERY_CHARGING:
            break;

        case HEVT_BATTERY_CHARGECOMPLETE:
            break;

        case HEVT_BATTERY_NOCHARGE:
            break;

        default:
        case HEVT_BATTERY_LVLCHANGE:
            // Not implemented
            ESP_LOGE(TAG, "Not implemented.");
            break;
    }
}

void local_charger_evt(hoja_charger_event_t evt)
{
    const char* TAG = "local_charger_evt";
    switch(evt)
    {
        case HEVT_CHARGER_PLUGGED:
            break;
        case HEVT_CHARGER_UNPLUGGED:
            break;
    }
}

void local_boot_evt(hoja_boot_event_t evt)
{
    esp_err_t err;
    const char* TAG = "local_boot_evt";
    switch(evt)
    {
        case HEVT_BOOT_NOBATTERY:
            ESP_LOGI(TAG, "No battery detected.");
        case HEVT_BOOT_PLUGGED:
            if (evt == HEVT_BOOT_PLUGGED)
            {
                ESP_LOGI(TAG, "Plugged in.");
            }   
            /*if (loaded_settings.controller_mode == HOJA_CONTROLLER_MODE_RETRO)
            {
                
            }
            err = util_wired_detect_loop();
            if (!err)
            {
                ESP_LOGI(TAG, "Started wired retro loop OK.");
                rgb_setall(COLOR_RED, led_colors);
                rgb_show();
            }
            else
            {
                ESP_LOGE(TAG, "Failed to start wired retro loop.");
            }*/

            // USB test
            /*
            hoja_set_core(HOJA_CORE_USB);
            core_usb_set_subcore(USB_SUBCORE_XINPUT);
            */

            // BT DInput test
            hoja_set_core(HOJA_CORE_BT_DINPUT);

            err = hoja_start_core();

            if (err == ESP_OK)
            {
                rgb_setall(COLOR_GREEN, led_colors);
                rgb_show();
            }
            

            break;

        case HEVT_BOOT_UNPLUGGED:
            ESP_LOGI(TAG, "Unplugged.");
            enter_sleep();
            break;
    }
}

// Callback to handle HOJA events
void local_event_cb(hoja_event_type_t type, uint8_t evt, uint8_t param)
{   
    switch(type)
    {
        default:
            ESP_LOGI("local_event_cb", "Unrecognized event occurred: %X", (unsigned int) type);
            break;

        case HOJA_EVT_BOOT:
            local_boot_evt(evt);
            break;

        case HOJA_EVT_SYSTEM:
            local_system_evt(evt);
            break;

        case HOJA_EVT_CHARGER:
            local_charger_evt(evt);
            break;

        case HOJA_EVT_BT:
            local_bt_evt(evt);
            break;

        case HOJA_EVT_BATTERY:
            local_battery_evt(evt, param);
            break;

        case HOJA_EVT_GC:
            local_gc_evt(evt, param);
            break;

        case HOJA_EVT_NS:
            local_ns_evt(evt, param);
            break;

        case HOJA_EVT_USB:
            local_usb_evt(evt);
            break;

        case HOJA_EVT_WIRED:
            local_wired_evt(evt);
            break;
    }
}

void app_main()
{
    const char* TAG = "app_main";

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

    util_i2c_initialize();
    util_battery_set_type(BATTYPE_BQ25180);
    util_rgb_init(led_colors, RGB_MODE_GRB);

    hoja_register_button_callback(local_button_cb);
    hoja_register_analog_callback(local_analog_cb);
    hoja_register_event_callback(local_event_cb);

    err = hoja_init();
}
