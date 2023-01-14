#ifndef CORE_SWITCH_COMMS_H
#define CORE_SWITCH_COMMS_H

#include "core_switch_backend.h"

typedef uint8_t ns_core_subcmd_t;

// Sub-command repsponse start index
#define SUB_C_DATA_IDX 10

// Sub-command response minimum full input report length
#define SUB_C_RESPONSE_LENGTH_DEFAULT 15

void ns_comms_handle_command(uint8_t command, uint16_t len, uint8_t* p_data);

void ns_comms_handle_subcommand(uint8_t command, uint16_t len, uint8_t* p_data);


//void ns_comms_handle_paircommand(uint8_t command, uint16_t len, uint8_t* p_data);

//void ns_comms_handle_mcucommand(uint8_t command, uint16_t len, uint8_t* p_data);

#endif
