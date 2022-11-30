#include "util_common.h"

/**
 * @brief Get the value of a specified bit in a 32 bit unsigned int. Returns bool
 * representing a bit being a 1 or a 0.
 * @param bytes uint32_t type. Value you want to get a bit from.
 * @param bit   uint8_t type. Bit number you want to obtain.
*/
bool util_getbit(uint32_t bytes, uint8_t bit)
{
    uint8_t tmp = bit;
    if (bit > 31)
    {
        tmp -= 32;
    }
    return (bytes >> tmp) & 0x1;
}
