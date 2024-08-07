# IC-705-BLE, BT Classic, and USB Host Serial Examples

Disclainmer: this is a work in progress!

Demonstrates how to connect to the Icom IC-705 using 1 of 3 methods.  Most BT connections to the IC-705 to date (Aug 2024) have been made using BT classic SPP. With the newer MStack Core3, which uses an ESP32-S3, it does not support BT classic SPP, only has WiFi and BT5 compatible BLE.

Connection mnethods:

1. Bluetooth Low Energy (BLE) Serial Port Profile for CI-V control.  This is a modified version of work done in 2018 a https://github.com/ThingEngineer/ESP32_BLE_client_uart with changes from a scanner example to (re) discover and reconnect continuously. The ESP32-S3 does not have BT Classic SPP so BLE or USB Host is necessary. The IC-705 BT CI-V connect sequence appears to be undocumented but a precious few clues were found online combined with lots of test observations to get a working setup on BLE.  This build seems to be stable now but it is still early days with an undocumented interface. Please raise issues with test scenario when a problem is observed.  I tested this on a M5Stack Core3 SE which is based on a ESP32-S3.

2. BT Classic SPP. Several examples exist. This will pair with the radio quickly using a confirmation code on the radio side only. Once paired, if a connection is lost it will reconnect reasonably fast.  I tested this on an older 4MB flash MStack Basic ESP32 module.  It does not do BLE.  

3. USB Host port. Connects directly to a radio on a 2nd USB port configured for Host role. It has the advantage of no connection delays, no undocumented access protocol like in BLE, and works with a wider range of radios. Where RFI in and out of the radio via the USB cable is a concerrn, this might be mitigated by using a 6" cable with ferrites. Then run buffered IO wiring to the relays and gear.  Here I used a M5Stack Core Basic and snapped on a M5Stack USB V1.2 USB module,  Does both roles. I have not got this work on the Core3 yet.

Folders on this repo:

1. IC705_ESP32_BLE_client_uart:  This is the simplest demo if pairing and reconnecting to an IC-705 using BLE.  With the radio in pairing mode, turn on your device and it will silently pair. Once paired, it will reconnect upon any disconnect. It is a one shot deal and won't auto-recover from disconnects, you must reset the device each time to connect.  It reads frequency and PTT status, nothing else.

2. IC-705_BLE_Decoder_Simple: This an extended version of the simple program above.  Nothing fancy but it will auto-reconnect a soon as it can find the server again.  I may add a few features but it is largely done, serving its purpose for now.  This and the UART example above were tested on a M5Stack Core3-SE. The CPU model is new and the IO libraries are still catching up. I have not yet got the HMI encoder and 4In/8Out module working. Or the USB Host moduleyet, just have not tested that yet.

3. M5Stack_Core_Basic_CI-V_Band_Decoder. I need to get a working package done soon so went with the old Core Basic module.  This is BT classic SPP and I use USB Host Serial and 4In/8out modules plus a DIN or PLC base so I can mount it. The HMI Encoder/Switch module also works when I am ready for it. For features this has asimple UIO with Frequency, band and PTT status. Will add some things like time and grid square.  The 4in/8out module is used for buffred IO (if used) and USB V1.2 module (for USB host if used). Thius one is a work in progress, the 2 above are mostly done as examples. 

I am now starting to add in the code for the IO module for band decoding outputs and to break out PTT per band. As these targeted radios only have 1 PTT jack for all bands, a method to route PTT to external RF amplifiers is needed. Set a single variable in the code for Host or BT mode, and set the radio CI-V bus address if needed. There is a dormant function to auto-discover the model I-V address, I may activate that later.  This code is set up to make changing between USB and BT easily done by the UI. On my ToDo list along with more limited UI work.

This version has the USB host capability but as of Aug 7, 2024 it runs for a limited time and stops. Hoping to solve that soon. The USB Host code example I built on supports several CPU types and I left that code in place for the case I want to later try this on Pico or some other model.


Hardware options:

The M5Stack 4in/8out IO module supports 1A@24VDC on each port. Also acts as 24V or 12V power supply. Can use wired band decoder inputs from radios like the Elecraft K3 or K4, and some Yaesu radios.  Other future band decoder input options include analog voltage (Like the IC-706 or Yaesu FT-817) and old school RS-232 serial port. LAN control on an Arduino appears to be a lot of work, it is also pubicly undocumented.

