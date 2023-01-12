#ifndef CORE_SWITCH_VIBRATION_H
#define CORE_SWITCH_VIBRATION_H

#include "core_switch_backend.h"

/**
 * @brief Struct for holding vibration data
 * Makes for easier reading in other functions.
 */
struct vibration_data_s
{
    uint8_t l_high_band_freq;
    union
    {
        struct
        {
            uint8_t l_high_band_amp_lsb : 1;
            uint8_t l_high_band_amp : 7;
        };
        uint8_t l_high_band_amp_combined;
    };

    union
    {
        struct
        {
            uint8_t l_low_band_freq : 7;
            uint8_t l_low_band_freq_msb : 1;
        };
        uint8_t l_low_band_freq_combined;
    };
    uint8_t l_low_band_amp;

    uint8_t r_high_band_freq;
    union
    {
        struct
        {
            uint8_t r_high_band_amp_lsb : 1;
            uint8_t r_high_band_amp : 7;
        };
        uint8_t r_high_band_amp_combined;
    };

    union
    {
        struct
        {
            uint8_t r_low_band_freq : 7;
            uint8_t r_low_band_freq_msb : 1;
        };
        uint8_t r_low_band_freq_combined;
    };
    uint8_t r_low_band_amp;
};

struct vibration_data_s switch_vibration_parse(uint8_t* p_data);

void switch_vibration_handle(struct vibration_data_s vibration_data);

#endif
