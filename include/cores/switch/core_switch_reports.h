#ifndef CORE_SWITCH_REPORTS_H
#define CORE_SWITCH_REPORTS_H

#include "core_switch_backend.h"
#include "switch/core_switch_types.h"

/** 
 * These functions and parameters are all related
 * to the building of input reports for various
 * controller configurations and different response types
 */

// Larger buffer size than we need.
#define NS_INPUT_REPORT_BUFFERSIZE  60

/**
 * @brief Input report array. Items in this array
 * get set before sending a response report.
 */
extern uint8_t ns_input_report[NS_INPUT_REPORT_BUFFERSIZE];
extern uint8_t ns_input_report_id;
extern uint16_t ns_input_report_size;

//#define INPUT_FREQUENCY_FAST    9 //9
#define INPUT_FREQUENCY_SLOW    20
#define INPUT_FREQUENCY_FAST    10

void ns_report_clear(void);

void ns_report_setid(uint8_t report_id);

void ns_report_setack(uint8_t ack);

void ns_report_setsubcmd(uint8_t sub_cmd);

void ns_report_settimer(void);

void ns_report_setbattconn(void);

void ns_report_setinputreport_full(ns_input_long_s *ns_input_long);

void ns_report_setinputreport_short(ns_input_short_s *ns_input_short);

void ns_report_bulkset(uint8_t start_idx, uint8_t* data, uint8_t len);

void ns_report_sub_setdevinfo(void);

void ns_report_sub_triggertime(uint16_t time_10_ms);

void ns_report_sub_setspidata(uint8_t* data, uint8_t len);

// Input Report Send Tasks
void ns_report_task_sendshort(void * parameters);

void ns_report_task_sendstandard(void * parameters);

void ns_report_task_sendempty(void * parameters);

#endif
