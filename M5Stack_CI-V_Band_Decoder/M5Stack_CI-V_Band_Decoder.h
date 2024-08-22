/*************************************************************************
      ICOM CI-V Band Decoder and PTT breakout
      K7MDL 8/2024

      BT Classic Serial or USB connection for IC-705
      USB for other models 
      Runs on M5Stack Core Basic, Core2. All have issues with USB Host compatibility as of Aug 2024.

      This is very basic and very small self-contained CI-V band decoder that can display transverter frequency with suitable offsets, 
        change bands with touch or external input and output (switch, radio decoder outputs), 2, 4, 8, or more I/O outputs.  
        Depending on the stacking module(s) chosen, can be relays, MOSFETs (1A @ 24VDC), or a proto board for DIY interfaces.  
        They also have a 9 to 24VDC power supply input and batteries of various sized.  
        With the small size you can stick it on the back of a radio or the back of a dish on a tropid.  
        Generally no hardware packaging is required. Just plug together and wire up to the connectors.
        There are several code projects that are similar, some do not have the IO., this is a customization of one of them.  
        My 905-CIV project is Teensy based and I have not added WFi or BT to it yet.  This M5Stack approach is a quick no-build mini solution
        enhanced to do some multi-byte CI-V messages and monitor PTT status to echo it out to the seledcted band's amps and/or Xvtrs and antenna relays
        There is opportunity to do some fancy graphics, present some shortcut buttons for things like memories and a band select menu that knows about transverters.

        For this to work with the 705 Bluetooth, it must be run on a chipset and BT code library that supports BT Classic serial port profile. 
        The newer M5Stack CoreS3 (ESPS3 based) and CoreS3SE apepar to only support BT , mesh and BLE, is not backward compatible with BT Classic SSP/
        The profiles listed on the 705 BT Info page lists SPP and LE. I suspect the LE is for audio connections and not serial.  
        Maybe that will chanbge in the future, no one seems to have in insight into this.  I can discover the radio with BLE, but no connection yet,
        still working on it in my spare time.

      This will Pair BT with a Passkey, no action required on teeh decoder for passkey or for radio address 

      To use this with BT on the IC-705 you first pair it on the 705.  
      Once paired, this device will auto-(re)connect.  
      This device operates in Master mode so power up order does not matter.
      On first connection after pairing the radio's CI-V address is discovered and used so there is no configuraton required

      For BT connection the IC-705 side settings required are 
      1. Bluetooth ON
      2. AutoConnect ON
      3. Data-Device-> Serialport Function -> CI-V (Echo Back ON)
      4. Pair the radio to this device:
            On the 705 menu SET -> BLUETOOTH SET -> Pairing Reception
            Power on the BT Band Decoder.   If it times out, power cycle the decoder to retry
            "IC-705 Decoder 1" device name will display when it is discovered by the radio
            The radio will prompt to confirm a passkey number, it should match the decoder display if any attached or on a serial terminal
            Press OK
            The Decoder will auto accept the passkey confirm from the radio and connect and start operating
            It is now paired and connected and will auto-reconnect as needed.


      Tested on the IC-705 (BT andf USB) and IC-905 (USB).  Should work with other models like the 9700.

      This M5Stack decoder is a mini version of my large USB CI-V Band Decoder for the 905 and 705 which is built on a Teensy and passes both of radio's 2 
          USB serial port data (CI-V and GPS) on to a optional PC. It has options for 7" or 4.3" touch display or encoders and can do radio control allowing
          for transverter control with each Xvtr band keeping its own parameters.
          https://github.com/K7MDL2/ICOM_IC-905_CIV
      
      I have a hardware/softaare big Band decoder at https://github.com/K7MDL2/RF-Power-Meter-V1
      
      The Teensy based 905 decoder uses the UI and control framewark from my Teensy SDR project at https://github.com/K7MDL2/KEITHSDR 
      
      Derived from the following project, thanks!
      _________________________________________________
      T1 Interface
      V2.0 (12.09.2023)
      Peter Jonas DL5DLA

      This code, running on an ESP32, requests the current frequency
      from the ICOM IC-705 via bluetooth connection, and sends the
      related band number to the Elecraft T1. Based on the band
      number, the T1 applies the stored tuning settings. A re-tuning
      is possible directly after changing the frequency. For the next
      few seconds the T1 waits for a carrier and will tune again. 

      For first use, the ESP32 must be paired to the IC-705. Later on
      the ESP32 should be powered already, before the IC-705 is
      switched on. If someone knows how the bluetooth connection
      can be setup the other way around (switch on the IC-705, then
      the esp32), please let me know.

      The code is compiled on PlatformIO, but should also work using
      the Arduino IDE.

      This code is based on the code from
        Ondrej Kolonicny (ok1cdj) https://github.com/ok1cdj/IC705-BT-CIV
      and
        Matthew Robinson (VK6MR) https://zensunni.org/blog/2011/01/19/arduino-elecraft-t1-interface/

      Many thanks to both for their example codes giving me the first insight on how
      to deal with the communication between the IC-705 and T1.

      In this version the ESP32 device is set as bluetooth master and the IC-705 as client.
      Due to this the bluetooth connection is more robust, and the IC-705 is reconnected
      no matter if the TRX or the ESP32 is switched off temporarly.

      If you are using BT Classic SPP then it is important to set the "bd_address" in the next lines below!
*************************************************************************/
#ifndef BT_USB_
#define BT_USB_

