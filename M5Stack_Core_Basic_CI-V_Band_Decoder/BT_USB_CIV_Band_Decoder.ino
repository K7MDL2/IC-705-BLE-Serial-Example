/*************************************************************************
      ICOM CI-V Band Decoder and PTT breakout
      K7MDL 8/2024

      BT Classic Serial or USB connection for IC-705
      USB for other models 
      Runs on M5Stack Core Basic, possible Core2. Core3 has issues with USB Host compatibility as of Aug 2024.

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
//#include <M5CoreS3.h>
//#include <M5Stack.h>
//#include <M5Unified.h>
#include "M5_Max3421E_Usb.h"
#include "SPI.h"
#include "Wire.h"
#include "MODULE_4IN8OUT.h"
#include "CIV.h"
#include "time.h"

// Chose the combination needed.  Note that at least one service must be enabled.
#define BTCLASSIC   // Can define BTCLASSIC *** OR ***  BLE, not both.  No BT version is OK if USB Host is enabled
                    // BT Classic does not work on Core3.  It might on Core2 (untested)
//#define BLE    // only works on Core3, maybe on Core2 (untested)
//#define USBHOST   // if no BLE or BTCLASSIC this must be enabled.

//#define PC_PASSTHROUGH   // fwd through BT or USBHOST data to a PC if connected.  All debug must be off!

#ifndef PC_PASSTHROUGH   // shut off by default when PASSTHRU MODE is on
  #define PRINT_VFO_TO_SERIAL // uncomment to visually see VFO updates from the radio on Serial
  #define PRINT_PTT_TO_SERIAL // uncomment to visually see PTT updates from the radio on Serial
#endif 

//#define SEE_RAW_RX // see raw hex messages from radio
//#define SEE_RAW_TX // see raw hex messages from radio

#ifdef BTCLASSIC  // can set to BT on or off at startup
  #include "BluetoothSerial.h"
  bool BT_enabled = 1;  // configuration toggle between BT and USB - Leave this 0, must start on USB Hoset first, then can switch over.
#endif

#ifdef BLE   // can set to BT on or off at startup
  bool BT_enabled = 1;  // configuration toggle between BT and USB - Leave this 0, must start on USB Hoset first, then can switch over.
#endif

#define IC705 0xA4
#define IC905 0xAC
uint8_t radio_address = 0;  //Transceiver address.  0 allows auto-detect on first messages form radio
bool auto_address = true;   // If true, detects new radio address on connection mode changes
                            // If false, then the last used address, or the preset address is used.  
                            // If Search for Radio button pushed, then ignores this and looks for new address
                            //   then follows rules above when switch connections

enum band_idx {BAND_AM,BAND_160M,BAND_80M,BAND_60M,BAND_40M,BAND_30M,BAND_20M,BAND_17M,BAND_15M,BAND_12M,BAND_10M,BAND_6M,
                BAND_FM,BAND_AIR,BAND_2M,BAND_1_25M,BAND_70cm,BAND_33cm,BAND_23cm,BAND_13cm,BAND_9cm,BAND_6cm,BAND_3cm,
                BAND_24G,BAND_47G,BAND_76G,BAND_122G,B_GENERAL,NUM_OF_BANDS};

//#define NUM_OF_BANDS B_GENERAL+1

bool XVTR         = 1;
bool XVTR_enabled = 0;   // set to 1 when a transverter feature is active
// Edit the bands table farther down the page to enter the fixed LO offset (in Hz) to add to radio dial
// frequency for the transverter band of interest. Only 1 band supported at this point
uint8_t XVTR_Band = 0;   // Xvtr band to display - temp until a band select menu is built

uint8_t brightness = 130;  // 0-255

// NOTE: With a single USB vertial Serial port to the PC, ANY debug on Serial will interfere with a program like WSJT-X passing through to teh radio.

// ######################################################################
// Enter the BD_ADDRESS of your IC-705. You can find it in the Bluetooth
// settings in section 'Bluetooth Device Information'

uint8_t bd_address[6] = { 0x30, 0x31, 0x7d, 0x33, 0xbb, 0x7f };
// ######################################################################

// These pins are used byte the USB Host sheild M5_USBH_Host USBHost(&SPI, 18, 23, 19, 5, 35);  // Core basic
//#define DATA_PIN 5  // GPIO18 input/output
//#define TUNE_PIN 2  // GPIO26 (output)

#define CONTROLLER_ADDRESS 0xE0  //Controller address
#define BROADCAST_ADDRESS 0x00

#define START_BYTE 0xFE  // Start byte
#define STOP_BYTE 0xFD   // Stop byte

#define CMD_READ_FREQ 0x03  // Read operating frequency data

#define POLL_PTT_DEFAULT   15   // poll the radio for PTT status odd numbers to stagger them a bit
                                // USB on both the 705 and 905 respond to PTT requests slower on USB than BT on the 705.
#define POLL_PTT_USBHOST   27   // Dynamically changes value based on detected radio address.
                                // By observation, on USB, the radio only responds once every few seconds when the radio
                                //   has not changed states.  It will immediately reply to a poll if the Tx state changed.
                                //   Still have to poll fast for controlling external PTT, most requests will not be answered.
                                //   Unlike other modes.  BT seems to answer every request. USB2 engine is likely the same in
                                //   all radios, where BT got a capacity upgrade.  The 905 acts the same as the 905 (905 is USB only)
                                //   Have not compared to a LAN connection.
#define POLL_RADIO_FREQ   308   // poll the radio for frequency
#define POLL_RADIO_UTC   1000   // poll radio for time and location
#define POLL_BAND_INPUT_LINES  1000  // read the IO module input lines.  Maybe band and /or PTT.  
                                   // If band only usage then can be set slower.

uint8_t UTC = 1;  // 0 local time, 1 UTC time
extern Adafruit_USBH_CDC SerialHost;

volatile bool get_new_address_flag = false;
volatile bool restart_USBH_flag = false;
volatile bool restart_BT_flag = false;
extern volatile bool USBH_connected;
extern uint8_t USBHost_ready;

//#define SSP                           // use BT SSP - pair with a passkey
//#ifdef SSP
bool confirmRequestPending = true;
//#endif

// if no BT option chosen then set BT_enabled to 0;
#if !defined (BTCLASSIC)  && !defined (BLE)
bool BT_enabled = 0;  // configuration toggle between BT and USB - Leave this 0, must start on USB Hoset first, then can switch over.
#endif 

MODULE_4IN8OUT module;

#ifdef BTCLASSIC
  #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
  #error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
  #endif

  #if !defined(CONFIG_BT_SPP_ENABLED)
  #error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
  #endif

  #define BT_DISCOVER_TIME 10000
  esp_spp_sec_t sec_mask = ESP_SPP_SEC_NONE;  // or ESP_SPP_SEC_ENCRYPT|ESP_SPP_SEC_AUTHENTICATE to request pincode confirmation
  //esp_spp_role_t role=ESP_SPP_ROLE_SLAVE; // or ESP_SPP_ROLE_MASTER
  esp_spp_role_t role = ESP_SPP_ROLE_MASTER;  // or ESP_SPP_ROLE_MASTER
#endif

// Function prototypes:
void configRadioBaud(uint16_t);
uint8_t readLine(void);
bool searchRadio();
void sendCatRequest(const uint8_t cmd_num, const uint8_t Data[], const uint8_t Data_len);  // first byte in Data is length
void printFrequency(void);
void processCatMessages();
void sendBit(int);
void sendBand(byte);

const uint64_t decMulti[] = { 100000000000, 10000000000, 1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1 };

#define BAUD_RATES_SIZE 4
const uint16_t baudRates[BAUD_RATES_SIZE] = { 19200, 9600, 4800, 1200 };


struct Bands {
    char        band_name[6];  // Freindly name or label.  Default here but can be changed by user.
    uint64_t    edge_lower;     // band edge limits for TX and for when to change to next band when tuning up or down.
    uint64_t    edge_upper;
    uint64_t    Xvtr_offset;
};

const struct Bands bands[NUM_OF_BANDS] = {
  {   "AM",       535000,      1705000,           0},   // AM
  { "160M",      1800000,      2000000,           0},   // 160m
  {  "80M",      3500000,      4000000,           0},   // 80m
  {  "60M",      5351000,      5367000,           0},   // 60m
  {  "40M",      7000000,      7300000,           0},   // 40m
  {  "30M",     10100000,     10150000,           0},   // 30m
  {  "20M",     14000000,     14350000,           0},   // 20m
  {  "17M",     18068000,     18168000,           0},   // 17m
  {  "15M",     21000000,     21450000,           0},   // 15m
  {  "12M",     24890000,     24990000,           0},   // 12m
  {  "10M",     28000000,     29700000,           0},   // 10m
  {   "6M",     50000000,     54000000,           0},   // 6m
  {   "FM",     88000000,    108000000,           0},   // FM
  {  "Air",    118000000,    137000000,           0},   // AIR
  {   "2M",    144000000,    148000000,           0},   // 2m
  {"1.25M",    222000000,    225000000,   194000000},   // 222
  { "70cm",    430000000,    450000000,           0},   // 430/440
  { "33cm",    902000000,    928000000,   758000000},   // 902
  { "23cm",   1240000000,   1300000000,  1152000000},   // 1296Mhz
  { "13cm",   2304000000,   2450000000,  1870000000},   // 2.3 and 2.4GHz
  {  "9cm",   3300000000,   3500000000,           0},   // 3.3GHz
  {  "6cm",   5650000000,   5925000000,           0},   // 5.7GHz
  {  "3cm",  10000000000,  10500000000,           0},   // 10GHz
  {  "24G",  24000000000,  24002000000,           0},   // 10GHz
  {  "47G",  47000000000,  47002000000,           0},   // 10GHz
  {  "76G",  76000000000,  76002000000,           0},   // 10GHz
  { "122G", 122000000000, 122002000000,           0},   // 10GHz
  { "GENE",            0, 123000000000,           0}    // 10GHz
};

String title = "CIV Band Decoder";   // make exactly 16 chards if used as the BT device name
uint16_t baud_rate;            //Current baud speed
uint32_t readtimeout = 10;          //Serial port read timeout
static uint8_t read_buffer[2048];      //Read buffer
uint64_t frequency;            //Current frequency in Hz
uint8_t band = B_GENERAL;
uint32_t timer;
uint16_t background_color = BLACK;
uint16_t text_color = WHITE;
uint8_t PTT = 0;
uint8_t prev_PTT = 1;
extern char Grid_Square[];

String modes;

uint8_t prev_band = 0xFF;
uint64_t prev_frequency = 0;
bool btConnected = false;
bool btPaired = false;
uint32_t temp_passkey;
uint16_t poll_radio_ptt = POLL_PTT_DEFAULT;   // can be changed with detected radio address.  

#ifdef BTCLASSIC
void callback(esp_spp_cb_event_t, esp_spp_cb_param_t *);
BluetoothSerial SerialBT;

// ------------------------------------------
//   Callback to get info about connection
// ------------------------------------------
void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {  // 34
    btConnected = true;
    btPaired = true;
    Serial.println("BT Client Connected");
  } else if (event == ESP_SPP_CLOSE_EVT)  // 27
  {
    btConnected = false;
    if (btPaired) Serial.println("BT Client disconnected");  // suppress when we are not paired yet
  }
}
#endif

// ----------------------------------------
//      Connect to bluetooth
// ----------------------------------------
void configRadioBaud(uint16_t baudrate) {

  // Connect to client:


  #ifdef BTCLASSIC
  //DPRINTF("Connect to bluetooth client ...");
  if (BT_enabled) {
    if (btPaired && !btConnected) {
      btConnected = SerialBT.connect(bd_address, role);  // connect as master so the order of power up between this and the radio do not matter
      DPRINTLNF("Trying to connect to Transceiver");
    }
    if (btPaired && btConnected)
      DPRINTLNF("Transceiver paired and connected");
  }
  #endif
}

// ----------------------------------------
//    Read incoming line from bluetooth
// ----------------------------------------
uint8_t readLine(void) {
  uint8_t byte;
  uint8_t counter = 0;
  uint32_t ed = readtimeout;  // not initialized!

  #ifdef BTCLASSIC
  if (BT_enabled) 
  {
    while (btConnected) {
      while (!SerialBT.available()) {
        if (--ed == 0 || !btConnected) return 0;  // leave the loop if BT connection is lost
      }
      ed = readtimeout;
      byte = SerialBT.read();
      if (byte == 0xFF) continue;  //TODO skip to start byte instead

      read_buffer[counter++] = byte;
      if (STOP_BYTE == byte) break;

      if (counter >= sizeof(read_buffer)) return 0;
    }
  } // BT_Classic
  else 
  {
  #endif
    while (USBH_connected) {
      while (!SerialHost.available()) {
        if (--ed == 0 || !USBH_connected) return 0;  // leave the loop if connection is lost
      }
      ed = readtimeout;
      byte = SerialHost.read();
      if (byte == 0xFF) continue;  //TODO skip to start byte instead

      read_buffer[counter++] = byte;
      if (STOP_BYTE == byte) break;

      if (counter >= sizeof(read_buffer)) return 0;
    }  // host
  #ifdef BTCLASSIC
  }  // usb host
  #endif

  #ifdef PC_PASSTHROUGH
  // SerialHost or SerialBT -> Serial
  if (counter)
  {
    Serial.write(read_buffer, counter);
    Serial.flush();
  }
  #endif

  return counter;
}


// ----------------------------------------
//       Get address of transceiver
// ----------------------------------------
bool searchRadio() {
  for (uint8_t baud = 0; baud < BAUD_RATES_SIZE; baud++) {
    configRadioBaud(baudRates[baud]);

    //DPRINTLNF("Send out Frequency Request");
    sendCatRequest(CIV_C_F_READ, 0, 0);
    vTaskDelay(50);
    //DPRINTLNF("Look for response from radio");

    if (readLine() > 0) {
      if (read_buffer[0] == START_BYTE && read_buffer[1] == START_BYTE) {
        radio_address = read_buffer[3];
      }
      return true;
    }
  }
  radio_address = 0xFF;
  return false;
}

// ----------------------------------------
//    get band from frequency
// ----------------------------------------
byte getBand(uint64_t _freq) 
{
  for (uint8_t i = 0; i < NUM_OF_BANDS; i++) {
    if (_freq >= bands[i].edge_lower && _freq <= bands[i].edge_upper) {
      if (i >= B_GENERAL) return B_GENERAL;
      return i;
    }
  }
  return 0xFF;  // no band for considered frequency found
}

// ----------------------------------------
//      Send CAT Request
// ----------------------------------------
void sendCatRequest(const uint8_t cmd_num, const uint8_t Data[], const uint8_t Data_len)  // first byte in Data is length
{
  int8_t msg_len;
  uint8_t req[50] = { START_BYTE, START_BYTE, radio_address, CONTROLLER_ADDRESS };

  //DPRINTF("sendCatRequest: USBH_connected = "); DPRINTLN(USBH_connected);

  for (msg_len = 0; msg_len <= cmd_List[cmd_num].cmdData[0]; msg_len++)  // copy in 1 or more command bytes
    req[msg_len + 4] = cmd_List[cmd_num].cmdData[msg_len + 1];

  msg_len += 4;  // Tee up to add data if any

  if (Data_len != 0)  // copy in 1 or more data bytes, if any
  {
    for (uint8_t j = 0; j < Data_len; j++)  //pick up with value i
      req[msg_len++] = Data[j];
  } else

    msg_len -= 1;

  req[msg_len] = STOP_BYTE;

  //#define SEE_RAW_TX  // an also be set at top of file
  #ifdef SEE_RAW_TX 
  Serial.print(F("--- Tx Raw Msg: "));
  for (uint8_t k = 0; k <= msg_len; k++) {
    Serial.print(req[k], HEX); Serial.print(F(","));
  }
  Serial.print(F(" msg_len = ")); Serial.print(msg_len); Serial.println(F(" END"));
  #endif

  //Serial.print(F("Poll rate = ")); Serial.print(poll_radio_ptt);
  //Serial.print(F("   BT connected = ")); Serial.print(btConnected);
  //Serial.print(F("   USBH connected = ")); Serial.println(USBH_connected);
  
  //#define RAWT  // for a more detailed look
  if (USBH_connected && !btConnected)
  {
    if (msg_len < sizeof(req) - 1)  // ensure our data is not longer than our buffer
    {
    #ifdef SEE_RAWT
      DPRINTF("Snd USB Host Msg: ");
    #endif
      for (uint8_t i = 0; i <= msg_len; i++) 
      {
    #ifdef SEE_RAWT
        Serial.print(req[i], HEX);
    #endif
        #ifndef PC_PASSTHROUGH
          if (!SerialHost.write(req[i]))
            DPRINTLNF("sendCatRequest: USB Host tx: error");
        #endif

        #ifdef SEE_RAWT
        DPRINTF(",");
        #endif
      }
      #ifdef SEE_RAWT
      DPRINTF(" END TX MSG, msg_len = ");DPRINTLN(msg_len);
      #endif
    } 
    else 
    {
      DPRINTLNF("sendCatRequest: USB Host buffer overflow");
    }
  }
  #ifdef BTCLASSIC
  else if (btConnected && !SerialBT.isClosed() && SerialBT.connected()) 
  {
    if (msg_len < sizeof(req) - 1)  // ensure our data is not longer than our buffer
    {
      #ifdef SEE_RAWT
      DPRINTF("Snd BT Msg: ");
      #endif
      for (uint8_t i = 0; i <= msg_len; i++) {
        #ifdef SEE_RAWT
        Serial.print(req[i], HEX);
        #endif
        if (!SerialBT.write(req[i]))
          DPRINTLNF("sendCatRequest: BT tx: error");
        #ifdef SEE_RAWT
        DPRINTF(",");
        #endif
      }
      #ifdef SEE_RAWT
      DPRINTF(" END TX MSG, msg_len = "); DPRINTLN(msg_len);
      #endif
    } 
    else 
    {
      DPRINTLNF("sendCatRequest: BT buffer overflow");
    }
  }
  else
    DPRINTLNF(" No open BT or USB Host ports to send to");
  #endif // BTCLASSIC
}

// ----------------------------------------
//      Print the received frequency
// ----------------------------------------
void read_Frequency(uint8_t data_len) 
{
    frequency = 0;
    //FE FE E0 42 03 <00 00 58 45 01> FD ic-820  IC-705  5bytes, 10bcd digits
    //FE FE 00 40 00 <00 60 06 14> FD ic-732
    //FE FE E0 AC 03 <00 00 58 45 01 01> FD  IC-905 for 10G and up bands - 6bytes, 12 bcd digits
    uint64_t mul = 1;

    // use the data length to loop an extra byte when needed for the IC905 on 10GHz bands and up
    for (uint8_t i = 5; i < 5 + data_len; i++) 
    {   
      if (read_buffer[i] == 0xFD) continue;  //spike
      frequency += (read_buffer[i] & 0x0F) * mul; mul *= 10;    // * decMulti[i * 2 + 1];
      frequency += (read_buffer[i] >> 4) * mul; mul   *= 10;      //  * decMulti[i * 2];
    }
    
    if (XVTR_enabled)
      frequency += bands[XVTR_Band].Xvtr_offset;
  
    band = getBand(frequency);

    //Serial.printf("Freq %-13llu  band =  %d  Xvtr_Offset = %llu  datalen = %d   btConnected %d   USBH_connected %d   BT_enabled %d   radio_address %X\n", frequency, band, bands[XVTR_Band].Xvtr_offset, data_len, btConnected, USBH_connected, BT_enabled, radio_address);
}

#ifdef BTCLASSIC
#ifdef SSP

void BTConfirmRequestCallback(uint32_t numVal) {
  confirmRequestPending = true;
  DPRINTF("Confirmation ID: ");
  DPRINTLN(numVal);
  //M5.Lcd.setTextColor(text_color, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
  //M5.Lcd.setTextSize(2);                              // to Set the size of text from 0 to 255
  //M5.Lcd.setCursor(10, 90);                           //Set the location of the cursor to the coordinates X and Y
  //M5.Lcd.drawString(" Passkey is %d" + String(numVal), 10, 90, 5);
  temp_passkey = numVal;
  SerialBT.confirmReply(true);
}

void BTAuthCompleteCallback(bool success) {
  if (success) {
    DPRINTLNF("Pairing success!!");
    btPaired = true;
    confirmRequestPending = false;
    //M5.Lcd.setTextColor(background_color);            //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
    //M5.Lcd.setTextSize(2);                            // to Set the size of text from 0 to 255
    //M5.Lcd.setCursor(10, 90);                         //Set the location of the cursor to the coordinates X and Y
    //M5.Lcd.drawString(" Passkey is %d" + String(temp_passkey), 10, 90, 5);  // erase passkey from screen now that we are paired
  } else {
    DPRINTLNF("Pairing failed, (Timed out or rejected by user!!");
    btPaired = false;
  }
}
#endif
#endif

// ----------------------------------------
//         sendBit
// ----------------------------------------
void sendBit(int bit) {

  //digitalWrite(DATA_PIN, HIGH);
  if (bit != 0) {
    vTaskDelay(4);
  } else {
    delayMicroseconds(1500);
  }

  //digitalWrite(DATA_PIN, LOW);
  delayMicroseconds(1500);
}

// --------------------------------------------------
//   Process the received messages from transceiver
// --------------------------------------------------
void processCatMessages() {
  /*
    <FE FE E0 42 04 00 01 FD  - LSB
    <FE FE E0 42 03 00 00 58 45 01 FD  -145.580.000

    FE FE - start bytes
    00/E0 - target address (broadcast/controller)
    42 - source address
    00/03 - data type
    <data>
    FD - stop byte
  */

  uint8_t cmd_num;
  //static uint8_t TX_last = 0;
  uint8_t match = 0;
  uint8_t data_start_idx = 0;
  uint8_t data_len = 0;
  uint8_t data[20] = {};

  #ifdef BTCLASSIC
    while ( btConnected && SerialBT.available() || !BT_enabled && SerialHost.available())  // &&  USBH_connected))
  #else
    while (!BT_enabled && SerialHost.available())  // &&  USBH_connected))
  #endif
  {
    bool knowncommand = true;
    int i;
    int msg_len;

    cmd_num = 255;
    match = 0;

    if ((msg_len = readLine()) > 0) {

      //#define SEE_RAW_RX
      #ifdef SEE_RAW_RX
        Serial.print(F("+++ Rx Raw Msg: "));
        for (uint8_t k = 0; k < msg_len; k++) {
          Serial.print(read_buffer[k], HEX);
          Serial.print(F(","));
        }
        Serial.print(F(" msg_len = "));
        Serial.print(msg_len);
        Serial.println(F(" END"));
      #endif

      if (read_buffer[0] == START_BYTE && read_buffer[1] == START_BYTE) {
        if (read_buffer[3] == radio_address) {
          if (read_buffer[2] == CONTROLLER_ADDRESS || read_buffer[2] == BROADCAST_ADDRESS) {

            for (cmd_num = CIV_C_F_SEND; cmd_num < End_of_Cmd_List; cmd_num++)  // loop through the command list structure looking for a pattern match
            {
              //DPRINTF("processCatMessageslist: list index = "); DPRINTLN(cmd_num);
              for (i = 1; i <= cmd_List[cmd_num].cmdData[0]; i++)  // start at the highest and search down. Break out if no match. Make it to the bottom and you have a match
              {

                //DPRINTF("processCatMessages: byte index = "); DPRINTLN(i);
                //Serial.printf("processCatMessages: cmd_num=%d from radio, current byte from radio = %X  next byte=%X, on remote length=%d and cmd=%X\n",cmd_num, read_buffer[3+i], read_buffer[3+i+1], cmd_List[cmd_num].cmdData[0], cmd_List[cmd_num].cmdData[1]);
                if (cmd_List[cmd_num].cmdData[i] != read_buffer[3 + i]) {
                  //DPRINTF("processCatMessages: Skip this one - Matched 1 element: look at next field, if any left. CMD Body Length = ");
                  //DPRINT(cmd_List[cmd_num].cmdData[0]); DPRINTF(" CMD  = "); Serial.print(cmd_List[cmd_num].cmdData[i], HEX);DPRINTF(" next RX byte = "); DPRINTLN(read_buffer[3+i+1],HEX);
                  match = 0;
                  break;
                }
                match++;
                //DPRINTF("processCatMessages: Possible Match: Len = "); Serial.print(cmd_List[cmd_num].cmdData[0],DEC); DPRINTF("  CMD1 = "); Serial.print(read_buffer[4],HEX);
                //DPRINTF(" CMD2  = "); Serial.print(read_buffer[5],HEX); DPRINTF(" Data1/Term  = "); DPRINTLN(read_buffer[6],HEX);
              }

              //if (read_buffer[3+i] == STOP_BYTE)  // if the next byte is not a stop byte then it is thge next cmd byte or maybe a data byte, depends on cmd length

              if (match && (match == cmd_List[cmd_num].cmdData[0])) {
                //DPRINTF("processCatMessages:    FOUND MATCH: Len = "); Serial.print(cmd_List[cmd_num].cmdData[0],DEC); DPRINTF("  CMD1 = "); Serial.print(read_buffer[4],HEX);
                //DPRINTF(" CMD2  = "); Serial.print(read_buffer[5],HEX);  DPRINTF(" Data1/Term  = "); Serial.print(read_buffer[6],HEX); DPRINTF("  Message Length = "); DPRINTLN(msg_len);
                break;
              }
            }

            data_start_idx = 4 + cmd_List[cmd_num].cmdData[0];
            data_len = msg_len - data_start_idx - 1;

            uint8_t k;
            for (k = 0; k < msg_len; k++)
              data[k] = read_buffer[data_start_idx + k];

            //Serial.printf("cmd = %X  data_start_idx = %d  data_len = %d\n", cmd_List[cmd_num].cmdData[1], data_start_idx, data_len);

            if (cmd_num >= End_of_Cmd_List - 1) {
              //Serial.printf("processCatMessages: No message match found - cmd_num = %d  read_buffer[4 & 5] = %X %X\n", cmd_num, read_buffer[4], read_buffer[5]);
              knowncommand = false;
            } else {
              CIV_Action(cmd_num, data_start_idx, data_len, msg_len, read_buffer);
            }
          }  // is controller address
        }    // is radio address
      }      // is preamble
    }        // readline
  }          // while
}

