#ifndef RSC_DESCRIPTORS_H
#define RSC_DESCRIPTORS_H

#define HID_PROD_XINPUT 0x0B13
#define HID_VEND_XINPUT 0x045E

#define HID_PROD_GCA    0x0337
#define HID_VEND_GCA    0x057E

#define HID_PROD_NSPRO  0x2009
#define HID_VEND_NSPRO  0x057E

#define HID_PROD_DINPUT 0xA713
#define HID_VEND_DINPUT 0x20D6

#include "hoja_includes.h"

extern const uint8_t bt_hid_report_descriptor[94];

extern const uint8_t xinput_hid_report_descriptor[306];

extern const uint8_t procon_hid_descriptor[134];

extern const uint8_t gc_hid_report_descriptor[106];

#endif