//#define CORE2LIB   // applies only to Core2 - forces M5Core2 lib vs M5Unified  - Touch works better with M5Unified

#if defined ( CONFIG_IDF_TARGET_ESP32S3 )
  #include <M5CoreS3.h>
  static m5::touch_state_t prev_state;
  #define SD_SPI_SCK_PIN  36
  #define SD_SPI_MISO_PIN 35
  #define SD_SPI_MOSI_PIN 37
  #define SD_SPI_CS_PIN   4
  #define CORE3

#elif defined ( ARDUINO_M5STACK_CORE2 ) || defined ( ARDUINO_M5STACK_Core2 )
  #ifdef CORE2LIB
    #include <M5Core2.h>    // USB Host sort to works
  #else
    #include <M5Unified.h>  // kills off USB Host
  #endif
  #define SD_SPI_SCK_PIN  18
  #define SD_SPI_MISO_PIN 38
  #define SD_SPI_MOSI_PIN 23
  #define SD_SPI_CS_PIN   4
  #define CORE2
#else
  #include <M5Stack.h>
  #define CORE
#endif

#include <stdint.h>
#include <Update.h>
#include "Wire.h"
#include "time.h"
#include "FS.h"
#include <SPI.h>
#include <SD.h>

#define IC705 0xA4
#define IC905 0xAC
#define RADIO_ADDR IC705

// NOTE: With a single USB virtual Serial port to the PC, ANY debug on Serial will interfere with a program like WSJT-X passing through to teh radio.

#define CONTROLLER_ADDRESS 0xE0  //Controller address
#define BROADCAST_ADDRESS 0x00

#define START_BYTE 0xFE  // Start byte
#define STOP_BYTE 0xFD   // Stop byte

#define CMD_READ_FREQ 0x03  // Read operating frequency data

#define POLL_PTT_DEFAULT 47  // poll the radio for PTT status odd numbers to stagger them a bit \
                             // USB on both the 705 and 905 respond to PTT requests slower on USB than BT on the 705. \
                             // Also polls the wired inputs
#define POLL_PTT_USBHOST 167  // Dynamically changes value based on detected radio address. \
                             // By observation, on USB, the radio only responds once every few seconds when the radio \
                             //   has not changed states.  It will immediately reply to a poll if the Tx state changed. \
                             //   Still have to poll fast for controlling external PTT, most requests will not be answered. \
                             //   Unlike other modes.  BT seems to answer every request. USB2 engine is likely the same in \
                             //   all radios, where BT got a capacity upgrade.  The 905 acts the same as the 905 (905 is USB only) \
                             //   Have not compared to a LAN connection.
#define POLL_RADIO_FREQ 308  // poll the radio for frequency
#define POLL_RADIO_UTC 1000  // poll radio for time and location

