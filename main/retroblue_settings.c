#include "retroblue_settings.h"

RetroBlueSettings loaded_settings = {};

void rb_settings_init(void)
{
    loaded_settings.controller_core = 0;
    loaded_settings.client_bt_address[0] = 0xEC;
    loaded_settings.client_bt_address[1] = 0x0B;
    loaded_settings.client_bt_address[2] = 0xDF;
    loaded_settings.client_bt_address[3] = 0x31;
    loaded_settings.client_bt_address[4] = 0x54;
    loaded_settings.client_bt_address[5] = 0x5A;
}