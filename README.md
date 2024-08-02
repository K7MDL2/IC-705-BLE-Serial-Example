# IC-705-BLE-Serial-Example
Demonstrates how to connect to the Icom IC-705 using Bluetooth Low Energy (BLE) Serial Port Profile for CI-V control.  This is a modified version of work done in 2018 a https://github.com/ThingEngineer/ESP32_BLE_client_uart

Most BT connections to the IC-705 to date (Aug 2024) have been made using BT classic SPP. With the newer MStack CoreS3, which uses an ESP32-S3, it does not support BT classic, only has WiFi and BT5 compatible BLE.

Pairing is simple and silent, and will auto-reconnect as long you you do not delete the Pairing from the radio.

To use it, go to the IC-705 Pairing Reception menu, then reset the M5Stack Core3. On boot up the ESP32-S3 will silently pair with the radio. The Pairing Reception menu will close quickly.  The device name (BT705 CIV Decoder) will show as connected in the Pairing/Connect menu.  To delete the device, touch the device name, disconnect, then long touch the device name, delete the pairing.

The radio specs says the 705 is using BT4.2 so requires the ESP32 be in compatibility mode. BT5 is not the same as BT4.2 and earlier.  Will be interesting to try with BLE dongles on my IC-905 and other radios like the 9700.

You will find folders here for a few enhanced versions.  One adds screen UI for title, frequency, PTT state, and connection state.  Also tests the buttons on a CoreS3 and CoreS3-SE, status will be shown on the Serial terminal.


UUID from device:
I noted that on BT Classic, the BT address is shown on the 705 Paired devices list.  For BLE the UUID I send is not used and a number is auto_assigned starting with 1, incrmenting for each new device I try pairing with a unique name.  

CI-V Pairing and Sign in state engine:
In the IC-705_BLE_Decoder_Simple program I now track the state of pairing and sign in.  If we are connected and not yet paired, we will get a reply to all 3 sign in/pairing messages. The 0x63 wil lreturn with a 1 on sucessful pairing and 0 if already paired (assuming opther messages are returned OK). Will retry signing in if we can connect but not get the expected paired and/or signed in responses.

assuming we have a BLE connction to server (radio) we get these CI-V sign in response patterns:

Send out all 3 messages to connect to CIV

// Looking at other code it it looked like it started with sending a UUID string. From where? The number on the radio is formatted as a 6-byte BT address
// The radio does not reply but otherwise works goes on and pairs, communicates.
// I think it should be a BT address but efforts to send a fake one with and without colons I got no response and the radio assigned a number starting with 1.
// I tried all sorts of comboes of numers for the BT address (hex, bcd, colons) none got a response
// Las I tried not sending msg 61 at all.  It stil lpaired and auto assigned a number.
// Conclusion: FE F1 00 61 is not the ID string, or more likely for BLE, it does not use one and alawys auto assigns a number in the radio.

// ****** Not used in latest code - left here to document attempts to send one and my observations
ADDR            = 0xFE, 0xF1, 0x00, 0x61, BT_Address string, 0xFD
radio reply     = None Observed
// ************************************************************************************************

// ********  Below is a workign sequence **********************************************************

NAME            = 0xFE, 0xF1, 0x00, 0x62, name string, 0xFD  // ASppears to be limited to 16 bytes
radio reply     = 0xFE, 0xF1, 0x00, 0x62, 0xFD

TOKEN           = 0xFE, 0xF1, 0x00, 0x63, 0xEE, 0x39, 0x09, 0x10, 0xFD
radio reply     = 0xFE, 0xF1, 0x00, 0x63, 0x00, 0xFD  //  0 is already paired PAIR = EXISTING
radio reply     = 0xFE, 0xF1, 0x00, 0x63, 0x01, 0xFD  //  1 pairing accepted  PAIR = SUCCESS 

CI-V bus access granted (CIV_CONN)
radio reply     = 0xFE, 0xF1, 0x00, 0x64, 0xFD    // CI-V buss access is granted

******************************  Notes *************************************************************

When Pairing, we get these reponses  
NAME. TOKEN, PAIR==SUCCESS, CIV_CONN

If already paired, we get these reponses
TOKEN, PAIR=EXISTING, CIV_CONN

If we are not already paired and the radio is not in pairing reception mode, there are no CIV responses but we will remain connected to the radio BLE server. We can retry until we get disconnected or power off. 

If the radio is connected to another device, then same as above, no Ci_V response but stays connected.  Radio accepts up to 4 connections so if there are enough headsets, mics, PTT buttons, CI-V monitors/decoders connected, might get a server connection fail.

