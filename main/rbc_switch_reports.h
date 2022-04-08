#ifndef RBC_SWITCH_REPORTS_H
#define RBC_SWITCH_REPORTS_H

#include "rbc_switch_core.h"

/** 
 * These functions and parameters are all related
 * to the building of input reports for various
 * controller configurations and different response types
 */

/**
 * @brief Used to indicate which type
 * of input response to use. The long form or
 * short form.
 * @param NS_BM_SHORT Short input report - Simple report
 * @param NS_BM_LONG Long input report - Standard report
 */
// BUTTON MODES
#define NS_BM_SHORT 0
#define NS_BM_LONG  1

/**
 * @brief Input report array. Items in this array
 * get set before sending a response report.
 */
uint8_t ns_input_report[362];
uint16_t ns_input_report_size;
uint16_t ns_report_timer;

#define INPUT_FREQUENCY_FAST    16
#define INPUT_FREQUENCY_SLOW    68

uint8_t ns_input_frequency;
uint8_t ns_timeout_counter;


void ns_report_clear(void);

void ns_report_setid(uint8_t report_id);

void ns_report_setack(uint8_t ack);

void ns_report_setsubcmd(uint8_t sub_cmd);

void ns_report_settimer(void);

void ns_report_setbattconn(void);

void ns_report_setbuttons(uint8_t button_mode);

void ns_report_bulkset(uint8_t start_idx, uint8_t* data, uint8_t len);

void ns_report_sub_setdevinfo(void);

void ns_report_sub_triggertime(uint16_t time_10_ms);

void ns_report_sub_setspidata(uint8_t* data, uint8_t len);

// Input Report Send Tasks
void ns_report_task_sendshort();

void ns_report_task_sendstandard(void * parameters);

void ns_report_task_sendempty(void * parameters);

#endif
