# IC-705-BLE, BT Classic, and USB Host Serial Examples

Disclaimer: this is a work in progress!  

Be sure to check out the Hardware Reference page at https://github.com/K7MDL2/IC-705-BLE-Serial-Example/wiki/Hardware

![K7MDL BT CI-V decoders](https://github.com/user-attachments/assets/d489833c-0e2d-4ca0-8f54-b16cf572a62b)

Latest summary: As of Sept 10 - Now save and restore Split mode.  Split is often turned on when using WSJT-X on a band, and needs to be off for SSB/CW usually.  Like the other saved parameters, it simply saves and restores what is found at band change. 
 Also, a small change that fixed a crash, in BLE mode at least - when a standard Mode message is received (which has no info about Datamode), which occurs for radio side band changes, I no longer immediately request extended mode info. That info is polled every 1-2 seconds already so there is no pressing need to get faster extended info back.  I was testing a HF-6M amp this week and used the 4Relay module for PTT.  Changed the GPIO PTT output pattern defines to actuate the 4th relay, and it worked as expected for all bands.  Most testing to date has been on XVTR band switching.  This is in addition to the 4IN/8OUT module which the lower 4 outputs are in (logical) parallel with the Relay module.  Last, I have added rotation and lowered the screen brightness for the M3AtomS3 so the USB cord (used for power and programming/debugging only) exits the side, not the bottom, for easier attachment to a horizontal surface.

This now runs on the tiny M5AtomS3, M5StampS3 and M5StampC3U.  I have added more #define control for the various IO options and they can generally all run in parallel. Also when compiling on the M5AtomS3, the 0.85" color LCD is now formatted with the same info as the larger screen.  The screen doubles as a pushbutton so you can cycle through the XVTR bands without any attached IO hardware. Button press detection can use some improvement.

![M5AtomS3 rotated](https://github.com/user-attachments/assets/372bd252-4766-4452-b056-82d447edbdbb)

I received some new IO devices today, the EXT.IO2 V1.1 UNIT and the 4RELAY MODULE 13.2. v1.1.  Added support in the decoder code for input from the EXT.IO2 -OR- the 4IN8OUT (only 1 allowed) and outputs on either, or both, of the 4IN8OUT and 4RELAY modules. 

Added a new input pattern to band matching list in Decoder.h.  Now when a wired input or XVTR button is pressed, it searches the list for the first match.  Since there are only 3 inputs allocated for band input (4th is PTT from radio), you can select one of 3 bands, or use a custom or BCD pattern (defined in the decoder tables today) to get many more. In theory we can add a 2nd IO module for 8 inputs and 16 outputs, or more relay modules adding 4 more relays to that total, or use the EXT.IO2 GPIO extender plugged into Port A normally, and can rejigger them for more inputs.  The EXT.IO2 input voltage is limited to 3.3V.  The IC-705 has 5V open circuit so I elected to stick with the 4IN8OUT module for inputs since it has resistors that divide down the voltage well under 3.3V for a total current draw of only 0.85ma.

In this code I set the wired inputs up to select only 2 XVTR bands, 903 and 2304MHz.  On the 4relay module, relay1 is 903 active, relay 2 is 903_PTT, relay 3 is 2304 band active, and relay 4 is 2304_PTT.  The 4IN8OUT module outputs are the same, in parallel today until a better means of config is created to differentiate then module types.  Modules may or may not be present complicating config rules, plus the final rules need to be read from SD card.

Aug 26 - I have a version of the BLE decoder running on a M5StampC3U module. Very small, low cost, no screen or IO module but will fit into a very small box and tuck out of the way or fit inside another box such as one holding coax switches.

Goal is to flip the roles around, turning it into a USB to BLE CI-V interface dongle that can plug into USB only radios, like my 905. Does the band decode stuff plus offers a BT connection to a PC for diital modes and logging, including the XVTR frequency translation both directions.  Plan to use OTG features of the S3 for USBHost, or the onboard USB bridge chip on the M5StampC3U for USBHost, see if these work better than the USB module. Without a SD card, I plan to store the settings in the 8K RTC SRAM by creating a 'file' using Preferences.

At each band change, the entire bands table is written to SD card.  It is read back at startup.  Per band settings updated to include band, frequency, mode, filter, datamode, AGC, Preamp, and Atten for any band.  These parameters are saved and restored on band changes for all XVTR bands and the band active prior to switching into XVTR mode.  The goal is to set the radio back to reasonably normal state as if the Xvtr usage never occured.  This is a major convenience feature when operating with both Xvtrs and direct bands.  Most of the dev and testing recently has been with BLE on the Core2.  Buttons are working nicely.  

CoreS3-SE now works with IO and BLE. Core2 does both BLE and BT Classic, IO works also.  Core Basic is BT Classic.  So all generations are operational now. SD card config is only used for BT Classic BT address today.  BLE is now using a tool generated UUID.

For the 4IN8OUT module to work on i2c on the CoreS3, it required &Wire1, vs. &Wire used for the older models.  The SDA and SCL pins (12 & 11) seem to be ignored, can swap them no impact.  This was spotted in their CoreS3 example motor encoder driver.  Below is how it looks for Core3 now. 

      while (!module.begin(&Wire1, 12, 11, MODULE_4IN8OUT_ADDR)) ...

This was good news, it means I could proceeed to add BLE to the code, now that IO works, which I consider a necessity. USB Host is still a problem.  I updated the BLE_Decoder_Simple.ino.

Can now open config.ini on the SD card and update the BT address to match your rig. Also, if an update.bin file (new firmware image binary) is found, it will upload it, replacing the old program, then delete the update file from the SD card. To try this, first upload the latest here which has this capability, then find the .bin file in your temp sketch folder (path visible in the output section).  Rename it to update.bin, copy it to the root of a SD card.  Insert, reset the CPU.  Update status is on screen, goes fast.  

For the config file: Create an empty config.ini file in the root folder on a FAT formatted SD card.  Create 1 line with with your address following the sample below:

      bd_address = 30:31:7d:BA:44:F9

You may have other lines starting with a ';' to ignore them.


As of Aug 17, 2024, on a M5Stack Core Basic and Core2, BT Classic connection to the 705 runs reliably, and the 4-In/8-Out module works.  That is not the case yet for USBHost (works sometimes, then it does not for seemingly unrelated reasons, or no reason at all.  The first fully working version has been given to a field tester along with the radio I borrowed from him.  It is simply routing PTT between 2 transverters.  Can use the radio's Polled TX state or the wired PTT input as the PTT source so there is the option of no wired connection to the radio.  
CoreS3 furher off, cannot talk to the IO module yet. An early version of this program works on USB Host before the graphics and IO module added.  IO module uses I2C and nails the the USB Host (SPI) and hangs everything. M5Unified Lib also breaks USB Host. Using M5stack on Core Basic and M5Core2.h on Core2 for best USB results.   

Last I moved button monitoring for all 3 CPU models into the main loop, outside of App_loop and any FreeRTOS task where they work better. The Core2 touch buttons are very fast using the M5Unified touch buttons. The Core2 lib Core Basic buttons barely work.  Have not tried BLE on Core2 yet. 

The IO module uses a 4.7K resistor to 3.37VDC.  Draws 0.8ma when grounded.  Output pins are Vin (such as 12V or 5V if USB powered) on pin 1 of each output.  Pin 2 is a MOSFET to GND.  A DVM in continuity/beep mode will tell you if the outputs are working or not.  The IC-705 Send output transistor can handle 200ma in TX to GND, and up to 20VDC in RX.  Be sure the radio power GND is common with the M5Stack GND.  On USB power the M5Stack power GND may be floating. You can supply 12VDC on the IO module, has the red label.  Pin 2 is GND and is where you can connect GND for your outputs.  Pin 1 of the outputs are Vin (+5 or +12 for example).  

_________________________________________________________________________________

Demonstrates how to connect to the Icom IC-705 using 1 of 3 methods.  Most BT connections to the IC-705 to date (Aug 2024) have been made using BT classic SPP. With the newer MStack Core3, which uses an ESP32-S3, it does not support BT classic SPP, only has WiFi and BT5 compatible BLE.

### Connection methods:

1. Bluetooth Low Energy (BLE) Serial Port Profile for CI-V control.  This is a modified version of work done in 2018 a https://github.com/ThingEngineer/ESP32_BLE_client_uart with changes from a scanner example to (re) discover and reconnect continuously. The ESP32-S3 does not have BT Classic SPP so BLE or USB Host is necessary. The IC-705 BT CI-V connect sequence appears to be undocumented but a precious few clues were found online combined with lots of test observations to get a working setup on BLE.  This build seems to be stable now but it is still early days with an undocumented interface. Please raise issues with test scenario when a problem is observed.  I tested this on a M5Stack Core3 SE which is based on a ESP32-S3.

2. BT Classic SPP. Several examples exist. This will pair with the radio quickly using a confirmation code on the radio side only. Once paired, if a connection is lost it will reconnect reasonably fast.  I tested this on an older 4MB flash M5Stack Basic ESP32 module.  It does not do BLE.  

3. USB Host port. Connects directly to a radio on a 2nd USB port configured for Host role. It has the advantage of no connection delays, no undocumented access protocol like in BLE, and works with a wider range of radios. Where RFI in and out of the radio via the USB cable is a concerrn, this might be mitigated by using a 6" cable with ferrites. Then run buffered IO wiring to the relays and gear.  Here I used a M5Stack Core Basic and snapped on a M5Stack USB V1.2 USB module,  Does both roles. I have not got this work on the Core3 yet.

Here is a Core Basic as of Aug 9, 2024.  Now sporting a UI makeover.  BT Classic and USB Host options, plus the USB to the PC.  Note the button labels to switch connection modes.  Search will force a look for a new radio address.  Probably replace with a menu system.  XV icon is lit up when a transverter band is active from a (future) band select window, or input wiring from a radio or switch.  The TX icon emulates the 705/905 TX status icon.  It is polling TX every 27ms or so. A bit better option is to connect the send output (PTT) into one of the decoder 4 inputs where it will get routed to an output based on the band active.  

Lat, Long, Time, Date are all in the CAT myPosition message. Grid is not passed along via CI-V so I calculate the 8-digit grid square and display on the screen below the Band: label.  Also added a transverter status icon in prep for that feature coming up.

![Aug-10-2024_UI_Xvtr_grid](https://github.com/user-attachments/assets/98c3a17e-c408-4c06-ae43-b29185718601)


A Core3-SE connected via BLE to the IC-705

![Core3 Se connected by BLE to IC-705](https://github.com/user-attachments/assets/ec1bdaf5-9b31-4029-80e0-d135be3527b0)


Old and New models posing for a family picture.  The Core3-SE has 4-In/8-Out, HMI, USB V1.2 and a DIN base modules. There are 2 batteries inside.

![Core Basic and Core3](https://github.com/user-attachments/assets/802f0022-3831-4342-a799-13e0ef6448ba)


### Folders on this repo:

1. IC705_ESP32_BLE_client_uart:  This is the simplest demo if pairing and reconnecting to an IC-705 using BLE.  With the radio in pairing mode, turn on your device and it will silently pair. Once paired, it will reconnect upon any disconnect. It is a one shot deal and won't auto-recover from disconnects, you must reset the device each time to connect.  It reads frequency and PTT status, nothing else.

2. IC-705_BLE_Decoder_Simple: This an extended version of the simple program above.  Nothing fancy but it will auto-reconnect a soon as it can find the server again.  I may add a few features but it is largely done, serving its purpose for now.  This and the UART example above were tested on a M5Stack Core3-SE. The CPU model is new and the IO libraries are still catching up. I have not yet got the HMI encoder and 4In/8Out module working. Or the USB Host moduleyet, just have not tested that yet.  Be sure you do not have the ESP32_BLE_Arduino library loaded, use the built in BLE lib.

3. M5Stack_Core_Basic_CI-V_Band_Decoder. This is BT Classic SPP and BT Low Energy (BLE) depending on the CPU capability.  USB Host support is there but is not stable. It supports several IO/Relay moduels and Units such as the  4In/8out module and 4-Relay module, plus a DIN or PLC base so I can mount it.  The IO can be used for PTT breakout per band and for automated transverter and/or antenna selection.  UI has Frequency, band, PTT, Xvtr, Time, date, grid square and menu buttons. It supports all bands (some may be transverter bands) up to 122GHz, setting key radio parameters such as Frequency, Mode, Filter, Preamp, Atten, AGC, Xvtr offset, and Split for each band and restores the original (last) non-Xvtr band paramaters. This program is full featured where the 2 above are built as breif examples. 

There is a function in use now that auto-discovers the model's CI-V address.  This code is set up to make changing between USB and BT easily done by the UI.  You can define a CI-V address as well.  A button will force a search for a new address.


### Hardware options:

Check out the Hardware Reference page at https://github.com/K7MDL2/IC-705-BLE-Serial-Example/wiki/Hardware

The M5Stack 4in/8out IO module supports 1A@24VDC on each port. Also acts as 24V or 12V power supply. Can use wired band decoder inputs from radios like the Elecraft K3 or K4, and some Yaesu radios.  Other future band decoder input options include analog voltage (Like the IC-706 or Yaesu FT-817) and old school RS-232 serial port. LAN control on an Arduino appears to be a lot of work, it is also pubicly undocumented.

A note about ESP32 and USB ports.  Many of the ESP32 chips have OTG USB ports supporting both Host and Device roles. I wanted to connect the M5Stack CPU module directly to a radio, and debug with a PC on the main USB port, or optionally, use that PC to run logging or digital mode programs like WSJT-X. Some of the ESP32-S3 boards have 2 USB ports on board, I have one with a 4.3" touch display I will be testing soon. At least the internal CPU port is OTG, the other is a Silicon Labs based Serial to USB bridge. For my project I needed small pre-packaged box with buffered IO to control radio gear and use a small display for info. 

On my Teensy projects I run up to 3 virtual serial ports on both the Device and Host sides. I control some SDR radios with the host port (RS-HFIQ). In this project I connect to an Icom IC-705 and IC-905. I plan to test on a borrowed IC-9700 once I get USB Host working reliably.  The radios have multiple virtual ports for audio and 2 serial ports, one for CI-V control and spectrum data, the other assignable, but most often used to output the internal GPS data in the form of standard NMEA strings. I also pass through both serial ports (no audio) to a optional PC so it can use the ports transparently.  On the ESP32 I have discovered the TinyUSB library used for Arduino should be able to support 2 virtual serial ports, just started looking into that.  On the PC side one port for CAT and one for debug would be nice.


### To Use this with an IC-705:

I set the default radio CI-V address to 0. It will autodetect the radio address on startup when it sees the first messages. Turn the VFO dial to accelerate this. In the software, you can set the radio_address.  For example, 0xA4 for the IC-705, 0xAC for the IC-905. I plan to make this an SD card config file choice later, or a menu button.  Same for swapping between BT and USB Host ports modes.  Today tye SD card config file contains only the BT Classic target radio BT address.  For BT Classic mode, you can set your radio's BT address in the top of the main file as the default address and not require a SD Card config file.  Not required for BLE.  If using multiple BLE devices, alter the UUID number slightly to differentiate them.  For both you may want to change the name so they display on the 705 uniquely.  The name must be 16 characters exactly.   

The SD card, if present, is used to store the radio parameters for all the repdefined bands low HF up to 122Ghz.  It starts with defaults then is updated with observed (polled) parameters as each band may be sleectged at the radio.  That data is primarily used for bands that have a Xvtr Offset and the last non-Xvtr band used before switching into XVtr mode.  The idea is to resatore the radio back to the way it was before being used as an IF rig.

Pairing is simple and silent, and will auto-reconnect as long you you do not delete the Pairing from the radio.

To use it, go to the IC-705 Pairing Reception menu, then reset the M5Stack CPU module. On boot up it will pair with the radio. The Pairing Reception menu will close quickly.  For BT classic you will see a confirmation code request dialog on the radio side. The device name (such as BT705 CIV Decoder) will show as connected in the Pairing/Connect menu.  To delete the device, touch the device name, disconnect, then long touch the device name, delete the pairing.

The radio specs says the 705 is using BT4.2 so requires the ESP32 be in compatibility mode to support both BT classic and BLE, which the 705 does.  BT5 is not the same as BT4.2 and earlier.  Will be interesting to try with BLE dongles on my IC-905 and other radios like the 9700.

On 8/8/2024 I tried to auto-switch between BT and USB Host.  It gets tricky.  You can configure BT to start up but can do so only after the USB Host has been run through a bit, else the USB host task hangs, usually causing an OS watchdog timeout and reboot, but often not. Also you only get 1 plug and unplug cycle, else timeout.  In this case a reboot is faster and easy way to work around this for now. The USB disconnect event calls ESP.restart().  

Instead of auto-failover, I now have 2 buttons that switch between modes. No reboot required. A (Left button) is BT mode, C (right button) is USB mode.  Hold the A (BT) button until you see the screen change to "Connecting to BT". Hold USB C button until you see it change.  BT goes into a blocking discovery loop so reading buttons without interrupts is not easy o break out of a loop of that. Maybe make it a one shot per button. You can use CI-V auto-address discovery which works beter if you roate the dial.  I can swap between 2 radios this way, one on BT and one on USB.

I tried to configure the USB Host and Peripheral (main) ports for 2 virtual serial channels. So far have not got it to work. It requires changing a config file setting. The Arduino library is preconfigured so wont pick up some edits.  They use a KConfig file approach with Cmake and have you to run esp idf.py menuconfig (once you loaded the TinyUSB component) in your project directory 'main' folder which is not set up like Arduino. the menuconfig writes out the KConfig file where the library will look for it and override the default. CFG_TUD_COUNT 2 is the key. The ESP32-S3, maybe all ESPs, did not support this until a few months ago. There is one example file cdc-multi, but requires the above process to work.  Big learning curve here.

I studied the timing of sent vs reply messages between BT and USB. The radio sends out new frequwncy values when you turn the VFO very fast and I get all of those. For a while every 4th reply was missing the first bytes. I reboooted the radio and never saw it again. That was on USB only. I also experienced replies taking 5 seconds over USB for quite a while.  Eventually this went away as I retructured control flags for the buttons so it was something in my code. BT however was very fast during that time and every TX is answered immediately, within a few milliseconds if not the same.

Technical details including the BT/BLE CI-V connection protocol are in Wiki pages
https://github.com/K7MDL2/IC-705-BLE-Serial-Example/wiki/Technical-Details
