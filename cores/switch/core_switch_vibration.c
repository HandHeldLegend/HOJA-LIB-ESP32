#include "core_switch_vibration.h"

void switch_vibration_parse(uint8_t* p_data)
{
    hoja_rumble_data.left_rumble = p_data[1];
    hoja_rumble_data.right_rumble = p_data[5];
}

void switch_vibration_handle(void)
{
    if (hoja_rumble_cb == NULL)
    {
        return;
    }

    hoja_rumble_cb(&hoja_rumble_data);
}