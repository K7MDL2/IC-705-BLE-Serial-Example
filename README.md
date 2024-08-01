# IC-705-BLE-Serial-Example
Demonstrates how to connect to the Icom IC-705 using Bluetooth Low Energy (BLE) Serial Port Profile for CI-V control.  This is a modified version of work done in 2018 a https://github.com/ThingEngineer/ESP32_BLE_client_uart

Most BT connections to the IC-705 to date (Aug 2024) have been made using BT classic SPP. With the newer MStack CoreS3, which uses an ESP32-S3, it does not support BT classic, only has WiFi and BT5 compatible BLE.

Pairing is simple and silent, and will auto-reconnect as long you you do not delete the Pairing from the radio.

To use it, go to the IC-705 Pairing Reception menu, then reset the M5Stack Core3. On boot up the ESP32-S3 will silently pair with the radio. The Pairing Reception menu will close quickly.  The device name (BT705 CIV Decoder) will show as connected in the Pairing/Connect menu.  To delete the device, touch the device name, disconnect, then long touch the device name, delete the pairing.

The radio specs says the 705 is using BT4.2 so requires the ESP32 be in compatibility mode. BT5 is not the same as BT4.2 and earlier.  Will be interesting to try with BLE dongles on my IC-905 and other radios like the 9700.

You will find folders here for a few enhanced versions.  One adds screen UI for title, frequency, PTT state, and connection state.  Also tests the buttons on a CoreS3 and CoreS3-SE, status will be shown on the Serial terminal.