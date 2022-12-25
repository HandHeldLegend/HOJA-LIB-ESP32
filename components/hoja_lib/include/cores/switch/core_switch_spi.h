#ifndef CORE_SWITCH_SPI_H
#define CORE_SWITCH_SPI_H

#include "core_switch_backend.h"

// Byte index for start read address on output report
#define SPI_STARTREAD_IDX   10

// Byte index for where to place SPI read data for input report
#define SPI_READ_OUTPUT_IDX 19

/**
 * These are the memory addresses we can use
 * as indicators as to which SPI data should be returned
 */
#define SPI_READ_SHIPMENT               0x5000
#define SPI_READLN_SHIPMENT             1

// Read Serial Number
#define SPI_READ_SERIALNUMBER           0x6000
#define SPI_READLEN_SERIALNUMBER        16

#define SPI_READ_FACTORY_SENSORSTICK    0x6080
#define SPI_READ_FACTORY_SENSORSTICK_2  0x6098
#define SPI_READLEN_FACTORY_SENSORSTICK 24

#define SPI_READ_COLORVALUES            0x6050
#define SPI_READLEN_COLORVALUES         24


#define SPI_READ_USER_STICKCONFIG       0x8010
#define SPI_READLEN_USER_STICKCONFIG    22

#define SPI_READ_FACTORY_CONFIGCALIB    0x6020
#define SPI_READ_FACTORY_CONFIGCALIB_2  0x603D
#define SPI_READLEN_FACTORYCONFIGCALIB  25

/**
 * These are the different emulated memory maps which we can refer to later.
 * The values should be pointers if the data is primarily used elsewhere.
 * The primary reason this exists is so we can more easily handle SPI read
 * and write commands and avoid having to re-write a ton of logic in the case
 * some of these items break/change over time.
 */

/* Internal functions for handling SPI read calls and the correct responses */
void ns_spi_readfromaddress(uint8_t offset_address, uint8_t address, uint8_t length);

void ns_spi_writetoaddress(uint32_t address, uint8_t length);

// Returns single byte located at address.
uint8_t ns_spi_getaddressdata(uint8_t offset_address, uint8_t address);

#endif
