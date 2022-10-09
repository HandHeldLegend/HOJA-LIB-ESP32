#include "util_rgb.h"

TaskHandle_t rgb_service_handle = NULL;
TaskHandle_t rgb_anim_handle = NULL;
spi_device_handle_t rgb_spi = NULL;

void rgb_service_task(void * parameters)
{
    // LEDs can have one update every 10ms (~100-120hz)
    for(;;)
    {
        hoja_rgb_cb();
        vTaskDelay(10/portTICK_RATE_MS);
    }
}

void rgb_brightness_set(uint8_t brightness, rgb_s *led_colors)
{
    float ratio = (float) brightness / 255.0;
    for (uint8_t i = 0; i < CONFIG_HOJA_RGB_COUNT; i++)
    {
        float newRed    = (float) led_colors[i].red     * ratio;
        float newGreen  = (float) led_colors[i].green   * ratio;
        float newBlue   = (float) led_colors[i].blue    * ratio;
        led_colors[i].red = (uint8_t) newRed;
        led_colors[i].green = (uint8_t) newGreen;
        led_colors[i].blue = (uint8_t) newBlue;
    }
}

rgb_s rgb_blend(rgb_s color1, rgb_s color2, uint8_t blend_amount)
{
    float ratio = blend_amount/255;
    int rdif = abs((color1.red + color2.red))    /2;
    int gdif = abs((color1.green + color2.green))/2;
    int bdif = abs((color1.blue + color2.blue))  /2;

    rdif = rdif * ratio;
    gdif = gdif * ratio;
    bdif = bdif * ratio;

    rgb_s output = {
        .red = color1.red + (uint8_t) rdif,
        .green = color1.green + (uint8_t) gdif,
        .blue = color1.blue + (uint8_t) bdif,
    };

    return output;
}

void rgb_create_packet(uint8_t *buffer, rgb_s *led_colors, uint8_t led_count, uint8_t rgb_mode)
{
    const char* TAG = "rgb_create_packet";

    // Clear the buffer
    memset(buffer, 0x0, CONFIG_HOJA_RGB_COUNT*RGB_BYTE_MULTIPLIER);
    uint8_t buffer_idx = 0;
    // Set up a splitter

    rgb_splitter_s s_red    = {0};
    rgb_splitter_s s_green  = {0};
    rgb_splitter_s s_blue   = {0};

    // Cycle through each LED first
    for(uint8_t i = 0; i < CONFIG_HOJA_RGB_COUNT; i++)
    {
        // There are three bytes to contend with
        // for each LED. Each color is split up to occupy three SPI bytes.
        // We have to cycle through each bit of each color and shift in the
        // appropriate data.
        memset(&s_red, 0, sizeof(rgb_splitter_s));
        memset(&s_green, 0, sizeof(rgb_splitter_s));
        memset(&s_blue, 0, sizeof(rgb_splitter_s));
        
        // Keep track of which bit
        // we are setting with an index.
        uint8_t s_idx   = 21;

        for(uint8_t b = 0; b < 8; b++)
        {
            uint8_t red_bit = 0;
            uint8_t green_bit = 0;
            uint8_t blue_bit = 0;

            // Account for GRB mode
            if (rgb_mode)
            {
                red_bit     = (led_colors[i].green  >> (7-b)) & 1;
                green_bit   = (led_colors[i].red    >> (7-b)) & 1;
            }
            else
            {
                red_bit     = (led_colors[i].red    >> (7-b)) & 1;
                green_bit   = (led_colors[i].green  >> (7-b)) & 1;
            }
            
            blue_bit   =    (led_colors[i].blue  >> (7-b)) & 1;
            
            // Set the bits in the splitter
            if (red_bit)
            {
                s_red.splitter      |= (RGB_HIGH << (s_idx));
            }
            else
            {
                s_red.splitter      |= (RGB_LOW << (s_idx));
            }

            if (green_bit)
            {
                s_green.splitter      |= (RGB_HIGH << (s_idx));
            }
            else
            {
                s_green.splitter      |= (RGB_LOW << (s_idx));
            }

            if (blue_bit)
            {
                s_blue.splitter      |= (RGB_HIGH << (s_idx));
            }
            else
            {
                s_blue.splitter      |= (RGB_LOW << (s_idx));
            }

            s_idx   -= 3;
        }

        // Once we've processed all 8 bits of the three colors, copy to our SPI buffer
        buffer[buffer_idx] = s_red.byte0;
        buffer[buffer_idx+1] = s_red.byte1;
        buffer[buffer_idx+2] = s_red.byte2;

        buffer[buffer_idx+3] = s_green.byte0;
        buffer[buffer_idx+4] = s_green.byte1;
        buffer[buffer_idx+5] = s_green.byte2;

        buffer[buffer_idx+6] = s_blue.byte0;
        buffer[buffer_idx+7] = s_blue.byte1;
        buffer[buffer_idx+8] = s_blue.byte2;

        // Increase our buffer idx
        buffer_idx += 9;
    }

}

hoja_err_t rgb_init()
{

    esp_err_t err; 

    // Set up SPI for rgb
    
    // Configuration for the SPI bus
    spi_bus_config_t buscfg={
        .mosi_io_num    = (int) CONFIG_HOJA_RGB_GPIO,
        .miso_io_num    = -1,
        .sclk_io_num    = -1,
        .quadwp_io_num  = -1,
        .quadhd_io_num  = -1,
    };

    // Configuration for the SPI master interface
    spi_device_interface_config_t devcfg={
        .mode           = 0,
        .clock_speed_hz = (APB_CLK_FREQ/32), //2.5Mhz
        .spics_io_num   = -1,
        .queue_size     = 7,
    };

    err = spi_bus_initialize(RGB_SPIBUS, &buscfg, SPI_DMA_CH_AUTO);

    err = spi_bus_add_device(RGB_SPIBUS, &devcfg, &rgb_spi);

    rgb_anim_s anim_data = {
        .colors1 = {COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK},
        .colors2 = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW},
        .frames = 120,
        .spi_handle = &rgb_spi,
    };

    return HOJA_OK;
}