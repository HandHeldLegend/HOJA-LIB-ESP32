#include "rbc_util_joybus_rmt.h"

// Default timings for Joybus data communication (nanoseconds)
#define JB_OUT_ZERO_LOW_NS      3000
#define JB_OUT_ZERO_HIGH_NS     1000
#define JB_OUT_ONE_LOW_NS       1000
#define JB_OUT_ONE_HIGH_NS      3000
#define JB_OUT_STOP_NS          2000

#define JB_OUT_IDLE_NS          5000

uint32_t jb_0l_ticks = 0;
uint32_t jb_0h_ticks = 0;
uint32_t jb_1l_ticks = 0;
uint32_t jb_1h_ticks = 0;
uint32_t jb_stop_ticks = 0;

uint32_t jb_idle_ticks = 0;

typedef struct {
    joybus_pad_t parent;
    rmt_channel_t rmt_channel;
    uint16_t cmd_len;
    uint8_t buffer[0];
} joybus_t;


/**
 * @brief Convert Joybus data to RMT format.
 *
 * @param[in] src: source data, to converted to RMT format
 * @param[in] dest: place where to store the convert result
 * @param[in] src_size: size of source data
 * @param[in] wanted_num: number of RMT items that want to get
 * @param[out] translated_size: number of source data that got converted
 * @param[out] item_num: number of RMT items which are converted from source data
 */
static void IRAM_ATTR joybus_rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size,
        size_t wanted_num, size_t *translated_size, size_t *item_num)
{
    if (src == NULL || dest == NULL) {
        *translated_size = 0;
        *item_num = 0;
        return;
    }
    const rmt_item32_t bit0 = {{{ jb_0l_ticks, 0, jb_0h_ticks, 1 }}};   //Logical 0
    const rmt_item32_t bit1 = {{{ jb_1l_ticks, 0, jb_1h_ticks, 1 }}};   //Logical 1
    const rmt_item32_t bitstop = {{{ jb_stop_ticks, 0, 0, 1}}};         // STOP bit
    size_t size = 0;
    size_t num = 0;
    uint8_t *psrc = (uint8_t *)src;
    rmt_item32_t *pdest = dest;
    while (size < src_size && num < wanted_num) {
        for (int i = 0; i < 8; i++) {
            // MSB first
            if (*psrc & (1 << (7 - i))) {
                pdest->val =  bit1.val;
            } else {
                pdest->val =  bit0.val;
            }
            num++;
            pdest++;
        }
        size++;
        psrc++;
    }

    // Append stop bit :)
    size++;
    num++;
    pdest->val = bitstop.val;

    *translated_size = size;
    *item_num = num;
}

static esp_err_t joybus_send_command(joybus_pad_t *pad, uint8_t cmd, uint8_t *data, uint8_t len)
{
    esp_err_t ret = ESP_OK;

    joybus_t *joybus = __containerof(pad, joybus_t, parent);
    joybus->buffer[0] = cmd;
    for (uint8_t i = 0; i < len; i++)
    {
        joybus->buffer[i+1] = data[i];
    }
    joybus->cmd_len = (len*8)+1;

    rmt_write_sample(joybus->rmt_channel, joybus->buffer, len+1, true);

    return ESP_OK;
}

static esp_err_t joybus_del(joybus_pad_t *pad)
{
    joybus_t *joybus = __containerof(pad, joybus_t, parent);
    free(joybus);
    return ESP_OK;
}

joybus_pad_t *joybus_pad_new_rmt(const joybus_pad_config_t *config)
{
    joybus_pad_t *ret = NULL;

    joybus_t *joybus = calloc(1, sizeof(joybus_t));

    uint32_t counter_clk_hz = 0;
    rmt_get_counter_clock((rmt_channel_t)config->dev, &counter_clk_hz);
    ESP_LOGI("joybus_pad_new_rmt", "clock: %d", counter_clk_hz);
    float ratio = 0.25f;

    jb_0l_ticks = (uint32_t)12;
    jb_0h_ticks = (uint32_t)4;
    jb_1l_ticks = (uint32_t)4;
    jb_1h_ticks = (uint32_t)12;
    jb_stop_ticks = (uint32_t)8;

    jb_idle_ticks = (uint32_t)(ratio * JB_OUT_IDLE_NS);

    // Set Joybus to RMT adapter
    rmt_translator_init((rmt_channel_t)config->dev, joybus_rmt_adapter);

    joybus->rmt_channel = (rmt_channel_t)config->dev;
    joybus->parent.send_command = joybus_send_command;
    joybus->parent.del = joybus_del;

    return &joybus->parent;

}

joybus_pad_t * joybus_pad_init(uint8_t channel, uint8_t gpio)
{
    static joybus_pad_t *pad;
    const char* TAG = "joybus_pad_init";

    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(gpio, channel);
    config.tx_config.carrier_freq_hz = RBC_GAMECUBE_CONTROLLER_RESOLUTION_HZ;
    config.tx_config.carrier_level = RMT_CARRIER_LEVEL_LOW;
    config.tx_config.idle_level = RMT_IDLE_LEVEL_HIGH;
    // set counter clock to 2 MHz
    config.clk_div = 20;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

    // Install joybus translation driver
    joybus_pad_config_t joybus_config = JOYBUS_PAD_DEFAULT_CONFIG((joybus_pad_dev_t) config.channel);

    pad = joybus_pad_new_rmt(&joybus_config);

    if (!pad)
    {
        ESP_LOGE(TAG, "Installing Joybus driver failed!");
    }

    return pad;
}

esp_err_t joybus_pad_denit(joybus_pad_t *pad)
{
    joybus_t *joybus = __containerof(pad, joybus_t, parent);
    ESP_ERROR_CHECK(rmt_driver_uninstall(joybus->rmt_channel));
    return pad->del(pad);
}
