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
In the IC-705_BLE_Decoder_Simple program I now track the state of pairing and sign in.  If we are connected and not yet paired, we will get a reply to all 3 sign in/pairing messages. The 0x63 will return with a 1 on successful pairing and 0 if already paired (assuming other messages are returned OK). Will retry signing in if we can connect but not get the expected paired and/or signed in responses.

Assuming we have a BLE connection to server (radio) we get these CI-V sign in response patterns:
Send out all 3 messages to connect to CIV
      
Looking at other code it it looked like it started with sending a UUID string. From where? The number on the radio is formatted as a 6-byte BT address.  The radio does not reply but otherwise works goes on and pairs, communicates.
I think it should be a BT address but efforts to send one with and without colons I got no response and the radio assigned a number starting with 1.  I tried all sorts of combos of numbers for the BT address (hex, bcd, colons) none got a response.  To b clear, this is not BLE directly, but a CI-V message sent to the 705 to set up the 705 pairing. 
Last I tried not sending msg 61 at all.  It still paired and auto-assigned a number.

Conclusion: FE F1 00 61 is not the ID string, or more likely for BLE, it does not use one and always auto-assigns a number in the radio.

      // ****** Not used in latest code - left here to document attempts to send one and my observations
      ADDR            = 0xFE, 0xF1, 0x00, 0x61, BT_Address string, 0xFD
      radio reply     = None Observed
      // ************************************************************************************************
      
      // ********  Below is a working sequence **********************************************************
      
      NAME            = 0xFE, 0xF1, 0x00, 0x62, name string, 0xFD  // Appears to be limited to 16 bytes in the 705 display field
      radio reply     = 0xFE, 0xF1, 0x00, 0x62, 0xFD
      
      TOKEN           = 0xFE, 0xF1, 0x00, 0x63, 0xEE, 0x39, 0x09, 0x10, 0xFD  // a fixed value
      radio reply     = 0xFE, 0xF1, 0x00, 0x63, 0x00, 0xFD  //  byte 4 = 0 is already paired - PAIR = EXISTING
      radio reply     = 0xFE, 0xF1, 0x00, 0x63, 0x01, 0xFD  //  byte 4 = 1 is pairing accepted - PAIR = SUCCESS 
      
      CI-V bus access granted (CIV_granted)
      radio reply     = 0xFE, 0xF1, 0x00, 0x64, 0xFD    // CI-V bus access is granted
      
******************************  Notes *************************************************************

When Pairing, we get these reponses  
NAME, TOKEN, PAIR==SUCCESS, CIV_CONN

If already paired, we get these fewer reponses
TOKEN, PAIR=EXISTING, CIV_CONN

If we are not already paired and the radio is not in pairing reception mode, there are no CIV responses but we will remain connected to the radio BLE server.  More precisely, we need to reconnect at the BLE layer and resend the CI-V layer until the radio answers, which could be never, or a long time. We can retry until we get disconnected or power off. 

If the radio is connected to another device, then same as above, no CI-V response, but stays connected. Radio accepts up to 4 connections so if there are enough headsets, mics, PTT buttons, CI-V monitors/decoders connected, you might get a server connection fail.

I have observed that if the pairing process does not complete for what ever reason, the BLE link will disconnect after just a few seconds. This requires running through the client (aka central) connect-to-server (aka peripheral) function while waiting for a CI-V Access completion.  This would be a normal scenario if the radio is not in pairing mode, turned off, or out of range.

Aug 3, 2024 Note: The extended demo file IC-705_BLE_Decoder_Simple.ino has extensions to the stripped down uart demo file to to communicate with the 705.  I have been working on improving the ability to recover from disconnects.  Initially it did pretty good for pairing and reconnecting on short term disconnects,  However, if you turned off the the radio and wait a while, the connect-to-server function gets stuck at the "Connected to server" step.  With debug on the last BLECLient event is ESP_GATTC_UNREG_EVT which is client unregistered. See Issue #1 (now closed) for details.  The soluton was simple - avoid this state!.  In the onDisconnect callback doConnect is set to false.  This causes the main loop top run the scanner scanner until it finds a vaid radio. The only place that should set doConnect to true is in the Scanner callback onResult function when a valid radio is found and the address is set.   

While the onDisconnect solution worked to avoid connecting to non existing radios, it was too slow to be reliable.  I am now using pClient->isConnected in the main loop.  Have to be sure the pointer is not null before you use it, it does change and you owill see an exception crash.  Leaving * off pointer declarations also does that :-).

Other changes are targeted at avoiding malloc errors (which I saw) when rerunning Scan_BLE_Servers() by clearing the discovered list each scan.
      
      pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory

I also did more experiments with passing BT addresses in various formats but so far the radio has never acknowledged any 0x61 messages and the displayed address is always 00:00:00:00:00:xx where xx starts with 01 for each new BLE client paired.  Do not know if that is expected or not.

So as of Aug 4, I think the Decoder_Sinple version is a pretty robust template to add fancy UI, SD card config, band decoder and PTT breakout, etc.  This for me was my first BLE project, jsut happenm o pick one that seems to have an undocumented inteface to the radio.  Also the examples for BLE UART all seems to be server side.  Another oddity is Icom chose to make the RX and TX characteristic UUIDs the same, most products are different IDs. I assume this is because CI-V is a simplex bus.

While he reconnection is far beter, the longer yu use and look at the debug, there are still issues.  Ther is what looks like a scnario that tries to write to a connection that is not ready yet.  YOU have to tunr on Error level debug to see the events.  I also spotted a crash reboots.  So not out of woods yet.  

I think the BLE address think is still an open issue.  When a connection is down for long enough it seems like you have to (at times) re-pair to reconnect. Maybe it needs a valid BT address to correctly remember your connection.  It feels like it takes a long disconnect time for the radio to lose that memory.  Maybe something times out.

