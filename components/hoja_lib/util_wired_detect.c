#include "util_wired_detect.h"

xQueueHandle pcnt_evt_queue;   // A queue to handle pulse counter events

static void IRAM_ATTR pcnt_intr_handler(void *arg)
{
    int pcnt_unit = (int)arg;
    pcnt_evt_t evt;
    evt.unit = pcnt_unit;
    /* Save the PCNT event type that caused an interrupt
       to pass it to the main program */
    pcnt_get_event_status(pcnt_unit, &evt.status);
    xQueueSendFromISR(pcnt_evt_queue, &evt, NULL);
}

util_wire_det_t wired_detect()
{
    const char* TAG = "wired_detect";

    util_wire_det_t detected_type = DETECT_NONE;

    /* Initialize PCNT event queue and PCNT functions */
    pcnt_evt_queue = xQueueCreate(10, sizeof(pcnt_evt_t));

    // Set up Counter
    pcnt_config_t pcnt_config = {
        .pulse_gpio_num = CONFIG_HOJA_GPIO_NS_SERIAL,
        .ctrl_gpio_num = -1,
        .unit = PCNT_UNIT_0,
        .channel = PCNT_CHANNEL_0,
        .pos_mode = PCNT_COUNT_DIS,
        .neg_mode = PCNT_COUNT_INC, 
        .lctrl_mode = PCNT_MODE_KEEP,
        .hctrl_mode = PCNT_MODE_KEEP,
        .counter_h_lim = 64,
        .counter_l_lim = -10,
    };

    pcnt_unit_config(&pcnt_config);

    pcnt_set_event_value(PCNT_UNIT_0, PCNT_EVT_THRES_0, PULSE_MAX_WIRED);
    pcnt_event_enable(PCNT_UNIT_0, PCNT_EVT_THRES_0);
    pcnt_counter_pause(PCNT_UNIT_0);
    pcnt_counter_clear(PCNT_UNIT_0);

    /* Install interrupt service and add isr callback handler */
    pcnt_isr_service_install(0);
    pcnt_isr_handler_add(PCNT_UNIT_0, pcnt_intr_handler, (void *)PCNT_UNIT_0);
    pcnt_counter_resume(PCNT_UNIT_0);

    int done = 0;
    pcnt_evt_t evt;
    portBASE_TYPE res;
    int16_t count_out = 0;

    vTaskDelay(100/portTICK_PERIOD_MS);

    while (!done)
    {
        /* Wait for the event information passed from PCNT's interrupt handler.
         * Once received, decode the event type and print it on the serial monitor.
         */
        res = xQueueReceive(pcnt_evt_queue, &evt, 300 / portTICK_PERIOD_MS);
        if (res == pdTRUE) {
            pcnt_get_counter_value(PCNT_UNIT_0, &count_out);
            ESP_LOGI(TAG, "JOYBUS Event PCNT unit[%d]; cnt: %d", evt.unit, count_out);
            if (evt.status & PCNT_EVT_THRES_0) {
                ESP_LOGI(TAG, "THRES1 EVT");
                detected_type = DETECT_JOYBUS;
            }
            done = 1;
        }
        else{
            ESP_LOGI(TAG, "No Counts detected. Not Joybus");
            done = 1;
        }
    }
    xQueueReset(pcnt_evt_queue);
    pcnt_counter_pause(PCNT_UNIT_0);
    pcnt_counter_clear(PCNT_UNIT_0);
    done = 0;
    count_out = 0;
    pcnt_config.pulse_gpio_num = CONFIG_HOJA_GPIO_NS_CLOCK;
    pcnt_unit_config(&pcnt_config);

    pcnt_counter_clear(PCNT_UNIT_0);
    pcnt_counter_resume(PCNT_UNIT_0);

    while (!done && !detected_type)
    {
        /* Wait for the event information passed from PCNT's interrupt handler.
         * Once received, decode the event type and print it on the serial monitor.
         */
        res = xQueueReceive(pcnt_evt_queue, &evt, 300 / portTICK_PERIOD_MS);
        if (res == pdTRUE) {
            pcnt_get_counter_value(PCNT_UNIT_0, &count_out);
            ESP_LOGI(TAG, "SNES DETECT Event PCNT unit[%d]; cnt: %d", evt.unit, count_out);
            if (evt.status & PCNT_EVT_THRES_0) {
                ESP_LOGI(TAG, "THRES1 EVT");
                detected_type = DETECT_SNES;
            }
            done = 1;
        }
        else{
            ESP_LOGI(TAG, "No Counts detected. Not SNES");
            done = 1;
        }
    }

    pcnt_isr_service_uninstall();

    return detected_type;
}