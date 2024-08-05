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
In the IC-705_BLE_Decoder_Simple program I now track the state of pairing and sign in.  The BLE scanner will be restarted on a failed connection attempt, or keep scanning looking for a qualified server (the radio).  THE messages are so far undocumented. Looking at 1 code sample found, it sends a UUID string. From where? For BT classic SPP the BT address is displayed on the radio, formatted as a 6-byte BT address with colons.  For BLE a sequencial number a assigned starting with 1. The 2nd connection is 00:00:00:00:00:02 for example.

The 3 messages to pair or reconnect are 0x61, 0x62, 0x63. 0x64 is the final reply on success. The radio does not reply to a 0x61 msg but does use the number as evidenced by it creating a new pairing entry if you keep the same name. If you change the number, keep the name, and try to connect it will fail with a disconnect event.  I tried not sending msg 61 and it still pairs and auto-assigns a number a normal.

Conclusion: 0xFE 0xF1 0x00 0x61 is the UUID. THE UUIS is 32 bytes. The total msg length must be 41 bytes to get a 0x62 msg reply. You can still get a pairing and reconnect with less but the 0x62 reply message is suppressed.

Message format details:

      // ************************************************************************************************
      
Observation: If the ID message 0x61 sent is of a certain lemgth (41 bytes, 36 for the ID) then the name (0x62) reply is sent, otherwise the 0x62 is suppressed, still get the token reply.  Maybe a reject message collision, ?  Sending a BT address results in no reply for 0x62 (Name). Still works though. If the state engine requries the name reply mesg though, then have to get the ID right.

When only the ID number was changed a 2nd pairing with the same name but address shows as #2. So it is looking at the number even though there is no reply.
Change the ID length (hence the ID value) and a reconnect is rejected.

Try to connect with no existing matching pairing or radio not in pairing mode and you get an error resulting a a disconnect.
The below is seen whenyo have Warning or Error level debug turned on.

[  8513][E][BLERemoteCharacteristic.cpp:598] writeValue(): esp_ble_gattc_write_char: rc=259 Unknown ESP_ERR error
[  8523][E][BLERemoteCharacteristic.cpp:581] writeValue(): Disconnected


      // ********  Below is a working sequence **********************************************************
      
      UUID            = 0xFE, 0xF1, 0x00, 0x61, UUID, 0xFD  - must be 41bytes total, 36 for the UUID.
      radio reply     = None Observed

      NAME            = 0xFE, 0xF1, 0x00, 0x62, name string, 0xFD  // Name must be exactly 16 bytes!  - Too short and no reply - 21 bytes total 
      radio reply     = 0xFE, 0xF1, 0x00, 0x62, 0xFD
      
      TOKEN           = 0xFE, 0xF1, 0x00, 0x63, 0xEE, 0x39, 0x09, 0x10, 0xFD  // a fixed value 9 byes total
      radio reply     = 0xFE, 0xF1, 0x00, 0x63, 0x00, 0xFD  //  byte 4 = 0 is already paired - PAIR = EXISTING
      radio reply     = 0xFE, 0xF1, 0x00, 0x63, 0x01, 0xFD  //  byte 4 = 1 is pairing accepted - PAIR = SUCCESS 
      
      CI-V bus access granted (CIV_granted)
      radio reply     = 0xFE, 0xF1, 0x00, 0x64, 0xFD    // CI-V bus access is granted
      

******************************  Notes *************************************************************

When Pairing, we get these reponses .  PAIR is an attribute of the TOKEN message, byte 4.
NAME, TOKEN, PAIR==SUCCESS, CIV_CONNECTED

If already paired, we get these fewer reponses
TOKEN, PAIR=EXISTING, CIV_CONNECTED

If we are not already paired and the radio is not in pairing reception mode, there are no CIV responses but we will remain a BLE server disconnect.  The code needs to restart scanning, get a new or different server address, and try again.  More precisely, need to reconnect at the BLE layer and resend the CI-V layer until the radio answers, which could be never, or a long time. We can retry until we get disconnected or power off. One of the problems with the long retry is the possiblity of malloc errors and heap crashes.

If the radio is connected to another device, then same as above, no CI-V response, but stays BLE connected (?? verify this). Radio accepts up to 4 connections (as I read the docs found) so if there are enough headsets, mics, PTT buttons, CI-V monitors/decoders connected, you might get a server connection fail.

I have observed that if the pairing process does not complete for what ever reason, the BLE link will disconnect after just a few seconds. This requires running through the client (aka central) connect-to-server (aka peripheral) function while waiting for a CI-V Access completion.  This would be a normal scenario if the radio is not in pairing mode, turned off, or out of range.

Aug 3, 2024 Note: The extended demo file IC-705_BLE_Decoder_Simple.ino has extensions to the stripped down uart demo file to to communicate with the 705.  I have been working on improving the ability to recover from disconnects.  Initially it did pretty good for pairing and reconnecting on short term disconnects,  However, if you turned off the the radio and wait a while, the connect-to-server function gets stuck at the "Connected to server" step.  With debug on the last BLECLient event is ESP_GATTC_UNREG_EVT which is client unregistered. See Issue #1 (now closed) for details.  The soluton was simple - avoid this state!.  In the onDisconnect callback doConnect is set to false.  This causes the main loop top run the scanner scanner until it finds a vaid radio. The only place that should set doConnect to true is in the Scanner callback onResult function when a valid radio is found and the address is set.   

While the onDisconnect solution worked to avoid connecting to non existing radios, it was too slow to be reliable.  I am now using pClient->isConnected in the main loop.  Have to be sure the pointer is not null before you use it, it does change and you owill see an exception crash.  Leaving * off pointer declarations also does that :-).

Other changes are targeted at avoiding malloc errors (which I saw) when rerunning Scan_BLE_Servers() by clearing the discovered list each scan.
      
      pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory

I also did more experiments with passing BT addresses in various formats but so far the radio has never acknowledged any 0x61 messages and the displayed address is always 00:00:00:00:00:xx where xx starts with 01 for each new BLE client paired.  Do not know if that is expected or not.

So as of Aug 4, I think the Decoder_Sinple version is a pretty robust template to add fancy UI, SD card config, band decoder and PTT breakout, etc.  This for me was my first BLE project, jsut happenm o pick one that seems to have an undocumented inteface to the radio.  Also the examples for BLE UART all seems to be server side.  Another oddity is Icom chose to make the RX and TX characteristic UUIDs the same, most products are different IDs. I assume this is because CI-V is a simplex bus.

While he reconnection is far beter, the longer you use and look at the debug, there are still issues.  There is what looks like a scenario that tries to write to a connection that is not ready yet. You have to turn on Error level debug to see the events.  I also spotted a crash reboots.  So not out of woods yet.  

When I thought everything was working I later found nothing or little works. I now think much of this was due to my trying to track the state of each message success and BLE link status. When changing the contents of the ID or Name message there replies stopped so the state engine failed.  Now I understand better when you get replies  and when you don't so I think it will be more sane now.  I just look for the final success message and track the BLE link status which is how you know about connection failures/rejections.   For this I am now using isConnected() (when there is a valid pointer).