A note about ESP32 and USB ports.  Many of the ESP32 chips have OTG USB ports supporting both Host and Device roles. I wanted to connect teh M5Stack CPU module directly to a radio, and debug with a PC on the main USB port, or optionally, use that PC to run logging or digital mode programs like WSJT-X. Some of the ESP32-S3 boards have 2 USB ports on board, I have one with a 4.3" touch display I will be testing soon. At least the internal CPU port is OTG, the other is a Silicon Labs based Serial to USB bridge. For my project I needed small pre-packages box with buffered IO to control radio gear and use a small display for info. 

On my Teensy projects I run up to 3 virtual serial ports on both the Device and Host sides. I control some SDR radios with the host port (RS-HFIQ). In this project I connect to a borrowed Icom IC-705 and my own IC-905. I plan to test on a borrowed IC-9700 soon.The radios have multiple virtual ports for audio and 2 serial ports, one for CI-V control and spectrum data, the other assignable but most often used to output the internal GPS data in the form of standard NMEA strings. I also pass through both serial ports (no audio) to a optional PC so it can use the ports transparently.  On the ESP32 I have just discovered the TinyUSB library used for Arduino should be able to support 2 virtual serial ports, just started looking into that.  On the PC side one port for CAT and one for debug would be nice.


To Use this with an IC-705:

In the software be sure to set the radio_address to 0xA4, it is likely the default now. The IC-905 is 0xAC. I plan to make this an SD card config file choice later, or a menu button.  Same for swapping between BT and USB Host ports modes.

Pairing is simple and silent, and will auto-reconnect as long you you do not delete the Pairing from the radio.

To use it, go to the IC-705 Pairing Reception menu, then reset the M5Stack Core3. On boot up the ESP32-S3 will silently pair with the radio. The Pairing Reception menu will close quickly.  The device name (BT705 CIV Decoder) will show as connected in the Pairing/Connect menu.  To delete the device, touch the device name, disconnect, then long touch the device name, delete the pairing.

The radio specs says the 705 is using BT4.2 so requires the ESP32 be in compatibility mode. BT5 is not the same as BT4.2 and earlier.  Will be interesting to try with BLE dongles on my IC-905 and other radios like the 9700.

Technical Details:

UUID from device:

I noted that on BT Classic, the BT address is shown on the 705 Paired devices list.  For BLE the UUID I send is not used for display and a number is auto_assigned starting with 1, incrementing for each new device I try pairing, same or unique name.  

CI-V Pairing and Sign-in state engine:

In the IC-705_BLE_Decoder_Simple program I now track the state of pairing and sign in.  The BLE scanner will be restarted on a failed connection attempt, or keep scanning looking for a qualified server (the radio).  The messages are so far undocumented. Looking at 1 code sample found, it sends a UUID string. From where is this derived? I made one up.  For BT classic SPP the BT address is displayed on the radio, formatted as a 6-byte BT address with colons.  For BLE a sequencial number is assigned starting with 1. The 2nd connection is 00:00:00:00:00:02 for example.

The 3 commands to pair or reconnect are 0x61, 0x62, 0x63. 0x64 is a final reply on success. The radio does not reply to a 0x61 msg but does use the number as evidenced by it creating a new pairing entry if you keep the same name and change the number. Try to connect while not paired and not in pairing mode will fail with a disconnect event.  I tried not sending msg 61 and it still pairs and auto-assigns a number as normal but you may find the reply for the name comand missing.

Conclusion: 0xFE 0xF1 0x00 0x61 is the UUID. The UUID is 36 bytes. The total msg length must be 41 bytes to get a 0x62 msg reply. You can still get a pairing and reconnect with less but the 0x62 reply message is suppressed.

Message format details:

// ********************************************************************************************
      
Observation: If the ID message 0x61 sent is of a certain lemgth (41 bytes, 36 for the ID) then the name (0x62) reply is sent, otherwise the 0x62 is suppressed, still get the token reply.  Maybe a reject message collision, ?  Sending a BT address results in no reply for 0x62 (Name). Still works though. If the state engine requires the name reply msg, then you have to get the ID right.

When only the ID number was changed a 2nd pairing with the same name but address shows as #2. So it is looking at the number even though there is no reply.  Change the ID length (hence the ID value) and a reconnect is rejected.

Try to connect with no existing matching pairing or radio not in pairing mode and you get an error resulting a a disconnect.

The below is seen when you have Warning or Error level debug turned on.

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

If we are not already paired and the radio is not in pairing reception mode, there are no CIV responses but we will receive a BLE server disconnect. The code needs to restart scanning, get a new or different server address, and try again.  More precisely, need to reconnect at the BLE layer and resend the CI-V layer until the radio answers, which could be never, or a long time. We can retry until we get disconnected or power off. One of the problems with the long retry is the possiblity of malloc errors and heap crashes.

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