// ----------------------------------------
//         sendBand
// ----------------------------------------
void sendBand(byte band) {

  unsigned long previousTime = 0;
  const long maxWaitTime = 50;

  // Pull the TUNE_PIN line high for half a second
  //digitalWrite(TUNE_PIN, HIGH);
  vTaskDelay(500);
  //digitalWrite(TUNE_PIN, LOW);

  // The ATU will pull the DATA_PIN line HIGH for 50ms

  previousTime = millis();
  //while(digitalRead(DATA_PIN) == LOW) {
  // Measure time the while loop is active, and jump out after maxWaiTime.
  // This ensures that the program does not lock in case the communication
  // with the ATU is temporarly broken

  //unsigned long currentTime = millis();
  //if (currentTime - previousTime > maxWaitTime) {
  //  DPRINTLNF("Error: No positive pulse from T1 detected!");
  //  return;
  //}
  //}

  //while(digitalRead(DATA_PIN) == HIGH) {
  //}
  // Wait 10ms
  //vTaskDelay(10);

  // and then send data on the DATA line
  //pinMode(DATA_PIN, OUTPUT);

  // 1 bits are HIGH for 4ms
  // 0 bits are HIGH for 1.5ms
  // Gap between digits is 1.5ms LOW

  // 1st bit
  //sendBit(band & 8);
  //sendBit(band & 4);
  //sendBit(band & 2);
  //sendBit(band & 1);

  // Leave the line LOW
  //digitalWrite(DATA_PIN, LOW);

  // and switch it back to an input
  //pinMode(DATA_PIN, INPUT);
}

