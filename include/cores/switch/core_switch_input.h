#ifndef CORE_SWITCH_INPUT_H
#define CORE_SWITCH_INPUT_H

#include "core_switch_backend.h"
#include "switch/core_switch_types.h"

extern ns_input_stickcaldata_s ns_input_stickcaldata;

void ns_input_translate_full(ns_input_long_s *ns_input_long);

void ns_input_translate_short(ns_input_short_s *ns_input_short);

bool ns_input_compare_short(ns_input_short_s *one, ns_input_short_s *two);

bool ns_input_compare_full(ns_input_long_s *one, ns_input_long_s *two);

#endif
