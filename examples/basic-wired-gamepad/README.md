# _BASIC GAMEPAD EXAMPLE_
This is the simplest buildable example for a HOJA controller.

## How to use this controller example
Wire up your controller according to the definitions in the main.c
All buttons should simply connect the GPIO pin to ground. Once started,
the gamepad will show as a BLE XInput gamepad for devices that have BLE support.

## Additional notes
HOJA contains some specific SDK settings. You can configure these easily using
the ESP-IDF VS Code extension. If you have any feedback on this example, please join
our discord at https://discord.gg/jFxtr9mzCB

Here are some SDK settings to look at if you are having issues
- Ensure FreeRTOS tickrate is set to 1000
- Enable BT Classic and BTC HID device
- Enable BT Dual mode
- Enable BTLE