#ifdef BTCLASSIC
void bt_loop(void) 
{
  if (BT_enabled && !btConnected)
  //while (BT_enabled) 
  {  //} && btPaired) { 
    //while (!chk_Buttons())
    //{
    //  vTaskDelay(1);
    // }

    draw_new_screen();
    DPRINTLNF("BT Loop");
    M5.Lcd.setTextColor(text_color, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
    M5.Lcd.drawString("Connecting to BT ...", 80, 100, 4);

    btConnected = SerialBT.connect(bd_address, role);
    if (btConnected) {
      DPRINTLNF("BT Transceiver reconnected");
      prev_band = 255;
      frequency = 0;  // cause the screen to refresh
      //break;
    }
    else
      DPRINTLNF("BT Transceiver not connected");
    
    draw_new_screen();
  }
}
#endif

uint8_t Get_Radio_address(void) {
  uint8_t retry_Count = 0;

    // if you do not see this print after a possible USB lockup 
    //   then the app loop task, or USBHost task is stalled.  Btn often still runs.
    //   app wont do anything untial and event comes in.
  if (get_new_address_flag == true) 
  {
      Serial.print("Getting new address.  radio address is now "); Serial.println(radio_address, HEX);
  }

  if ((BT_enabled && btConnected) || (USBH_connected && !BT_enabled))  // handle both USB and BT
  {
    get_new_address_flag = false;
    
    while (radio_address == 0x00 || radio_address == 0xFF || radio_address == 0xE0) 
    {      
      if (!searchRadio()) {
        DPRINTF("Radio not found - retry count = "); DPRINTLN(retry_Count);
        //M5.Lcd.fillRect(15, 70, 319, 40, background_color);
        //M5.Lcd.setTextSize(2); // to Set the size of text from 0 to 255
        //M5.Lcd.setCursor(15, 80); //Set the location of the cursor to the coordinates X and Y
        //M5.Lcd.drawString("Searching for Radio %d" + String(retry_Count), 15, 80, 3);
        vTaskDelay(100);
        if (retry_Count++ > 4)
          break;
      } else {
        DPRINTF("Radio found at "); DPRINTLN(radio_address, HEX);
        //M5.Lcd.fillRect(15, 70, 319, 40, background_color);
        //M5.Lcd.setTextSize(2); // to Set the size of text from 0 to 255
        //M5.Lcd.setCursor(15, 80); //Set the location of the cursor to the coordinates X and Y
        //M5.Lcd.drawString("Radio Found at %X" + String(radio_address), 15, 80, 3);
        Serial.println();
        vTaskDelay(10);
        if (USBH_connected && !btConnected) 
          poll_radio_ptt = POLL_PTT_USBHOST;
        else
          poll_radio_ptt = POLL_PTT_DEFAULT;
        draw_new_screen();
      }
    }
  }
  return retry_Count;
}

// ----------------------------------------
//         Setup
// ----------------------------------------
void usbh_setup(void) {
  //Serial.begin(115200);
}

#ifdef BTCLASSIC
//
// Sets up the screen to show some connection state
//
void BT_Setup(void) 
{

#if defined(SSP2) && defined(BT)  // (passkey sharing)
  //SerialBT.setPin("1234"); // doesn't seem to change anything
  SerialBT.enableSSP();
  SerialBT.onConfirmRequest(BTConfirmRequestCallback);
  SerialBT.onAuthComplete(BTAuthCompleteCallback);
  //DiscoverAsync();  // get a list of radios with available serial ports
#endif

  draw_new_screen();
  //M5.Lcd.setTextSize(2);     // to Set the size of text from 0 to 255
  //M5.Lcd.setCursor(40, 80);  //Set the location of the cursor to the coordinates X and Y
  //M5.Lcd.drawString("Connecting to BT ...", 40, 80, 3);

  SerialBT.register_callback(callback);
  // Setup bluetooth as master:
  if (!SerialBT.begin(title, true))  // true = master  must be 16 characters exactly
  {
    DPRINTLNF("An error occurred initializing Bluetooth");
    //abort();
  } else {
    DPRINTLNF("Bluetooth initialized");
    DPRINTLNF("The device started, now you can pair it with bluetooth!  Use Pairing Reception menu on the radio");
  }

  btConnected = SerialBT.connect(bd_address, role);
  vTaskDelay(1000);  // give some time
  draw_new_screen();
  DPRINTF("btPaired = "); DPRINTLN(btPaired);
  DPRINTF("btConnected = "); DPRINTLN(btConnected);
 
  if (btConnected) {
    btPaired = true;
    //M5.Lcd.fillScreen(background_color);
    //M5.Lcd.fillRect(40, 70, 319, 40, background_color);
    //M5.Lcd.setTextColor(text_color);
    //M5.Lcd.setTextSize(2);            // to Set the size of text from 0 to 255
    //M5.Lcd.setCursor(40, 80);         //Set the location of the cursor to the coordinates X and Y
    //M5.Lcd.drawString("Connected & Paired to BT", 40, 80, 3);  // erase connecting
  } else
      DPRINTLNF("Pair to Radio");
  draw_new_screen();
}
#endif
// -----------------------------------------------------------------------
//         Poll tadio for PTT and when CIV Echo Back is off, frequency
// --------------------------------------------------------------------------
void poll_radio(void) 
{
  static uint32_t time_last_freq = millis();
  static uint32_t time_last_UTC = millis();
  static uint32_t time_last_ptt = millis();

  if (USBH_connected && btConnected)
    //DPRINTLNF("poll_radio: Both enabled, Switch to USB Host port");
    //DPRINTF("+");
  if (USBH_connected && !btConnected)
    //DPRINTLNF("poll_radio: Using USB Host port");
    ;//DPRINTF("U");
  if (!USBH_connected && btConnected)
    //DPRINTLNF("poll_radio: Using BT port");
    ;//DPRINTF("B");
  if (!USBH_connected && !btConnected) {
    //DPRINTLNF("poll_radio: No ports open, nothing to do");
    //DPRINTF("-");
    return;  // nothing to send to
  }
  uint32_t stack_sz;
  stack_sz = uxTaskGetStackHighWaterMark( NULL );
  if (stack_sz < 500)
    Serial.printf("\n  $$$$$$$$$$$$$$$  App Loop: Stack Size Low Space Warning < 500 words left free:  %lu\n",stack_sz);

  if (radio_address != 0x00 && radio_address != 0xFF && radio_address != 0xE0)
  {
    if (millis() >= time_last_freq + POLL_RADIO_FREQ)  // poll every X ms
    {
      sendCatRequest(CIV_C_F_READ, 0, 0);  // Get TX status
      vTaskDelay(2);
      processCatMessages();      
      time_last_freq = millis();
    }

    if (millis() >= time_last_ptt + poll_radio_ptt)  // poll every X ms
    {
      sendCatRequest(CIV_C_TX, 0, 0);  // Get TX status
      vTaskDelay(2);
      processCatMessages();
      time_last_ptt = millis();
    }

    if (millis() >= time_last_UTC + POLL_RADIO_UTC)  // poll every X ms
    {
      if (radio_address == IC905)                   //905
        sendCatRequest(CIV_C_UTC_READ_905, 0, 0);  //CMD_READ_FREQ);
      else if (radio_address == IC705)              // 705
        sendCatRequest(CIV_C_UTC_READ_705, 0, 0);  //CMD_READ_FREQ);
      vTaskDelay(2);
      processCatMessages();
      sendCatRequest(CIV_C_MY_POSIT_READ, 0, 0);  //CMD_READ_FREQ);
      vTaskDelay(2);
      processCatMessages();
      time_last_UTC = millis();
      
      // test for stack size
      uint32_t stack_sz;
      stack_sz = uxTaskGetStackHighWaterMark( NULL );
      if (stack_sz < 500)
        Serial.printf("\n  $$$$$$$$$$$$$$$  App Loop: Stack Size Low Space Warning < 500 words left free:  %lu\n",stack_sz);
    }
  }
}

//
//    formatVFO()
//
char *formatVFO(uint64_t vfo) 
{
  static char vfo_str[20] = { "" };
  //if (ModeOffset < -1 || ModeOffset > 1)
  //vfo += ModeOffset;  // Account for pitch offset when in CW mode, not others

  uint32_t MHz = (vfo / 1000000 % 1000000);
  uint16_t Hz = (vfo % 1000)/10;
  uint16_t KHz = ((vfo % 1000000) - Hz) / 1000;
  sprintf(vfo_str, "%lu.%03u.%02u", MHz, KHz, Hz);

  ///sprintf(vfo_str, "%-13s", "012345.123.123");  // 999GHZ max  47G = 47000.000.000
  ///DPRINT("New VFO: ");DPRINTLN(vfo_str);
  return vfo_str;
}

void draw_new_screen(void)
{
  int16_t x = 46;  // start position
  int16_t y = 16;
  int16_t x1 = 300;  // end of a line
  int16_t y1 = 10;
  int16_t h = 20;
  int16_t color = YELLOW;
  int16_t font_sz = 4;  // font size
  DPRINTLNF("+++++++++draw new screen");

  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_YELLOW, background_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.drawString("CI-V band Decoder", (int)(M5.Lcd.width()/2), y, font_sz);
  M5.Lcd.drawFastHLine(1,y+13,319,RED);   // separator below title
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.setTextColor(TFT_CYAN, background_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
  #if (defined (BT_CLASSIC) || defined (BLE)) && defined (USBHOST)
    M5.Lcd.drawString("BT Mode       Search       USB Mode", (int)(M5.Lcd.width()/2), 220, 2);
  #elif (defined (BT_CLASSIC) || defined (BLE)) && !defined (USBHOST)
    M5.Lcd.drawString("BT Mode       Search               ", (int)(M5.Lcd.width()/2), 220, 2);
  #elif defined (USBHOST)
    M5.Lcd.drawString("                Search       USB Mode", (int)(M5.Lcd.width()/2), 220, 2);
  #else 
    if (XVTR)
    M5.Lcd.drawString("                Search         XVTR  ", (int)(M5.Lcd.width()/2), 220, 2);
    else
    M5.Lcd.drawString("                Search               ", (int)(M5.Lcd.width()/2), 220, 2);
  #endif
  // write the Band and PTT icons
  display_Freq(frequency, true);
  display_PTT(PTT, true);
  display_Band(band, true);  // true means draw the icon regardless of state
  display_Xvtr(XVTR_enabled, true);
  display_Time(UTC, true);
  display_Grid(Grid_Square, true);
}

//  _UTC does nothing now but can be used to change a future clock label
void display_Time(uint8_t _UTC, bool _force)
{
  static uint32_t time_last_disp_UTC = millis();

  if ((millis() >= time_last_disp_UTC + POLL_RADIO_UTC) || _force)
  {
    char temp_t[15] = {};
    int x = 10;
    int x1 = 310;
    int y = 52;
    int font_sz = 4;

    M5.Lcd.setTextColor(LIGHTGREY, background_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535

    M5.Lcd.setTextDatum(ML_DATUM);  // x is left side
    sprintf(temp_t, "%02d/%02d/%02d", month(), day(), year());
    M5.Lcd.drawString("" + String(temp_t), x, y, font_sz);

    M5.Lcd.setTextDatum(MR_DATUM);  // x1 is right side 
    sprintf(temp_t, "%02d:%02d:%02d", hour(), minute(), second());
    M5.Lcd.drawString("" + String(temp_t), x1, y, font_sz);

    time_last_disp_UTC = millis();
  }
}

void display_Xvtr(bool _band, bool _force)
{
  static uint8_t _prev_band = 1;
  String Xvtr = "XV";
  int x = 260;
  int y = 150; 
  int x1 = x-33;  // upper left corner of outline box
  int y1 = y-18; 
  int font_sz = 4;   // font size
  int w = 38;  // box width 
  int h = 30;  // box height
  int r = 4;   // box radius corner size

  M5.Lcd.setTextDatum(MR_DATUM);
  
  if (_band != _prev_band || _force)
  {
    //DPRINTF("XVTR ON = "); DPRINTLN(_band);

    if (_band)
    {
      M5.Lcd.fillRoundRect(x1, y1, w, h, r, TFT_BLUE);
      M5.Lcd.setTextColor(WHITE); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
      M5.Lcd.drawString(Xvtr, x, y, font_sz);
    }
    else 
    {
      M5.Lcd.fillRoundRect(x1, y1, w, h, r, background_color);
      M5.Lcd.setTextColor(TFT_BLUE); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
      M5.Lcd.drawString(Xvtr, x, y, font_sz);
    }
    M5.Lcd.drawRoundRect(x1, y1, w, h, r, TFT_BLUE);

    _prev_band = _band;
  }
}

void display_PTT(uint8_t _PTT_state, bool _force)
{
  static uint8_t _prev_PTT_state = 1;
  String PTT_Tx = "TX";
  //String PTT_Rx = " Rx ";
  int x = 310;
  int y = 150; 
  int x1 = x-33;  // upper left corner of outline box
  int y1 = y-18; 
  int font_sz = 4;   // font size
  int w = 38;  // box width 
  int h = 30;  // box height
  int r = 4;   // box radius corner size

  M5.Lcd.setTextDatum(MR_DATUM);
  
  if (_PTT_state != _prev_PTT_state || _force)
  {
    #ifdef PRINT_PTT_TO_SERIAL
      Serial.print("*********************************************** PTT = ");Serial.println(_PTT_state);
    #endif
    
    if (_PTT_state)
    {
      M5.Lcd.fillRoundRect(x1, y1, w, h, r, RED);
      M5.Lcd.setTextColor(WHITE); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
      M5.Lcd.drawString(PTT_Tx, x, y, font_sz);
    }
    else 
    { 
      M5.Lcd.fillRoundRect(x1, y1, w, h, r, background_color);
      M5.Lcd.setTextColor(RED); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
      M5.Lcd.drawString(PTT_Tx, x, y, font_sz);
    }
    M5.Lcd.drawRoundRect(x1, y1, w, h, r, RED);

    _prev_PTT_state = _PTT_state;
  }
 }

void display_Freq(uint64_t _freq, bool _force)
{
  static uint64_t _prev_freq;
  int16_t x = 1;  // start position
  int16_t y = 104;
  int16_t color = TFT_WHITE;
  int16_t font_sz = 6;  // font size

  if ((_freq != _prev_freq && _freq != 0) || _force)
  {    
    //if (XVTR_enabled)
    //  _freq += bands[band].Xvtr_offset;
    
    #ifdef PRINT_VFO_TO_SERIAL
    Serial.printf("VFOA: %13sMHz - Band: %s\n", formatVFO(_freq), bands[band].band_name);
    #endif
    
    //M5.Lcd.fillRect(x, y, x1, y1, background_color);
    M5.Lcd.setTextDatum(MC_DATUM);     
    M5.Lcd.setTextColor(background_color, background_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
    M5.Lcd.drawString("" + String(formatVFO(_prev_freq)), (int)(M5.Lcd.width()/2), y, font_sz);
    
    M5.Lcd.setTextColor(color, background_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
    M5.Lcd.drawString("" + String(formatVFO(_freq)), (int)(M5.Lcd.width()/2), y, font_sz);
    
    _prev_freq = _freq;
  }
}

void display_Band(uint8_t _band, bool _force)
{
  static uint8_t _prev_band = 255;
  
  M5.Lcd.setTextDatum(ML_DATUM);

  if (_band != _prev_band || _force)
  {    
    int x = 8;
    int y = 150; 
    int font_sz = 4;

    // Update our outputs
    Band_Decode_Output(band);    
    //sendBand(band);   // change the IO pins to match band
    
    //Serial.printf("Band %s\n", bands[_band].band_name);
    
    M5.Lcd.setTextDatum(ML_DATUM); 
    M5.Lcd.setTextColor(background_color, background_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
    M5.Lcd.drawString("Band: " + String(bands[_prev_band].band_name), x, y, font_sz);
    M5.Lcd.setTextColor(TFT_CYAN); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
    M5.Lcd.drawString("Band: " + String(bands[_band].band_name), x, y, font_sz);
    _prev_band = _band;
  }
}

void display_Grid(char _grid[], bool _force)
{
  static char _last_grid[9] = {};
  // call to convert the strings for Lat and long fronm CIV to floats and then caluclate grid 
  if ((strcmp(_last_grid, _grid)) || _force)
  {    
    int x = 8;
    int y = 184;
    int font_sz = 4;

    //Serial.printf("Grid Square = %s\n",_grid);

    M5.Lcd.setTextDatum(ML_DATUM); 
    M5.Lcd.setTextColor(background_color, background_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
    M5.Lcd.drawString("" + String(_grid), x, y, font_sz);
    M5.Lcd.setTextColor(TFT_GREEN, background_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
    M5.Lcd.drawString("" + String(_grid), x, y, font_sz);
    strcpy(_last_grid, _grid);
  }
}

#ifdef BTCLASSIC
void restart_BT(void)
{
  if (btConnected) 
  {
    DPRINTLNF("restart_BT was called but we are already connected!");  
       return;
  }
  BT_enabled = true;
  DPRINTLNF("Btn A pressed -OR- restart called  ******************** BT Selected ******************************");
  frequency = 0;
  if (auto_address) radio_address = 0;
  BT_Setup();
  SerialBT.flush();
}
#endif 

void restart_USBH(void)
{
  if (!BT_enabled) 
  {
    DPRINTLNF("restart_USBH was called but we are already enabled");  
       return;
  }
  BT_enabled = false;
  DPRINTLNF("Btn C pressed -OR- restart called  -------------------- USB Selected ------------------------------");
  frequency = 0;
  if (auto_address) radio_address = 0;
  #ifdef BTCLASSIC
    SerialBT.disconnect();
    SerialBT.end();
  #endif
}

uint8_t chk_Buttons(void)
{ 
  static uint8_t xvtr_band_select = 0;  // rotate through a few transverter bands.  Temp until we get a select window

  M5.update();
  
  if (M5.BtnA.wasReleased())    //  M5.BtnA.pressedFor(1000, 200)) 
  {
    Serial.println("BtnA pressed - Switch to BT mode");
    #ifdef BTCLASSIC
      BT_enabled = true;  // allows operaor to turn on BT if BT feature is active
      restart_BT_flag = true;

      // restart_BT();
    #endif
    return 1;
  }
  
  if (M5.BtnB.wasReleased())    //  M5.BtnB.pressedFor(1000, 200)) 
  {
    radio_address = 0;
    Serial.println("BtnB pressed: Scan for new radio address");
    get_new_address_flag = true;
    //Get_Radio_address();
    return 1;
  }

  if (M5.BtnC.wasClicked() || M5.BtnC.wasReleased())    //M5.BtnC.pressedFor(1000, 200)) 
  {   // Since the first version won't have USB Host (unrelaible so far) reuse the button for a single Xvtr band for now
    #ifdef USBHOST
      Serial.println("BtnC pressed - Switch to USB Host mode");
      restart_USBH_flag = true;
    #else
      if (XVTR)  // Btn used for USB or Xvtr for now
      { 
        switch (xvtr_band_select)  // index our way through a curated list.
        { 
          case 0: XVTR_Band = BAND_1_25M; xvtr_band_select++; XVTR_enabled = true;  break;
          case 1: XVTR_Band = BAND_33cm;  xvtr_band_select++; XVTR_enabled = true;  break;
          case 2: XVTR_Band = BAND_23cm;  xvtr_band_select++; XVTR_enabled = true;  break;
          case 3: XVTR_Band = BAND_13cm;  xvtr_band_select++; XVTR_enabled = true;  break;
          default: XVTR_enabled = false; 
                    xvtr_band_select = 0; // rotate back to the bottom
        }        
        //sendCatRequest(CIV_C_F1_SEND, 0, 0);
        Serial.printf("BtnC pressed - Select a Xvtr band - index = %d   Xvtr_Band = %s   Xvtr_enabled %d\n", xvtr_band_select-1, bands[XVTR_Band].band_name, XVTR_enabled);
        //XVTR_enabled = !XVTR_enabled;  // toggle Xvtr mode
        //XVTR_Band = BAND_13cm;
        // call decoder here to change outputs accordinglyideally the normal band change shoudl pick it up though.
      }
    #endif

    return 1;
  }
  return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Called by main USBHost comms loop setup().
//  Call BT setup stuff when enabled.
//  Today only 1 of the intefaces is used to talk to the radio.
//  An optional PC connection is by the normal CPU USB port.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// called by main comms setup to start either USB or BT
void app_setup(void) 
{  
  Serial.printf("Begin App Setup, battery level = %d\n", M5.Power.getBatteryLevel());
  //M5.Power.setPowerVin(1);
  M5.Lcd.setBrightness(brightness);  // 0-255.  burns more power at full, but works in daylight decently
  //M5.Lcd.drawString(title, 5, 5, 4);

  Module_4in_8out_setup();   //Set up our IO modules comms on I2C

  #ifdef USBHOST
  int count_usb = 0;
  while (count_usb < 60 && USBHost_ready == 2)  // 0 = not mounted.  1 = mounted, 2 = system not initialized
  {
    count_usb++;
    delay(100);
    DPRINTF("Waiting for USB Initialization -  Retry count = "); DPRINT(count_usb); 
    DPRINTF("   USB mount status = "); DPRINTLN(USBHost_ready);
  }
  DPRINTF("USB mount status = "); DPRINTLN(USBHost_ready);
  #endif

  Module_4in_8out_Output_test(); 
  
  draw_new_screen();

  usbh_setup();  // Just talk normal USB serial

  #ifdef BTCLASSIC
  if (BT_enabled)
    restart_BT();
  else
  #endif
    restart_USBH();
  
  M5.update();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// called by main USBHost comms loop
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void app_loop(void) 
{ 
  static int32_t loop_time = 0;
  static int32_t loop_max_time = 0;
  int32_t loop_time_threshold = 20;
  static int32_t prev_loop_time = 0;
  uint8_t Band_temp;
  uint8_t PTT_temp;
  uint8_t decode_in;
  static uint32_t last_input_poll = 0;
  static uint8_t PTT_temp_last = 0;

  loop_time = millis();  // waternmark
  
  #ifdef PC_PASSTHROUGH 
  uint8_t buf[64];     
  // Serial -> SerialHost
  if (Serial.available()) {
    size_t count = Serial.read(buf, sizeof(buf));
    if (SerialHost && SerialHost.connected()) {
      SerialHost.write(buf, count);
      SerialHost.flush();
    }
  }
  #else  
    poll_radio();   // do nto sed stuff to radio when a PC app is doing the same
  #endif

  #ifdef BTCLASSIC
    bt_loop();  // handle all BT serial messaging in place of the USB host serial
  #endif

  #ifdef USBHOST
  if (restart_USBH_flag)
  {
    restart_USBH();
    restart_USBH_flag = false;
  }
  #endif

  #ifdef BTCLASSIC
  if (restart_BT_flag)
  {
    restart_BT();
    restart_BT_flag = false;
  }
  #endif
  
  #ifndef USBHOST
    chk_Buttons();
  #endif
  
  Get_Radio_address();  // can autodiscover CI-V address if not predefined.

  processCatMessages();  // look for delayed or unsolicited messages from radio
  
  if (millis() > last_input_poll + POLL_BAND_INPUT_LINES)
  {
    decode_in = Module_4in_8out_Input_scan();
    
    Band_temp = ~decode_in & 0x07;  // extract the lower 3 of 4 pins for band select.
    
    PTT_temp  = (~decode_in & 0x08) >> 3;  // extract the 4rh input to pass on PTT through slected bands IO pin.
    // update outputs on change of PTT state
    if (PTT_temp != PTT_temp_last)  // only call when the state changes
    {
      PTT_Output(band, PTT_temp);   // should add debounce but cpu in the IO module seems to be doing that already well enough
      PTT_temp_last = PTT_temp;
    }

    last_input_poll = millis();
    Serial.print(">>>> PTT = ");Serial.print(PTT_temp);Serial.print("   >>>> Band In = ");Serial.println(Band_temp, HEX);
  }
  // Inputs are in the middle of 2x 4.7K between 3.3V and GND.  1 is open, 0 is closed.
  if (0)  //(Band_temp != band)   // only do something if it is different
  {
    // translate input band pattern to band index then send to band Decode output
    switch (Band_temp)  // this could be BCD, parallel. For now assume 3 lines in paralle for 3 Xvtrs. Only have 4 inputs on module
    {
      case 1: band = BAND_1_25M; XVTR_enabled = true; break;
      case 2: band = BAND_33cm; XVTR_enabled = true; break;
      case 4: band = BAND_13cm; XVTR_enabled = true; break;
      default:  XVTR_enabled = false; break;
      Serial.printf("Decoder Band Input pattern = %d, Xvtr enabled = %d,  Band input = %d,  PTT Input = %d\n",decode_in, XVTR_enabled, Band_temp, PTT_temp);
    }
  }

  //if (frequency != 0)
  //{
    display_Time(UTC, false);
    display_Freq(frequency, false);
    display_PTT(PTT, false);
    display_Band(band, false);  // true means draw the icon regardless of state
    display_Xvtr(XVTR_enabled, false);
    display_Grid(Grid_Square, false);
  //}

  M5.update();

  // Measure our current and max loop times
  int32_t temp_time = millis() - loop_time;   // current loop duration

  if ((temp_time > loop_max_time) || temp_time > loop_time_threshold)
  {  
    if (temp_time > loop_max_time)
        loop_max_time = temp_time;
    //Serial.print("!");   // Turn on to see RTOS scheduling time allocated visually
    if (loop_max_time > loop_time_threshold)
    {
      Serial.printf("! loop time > %d  current time = %d  max time seen %d\n",loop_time_threshold, temp_time, loop_max_time); 
      //Serial.println(" App loop time > 500!");
      M5.Lcd.setTextDatum(ML_DATUM); 
      M5.Lcd.setTextColor(WHITE, background_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
      M5.Lcd.drawString("!", 0, 0, 2);
      if (loop_max_time > 3000 && SerialBT.isClosed() && SerialBT.isReady())
        restart_BT(); // try this as a USBHost lockup failover short of having the btn task 
    }                  //    delete and restart the app task, or even the USBHost task
  }
  else
  {
    M5.Lcd.setTextDatum(ML_DATUM);   // erase the marker
    M5.Lcd.setTextColor(background_color, background_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
    M5.Lcd.drawString("!", 0, 0, 2);
  }
}