// Chose the combination needed.  Note that at least one service must be enabled.
//#define BTCLASSIC   // Can define BTCLASSIC *** OR ***  BLE, not both.  No BT version is  OK if USB Host is enabled
                    // BT Classic does not work on Core3.  It might on Core2 (untested)
#define BLE         // Core 3.  Maybe works on Core 2, TBD
//#define USBHOST   // if no BLE or BTCLASSIC this must be enabled.
#define IO_MODULE   // enable the 4-In/8-Out module
#define SDCARD      // enable sd card features

//#define PC_PASSTHROUGH   // fwd through BT or USBHOST data to a PC if connected.  All debug must be off!

#ifndef PC_PASSTHROUGH        // shut off by default when PASSTHRU MODE is on
  #define PRINT_VFO_TO_SERIAL // uncomment to visually see VFO updates from the radio on Serial
  #define PRINT_PTT_TO_SERIAL // uncomment to visually see PTT updates from the radio on Serial
#endif 

//--------------------------------------      END OF USER CONFIG    ---------------------------------------------------------------

//*************************************************************************************************************************************
//*************************************  should not be any user config below this point *********************************************
//*************************************************************************************************************************************

#if defined ( CONFIG_IDF_TARGET_ESP32S3 ) 
  #undef BTCLASSIC   // Only BLE on S3
  //#define USBHOST   // use this for now until BLE code added
  //#define WATCH_BLE_SERIAL
#elif defined ( ARDUINO_M5STACK_CORE2 ) || defined ( ARDUINO_M5STACK_Core2 )  // For Core2 both BLE and BT Classic OK
  #if defined ( BTCLASSIC ) && defined ( BLE)
    #undef BLE   // BTCLASSIC is more reliable and faster connect so far.
  #endif
  //#define USBHOST   // use this for now until BLE code added
  //#define WATCH_BLE_SERIAL
#else
  #undef BLE    // BLE does not work on Core Basic so BTCLAsssic only
#endif

//#define SSP                           // use BT SSP - pair with a passkey

// After #defines
#include "DebugPrint.h"
#include "CIV.h"

// Function prototypes:
void configRadioBaud(uint16_t);
uint8_t readLine(void);
bool searchRadio();
void sendCatRequest(const uint8_t cmd_num, const uint8_t Data[], const uint8_t Data_len);  // first byte in Data is length
void printFrequency(void);
void processCatMessages();
void sendBit(int);
void sendBand(byte);
void printDirectory(File dir, int numTabs);
void UpdateFromFS(fs::FS &fs);
void read_Frequency(uint8_t data_len);
void draw_new_screen(void);
unsigned int hexToDec(String hexString);
void display_Freq(uint64_t _freq, bool _force);
void display_Time(uint8_t _UTC, bool _force);
void display_Xvtr(bool _band, bool _force);
void display_PTT(uint8_t _PTT_state, bool _force);
void display_Band(uint8_t _band, bool _force);
void display_Grid(char _grid[], bool _force);
void SetFreq(uint64_t Freq);

const uint64_t decMulti[] = { 100000000000, 10000000000, 1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1 };

#define BAUD_RATES_SIZE 4
const uint16_t baudRates[BAUD_RATES_SIZE] = { 19200, 9600, 4800, 1200 };

struct Bands {
  char band_name[6];    // Freindly name or label.  Default here but can be changed by user.
  uint64_t edge_lower;  // band edge limits for TX and for when to change to next band when tuning up or down.
  uint64_t edge_upper;
  uint64_t Xvtr_offset;  // Offset to add to radio frequency.
                         // When all is correct, it will be within the band limits and allow PTT and Band decoder outputs
  uint64_t VFO_last;     // store the last used frequency on each band.
                         // Subtract the LO offset and send the result to the radio
};

extern uint8_t USBHost_ready;  // 0 = not mounted.  1 = mounted, 2 = system not initialized
extern bool USBH_connected;
extern bool restart_USBH_flag;
extern bool restart_BT_flag;
extern uint16_t background_color;
extern uint64_t frequency;

#endif