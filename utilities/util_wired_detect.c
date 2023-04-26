#include "util_wired_detect.h"

TaskHandle_t util_wired_loop_task = NULL;

#define PCNT_HIGH_LIMIT 64
#define PCNT_LOW_LIMIT -64

#define PCNT_SNES_COUNT 15
#define PCNT_NS_COUNT   4

// PRIVATE FUNCTIONS
// -----------------
static bool wired_pcnt_on_reach(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_wakeup;
    QueueHandle_t queue = (QueueHandle_t)user_ctx;
    // send event data to queue, from this interrupt callback
    xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);
    return (high_task_wakeup == pdTRUE);
}

util_wire_det_t util_wired_get(void)
{
    const char* TAG = "wired_detect";

    util_wire_det_t detected_type = DETECT_NONE;

    QueueHandle_t pcnt_evt_queue = xQueueCreate(10, sizeof(pcnt_evt_t));   // A queue to handle pulse counter events

    pcnt_unit_config_t unit_config = {
        .high_limit = PCNT_HIGH_LIMIT,
        .low_limit = PCNT_LOW_LIMIT,
    };
    pcnt_unit_handle_t joybus_pcnt_unit = NULL;
    pcnt_unit_handle_t nes_pcnt_unit = NULL;
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &joybus_pcnt_unit));
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &nes_pcnt_unit));

    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 500,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(joybus_pcnt_unit, &filter_config));
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(nes_pcnt_unit, &filter_config));

    // First, set up for Joybus detect.
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = CONFIG_HOJA_GPIO_NS_SERIAL,
        .level_gpio_num = -1,
    };

    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = CONFIG_HOJA_GPIO_NS_CLOCK,
        .level_gpio_num = -1,
    };

    pcnt_channel_handle_t pcnt_chan_a = NULL;
    pcnt_channel_handle_t pcnt_chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(joybus_pcnt_unit, &chan_a_config, &pcnt_chan_a));
    ESP_ERROR_CHECK(pcnt_new_channel(nes_pcnt_unit, &chan_b_config, &pcnt_chan_b));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_HOLD, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_HOLD, PCNT_CHANNEL_EDGE_ACTION_INCREASE));

    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(joybus_pcnt_unit, PCNT_NS_COUNT));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(nes_pcnt_unit, PCNT_NS_COUNT));

    pcnt_event_callbacks_t cbs = {
        .on_reach = wired_pcnt_on_reach,
    };

    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(joybus_pcnt_unit, &cbs, pcnt_evt_queue));
    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(nes_pcnt_unit, &cbs, pcnt_evt_queue));

    gpio_pullup_en(CONFIG_HOJA_GPIO_NS_SERIAL);

    ESP_LOGI(TAG, "enable pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_enable(joybus_pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_enable(nes_pcnt_unit));

    ESP_LOGI(TAG, "clear pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_clear_count(joybus_pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(nes_pcnt_unit));

    ESP_LOGI(TAG, "start pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_start(joybus_pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(nes_pcnt_unit));

    int joybus_pulse_count = 0;
    int nes_pulse_count = 0;
    int event_count = 0;
    bool done = false;

    while(!done)
    {
        if (xQueueReceive(pcnt_evt_queue, &event_count, pdMS_TO_TICKS(1000))) {
            pcnt_unit_get_count(joybus_pcnt_unit, &joybus_pulse_count);
            pcnt_unit_get_count(nes_pcnt_unit, &nes_pulse_count);
            if (joybus_pulse_count >= PCNT_NS_COUNT)
            {
                ESP_LOGI(TAG, "Joybus Detected with counts: %d", joybus_pulse_count);
                done = true;
                detected_type = DETECT_JOYBUS;
            }
            else if (nes_pulse_count >= PCNT_NS_COUNT)
            {
                ESP_LOGI(TAG, "SNES Detected with counts: %d", nes_pulse_count);
                done = true;
                detected_type = DETECT_SNES;
            }
        } else {
            ESP_LOGI(TAG, "Retro not detected.");
            done = true;
        }
    }
    

    pcnt_unit_clear_count(joybus_pcnt_unit);
    pcnt_unit_clear_count(nes_pcnt_unit);

    pcnt_unit_stop(joybus_pcnt_unit);
    pcnt_unit_stop(nes_pcnt_unit);

    pcnt_unit_disable(joybus_pcnt_unit);
    pcnt_unit_disable(nes_pcnt_unit);

    pcnt_del_channel(pcnt_chan_a);
    pcnt_del_channel(pcnt_chan_b);

    pcnt_del_unit(joybus_pcnt_unit);
    pcnt_del_unit(nes_pcnt_unit);

    return detected_type;
}

void util_wired_detect_task(void * params)
{
    ESP_LOGI("util_wired_detect_task", "Starting task to find wired connection...");
    for(;;)
    {
         // IO configuration we can reuse
        gpio_config_t io_conf = {};

        // Set up IO pins for scanning button matrix
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.pin_bit_mask = (1ULL << CONFIG_HOJA_GPIO_NS_SERIAL);
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        gpio_config(&io_conf);

        util_wire_det_t type = util_wired_get();
        // Send appropriate event to have action taken
        switch(type)
        {
            default:
            case DETECT_NONE:
                break;
            case DETECT_JOYBUS:
                hoja_event_cb(HOJA_EVT_WIRED, HEVT_WIRED_GAMECUBE_DETECT, NULL);
                vTaskDelete(util_wired_loop_task);
                util_wired_loop_task = NULL;
                break;
            case DETECT_SNES:
                hoja_event_cb(HOJA_EVT_WIRED, HEVT_WIRED_SNES_DETECT, NULL);
                vTaskDelete(util_wired_loop_task);
                util_wired_loop_task = NULL;
                break;
        }
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
}
// -----------------
// -----------------

// PUBLIC FUNCTIONS
// -----------------

/**
 * @brief Runs once and spits out an event callback telling you which
 * retro console has been detected.
*/
void util_wired_detect(void)
{
    util_wire_det_t type = util_wired_get();

    // Send appropriate event to have action taken
    switch(type)
    {
        default:
        case DETECT_NONE:
            hoja_event_cb(HOJA_EVT_WIRED, HEVT_WIRED_NO_DETECT, 0x00);
            break;
        case DETECT_JOYBUS:
            hoja_event_cb(HOJA_EVT_WIRED, HEVT_WIRED_GAMECUBE_DETECT, 0x00);
            break;
        case DETECT_SNES:
            hoja_event_cb(HOJA_EVT_WIRED, HEVT_WIRED_SNES_DETECT, 0x00);
            break;
    }
}

/**
 * @brief Starts a loop that will run until it detects a retro console.
 * It then spits out an event callback telling you which retro console
 * has been detected.
*/
hoja_err_t util_wired_detect_loop(void)
{
    if (util_wired_loop_task != NULL)
    {
        vTaskDelete(util_wired_loop_task);
        util_wired_loop_task = NULL;
    }

    xTaskCreatePinnedToCore(util_wired_detect_task, "Wired Utility Detect Loop", 4048, NULL, 2, util_wired_loop_task, HOJA_CORE_CPU);

    return HOJA_OK;
}

