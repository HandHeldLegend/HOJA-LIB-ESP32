/* This document will dig into the way CRC generation works using a
pre-generated lookup table. The example code is provided from 
https://github.com/darthcloud/BlueRetro/blob/master/main/wired/nsi.c 

First, a uint8_t crc is allocated to store our generated crc later.*/
uint8_t crc;

// Here's the function in question
static uint16_t nsi_items_to_bytes_crc(uint32_t item, uint8_t *data, uint32_t len, uint8_t *crc) {
    // Make a pointer to our CRC table
    const uint8_t *crc_table = nsi_crc_table;

    // Calculate the upper bit of the received data
    // This doesn't seem appropriate to call it bit_len as it
    // is the maximum bit we are using. the 'item' param is
    // an offset for where we want to start in rmt memory 
    uint32_t bit_len = item + len * 8;

    // Create a pointer to our starting RMT memory item
    // so we may increment it
    volatile uint32_t *item_ptr = &rmt_items[item].val;

    // Since the global crc has been passed as a pointer
    // we can set the CRC value to 0xFF as a starting point
    *crc = 0xFF;

    // Loop, incrementing the bit we are processing
    for (; item < bit_len; ++data) {

        // This inner loop handles the bit counting.
        // When the item is divisible by 8, it jumps to the next
        // iteration in the 'for' loop
        do {
            if (*item_ptr & BIT_ONE_MASK) {
                // If the bit is ONE
                // we XOR EQUALS the crc with the iteration
                // in the CRC table
                *crc ^= *crc_table;
            }
            
            // Increment the pointers
            ++crc_table;
            ++item_ptr;
            ++item;
        } while ((item % 8));
    }
    return item;
}

// 35 bytes are sent from the console to the controller
// byte 0 - command
// byte 1 - address 1
// byte 2 - address 2
// bytes 3 - 34 - data (32 bytes)

// the calculated CRC should be only on the 32 bytes
// In our case we are only one controller
// and we always know where the data will be.

// Output this directly to the RMT address of our choosing
void n64_calculate_crc(uint32_t *input, uint32_t *output)
{
    uint8_t bit_len = 32*8;
    uint8_t crc = 0xFF;
    // Make a pointer to our CRC table
    const uint8_t *crc_table = nsi_crc_table;
    // Set pointer at address
    volatile uint32_t *item_ptr = &input;

    // Loop, incrementing the bit we are processing
    for (uint8_t i = 0; i < bit_len; ++i) {

        for (uint8_t l = 0; l<8; l++)
        {
            rmt_item32_t item {
                .val = *item_ptr,
            }
            if (item.duration0 < item.duration1) {
                // If the bit is ONE
                // we XOR EQUALS the crc with the iteration
                // in the CRC table
                crc ^= *crc_table;
            }
            
            // Increment the pointers
            ++crc_table;
            ++item_ptr;
        }
    }

    // Set our item pointer address to the output
    item_ptr = output + 7;
    // At this point our CRC is calculated
    // Write it to the correct memory address
    for(int i = 7; i >= 0; i--)
    {
        *item_ptr = (crc & 1) ? JB_HIGH.val : JB_LOW.val;
        crc >>= 1;
        item_ptr--;
    }
}
