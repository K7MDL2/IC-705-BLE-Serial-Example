# IC-705-BLE, BT Classic, and USB Host Serial Examples featuring a CI-V decoder and 3-Band Transverter Box

Disclaimer: this is a work in progress!    I have begun to rewrite this page moving usage, tech, and dev content to Wiki pages.  See https://github.com/K7MDL2/IC-705-BLE-Serial-Example/wiki

Be sure to check out project Wiki pages. There are now 2 related constructions sung the same code
  1. CI-V Band decoder and Display  - see the Hardware Reference page at https://github.com/K7MDL2/IC-705-BLE-Serial-Example/wiki/Hardware
  2. BCD wired input Band decoder + 3 band transveer box. Has PTT and antenna breaksout for VHFbands 50, 144, 222, 432, 902/3, qn 1296.  See https://github.com/K7MDL2/IC-705-BLE-Serial-Example/wiki/3-band-Transverter-Box

SD card usage is here: https://github.com/K7MDL2/IC-705-BLE-Serial-Example/wiki/SD-Card-Usage

![K7MDL BT CI-V decoders](https://github.com/user-attachments/assets/d489833c-0e2d-4ca0-8f54-b16cf572a62b)

The latest updates below are mostly related to the new 3-band Xvtr box build leveraging the same CI-V Decoder/Display code to make using the 705 as an IF rig even easier by making the Xvtrs appear highly integrated to the 705 as can be.

Latest Update: 26 Dec 2024

Added code for the i2c INA226 voltage and current sensor that is wired in series with the front panel power switch.  Measured the current 
with my DVM and calibrated the current output.  Voltage is very close by default. Displays in debug every 1 second.  This will be displayed on the OLED display.

Added code for SSD1306 type 0.91" 128x32 OLED display.  It has 4 screens, each with a single row of large text.  
      1. Xvtr Box bands = "HF/6M", "2M", "70cm", "1.25cm", "33cm", "23cm"
      2. "Rx" or "Tx" on hte left side, "XVTR" on the right side if one of the 3 Xvtr bands is actve (222, 902/3 or 1296)
      3. voltage in VDC
      4. current in Amps DC

25 Dec 2024

My SP6T IF switch on the TX IF path has 3 bad ports out of the 6.  RF4-6 are good. I moved 222 TX IF input cable from RF3 to RF4 until a replacement board arrives.  The port assignments are arbitrary, chosen for the cleanest cabling.  RF3 for 222 is on the opposite side from RF4,5 & 6 which serves 903 and 1296.

I measured 2.4dB loss through the TC board (which is the IF T/R switch + RX and TX attenuators) and the active SP6T IF switch port for each band.  The transverters require -5dBm to +3dBm drive, 10dBm max.  The IC-705 can put output up to 40dBm. I used a 10W 30dB attenuator for 32.4 dB total attenuation and raised the 705 output power until I started to see the transverter output flatten out (compression point), around 40-70%.  Can set the 705's RF Output max power limit setting to the final value.

I moved all input line to the CPU GPIO pins and the IF SP6T switch 3 control wires as well.  Could have left those 3 on the port expander.  The main goal was to move PTT, which is scanned every 4ms or so, onto the CPU, avoiding rapid i2c bus activity which generates excessive noise on HF bands.  Things are looking pretty clean now.  Only the 903 and 1296 TTL control lines are left on the MCP23017 PB0-7 range of pins.  PA0-7 range of pins on module #1 only has 3 lines left on it now, PTT for each of the 3 Xvtr boards. A ULN2803A module slips on the MCP23017 module pins for buffering.  The PTT lines have 9VDC on them.

As of now the basic box is working on RX and TX except the internal 10W attenuator is not installed yet so TX requires special care.

Items left to finish
      1. Replace 1 SP6T RF switch board (TX IF)
      2. Make final design decision about the PTT and 3 band decoder input lines.  They are direct to the CPU right now with weak 3.3V pullups enabled.  These should be buffered to minimize chance for CPU damage. A new pullup source facing the radio side will be needed on the buffer outputs.  This could be done in the Radio side display controller.  I have a "PLC Prototype" M5Stack module which has a 9-24VDc to 5V regulator, perf board and edge connector.   I  may be able to fit a port expander and buffer with pullup resistors in there.  I have a small space left on the control board to wire in a 4 pin connector and 4 transistors, caps and some resistors.  Can use the the bottom side of the control board for most of the likely SMD parts.  Already have connectors installed for the i2c connected display and current sensor.
      3. Once the DC converter is mounted, install suitable heat sink solution to the back panel and mount the 50ohm RF terminating resistor, used to attenuate 10W down to < 10mW.
      4. Install RF bypass caps on the 6 PTT outputs.

These items add extra features like boosting RF output to more usable levels.
      5. Await delivery of the 50W 903 RF pallet.
      6. Await delivery of 120W DC-DC Converter, 12V to 28V @ 5A.  It is for the 50W 903 PA RF pallet.
      7. Await delivery of the 2W 1296 amp module.  Required to boost 100mW Xvbtr output up to 1.5W for the external SG-Labs 25W 1296 amp/LNA unit.
      8. Code for the INA226 V and I sensor.
      9. code for the SSD1306 compatible OLED display
      10. 3D print the surface mounted OLED display bezel and run i2c cable through the front panel.  Watch for RFI.
      

Dec 24, 2024

Changed the Radio side controller UI to directly select one of the 3 Xvtr bands.  Tap it again to exit Xvtr mode.  The 3 buttons are now assigned to 222, 903 and 1296.  To change bands tap one of the others.  To exit Xvtr mode, tap the active Xvtr band button.

Optimized Wired PTT scanning time in both the display and Xvtr box (XVBox) controllers to not miss radio wired PTT events, balanced with increased noise from high speed i2c bus activity heard when on 222Mhz using the 21Mhz IF.  Switched both 903 and 222 to 28MHz IF since 903 is actually 22Mhz IF and is out of the ham band which an unmodified IC-705 won't transmit.  The 28Mhz IF also was relatively free of the i2c noise, especially with the scan timing and i2c bus speed choices applied.  After all that, I discovered I missed the primary source of radiated i2c bus noise, the 4" long i2c cable running down to the INA226 voltage and current sensor.  I plan to use a shielded cable but for now, I snapped a medium size ferrite on it which has solved 80% of the noise from that.  Using  a 28Mhz IF has taken care of most of the the remaining 20% of i2c noise.

*Noise Update*: Addressed the noise issue (mostly) by moving the band, PTT input, and SP6T IF switch control wiring to the unused CPU IO pins (3,4,5,6,7,8,10, +5V, GND).  PTT is the only pin requiring fast scanning.  Moving PTT to the CPU lets the I2c bus operate only on band and PTT change events so no noise of measure.  The +5V and GND may be useful for external pullups, TBD.

I am pondering the physical solution to buffer the internal Xvtr PTT lines and external band decoder and PTT Input lines, add pullups, and RFI proof them.  Probably a new board mounted on top of the 903/1296 stack.  I am also considering laying out a control board PCB which with surface mount parts and ground planes for i2c bus radiated noise sheilding.  I can consolidate all this stuff and not require the space the MCP23017 and ULN2803 modules take up. 

On the M5Core2 controller side I am looking at using the M5Stack prototype base module which has inside a 5V regulator, 12V coaxial power jack, proto board space and edge connectors.  Put a MCP23017 and ULN2803 on the board, tap into the bus for i2c pins, and I can eliminate the 4IN/8Out IO module.  Also do not need the 4-Relay module.  It will have 5V pullup and bypass caps al lconveniently inside with a simple multiconnector interface to a siongle 5 wire control cable to the XVBox


Dec 23, 2024

Several minor changes and setup for future module expansions.
I fed -70dBm signal into each transverter and initially only 903 had the expected output (close to S9 with 705 preamp on).  Bypassed all cables, not the problem.  Had a partially bad SP6T solid state RF switch (used for the IF routing to the Xvtrs). I knew one of the 2 boards (RX and TX paths) was bad and it happened to be the one on the RX path.  I could move 1 Xvtr output cable but not the other so I swapped the TX board into the RX position.  RX is working but the outputs were still generally too low.  

The TC board has an optional MMIC (IC1) with 330ohm bias resistor I prepopulated during the build to add some IF stage RX path gain and also has a trim pot.  I cut the trace and installed a MAR3 (12dB gain), the existing 330ohm bias resistor on the max 15V supply is just about right, maybe a tad too high.  I think 280ohms is what the calculator called for.  Considered using a MAR6+ but that was over 20dB gain and felt like a bit much.  The IC-705 seems less sensitive on 144Mhz than my K3 144 internal Xvtr.  I also noted that the 1296 Xvtr conversion gain is less when configured for 50Mhz IF vs. 144Mhz IF.  So I configured the software for 144 IF.   Waiting for a new SP6T RF Switch board in early January.  This will effect my TX testing, still to be done.

End result is I now have about S9 on 222 and 903 without preamps (21Mhz IF) and S7 on 1296 with 144 preamp on.  Before I was lucky to get S1 on 1296.  I also have a SG-Labs 1296 25W PA+LNA combo that buimps up the signal more so all looks proper now.

Modified the wired inpout scanning to make Wired PTT much faster.  Wired PTT does not poll the radio, clogging up the BT comm channel, so there is no issue to scan it at max possible rate.  I am generally seeing main loop times <10ms.  I slowed down some info and display update polling rates.  I should figure out how to set wired PTT up as an interrupt.


Dec 22, 2024

I built a 3-band Transverter box adding missin VHF/UHF bands for 222, 902/3 and 1296 to the IC-705, or any other multiband radio with HF, 6, 144, and 432 bands, which there are many.  
![K7MDL IC-705 3-Band Transverter Box - Dec 2024 - Top View Front](https://github.com/user-attachments/assets/1c883ef0-376b-41ee-81da-cb7c4c26b37c)
![K7MDL IC-705 3-Band Transverter Box - Dec 2024 - Back View](https://github.com/user-attachments/assets/2dd7e8d9-e548-4fd3-b8d0-b0611b4c8bf5)

I used the latest transverter boards from UR3LM, all now have the option for a external 10Mhz reference and can select between 2 IF frequencies.  I used 21Mhz IF option for 222 and 903 which gives greater band coverage thean teh usual 28-30 many radios are limited to.  I am using 50Mhz for the 1296 Xvtr. Using SP6T switches you could easily handle 6 transverters.  If you only have a HF/6M or even a HJF only IF rig, you can use a unity gain xvtr board to convert IF outputs down to HF.  If I was just doing 6-7 bands covering 50-1296 I would just use a Q5 Signal multi-band converter.  It is more compact, higher power oputput (25 or 50W per band) and will be about the same cost.  With only 3 Xvtrs, the cost is worth the fun of the build.

On order for January delivery are a mini-sized 28VDC 50W 903MHz enclosed RF pallet, a 12-28VDC converter for the 903 amp, and a 2W 1296 booster amp. These will all mount inside or on the back panel of my Cheval box which features thicker finned heat sinking end panels.  The 2W is jsut right to drive my SG-Labs 25W 1296 AMP+LNA combo. It requires 1.5W drive after coax losses.  The 903 and 1296 boards output 100mW max.  The 222 is about 8-10W.

I had a DEMI/Q5 4 port 10MHz distribution box in and enclosure that is mounted in teh bottom.  Stacked on top is a 10MHz OCXO and the control board which iis a few modules and mounted on vectorboard and minimal wiring run between the modules for i2c bus and 12, 5 and 3.3V.  There are jacks for other modules - OLED display for the front (not coded or installed yet) whcih will mount in a 3D printed surface-mount bezel.  ALso control for ht stacked pair of solid state SP6T RF coax switches for routing the attenuatoed IF ampongs teh 3 Xvtr boards.

For control I embedded a M5STAMPC3U CPU module into the unit to control around 30 signals - power relays, PTT breakout internally and externally for amps, T/R relays, solid state and mechanical coax switches for IF and RF sides. I had an unbuilt 2005-era Downeast Microwave TC kit which does IF-side T/R and has an option for 28V boost if you have 28V relays. I found inexpensve nearly new surplus 18GHz rated TTL controlled 12V SP4T and SPDT switches fronm W5SWL eBay store so I did not use that feature.  The TC board has attenuation control on both the RX and TX paths, and will attenuate up to 10W - a good match for the IC-705.  It will knock down the 10W max possible about 30dB which does not exceed the safe maximum for the 3 transverter boards. The 705 RF power control is then  turned down to around 1W or less for the normal 1-3mW range usage. You can use the UR3LM attnuaor/TR board instead if you like.

The M5STAMPC3U is using this same code with added #defines to account for minor changes, particularly the 2x MCP23017 16-port I2C connected port expander modules  This gives me over 40 IO ports total, most bi-directional.  I use off the shelf ULN2803 modules plugged onto the MCP23017 board for buffered input and outputs.  I also have installed and wired an I2C INA226 Bi-directional Voltage Current Monitor module.  Have not coded that up yet, same for the 0.91" OLED SSD1306 i2c display.   The biggest code changes for the M5StampC3U is expanding the IO - 8 inputs and 32 outputs.  This also meant upsizing the band config patterns for each band from 8bits to 16bits.

I used a M5Core2 with 4-In/8-Out module at the radio for the BT band decoding and transverter control and frequency display.  It uses 4 wires run to the M5StampC3U in the box. 3 wires are BCD-coded band info and 1 wire is for PTT.  I used a separate controller in the box because this will be set in the back seat of my rover truck, or away from the operating position.  If you where just using box this at home and have desk space, you could mount the M5Core2 and add i2C IO modules to it and skip the M5StampC3U.   M5Stack has flush panel mount frames for the M5Core series.   Max 12V current draw should be < 10amp due to the 50W 903 amp.  Normally it is < 3A.   I am seeing about 1A for RX mode.  I chose to power only the Xvtr board in use, saving power and possible interference between them.

The SP4T coax switch bypasses the Xvtr box while also breaking out the 705 common RF cable for dedicated band antennas.  HF/50, 144, and 432 pass through the SP4T coax switch bypassing the whole Xvtr system.  PTT breakout is provided for HF/6M, 144, 222, 432, 902/3 and 1296 bands for amp PTT.  Every band has its own antenna jack.  Except for the 222 RF output, all IF and RF outputs are split into Rx and TX.  You can easily add attenuators and LNAs as needed, or relocate the 903/1296 T/R switches outside the box such as on a mast with a preamp and split RF.  I have external RF amps for most of the bands.  This coax and PTT breakout make it easy.  

If you already have transverters (for any band) you could package this differently by using external transverters and focus this build on the control and RF/PTT breakout and sequencing.  I now have a IC-905 for higher bands so I did not go this route.  I wanted to base the rover truck on the IC-905 since it has 1296 and higher bands, but adding 222 which uses a 28Mhz IF (or 21) won't work.  Same for the UR3LM 903 board which happens to be 28/21Mhz IF vs the standard 144Mhz IF.  

I originally envisioned this box having 6 transverter bands to allow me to use my Hermes Lite 2 (HL2) as a SDR-based IF rig with just an ethernet connection to the remotely placed package, but I found relying on a computer screen in a mobile environment too slow and cumbersome, even with control pods or dedicated controller like the PiHPSDR (I have 2 of themn).  Ease of use by the apps for switching between digital and voice, logging between multiple radios/bands and operating quickly without a laptop all factored in.

Set config #defines 
      CLEAN_SD_DB_FILE - overwrites SD card saved data file with new band data structure
      XVBOX - radio-side controller IF band data changes for the UR3LM transveter boards using 21 and 50Mhz IF instead of 28 and 144.
      M5STAMPC3U - builds the code for a (currently) headless embedded controller in the Transverter Box, set for 3 bands, 222, 903, and 1296.  

Eventually I will move a lot of config to the SD card.


![K7MDL IC-705 3-Band Transverter Box - Dec 2024 - Top View](https://github.com/user-attachments/assets/776fd484-0e7e-459f-81a9-9e05f6ebf51c)
![K7MDL IC-705 3-Band Transverter Box - Dec 2024 - Control Board with M5StampC3U](https://github.com/user-attachments/assets/76a96611-48fa-40c3-a52b-4a43ad44704f)
![K7MDL IC-705 3-Band Transverter Box - Dec 2024 - 12V power distribution](https://github.com/user-attachments/assets/2656da74-567d-4d7c-a353-442eeb2dfbea)


*****************************************

As of Nov 23, 2024 - For a user config request, I changed configuration to activate 4 XVtr bands. I set them to 903 (145 IF), 2304 (434 IF), 3400 (144 IF), and 5760 (432 IF). The outputs 1-4 are PTT for those 4 XVtr bands.  Added new WIRED_PTT #define located with the PTT polling delay #defines to make it easier to find and set (polling vs wired).  Wired is faster.   You can use the XVTR button or use wired inputs 1-4 to change Xvtr bands.

As of Sept 10 - Now save and restore Split mode.  Split is often turned on when using WSJT-X on a band, and needs to be off for SSB/CW usually.  Like the other saved parameters, it simply saves and restores what is found at band change. 
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



As of Aug 17, 2024, on a M5Stack Core Basic and Core2, BT Classic connection to the 705 runs reliably, and the 4-In/8-Out module works.  That is not the case yet for USBHost (works sometimes, then it does not for seemingly unrelated reasons, or no reason at all.  The first fully working version has been given to a field tester along with the radio I borrowed from him.  It is simply routing PTT between 2 transverters.  Can use the radio's Polled TX state or the wired PTT input as the PTT source so there is the option of no wired connection to the radio.  
CoreS3 furher off, cannot talk to the IO module yet. An early version of this program works on USB Host before the graphics and IO module added.  IO module uses I2C and nails the the USB Host (SPI) and hangs everything. M5Unified Lib also breaks USB Host. Using M5stack on Core Basic and M5Core2.h on Core2 for best USB results.   

Last I moved button monitoring for all 3 CPU models into the main loop, outside of App_loop and any FreeRTOS task where they work better. The Core2 touch buttons are very fast using the M5Unified touch buttons. The Core2 lib Core Basic buttons barely work.  Have not tried BLE on Core2 yet. 

The IO module uses a 4.7K resistor to 3.37VDC.  Draws 0.8ma when grounded.  Output pins are Vin (such as 12V or 5V if USB powered) on pin 1 of each output.  Pin 2 is a MOSFET to GND.  A DVM in continuity/beep mode will tell you if the outputs are working or not.  The IC-705 Send output transistor can handle 200ma in TX to GND, and up to 20VDC in RX.  Be sure the radio power GND is common with the M5Stack GND.  On USB power the M5Stack power GND may be floating. You can supply 12VDC on the IO module, has the red label.  Pin 2 is GND and is where you can connect GND for your outputs.  Pin 1 of the outputs are Vin (+5 or +12 for example).  

_________________________________________________________________________________


### Connection methods:

The IC-705 uses 1 of 4 connection methods for CI-V data - WiFi (WAN), BT Classic, BLE and USB Host mode.  The newer MStack CoreS3, which uses an ESP32-S3, only supports BLE and not BT Classic.  The Core2 supports both.  Core Basic is BT Classic only.  All models support WiFi but I am not using WiFi today.

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

4. Other folders:  There are other folders here storing some modified works for some CPU modules such as the M5StampC3U.  In general I am adding #defines into the CI-V Band Decoder files to account for CPU models rather than split off code and attempt to maintain the overlapping code between them.  

There is a function in use now that auto-discovers the model's CI-V address.  This code is set up to make changing between USB and BT easily done by the UI.  You can define a CI-V address as well.  A button will force a search for a new address.

### Hardware options:

Check out the Hardware Reference page at https://github.com/K7MDL2/IC-705-BLE-Serial-Example/wiki/Hardware

The M5Stack 4in/8out IO module supports 1A@24VDC on each port. Also acts as 24V or 12V power supply. Can use wired band decoder inputs from radios like the Elecraft K3 or K4, and some Yaesu radios.  Other future band decoder input options include analog voltage (Like the IC-706 or Yaesu FT-817) and old school RS-232 serial port. LAN control on an Arduino appears to be a lot of work, it is also pubicly undocumented.

A note about ESP32 and USB ports.  Many of the ESP32 chips have OTG USB ports supporting both Host and Device roles. I wanted to connect the M5Stack CPU module directly to a radio, and debug with a PC on the main USB port, or optionally, use that PC to run logging or digital mode programs like WSJT-X. Some of the ESP32-S3 boards have 2 USB ports on board, I have one with a 4.3" touch display I will be testing soon. At least the internal CPU port is OTG, the other is a Silicon Labs based Serial to USB bridge. For my project I needed small pre-packaged box with buffered IO to control radio gear and use a small display for info. 

On my Teensy projects I run up to 3 virtual serial ports on both the Device and Host sides. I control some SDR radios with the host port (RS-HFIQ). In this project I connect to an Icom IC-705 and IC-905. I plan to test on a borrowed IC-9700 once I get USB Host working reliably.  The radios have multiple virtual ports for audio and 2 serial ports, one for CI-V control and spectrum data, the other assignable, but most often used to output the internal GPS data in the form of standard NMEA strings. I also pass through both serial ports (no audio) to a optional PC so it can use the ports transparently.  On the ESP32 I have discovered the TinyUSB library used for Arduino should be able to support 2 virtual serial ports, just started looking into that.  On the PC side one port for CAT and one for debug would be nice.


### To Use this with an IC-705:

I set the default radio CI-V address to 0. It will autodetect the radio address on startup when it sees the first messages. Turn the VFO dial to accelerate this. In the software, you can set the radio_address.  For example, 0xA4 for the IC-705, 0xAC for the IC-905. I plan to make this an SD card config file choice later, or a menu button.  Same for swapping between BT and USB Host ports modes.  Today the SD card config file contains only the BT Classic target radio BT address.  For BT Classic mode, you can set your radio's BT address in the top of the main file as the default address and not require a SD Card config file.  Not required for BLE.  If using multiple BLE devices, alter the UUID number slightly to differentiate them.  For both you may want to change the name so they display on the 705 uniquely.  The name must be 16 characters exactly.   

See the Wiki page for pairing to the radio at https://github.com/K7MDL2/IC-705-BLE-Serial-Example/wiki/Pairing-the-Radio

The SD card, if present, is used to store the radio parameters for all the predefined bands, low HF up to 122Ghz.  It starts with defaults then is updated with observed (polled) parameters as each band may be selected at the radio.  That data is primarily used for bands that have a Xvtr Offset and the last non-Xvtr band used before switching into XVtr mode.  The idea is to restore the radio back to the way it was before being used as an IF rig.

Technical details including the BT/BLE CI-V connection protocol are in Wiki pages
https://github.com/K7MDL2/IC-705-BLE-Serial-Example/wiki/Technical-Details
