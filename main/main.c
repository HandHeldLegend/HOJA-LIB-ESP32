#include "main.h"

// Scanning pins for keypad config
#define GPIO_BTN_SCANA      GPIO_NUM_26
#define GPIO_BTN_SCANB      GPIO_NUM_33
#define GPIO_BTN_SCANC      GPIO_NUM_23
#define GPIO_BTN_SCAND      GPIO_NUM_17

// Port pins for keypad config
#define GPIO_BTN_PORTA      GPIO_NUM_27  
#define GPIO_BTN_PORTB      GPIO_NUM_25
#define GPIO_BTN_PORTC      GPIO_NUM_32
#define GPIO_BTN_PORTD      GPIO_NUM_18

// Button pins (mostly uneeded but looks nicer in code)
#define GPIO_BTN_A          GPIO_NUM_26
#define GPIO_BTN_B          GPIO_NUM_33
#define GPIO_BTN_X          GPIO_NUM_23
#define GPIO_BTN_Y          GPIO_NUM_17
#define GPIO_BTN_DU         GPIO_NUM_23 
#define GPIO_BTN_DL         GPIO_NUM_26
#define GPIO_BTN_DD         GPIO_NUM_17
#define GPIO_BTN_DR         GPIO_NUM_33
#define GPIO_BTN_L          GPIO_NUM_33    
#define GPIO_BTN_ZL         GPIO_NUM_26
#define GPIO_BTN_R          GPIO_NUM_23
#define GPIO_BTN_ZR         GPIO_NUM_17
#define GPIO_BTN_START      GPIO_NUM_33
#define GPIO_BTN_SELECT     GPIO_NUM_23
#define GPIO_BTN_HOME       GPIO_NUM_26
#define GPIO_BTN_CAPTURE    GPIO_NUM_17

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
#define GPIO_INPUT_PIN_MASK     ( (1ULL<<GPIO_BTN_SCANA) | (1ULL<<GPIO_BTN_SCANB) | (1ULL<<GPIO_BTN_SCANC) | (1ULL<<GPIO_BTN_SCAND) )
#define GPIO_INPUT_PORT_MASK    ( (1ULL<<GPIO_BTN_PORTA) | (1ULL<<GPIO_BTN_PORTB) | (1ULL<<GPIO_BTN_PORTC) | (1ULL<<GPIO_BTN_PORTD) )

// Masks to clear all relevant bits when doing keypad scan
#define GPIO_INPUT_CLEAR0_MASK  ( (1ULL<<GPIO_BTN_PORTA) | (1ULL<<GPIO_BTN_PORTB) | (1ULL<<GPIO_BTN_PORTD) )
#define GPIO_INPUT_CLEAR1_MASK  ( (1ULL<<(GPIO_BTN_PORTC-32)) )

// Define pins for Nintendo NES/SNES wired pad latch/clock
#define PAD_PIN_CLOCK   GPIO_NUM_15
#define PAD_PIN_LATCH   GPIO_NUM_14

// Define pin for Nintendo wired serial line (GameCube and SNES)
#define PAD_PIN_SERIAL  GPIO_NUM_19

// Variables used to store register reads
uint32_t regread_low = 0;
uint32_t regread_high = 0;

bool getbit(uint32_t bytes, uint8_t bit)
{
    return (bytes >> bit) & 0x1;
}

// Set up function to update inputs
// This will scan the sticks/buttons once
// at a refresh rate determined by the core.

// Used to determine delay period between each scan (microseconds)
#define US_READ  15

