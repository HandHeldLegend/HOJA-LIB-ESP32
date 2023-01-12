#ifndef RSC_DESCRIPTORS_H
#define RSC_DESCRIPTORS_H

#define HID_PROD_XINPUT 0x0B13
#define HID_VEND_XINPUT 0x045E

#define HID_PROD_GCA    0x0337
#define HID_VEND_GCA    0x057E

#define HID_PROD_NSPRO  0x2009
#define HID_VEND_NSPRO  0x057E

#define HID_PROD_DINPUT 0xA715
#define HID_VEND_DINPUT 0x20D6

#include "hoja_includes.h"

#define DINPUT_HID_REPORT_MAP_LEN 125
extern const uint8_t dinput_hid_report_descriptor[DINPUT_HID_REPORT_MAP_LEN];

#define XINPUT_HID_REPORT_MAP_LEN 334
extern const uint8_t xinput_hid_report_descriptor[XINPUT_HID_REPORT_MAP_LEN];

#define PROCON_HID_REPORT_MAP_LEN 134
extern const uint8_t procon_hid_descriptor[PROCON_HID_REPORT_MAP_LEN];

#define GC_HID_REPORT_MAP_LEN 106
extern const uint8_t gc_hid_report_descriptor[GC_HID_REPORT_MAP_LEN];

#endif