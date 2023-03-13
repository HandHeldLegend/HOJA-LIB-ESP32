#ifndef UTIL_I2C_H
#define UTIL_I2C_H

#include "hoja_includes.h"

#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          100000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       3000

typedef enum
{
    UTIL_I2C_STATUS_AVAILABLE = 0,
    UTIL_I2C_STATUS_DISABLED  = 1,
} util_i2c_status_t;

// Used as a global variable so every core
// can be informed as to the status of i2c
extern util_i2c_status_t util_i2c_status;

hoja_err_t util_i2c_initialize(void);

#endif
