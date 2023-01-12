#include "util_gamepad.h"

/**
 * @brief Takes in a mode, and dpad inputs, and returns the proper
 * code for D-Pad Hat for HID input.
 * @param mode Type of hat_mode_t
 * @param left_right Sum of 1 - left + right dpad inputs.
 * @param up_down    Sum of 1 - down + up dpad inputs.
*/
uint8_t util_get_dpad_hat(hat_mode_t mode, uint8_t left_right, uint8_t up_down)
{
    uint8_t ret = 0;
    switch(mode)
    {
        default:
        case HAT_MODE_DINPUT:
            ret = DI_HAT_CENTER;

        if (left_right == 2)
        {
            ret = DI_HAT_RIGHT;
            if (up_down == 2)
            {
                ret = DI_HAT_TOP_RIGHT;
            }
            else if (up_down == 0)
            {
                ret = DI_HAT_BOTTOM_RIGHT;
            }
        }
        else if (left_right == 0)
        {
            ret = DI_HAT_LEFT;
            if (up_down == 2)
            {
                ret = DI_HAT_TOP_LEFT;
            }
            else if (up_down == 0)
            {
                ret = DI_HAT_BOTTOM_LEFT;
            }
        }

        else if (up_down == 2)
        {
            ret = DI_HAT_TOP;
        }
        else if (up_down == 0)
        {
            ret = DI_HAT_BOTTOM;
        }

        return ret;
        break;

        case HAT_MODE_XINPUT:
                ret = XI_HAT_CENTER;

            if (left_right == 2)
            {
                ret = XI_HAT_RIGHT;
                if (up_down == 2)
                {
                    ret = XI_HAT_TOP_RIGHT;
                }
                else if (up_down == 0)
                {
                    ret = XI_HAT_BOTTOM_RIGHT;
                }
            }
            else if (left_right == 0)
            {
                ret = XI_HAT_LEFT;
                if (up_down == 2)
                {
                    ret = XI_HAT_TOP_LEFT;
                }
                else if (up_down == 0)
                {
                    ret = XI_HAT_BOTTOM_LEFT;
                }
            }

            else if (up_down == 2)
            {
                ret = XI_HAT_TOP;
            }
            else if (up_down == 0)
            {
                ret = XI_HAT_BOTTOM;
            }

            return ret;
            break;
    }
}