void button_task()
{
    // First set port D as low output
    GPIO.out_w1tc = (uint32_t) (1ULL<<GPIO_BTN_PORTD);
    ets_delay_us(US_READ);

    // Read the GPIO registers and mask the data
    regread_low = REG_READ(GPIO_IN_REG) & GPIO_INPUT_PIN_MASK;
    regread_high = REG_READ(GPIO_IN1_REG) & 0x2;
    
    // Grab the relevant button data
    // We OR-EQUALS because we don't want the possibility
    // of a button input getting dropped.
    g_button_data.b_right   |= !getbit(regread_low, GPIO_BTN_A);
    g_button_data.b_down    |= !getbit(regread_high, GPIO_BTN_B-32);
    g_button_data.b_up      |= !getbit(regread_low, GPIO_BTN_X);
    g_button_data.b_left    |= !getbit(regread_low, GPIO_BTN_Y);

    // Release port D Set port C
    GPIO.out_w1ts = (uint32_t) (1ULL<<GPIO_BTN_PORTD);
    GPIO.out1_w1tc.val = (uint32_t) 0x1; //GPIO_BTN_PORTC
    ets_delay_us(US_READ);

    // Read the GPIO registers and mask the data
    regread_low = REG_READ(GPIO_IN_REG) & GPIO_INPUT_PIN_MASK;
    regread_high = REG_READ(GPIO_IN1_REG) & 0x2;

    g_button_data.b_home    |= !getbit(regread_low, GPIO_BTN_HOME);
    g_button_data.b_start   |= !getbit(regread_high, GPIO_BTN_START-32);
    g_button_data.t_r       |= !getbit(regread_low, GPIO_BTN_R);
    g_button_data.t_zr      |= !getbit(regread_low, GPIO_BTN_ZR);

    // Release port C set port B
    GPIO.out1_w1ts.val = (uint32_t) 0x01; //GPIO_BTN_PORTC
    GPIO.out_w1tc = (uint32_t) (1ULL<<GPIO_BTN_PORTB);
    ets_delay_us(US_READ);

    // Read the GPIO registers and mask the data
    regread_low = REG_READ(GPIO_IN_REG) & GPIO_INPUT_PIN_MASK;
    regread_high = REG_READ(GPIO_IN1_REG) & 0x2;

    g_button_data.d_left        |= !getbit(regread_low, GPIO_BTN_DL);
    g_button_data.d_right       |= !getbit(regread_high, GPIO_BTN_DR-32);
    g_button_data.b_select      |= !getbit(regread_low, GPIO_BTN_SELECT);
    g_button_data.b_capture     |= !getbit(regread_low, GPIO_BTN_CAPTURE);

    // Release port B set port A
    GPIO.out_w1ts = (uint32_t) (1ULL<<GPIO_BTN_PORTB);
    GPIO.out_w1tc = (uint32_t) (1ULL<<GPIO_BTN_PORTA);
    ets_delay_us(US_READ);

    // Read the GPIO registers and mask the data
    regread_low = REG_READ(GPIO_IN_REG) & GPIO_INPUT_PIN_MASK;
    regread_high = REG_READ(GPIO_IN1_REG) & 0x2;
    // Release port A
    GPIO.out_w1ts = (uint32_t) (1ULL<<GPIO_BTN_PORTA);

    g_button_data.t_zl      |= !getbit(regread_low, GPIO_BTN_ZL);
    g_button_data.t_l       |= !getbit(regread_high, GPIO_BTN_L-32);
    g_button_data.d_up      |= !getbit(regread_low, GPIO_BTN_DU);
    g_button_data.d_down    |= !getbit(regread_low, GPIO_BTN_DD);
}

// Separate task to read sticks.
// This is essential to have as a separate component as ADC scans typically take more time and this is only
// scanned once between each polling interval. This varies from core to core.
void stick_task()
{
    const char* TAG = "stick_task";
    // read stick 1 and 2

    g_stick_data.lsx = (uint16_t) adc1_get_raw(ADC_STICK_LX);
    g_stick_data.lsy = (uint16_t) adc1_get_raw(ADC_STICK_LY);
    g_stick_data.rsx = (uint16_t) adc1_get_raw(ADC_STICK_RX);
    g_stick_data.rsy = (uint16_t) adc1_get_raw(ADC_STICK_RY);

    return;
}

void app_main()
{
    const char* TAG = "app_main";

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
    GPIO.out1_w1ts.val = (uint32_t) 0x1;

    // Set up IO pins for Nintendo Pad Wired
    HOJA_PIN_SERIAL   = PAD_PIN_SERIAL;
    HOJA_PIN_CLOCK    = PAD_PIN_CLOCK;
    HOJA_PIN_LATCH    = PAD_PIN_LATCH;

    HOJA_PIN_I2C_SCL  = GPIO_NUM_22;
    HOJA_PIN_I2C_SDA  = GPIO_NUM_21;

    hoja_api_regbuttoncallback(button_task);
    hoja_api_regstickcallback(stick_task);

    hoja_api_init();

    core_usb_start();

}
