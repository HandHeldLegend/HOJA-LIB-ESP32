#ifndef RBC_UTIL_JOYBUS_RMT_TX_H
#define RBC_UTIL_JOYBUS_RMT_TX_H

#include "retroblue_backend.h"

#define RMT_TX_CHANNEL RMT_CHANNEL_0
#define RMT_RX_CHANNEL RMT_CHANNEL_1

uint32_t jb_idle_ticks;

/**
* @brief Joybus Pad Type
*
*/
typedef struct joybus_pad_s joybus_pad_t;

/**
* @brief Joybus Pad Device Type
*
*/
typedef void *joybus_pad_dev_t;

/**
* @brief Declare of Joybus Pad Type
*
*/
struct joybus_pad_s {
    /**
    * @brief Send command to GameCube or N64 console
    *
    * @param pad: Joybus Pad
    * @param cmd: unsigned 8 bit command
    * @param data: unsigned 8 bit array of command data
    * @param len: length of data in bytes
    *
    * @return
    *      - ESP_OK: Set RGB for a specific pixel successfully
    *      - ESP_ERR_INVALID_ARG: Set RGB for a specific pixel failed because of invalid parameters
    *      - ESP_FAIL: Set RGB for a specific pixel failed because other error occurred
    */
    esp_err_t (*send_command)(joybus_pad_t *pad, uint8_t cmd, uint8_t *data, uint8_t len);

    /**
    * @brief Free Joybus Pad resources
    *
    * @param pad: Joybus Pad
    *
    * @return
    *      - ESP_OK: Free resources successfully
    *      - ESP_FAIL: Free resources failed because error occurred
    */
    esp_err_t (*del)(joybus_pad_t *pad);
};

/**
* @brief Joybus Pad Configuration Type
*
*/
typedef struct {
    joybus_pad_dev_t dev; /*!< Joybus Pad device (e.g. RMT channel, PWM channel, etc) */
} joybus_pad_config_t;

/**
 * @brief Default configuration for Joybus Pad
 *
 */
#define JOYBUS_PAD_DEFAULT_CONFIG(dev_hdl) \
    {                                             \
        .dev = dev_hdl,                           \
    }

/**
* @brief Install a new Joybus Pad driver (based on RMT peripheral)
*
* @param config: Joybus Pad configuration
* @return
*      Joybus Pad instance or NULL
*/
joybus_pad_t *joybus_pad_new_rmt(const joybus_pad_config_t *config);

/**
 * @brief Init the RMT peripheral and Joybus Pad configuration.
 *
 * @param[in] channel: RMT peripheral channel number.
 * @param[in] gpio: GPIO number for the RMT data output.
 * @return
 *      Joybus Pad instance or NULL
 */
joybus_pad_t * joybus_pad_init(uint8_t channel, uint8_t gpio);

/**
 * @brief Denit the RMT peripheral.
 *
 * @param[in] pad: Joybus Pad
 * @return
 *     - ESP_OK
 *     - ESP_FAIL
 */
esp_err_t joybus_pad_denit(joybus_pad_t *pad);

#endif

