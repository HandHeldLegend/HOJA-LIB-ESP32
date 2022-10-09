#ifndef UTIL_RGB_H
#define UTIL_RGB_H

#include "hoja_includes.h"

#define RGB_SPIBUS  SPI3_HOST

#define RGB_MODE_RGB    0
#define RGB_MODE_GRB    1

#define RGB_T0H     1
#define RGB_T0L     2
#define RGB_T1H     2
#define RGB_T1L     1

#define RGB_HIGH    0x6
#define RGB_LOW     0x4

#define LED_COUNT 4
// How many bytes per LED?
#define RGB_BYTE_MULTIPLIER 9

typedef struct
{
    union
    {
        struct
        {
            uint8_t blue;
            uint8_t green;
            uint8_t red;
            uint8_t dummy;
        };
        uint32_t rgb;
    };
    
} rgb_s;

// We use this type to set the bits
// appropriately for the SPI output.
// Easier to work with one 32 bit int
// instead of each individual byte.
// 3 bytes are allocated to each color
typedef struct 
{
    union
    {
        struct 
        {
            uint8_t byte2;
            uint8_t byte1;
            uint8_t byte0;
            uint8_t dummy;
        };
        uint32_t splitter; 
    };
} rgb_splitter_s;

typedef struct
{
    spi_device_handle_t *spi_handle;
    rgb_s colors1[CONFIG_HOJA_RGB_COUNT];
    rgb_s colors2[CONFIG_HOJA_RGB_COUNT];
    uint16_t frames;
} rgb_anim_s;

#define COLOR_BLACK     (rgb_s) {.rgb = 0x000000}
#define COLOR_RED       (rgb_s) {.rgb = 0xFF0000}
#define COLOR_ORANGE    (rgb_s) {.rgb = 0xFF8000}
#define COLOR_YELLOW    (rgb_s) {.rgb = 0xFFFF00}
#define COLOR_GREEN     (rgb_s) {.rgb = 0x00FF00}
#define COLOR_TEAL      (rgb_s) {.rgb = 0x00FF80}
#define COLOR_CYAN      (rgb_s) {.rgb = 0x00FFFF}
#define COLOR_BLUE      (rgb_s) {.rgb = 0x0000FF}
#define COLOR_VIOLET    (rgb_s) {.rgb = 0x8000FF}
#define COLOR_PURPLE    (rgb_s) {.rgb = 0xFF00FF}



void rgb_brightness_set(uint8_t brightness, rgb_s *led_colors);

// Blend two colors together.
// color1 - The first color
// color2 - The second color
// blend_amount - The amount to blend the two colors. Between 0 and 255.
// Returns an rgb_s color type.
rgb_s rgb_blend(rgb_s color1, rgb_s color2, uint8_t blend_amount);

void rgb_create_packet(uint8_t *buffer, rgb_s *led_colors, uint8_t led_count, uint8_t rgb_mode);

// Update the LED colors and brightness.
void rgb_show(rgb_s *colors, uint8_t brightness);

// Initialize RGB Controller Backend
// Only support WS2812B Protocol!
hoja_err_t rgb_init();

#endif