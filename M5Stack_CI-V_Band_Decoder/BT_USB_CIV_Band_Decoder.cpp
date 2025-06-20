//
// BT_USB_CIV_Band_Decoder.cpp
//

#include <Update.h>
#include "FS.h"
#include <SPI.h>
#include <SD.h>
#include <stdint.h>
#include "Wire.h"
#include <BluetoothSerial.h>

#include "M5Stack_CI-V_Band_Decoder.h"
#include "CIV.h"
#include "BLE.h"
#include "Decoder.h"
#include "time.h"
#include "TimeLib.h"

//#define SEE_RAW_RX // see raw hex messages from radio
//#define SEE_RAW_TX // see raw hex messages from radio

//extern bool XVTR_enabled;
void UpdateFromFS(fs::FS &fs);
void printDirectory(File dir, int numTabs);
extern struct cmdList cmd_List[];
extern struct Modes_List modeList[];
uint8_t formatFreq(uint64_t vfo, uint8_t vfo_dec[]);
uint8_t getBand(uint64_t _freq);
extern void SendMessageBLE(uint8_t Message[], uint8_t len);
void write_bands_data(void);
void read_bands_data(void);
void refesh_display(void);
void band_Selector(uint8_t _band_input_pattern, bool ext_input);
void reply_to_PC(uint8_t cmd);

/*  copy of struct here from header file for easy reference.
struct Bands {
  char band_name[6];    // Freindly name or label.  Default here but can be changed by user.
  uint64_t edge_lower;  // band edge limits for TX and for when to change to next band when tuning up or down.
  uint64_t edge_upper;
  uint64_t Xvtr_offset;  // Offset to add to radio frequency.
                         // When all is correct, it will be within the band limits and allow PTT and Band decoder outputs
  uint64_t VFO_last;     // store the last used frequency on each band.
                         // for XVTR bands subtract the LO offset and send the result to the radio
  uint8_t mode_idx;      // current mode stored as index to the modeList table.
  uint8_t filt;          // current filt storeds in thr modeList table
  uint8_t datamode;
  uint8_t agc;            // store last agc.  Some radio/band/mode combos only have 1.
  uint8_t preamp;         // some bands there is no preamp (2.4G+ on 905).  Some radios/bands/modes combos have 1 preamp level, others have 2 levels.
  uint8_t atten;          // some bands there is no atten (some on 905).  Some radios/bands/mode combos have 1 atten level, others have more. 
  uint8_t split;          // Split mode on or off
  uint8_t rfpwr;          // RF Power set 0-255 range = 0-100%
  uint8_t InputMap;       // If input pattern matches this value, then select this band.  First match wins.
};
*/

// Put the LO (RF - IF = LO) value into the 4th column.
struct Bands bands[NUM_OF_BANDS] = {
  { "DUMMY", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF },                        // DUMMY Band to avoid using 0
  { "AM", 535000, 1705000, 0, 535000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BANDAM },                         // AM
  { "160M", 1800000, 2000000, 0, 1860000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND160M },                   // 160m
  { "80M", 3500000, 4000000, 0, 3573000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND80M },                     // 80m
  { "60M", 5351000, 5367000, 0, 5351000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND60M },                     // 60m
  { "40M", 7000000, 7300000, 0, 7074000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND40M },                     // 40m
  { "30M", 10100000, 10150000, 0, 10136000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND30M },                  // 30m
  { "20M", 14000000, 14350000, 0, 14074000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND20M },                  // 20m
  { "17M", 18068000, 18168000, 0, 18100000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND17M },                  // 17m
  { "15M", 21000000, 21450000, 0, 21074000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND15M },                  // 15m
  { "12M", 24890000, 24990000, 0, 24891500, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND12M },                  // 12m
  { "10M", 28000000, 29700000, 0, 28074000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND10M },                  // 10m
  { "6M", 50000000, 54000000, 0, 50125000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND6M },                    // 6m
  { "FM", 88000000, 108000000, 0, 95700000, 6, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BANDFM },                   // FM
  { "Air", 118000000, 137000000, 0, 119200000, 2, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BANDAIR },               // AIR
  { "2M", 144000000, 148000000, 0, 144200000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND144 },                // 2m
  { "1.25M", 222000000, 225000000, 194000000, 222100000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND222 },     // 222 with 28Mhz LO
  { "70cm", 430000000, 450000000, 0, 432100000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND432 },              // 430/440  No LO
  #ifdef XVBOX // for 705 transverter box, using 28Mhz IF for 903, 50Mhz for 1296, 28 for 222.  
    { "33cm", 902000000, 928000000, 874000000, 903100000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND902 },      // 902 with 28Mhz LO  
  #else  // typical usage 144 IF for 902, 144 IF for 1296
    { "33cm", 902000000, 928000000, 758000000, 903100000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND902 },      // 902  with 144Mhz LO
  #endif
  { "23cm", 1240000000, 1300000000, 1152000000, 1296100000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND1296 },  // 1296Mhz with 144Mhz LO
  { "13cm", 2300000000, 2450000000, 1870000000, 2304100000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND2400 },  // 2.3 and 2.4GHz
  { "9cm",  3400000000, 3410000000, 3256000000, 3400100000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND3300 },            // 3.3GHz
  { "6cm",  5650000000, 5925000000, 5328000000, 5760100000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND5760 },            // 5.7GHz
  { "3cm", 10000000000, 10500000000, 0, 10368100000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND10G },         // 10GHz
  { "24G", 24000000000, 24002000000, 0, 24031000000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND24G },         // 24GHz
  { "47G", 47000000000, 47002000000, 0, 47192100000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND47G },         // 47GHz
  { "76G", 76000000000, 76002000000, 0, 76000000000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND76G },         // 76GHz
  { "122G", 122000000000, 122002000000, 0, 122001000000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_BAND122G },     // 122GHz
  { "GENE", 0, 123000000000, 0, 432000000, 1, 1, 0, 1, 0, 0, 0, 25, DECODE_INPUT_B_GENERAL }                    // 0 to 122GHz
};

char title[17] = "CIV Band Decoder";  // make exactly 16 chards if used as the BT device name
uint16_t baud_rate;                   //Current baud speed
uint32_t readtimeout = 10;            //Serial port read timeout
//uint8_t read_buffer[2048];   //Read buffer
//extern uint64_t frequency;                 //Current frequency in Hz
uint8_t band = B_GENERAL;
uint32_t timer;
#ifndef  M5STAMPC3U
  uint16_t background_color = TFT_BLACK;
  uint16_t text_color = TFT_WHITE;
#endif
bool PTT = false;
bool prev_PTT = true;
extern char Grid_Square[];
bool BLE_buff_flag = false;
uint8_t radio_address = RADIO_ADDR;  //Transceiver address.  0 allows auto-detect on first messages form radio
bool auto_address = false;           // If true, detects new radio address on connection mode changes
                                     // If false, then the last used address, or the preset address is used.
                                     // If Search for Radio button pushed, then ignores this and looks for new address
                                     //   then follows rules above when switch connections
bool use_wired_PTT = WIRED_PTT;           // Selects source of PTT, wired input or polled state from radio.  Wired is preferred, faster.
bool XVTR = true;                    // Enables Xvtr support
  // Edit the bands table farther down the page to enter the fixed LO offset (in Hz) to add to radio dial
  // frequency for the transverter band of interest. Only 1 band supported at this point
uint8_t XVTR_Band = 0;      // Xvtr band to display - temp until a band select menu is built
uint8_t brightness = 110;   // 0-255
bool XVTR_enabled = false;  // true when a transverter feature is active
uint8_t read_buffer[64];  //Read buffer
uint8_t prev_band = 0xFF;
uint64_t prev_frequency = 0;
bool btConnected = false;
bool btPaired = false;
bool BLE_connected = false;
uint32_t temp_passkey;
uint16_t poll_radio_ptt = POLL_PTT_DEFAULT;  // can be changed with detected radio address.
static bool get_new_address_flag = false;
uint8_t UTC = 1;  // 0 local time, 1 UTC time
bool update_radio_settings_flag = false;
extern bool BtnA_pressed;
extern bool BtnB_pressed;
extern bool BtnC_pressed;
extern uint64_t frequency;
bool PC_to_Radio_Msg_Sent = false;
uint8_t last_PC_cmd = 255;
uint8_t Ext_Controller = 0x00;

// ######################################################################
// Enter the BD_ADDRESS of your IC-705. You can find it in the Bluetooth
// settings in section 'Bluetooth Device Information'
// Or better, create a config.ini file with the radio address on a line per this example
//    bd_address = 30:31:7D:33:BB:7F

//uint8_t bd_address[7] = { 0x30, 0x31, 0x7d, 0x33, 0xbb, 0x7f, 0x00 };  // Rick's 705
uint8_t bd_address[7] = { 0x30, 0x31, 0x7d, 0xBA, 0x44, 0xF9, 0x00 };  // Mike's 705
// ######################################################################

#ifdef M5STAMPC3U
  #ifdef INA226_I2C
    INA226 INA(0x40);    // Address on i2c bus is 0x40 by default.  
  #endif
  #ifdef SSD1306_OLED
    Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  #endif
#endif

#ifdef USBHOST
  // CDC Host object
  #include "M5_Max3421E_Usb.h"
  extern Adafruit_USBH_CDC SerialHost;
#endif

#ifdef EXT_IO2_UNIT
  #undef IO_MODULE   // only 1 at a time for now
#endif

#ifdef SSP
  bool confirmRequestPending = true;
#endif

// if no BT option chosen then set BT_enabled to 0;
#if !defined(BTCLASSIC) && !defined(BLE)
  bool BT_enabled = 0;  // configuration toggle between BT and USB - Leave this 0, must start on USB Hoset first, then can switch over.
#endif

#ifdef BTCLASSIC  // can set to BT on or off at startup
  #include "BluetoothSerial.h"
  bool BT_enabled = 1;  // configuration toggle between BT and USB - Leave this 0, must start on USB Hoset first, then can switch over.
  bool BLE_enabled = 0;
#endif

#ifdef BLE  // can set to BT on or off at startup
  //#include "BLE.h"
  bool BT_enabled = 0;  // configuration toggle between BT and USB - Leave this 0, must start on USB Hoset first, then can switch over.
  bool BLE_enabled = 1;
  extern void BLE_loop(void);
  extern void BLE_Setup(void);
  extern void Scan_BLE_Servers(void);
#endif

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

void reply_to_PC(uint8_t cmd)
{
  uint8_t send_freq[15] = {0};
  uint8_t vfo_dec[7] = {0};
  uint8_t f_len;
  uint8_t len;
  
  len = formatFreq(frequency, vfo_dec);  // Convert to BCD string

  if (cmd == 0x25)
  {
    send_freq[0] = 0xFE;
    send_freq[1] = 0xFE;
    send_freq[2] = Ext_Controller; //read_buffer[2];
    send_freq[3] = radio_address;
    send_freq[4] = cmd;
    send_freq[5] = read_buffer[5];
    send_freq[6] = vfo_dec[0];
    send_freq[7] = vfo_dec[1];
    send_freq[8] = vfo_dec[2];
    send_freq[9] = vfo_dec[3];
    send_freq[10] = vfo_dec[4];
    // Adjust for freq value length of 5bytes, or 6bytes for IC905
    if (radio_address == IC905 && frequency >= 10000000000) {   // > 1GHz is 6 bytes
      send_freq[11] = vfo_dec[5];
      send_freq[12] = 0xFD;
      send_freq[13] = 0x00;
      f_len = 13;
    }
    else {
      send_freq[11] = 0xFD;
      send_freq[12] = 0x00;
      f_len = 12;
    }   
  } else if (cmd == 0xFA || cmd == 0xFB) {  // translate controller address from 0xE5 to external user
    send_freq[0] = 0xFE;
    send_freq[1] = 0xFE;
    send_freq[2] = Ext_Controller; //read_buffer[2];
    send_freq[3] = radio_address;
    send_freq[4] = cmd;
    send_freq[5] = 0xFD;
    send_freq[6] = 0x00;
    f_len = 6;
    Serial.write(send_freq, f_len);  // send Accepted message back to radio
  } else { // 00, 03. 05
    send_freq[0] = 0xFE;
    send_freq[1] = 0xFE;
    if (read_buffer[4] == 0x00)
      send_freq[2] = 0x00;
    else
      send_freq[2] = Ext_Controller; //read_buffer[2];
    send_freq[3] = radio_address;
    send_freq[4] = cmd;
    send_freq[5] = vfo_dec[0];
    send_freq[6] = vfo_dec[1];
    send_freq[7] = vfo_dec[2];
    send_freq[8] = vfo_dec[3];
    send_freq[9] = vfo_dec[4];
    if (radio_address == IC905) {
      send_freq[10] = vfo_dec[5];
      send_freq[11] = 0xFD;
      send_freq[12] = 0x00;
      f_len = 12;
    }
    else {
      send_freq[10] = 0xFD;
      send_freq[11] = 0x00;
      f_len = 11;
    } 
  }
  // copy in frequency with xvtr offset appied, if any      
  //memcpy(&send_freq[5], vfo_dec, len);
  Serial.write(send_freq, f_len);
}

// ----------------------------------------
//    Read incoming line from bluetooth
// ----------------------------------------
uint8_t readLine(void) {
  uint8_t byte;
  uint8_t counter = 0;
  uint32_t ed = readtimeout;  // not initialized!

  if (BT_enabled) {
    #ifdef BTCLASSIC
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
    #endif
  } else if (BLE_connected && BLE_buff_flag) {
    for (int i = 0; i < sizeof(read_buffer); i++) {
      byte = read_buffer[i];
      if (byte == 0xFF) continue;  //TODO skip to start byte instead

      read_buffer[counter++] = byte;
      if (STOP_BYTE == byte) break;
      if (counter >= sizeof(read_buffer)) return 0;
    }
    BLE_buff_flag = false;  // reset and allow new data to arrive
    //DPRINTF("readLine: BLE read buffer length:"); DPRINTLN(counter);
  } else {
    #ifdef USBHOST
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
    #endif
  }  // usb host
  //DPRINTF("readLine: read buffer"); DPRINT((char) read_buffer); 
  //DPRINTF("readLine: read buffer length:"); DPRINTLN(counter);
  
  #ifdef PC_PASSTHROUGH
    // ****************************************************
    // SerialHost or SerialBT --> ***  PC side Serial  ***
    // ****************************************************
    // pass on to PC radio messages directed to other controllers than us
    uint8_t cmd = read_buffer[4];
    // ensure it is a full valid formatted CI-V command
    if (counter && read_buffer[0] == 0xFE && read_buffer[1] == 0xFE && read_buffer[counter-1] == 0xFD)
    {
      #ifndef SKIP_XVTR_FREQ_XLATE
      // cmd 0x05 changes freq, returns FB/FA.  03 and 25 return frequency.  00 is unsolicited frequency change from radio (turnging dial)
      if (XVTR_enabled && (cmd== 0x00 || (PC_to_Radio_Msg_Sent && last_PC_cmd == cmd && (cmd == 0x03 || cmd == 0x25))))
      {
        uint64_t ff;
        // convert the radio reported frequency CI-V message to transverter frequency 
        if (read_buffer[2] != CONTROLLER_ADDRESS) {
          reply_to_PC(cmd);
          PC_to_Radio_Msg_Sent = false;   // reset flag, allow local polling
          
          #ifdef DBG_TO_LCD
            M5.Lcd.setCursor(1,180);
            M5.Lcd.setTextSize(2);
            M5.Lcd.setTextColor(TFT_BLACK, TFT_BLACK);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
            M5.Lcd.printf("%02X %011llu", 0, frequency);
            M5.Lcd.setCursor(1,180);
            M5.Lcd.setTextColor(TFT_BLACK, TFT_YELLOW);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
            M5.Lcd.printf("%02X %011llu", read_buffer[4], frequency);
          #endif
        }
      }
      else  // not frequency related
      #else
      if (read_buffer[2] != CONTROLLER_ADDRESS)
      #endif
      {  
        if (PC_to_Radio_Msg_Sent && (cmd == 0x00 || cmd == 0xFB || cmd == 0xFA || cmd == last_PC_cmd)) {
          Serial.write(read_buffer, counter);
          PC_to_Radio_Msg_Sent = false;   // reset flag, allow local polling
          #ifdef DBG_TO_LCD
            M5.Lcd.setCursor(1,200);
            M5.Lcd.setTextSize(2);
            M5.Lcd.setTextColor(TFT_BLACK, TFT_BLACK);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
            M5.Lcd.printf("%02X", 0);
            M5.Lcd.setCursor(1,200);
            M5.Lcd.setTextColor(TFT_BLACK, TFT_WHITE);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
            M5.Lcd.printf("%02X", read_buffer[4]);
          #endif
        }
      }
      processCatMessages(); // To reduce polling needs, use the info other controllers are getting.
      Serial.flush();
      #ifdef DBG_TO_LCD
        M5.Lcd.setCursor(1,220);
        M5.Lcd.setTextSize(2);
        M5.Lcd.setTextColor(TFT_BLACK, TFT_BLACK);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
        M5.Lcd.printf("%02X", 0);
        M5.Lcd.setCursor(1,220);
        M5.Lcd.setTextColor(TFT_BLACK, TFT_CYAN);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
        M5.Lcd.printf("%02X", read_buffer[4]);
      #endif
    }
  #endif
 
  return counter;
}

// ----------------------------------------
//    Read incoming line from PC and write to radio
// ----------------------------------------
uint8_t pass_PC_to_radio(void) {
  uint8_t counter = 0;

  #ifdef PC_PASSTHROUGH
    uint8_t byte;
    uint8_t data_len;
    uint32_t ed = readtimeout;  // not initialized!
    static uint8_t r_buffer[64];  //Read buffer
    uint8_t send_freq[15] = {0};
    uint8_t vfo_dec[7] = {0};
    uint8_t f_len;

    while (1) {
      while (!Serial.available()) {
        if (--ed == 0) return 0;  // leave the loop if BT connection is lost
      }
      ed = readtimeout;
      byte = Serial.read();
      if (byte == 0xFF) continue;  //TODO skip to start byte instead

      r_buffer[counter++] = byte;
      if (STOP_BYTE == byte) break;

      if (counter >= sizeof(r_buffer)) return 0;
    }

    if (1) 
    {
      //if ( && r_buffer[2] == radio_address && (r_buffer[0] == 0xFE && r_buffer[1] == 0xFE)) {
      uint64_t f;
      uint64_t mul;
      uint8_t k;
      bool pass_on = true;
      static uint8_t band_last = 255;

      #ifndef SKIP_XVTR_FREQ_XLATE
        uint8_t cmd = r_buffer[4];
        // if inbound frequency change command then translate it if it is for a transverter band
        if (STOP_BYTE == byte && counter > 10 && r_buffer[2] == radio_address && r_buffer[0] == 0xFE && r_buffer[1] == 0xFE && (cmd == 0x05 || cmd == 0x25))
        {
          mul = 1;
          f = 0;
          k = 0;
          Ext_Controller = r_buffer[3];  // capure requesting contgroller ID.  Could be several.

          if (cmd == 0x25)
            k=1; // data is 1 byte further down the msg - byte 5 is VFO , 0 is A, 1 is B
          
          data_len = counter - 6+k;   // subtract total msg length bytes for constant bytes fe fe e0 ac cmd xxxxxx fd  
          
          for (uint8_t i = 5+k; i < 5+k + data_len; i++) {
            if (r_buffer[i] == 0xFD) continue;  //spike
            f += (r_buffer[i] & 0x0F) * mul; mul *= 10;  // * decMulti[i * 2 + 1];
            f += (r_buffer[i] >> 4) * mul; mul *= 10;  //  * decMulti[i * 2];
          }
          
          band = getBand(f);
          frequency = f;

          // If band has changed, last known values will be restored.
          // If the band is a XVTR band, then the XVTR_enabled is set and frequency offsets are applied 
          
          if (band_last != band) {
            band_Selector(bands[band].InputMap, true);
            band_last = band;
          }

          if (XVTR_enabled) {
            pass_on = false; // IF a Xvtr bnd then translate 
            
            // pass on frequency.  If it is for a Xvtr band SetFreq() will add the offset
            switch (cmd) {
                case 0x25: if (r_buffer[5] == 0x00)
                              SetFreq(frequency, CIV_C_F25A_SEND);   // vfo A 
                            else
                              SetFreq(frequency, CIV_C_F25B_SEND);  // vfo B
                            break;
                case 0x05: SetFreq(frequency, CIV_C_F1_SEND);  // send 0x05
                            break;
            }
          }

          #ifdef DBG_TO_LCD
            M5.Lcd.setCursor(1,140);
            M5.Lcd.setTextSize(2);
            M5.Lcd.setTextColor(TFT_BLACK, TFT_BLACK);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
            M5.Lcd.printf("%02X %011llu", 0, f);
            M5.Lcd.setCursor(1,140);
            M5.Lcd.setTextColor(TFT_WHITE, TFT_BLUE);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
            M5.Lcd.printf("%02X %011llu", last_PC_cmd, f);
          #endif
        }
      #endif

      if (pass_on)  // pass on to radio without any frequency manipulations
      {
        #if defined ( BTCLASSIC )
          if (btConnected && SerialBT.connected()) {
            SerialBT.write(r_buffer, counter);    
            SerialBT.flush();
          }
        #elif defined ( BLE )
          if (BLE_connected) {
            SendMessage(r_buffer, counter);
          }
        #endif
        //Serial.flush();
        
        #ifdef DBG_TO_LCD
          M5.Lcd.setCursor(1,160);
          M5.Lcd.setTextSize(2);
          M5.Lcd.setTextColor(TFT_BLACK, TFT_BLACK);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
          M5.Lcd.printf("%02X %011llu", 0, frequency);
          M5.Lcd.setCursor(1,160);
          M5.Lcd.setTextColor(TFT_BLACK, TFT_GREEN);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
          M5.Lcd.printf("%02X %011llu", last_PC_cmd, frequency);
        #endif
      }

      PC_to_Radio_Msg_Sent = true;
      last_PC_cmd = r_buffer[4];  // record the cmd byte sent to the radio for the PC.  
                                  // Used to ensure only the reply is sent back, not other stuff, or out of order
      //processCatMessages();  // this will extract stuff like mode, and for freqency will correct for Xvtr Offset if XVTR is enabled.
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
uint8_t getBand(uint64_t _freq) {
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

  msg_len += 3;  // Tee up to add data if any

  uint8_t j = 0;
  if (Data_len != 0) {              // copy in 1 or more data bytes, if any
    for (j = 0; j < Data_len; j++)  //pick up with value i
      req[msg_len++] = Data[j];
  }

  req[msg_len] = STOP_BYTE;
  req[msg_len + 1] = 0;  // null terminate for printing or conversion to String type

  //#define SEE_RAW_TX  // an also be set at top of file
  #ifdef SEE_RAW_TX
  DPRINTF("--> Tx Raw Msg: ");
  for (uint8_t k = 0; k <= msg_len; k++) {
    DPRINT(req[k], HEX);
    DPRINTF(",");
  }
  DPRINTF(" msg_len = ");
  DPRINT(msg_len + 1);
  DPRINTLNF(" END");
  #endif

  //#define RAWT  // for a more detailed look

  //if (btConnected && !SerialBT.isClosed() && SerialBT.connected())
  if (USBH_connected || BLE_connected || btConnected) {
    if (msg_len < sizeof(req) - 1) {  // ensure our data is not longer than our buffer
  #ifdef SEE_RAWT
      DPRINTF("Send CI-V Msg: ");
  #endif

  #if defined(BLE)
      SendMessageBLE(req, msg_len + 1);
  #else
      for (uint8_t i = 0; i <= msg_len; i++) {
  #ifdef SEE_RAWT
        DPRINT(req[i], HEX);
  #endif

  #if defined(USBHOST)
        if (!SerialHost.write(req[i]))
          DPRINTLNF("sendCatRequest: Tx: error");
  #elif defined(BTCLASSIC)
        if (!SerialBT.write(req[i]))
          DPRINTLNF("sendCatRequest: Tx: error");
  #endif

  #ifdef SEE_RAWT
        DPRINTF(",");
  #endif
      }
  #endif

  #ifdef SEE_RAWT
      DPRINTF(" END TX MSG, msg_len = ");
      DPRINTLN(msg_len);
  #endif

    } else {
      DPRINTLNF("sendCatRequest: Buffer overflow");
    }  // if overflow
  }    // if connected
}

// ----------------------------------------
//      Print the received frequency
//
//  This function is only called when the radio's reports a new frequency so is always the radio's real
//    frequency, which when in Xvtr mode, the CI-V message value is the IF frequency.
//
//  The value 'frequency' is a global and is the final displayed frequency, Xvtr band or not.
//    - On entry to this function, frequency is the global displayed value, may not be the actual radio (IF) frequency
//      which is yet to be extracted from the CI-V messaage.
//    - On exit from this function, frequency is either updated (non-XVtr bands) or Xvtr_offset applied, if any.
//
// ----------------------------------------
void read_Frequency(uint64_t freq, uint8_t data_len) {  // This is the displayed frequency, before the radio input, which may have offset applied
  if (frequency > 0) {                   // store frequency per band before it maybe changes bands.  Required to change IF and restore direct after use as an IF.
    //Serial.printf("read_Frequency: Last Freq %-13llu\n", frequency);
    if (!update_radio_settings_flag) {   // wait until any XVTR transition complete
      bands[band].VFO_last = frequency;  // store Xvtr or non-Xvtr band displayed frequency per band before it changes.
      prev_band = band;                  // store associated band index
    }
  }  // if an Xvtr band, subtract the offset to get radio (IF) frequency

  // Could do more validation here. Freq Calculation moved to CIV.cpp
  frequency = freq;

  if (XVTR_enabled)
    frequency += bands[XVTR_Band].Xvtr_offset;

  band = getBand(frequency);
  //if (!update_radio_settings_flag) {  //  ignore changes from radio initiated from gpio changes
  //  DPRINTF("Band Change from Radio for "); DPRINTLN(bands[band].band_name);
  //  band_Selector(band, true);
  //}
  
  //Serial.printf("read_Frequency: Freq %-13llu  band =  %d  Xvtr_Offset = %llu  datalen = %d   btConnected %d   USBH_connected %d   BT_enabled %d   BLE_connected %d  radio_address %X\n", frequency, band, bands[XVTR_Band].Xvtr_offset, data_len, btConnected, USBH_connected, BT_enabled, BLE_connected, radio_address);
  // On exit from this function we have a new displayed frequency that has XVTR_Offset added, if any.
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
  uint8_t data[50] = {};

  if (1) {
    bool knowncommand = true;
    int i;
    int msg_len;

    cmd_num = 255;
    match = 0;

    if ((msg_len = readLine()) > 0) {

//#define SEE_RAW_RX
#ifdef SEE_RAW_RX
      Serial.print(F("<++ Rx Raw Msg: "));
      for (uint8_t k = 0; k < msg_len; k++) {
        Serial.print(read_buffer[k], HEX);
        Serial.print(F(","));
      }
      Serial.print(F(" msg_len = "));
      Serial.print(msg_len);
      Serial.println(F(" END"));
      Serial.flush();
#endif
      if (read_buffer[0] == START_BYTE && read_buffer[1] == START_BYTE) {
        if (read_buffer[3] == radio_address) {
          //if (read_buffer[2] == CONTROLLER_ADDRESS || read_buffer[2] == BROADCAST_ADDRESS) {
          if ((read_buffer[2] >= 0xE0 && read_buffer[2] <=0xEF) || read_buffer[2] == BROADCAST_ADDRESS) {
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

#ifdef BTCLASSIC
  void bt_loop(void) {
    if (BT_enabled && !btConnected) {
      draw_new_screen();
      DPRINTF("BT Loop - Using bd_address:");  // print default bd_address
      for (int z = 0; z < 6; z++)
        DPRINT(bd_address[z], HEX);
      DPRINTLNF("");
      M5.Lcd.setTextColor(text_color, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
      M5.Lcd.drawString("Connecting to BT ...", 80, 100, 4);

      btConnected = SerialBT.connect(bd_address, role);
      if (btConnected) {
        DPRINTLNF("BT Transceiver reconnected");
        prev_band = 255;
        frequency = 0;  // cause the screen to refresh
        //break;
      } else
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
  if (get_new_address_flag == true) {
    Serial.print("Getting new address.  radio address is now ");
    Serial.println(radio_address, HEX);
  }

  if ((BT_enabled && btConnected) || (USBH_connected && !BT_enabled) || BLE_connected)  // handle both USB and BT
  {
    get_new_address_flag = false;

    while (radio_address == 0x00 || radio_address == 0xFF || (radio_address >= 0xE0 && radio_address <= 0xEF)) {
      if (!searchRadio()) {
        DPRINTF("Radio not found - retry count = ");
        DPRINTLN(retry_Count);
        //M5.Lcd.fillRect(15, 70, 319, 40, background_color);
        //M5.Lcd.setTextSize(2); // to Set the size of text from 0 to 255
        //M5.Lcd.setCursor(15, 80); //Set the location of the cursor to the coordinates X and Y
        //M5.Lcd.drawString("Searching for Radio %d" + String(retry_Count), 15, 80, 3);
        vTaskDelay(100);
        if (retry_Count++ > 4)
          break;
      } else {
        DPRINTF("Radio found at ");
        DPRINTLN(radio_address, HEX);
        //M5.Lcd.fillRect(15, 70, 319, 40, background_color);
        //M5.Lcd.setTextSize(2); // to Set the size of text from 0 to 255
        //M5.Lcd.setCursor(15, 80); //Set the location of the cursor to the coordinates X and Y
        //M5.Lcd.drawString("Radio Found at %X" + String(radio_address), 15, 80, 3);
        Serial.println();
        vTaskDelay(10);
        if (USBH_connected)
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
  void BT_Setup(void) {
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
    DPRINTF("btPaired = ");
    DPRINTLN(btPaired);
    DPRINTF("btConnected = ");
    DPRINTLN(btConnected);

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
//         Poll radio for PTT, frequency, and other modes.  BLE in particular only gets what and when you ask for it (so far)
// --------------------------------------------------------------------------
void poll_radio(void) {
  static uint32_t time_last_freq = millis();
  static uint32_t time_last_UTC = millis();
  static uint32_t time_last_ptt = millis();
  static uint32_t time_last_agc = millis();
  static uint32_t time_last_mode = millis();
  static uint32_t time_last_attn = millis();
  static uint32_t time_last_pre = millis();
  static uint32_t time_last_split = millis();
  static uint32_t time_last_rfpwr = millis();
  
  if (!PC_to_Radio_Msg_Sent && radio_address != 0x00 && radio_address != 0xFF && radio_address != CONTROLLER_ADDRESS) {
  
    if (millis() >= time_last_freq + POLL_RADIO_FREQ)  // poll every X ms
    {
      //DPRINTLN("poll freq");
      sendCatRequest(CIV_C_F_READ, 0, 0);  // Get current VFO
      vTaskDelay(2);
      processCatMessages();
      time_last_freq = millis();
    }

    if (millis() >= time_last_ptt + poll_radio_ptt)  // poll every X ms if not wired PTT
    {
      sendCatRequest(CIV_C_TX, 0, 0);  // Get TX status
      vTaskDelay(2);
      processCatMessages();
      time_last_ptt = millis();
    }

    if (millis() >= time_last_mode + POLL_RADIO_MODE)  // poll every X ms
    {
      sendCatRequest(CIV_C_F26A, 0, 0);  // Get mode, filter, and datamode status
      vTaskDelay(2);
      processCatMessages();
      time_last_mode = millis();
    }

    if (millis() >= time_last_agc + POLL_RADIO_AGC)  // poll every X ms
    {
      sendCatRequest(CIV_C_AGC_READ, 0, 0);  // Get mode, filter, and datamode status
      vTaskDelay(2);
      processCatMessages();
      time_last_agc = millis();
    }

    if (millis() >= time_last_attn + POLL_RADIO_ATTN)  // poll every X ms
    {
      sendCatRequest(CIV_C_ATTN_READ, 0, 0);  // Get mode, filter, and datamode status
      vTaskDelay(2);
      processCatMessages();
      time_last_attn = millis();
    }

    if (millis() >= time_last_pre + POLL_RADIO_PRE)  // poll every X ms
    {
      sendCatRequest(CIV_C_PREAMP_READ, 0, 0);  // Get mode, filter, and datamode status
      vTaskDelay(2);
      processCatMessages();
      time_last_pre = millis();
    }

    if (millis() >= time_last_split + POLL_RADIO_SPLIT)  // poll every X ms
    {
      sendCatRequest(CIV_C_SPLIT_READ, 0, 0);  // Get mode, filter, and datamode status
      vTaskDelay(2);
      processCatMessages();
      time_last_split = millis();
    }

    if (millis() >= time_last_rfpwr + POLL_RADIO_RFPWR)  // poll every X ms
    {
      sendCatRequest(CIV_C_RFPOWER, 0, 0);  // Get RF power level, each Xvtr and real band can be different
      vTaskDelay(2);
      processCatMessages();
      time_last_rfpwr = millis();
    }

    if (millis() >= time_last_UTC + POLL_RADIO_UTC)  // poll every X ms
    {
      if (radio_address == IC905)                  //905
        sendCatRequest(CIV_C_UTC_READ_905, 0, 0);  //CMD_READ_FREQ);
      else if (radio_address == IC705)             // 705
        sendCatRequest(CIV_C_UTC_READ_705, 0, 0);  //CMD_READ_FREQ);
      vTaskDelay(2);
      processCatMessages();
      sendCatRequest(CIV_C_MY_POSIT_READ, 0, 0);  //CMD_READ_FREQ);
      vTaskDelay(2);
      processCatMessages();
      time_last_UTC = millis();

      // test for stack size
      uint32_t stack_sz;
      stack_sz = uxTaskGetStackHighWaterMark(NULL);
      if (stack_sz < 500)
        Serial.printf("\n  $$$$$$$$$$$$$$$  App Loop: Stack Size Low Space Warning < 500 words left free:  %lu\n", stack_sz);
    }
  }
}

#ifdef SD_CARD

  char line_buffer[80] = { 0 };

  void printLineN(uint16_t lineNumber) {
    File myFile = SD.open("/config.ini", FILE_READ);  // Open the file "/hello.txt" in read mode.
    myFile.seek(0);
    char cr;

    for (uint16_t i = 0; i < (lineNumber - 1);) {
      cr = myFile.read();
      if (cr == '\n') {
        i++;
      }
    }

    Serial.print("printLineN: ");
    //Now we are at the right line
    while (true) {
      cr = myFile.read();
      Serial.write(cr);
      if (cr == '\n') {
        break;
      }
    }
    //a for loop with a read limit might be a good idea
  }

  // returns the value part of a string found with "desired_name"
  // Returns NULL if nothing valid found
  char *read_string(char *line_buffer, char const *desired_name) {
    static char name[64];
    static char val[64];
    char seperator[2];

    Serial.print(F("\nread_string: Line Buffer: "));
    Serial.println(line_buffer);
    uint8_t i = 0;

    while (sscanf(line_buffer, "%s %s %127[^\n]*%*s", name, seperator, val) != 0) {
      if (0 == strcmp(name, desired_name)) {
        Serial.print(F("read_string2: Name:"));
        Serial.print(name);
        Serial.print(F("   seperator:"));
        Serial.print(seperator);
        Serial.print(F("   val:"));
        Serial.println(val);

        return strdup(val);
      }
    }
    return NULL;
  }

  void write_bands_data(void) {
    if (SD.exists("/bands.dat")) {
      SD.remove("/bands.dat");  // delete old file and replace with new stuff
      DPRINTLNF("write_bands_data: Deleted old bands table file on SD card");
    }
    File myFile = SD.open("/bands.dat", FILE_WRITE);  // Create if needed and open the file for write
    if (myFile) {
      DPRINTLNF("write_bands_data: Writing bands table to SD card");
      // Write bands data table to the SD car
      myFile.write((uint8_t *)&bands, sizeof(bands));
    }
    myFile.close();
    DPRINTLNF("write_bands_data: Done writing file");
  }

  void read_bands_data(void) {
    Serial.println("read_bands_data: Opening /bands.data file for read");
    File myFile = SD.open("/bands.dat", FILE_READ);
    if (myFile) {
      myFile.read((uint8_t *)&bands, sizeof(bands) / sizeof(uint8_t));
      Serial.println("read_bands_data: Done reading file");
      myFile.close();
    } else {
      Serial.println("read_bands_data: Failed to open /bands.data file for read");
    }
  }

  uint16_t read_SD_Card(void) {
    char cr;
    uint16_t cnt = 0;
    uint8_t i = 0;
    char *line_buffer_temp;
    char line_buffer[80] = { 0 };
    char *token;
    char *rest = line_buffer;
    char address_temp[22] = { 0 };
    uint8_t bd_address_temp[7] = { 0 };
    uint8_t j = 0;
    uint8_t k;

    File myFile = SD.open("/config.ini", FILE_READ);  // Open the file in read mode.
    if (myFile) {
      Serial.println(F("config.ini Content:"));
      // Read the data from the file and print it until the reading is complete.
      while (myFile.available()) {

        //Serial.write(myFile.read());
        cr = myFile.read();
        line_buffer[i++] = cr;

        if (cr == '\n')  // Got a full line
        {
          cnt++;  // count the total lines in the file
          line_buffer[i - 1] = '\0';
          Serial.println(line_buffer);  // Got 1 whole line. print it out for a look-see

          if (line_buffer[0] != ';' && isAlphaNumeric(line_buffer[0])) {  // skip comment and non-content lines
            //Serial.print(F("Before match function: line Number: ")); Serial.print(cnt); Serial.print(F("  line buffer is ")); Serial.print(line_buffer);

            line_buffer_temp = read_string(line_buffer, "bd_address");

            int l = strlen(line_buffer_temp);
            Serial.print("Number digits read in for bd_address - len:");
            Serial.println(l);
            if (l != 18) {
              Serial.print("Wrong number of digits for bd_address - len:");
              Serial.println(l);
              break;
            }

            if (line_buffer_temp != NULL) {
              Serial.print(F("Found address line:"));
              Serial.println(line_buffer_temp);
              strcpy(address_temp, line_buffer_temp);

              // replace : with \0
              for (uint8_t k = 0; k < 18;) {
                //Serial.print("  addr_temp zero cnt:"); Serial.print(k); Serial.print("  val:"); Serial.println(address_temp[k]);
                if (address_temp[k] == ':') {
                  address_temp[k] = '\0';
                  //Serial.print("  addr_temp zero cnt sub:"); Serial.println(k);
                }
                address_temp[17] = '\0';
                k++;
              }

              //Serial.print("\n address_temp ascii byte: ");  Serial.println(address_temp);

              // convert each pair of the 6 acsii hex numbers to a number.
              for (k = 0; k < 16; k += 3) {
                uint8_t addr = (uint8_t)hexToDec((String)&address_temp[k]);
                bd_address_temp[j++] = addr;
                //Serial.print("   hex addr byte:0x");  Serial.print(addr, HEX);
                //Serial.print("   hex addr array byte:0x");  Serial.println(bd_address_temp[j-1], HEX);
              }
              bd_address_temp[6] = '\0';

              Serial.print(F(" Default bd_address:"));  // print default bd_address
              for (int z = 0; z < 6; z++)
                Serial.print(bd_address[z], HEX);
              Serial.println("");

              int y = (memcmp(bd_address, bd_address_temp, 6));  //  look to see if they are the same or not
              if (y != 0) {                                      // 0 is the same
                memcpy(bd_address, bd_address_temp, 6);          // valid address uee it.
                Serial.println("Updated bd_address from config.ini");
              } else {
                Serial.println("bd_address is the same as default");
              }
              //Serial.print(F("\nbd_address index:"));  Serial.println(j,DEC);
              break;
            } else {
              Serial.print(F("No Match Found line:"));
              Serial.println(cnt);
            }
          } else {
            Serial.print(F("Commented or invalid line:"));
            Serial.println(cnt);
          }
          i = 0;
        }
      }
      myFile.close();

      Serial.printf("number of lines in config.ini file %d\n", cnt);
      return cnt;
    } else {
      Serial.println("error opening config.ini");  // If the file is not open.
      return 0;
    }
    return cnt;
  }

  void performUpdate(Stream &updateSource, size_t updateSize) {
    if (Update.begin(updateSize)) {
      size_t written = Update.writeStream(updateSource);
      if (written == updateSize) {
        M5.Lcd.println("Written : " + String(written) + " successfully");
      } else {
        M5.Lcd.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
      }
      if (Update.end()) {
        M5.Lcd.println("OTA done!");
        if (Update.isFinished()) {
          M5.Lcd.println("Update successfully completed. Rebooting.");
        } else {
          M5.Lcd.println("Update not finished? Something went wrong!");
        }
      } else {
        M5.Lcd.println("Error Occurred. Error #: " + String(Update.getError()));
      }
    } else {
      M5.Lcd.println("Not enough space to begin OTA");
    }
  }

  void UpdateFromFS(fs::FS &fs) {
    File updateBin = fs.open("/update.bin");
    Serial.println("Update file has size " + String(updateBin.size()));
    delay(1000);

    if (updateBin) {
      if (updateBin.isDirectory()) {
        M5.Lcd.drawString("Error, update.bin is not a file", 1, 1, 2);
        updateBin.close();
        return;
      }

      size_t updateSize = updateBin.size();

      if (updateSize > 0) {
        M5.Lcd.drawString("Try to start update", 1, 20, 2);
        performUpdate(updateBin, updateSize);
      } else {
        M5.Lcd.drawString("Error, file is empty", 1, 20, 2);
      }

      updateBin.close();

      // whe finished remove the binary from sd card to indicate end of the process
      fs.remove("/update.bin");
    } else {
      M5.Lcd.drawString("Could not load update.bin from sd root", 1, 40, 2);
    }
  }

  void Update_from_SD() {
    if (!SD.begin()) {
      Serial.println("No SD Card");
    } else {
      Serial.println("SD Card found, try update");
      UpdateFromFS(SD);
    }
    Serial.println("Yay, SD Update worked!");
  }


  void printDirectory(File dir, int numTabs) {
    while (true) {
      File entry = dir.openNextFile();
      if (!entry) {
        // no more files
        break;
      }
      for (uint8_t i = 0; i < numTabs; i++) {
        Serial.print('\t');
      }
      Serial.print(entry.name());
      if (entry.isDirectory()) {
        Serial.println("/");
        printDirectory(entry, numTabs + 1);
      } else {
        // files have sizes, directories do not
        Serial.print("\t\t");
        Serial.println(entry.size(), DEC);
      }
      entry.close();
    }
  }
#endif

//
//    formatVFO()
//
char *formatVFO(uint64_t vfo) {
  static char vfo_str[20] = { "" };
  //if (ModeOffset < -1 || ModeOffset > 1)
  //vfo += ModeOffset;  // Account for pitch offset when in CW mode, not others

  uint32_t MHz = (vfo / 1000000 % 1000000);
  uint16_t Hz = (vfo % 1000) / 10;
  uint16_t KHz = ((vfo % 1000000) - Hz) / 1000;
  if (board_type == M5ATOMS3) {
    sprintf(vfo_str, "%lu.%03u.%01u", MHz, KHz, Hz/10);
  } else {
    sprintf(vfo_str, "%lu.%03u.%02u", MHz, KHz, Hz);
  }
  ///sprintf(vfo_str, "%-13s", "012345.123.123");  // 999GHZ max  47G = 47000.000.000
  ///DPRINT("New VFO: ");DPRINTLN(vfo_str);
  return vfo_str;
}

void draw_new_screen(void) {
  int16_t x = 46;  // start position
  int16_t y = 16;
  int16_t w = 319;  // end of a line
  int16_t y1 = y + 13;
  //int16_t h = 20;
  int16_t font_sz = 4;  // font size
  //DPRINTLNF("+++++++++draw new screen");

  #ifndef  M5STAMPC3U
    if (board_type == M5ATOMS3) {
      font_sz = 3;  // downsize for Atom
      x = 46;  // start position
      y = 6;
      w = 128;  // end of a line
      y1 = 14;   // height of line
      //h = 12;
    }
    int16_t color = TFT_YELLOW;
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_YELLOW, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
    M5.Lcd.setTextDatum(MC_DATUM);
    //M5.Lcd.drawString("CI-V band Decoder", (int)(M5.Lcd.width() / 2), y, font_sz);
    M5.Lcd.drawString(title, (int)(M5.Lcd.width() / 2), y, font_sz);
    M5.Lcd.drawFastHLine(1, y1, w, TFT_RED);  // separator below title

    if (board_type != M5ATOMS3) {
      M5.Lcd.setTextDatum(MC_DATUM);
      M5.Lcd.setTextColor(TFT_CYAN, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
    #if (defined(BT_CLASSIC) || defined(BLE)) && defined(USBHOST)
      M5.Lcd.drawString("BT Mode       Search       USB Mode", (int)(M5.Lcd.width() / 2), 220, 2);
    #elif (defined(BT_CLASSIC) || defined(BLE)) && !defined(USBHOST)
      M5.Lcd.drawString("                Search          XVTR ", (int)(M5.Lcd.width() / 2), 220, 2);
    #elif defined(USBHOST)
      M5.Lcd.drawString("                Search       USB Mode", (int)(M5.Lcd.width() / 2), 220, 2);
    #elif defined (XVBOX)
      M5.Lcd.drawString(" 222             903             1296 ", (int)(M5.Lcd.width() / 2), 220, 2);
    #else
      if (XVTR)
        M5.Lcd.drawString("                Search         XVTR ", (int)(M5.Lcd.width() / 2), 220, 2);
      else
        M5.Lcd.drawString("                Search              ", (int)(M5.Lcd.width() / 2), 220, 2);
    #endif
    }
  #endif // M5STAMPC3U

  // write the Band and PTT icons
  display_Freq(frequency, true);
  display_PTT(PTT, true);
  display_Band(band, true);  // true means draw the icon regardless of state
  display_Xvtr(XVTR_enabled, true);
  display_Time(UTC, true);
  display_Grid(Grid_Square, true);
}

//  _UTC does nothing now but can be used to change a future clock label
void display_Time(uint8_t _UTC, bool _force) {
  static uint32_t time_last_disp_UTC = millis();

  if ((millis() >= time_last_disp_UTC + POLL_RADIO_UTC) || _force) {
    char temp_t[15] = {};
    int x = 10;
    int x1 = 310;
    int y = 52;
    int font_sz = 4;

    #ifndef M5STAMPC3U
      M5.Lcd.setTextColor(TFT_LIGHTGREY, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
      if (board_type == M5ATOMS3) {
        font_sz = 3;  // downsize for Atom
        x = 1;
        x1 = 127;
        y = 25;
        font_sz = 3;
      }
      M5.Lcd.setTextDatum(ML_DATUM);  // x is left side
      sprintf(temp_t, "%02d/%02d/%02d", month(), day(), year());
      M5.Lcd.drawString(temp_t, x, y, font_sz);
      M5.Lcd.setTextDatum(MR_DATUM);  // x1 is right side
      sprintf(temp_t, "%02d:%02d:%02d", hour(), minute(), second());
      M5.Lcd.drawString(temp_t, x1, y, font_sz);
    #endif

    time_last_disp_UTC = millis();
  }
}

void display_Xvtr(bool _band, bool _force) {
  static uint8_t _prev_band = 1;
  char Xvtr[3] = "XV";
  int x = 260;
  int y = 150;
  int x1 = x - 33;  // upper left corner of outline box
  int y1 = y - 18;
  int font_sz = 4;  // font size
  int w = 38;       // box width
  int h = 30;       // box height
  int r = 4;        // box radius corner size

  #ifndef M5STAMPC3U
    M5.Lcd.setTextDatum(MR_DATUM);
  #endif

  if (_band != _prev_band || _force) {
    //DPRINTF("XVTR ON = "); DPRINTLN(_band);
    
    #ifndef M5STAMPC3U
      if (board_type == M5ATOMS3) {
        font_sz = 3;  // downsize for Atom
        x = 99;
        y = 118;
        x1 = x-16;
        y1 = y-8; 
        w = 20;
        h = 14;
        r = 4;
      }

      if (_band) {
        M5.Lcd.fillRoundRect(x1, y1, w, h, r, TFT_BLUE);
        M5.Lcd.setTextColor(TFT_WHITE);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
        M5.Lcd.drawString(Xvtr, x, y, font_sz);
      } else {
        M5.Lcd.fillRoundRect(x1, y1, w, h, r, background_color);
        M5.Lcd.setTextColor(TFT_BLUE);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
        M5.Lcd.drawString(Xvtr, x, y, font_sz);
      }
      M5.Lcd.drawRoundRect(x1, y1, w, h, r, TFT_BLUE);
    #endif // M5STAMPC3U
    
    _prev_band = _band;
  }
}

void display_PTT(bool _PTT_state, bool _force) {
  static bool _prev_PTT_state = true;
  char PTT_Tx[3] = "TX";
  int x = 310;
  int y = 150;
  int x1 = x - 33;  // upper left corner of outline box
  int y1 = y - 18;
  int font_sz = 4;  // font size
  int w = 38;       // box width
  int h = 30;       // box height
  int r = 4;        // box radius corner size

  #ifndef M5STAMPC3U
    M5.Lcd.setTextDatum(MR_DATUM);
    if (board_type == M5ATOMS3) {
      font_sz = 3;  // downsize for Atom
      x = 124;
      y = 118;
      x1 = x-16;
      y1 = y-8; 
      w = 20;
      h = 14;
      r = 4;
    }
    
    if (_PTT_state != _prev_PTT_state || _force) {
      #ifdef PRINT_PTT_TO_SERIAL
        Serial.print(F("*********************************************** PTT = "));
        Serial.println(_PTT_state);
      #endif
      if (_PTT_state) {
        M5.Lcd.fillRoundRect(x1, y1, w, h, r, TFT_RED);
        M5.Lcd.setTextColor(TFT_WHITE);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
        M5.Lcd.drawString(PTT_Tx, x, y, font_sz);
      } else {
        M5.Lcd.fillRoundRect(x1, y1, w, h, r, background_color);
        M5.Lcd.setTextColor(TFT_RED);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
        M5.Lcd.drawString(PTT_Tx, x, y, font_sz);
      }
      M5.Lcd.drawRoundRect(x1, y1, w, h, r, TFT_RED);
      _prev_PTT_state = _PTT_state;
    }
  #endif
}

void display_Freq(uint64_t _freq, bool _force) {
  static uint64_t _prev_freq;
  int16_t x = 1;  // start position
  int16_t y = 104;
  #ifdef DBG_TO_LCD
    int16_t font_sz = 3;  // font size
  #else
    int16_t font_sz = 6;
  #endif
  
  if ((_freq != _prev_freq && _freq != 0) || _force) {
    #ifdef PRINT_VFO_TO_SERIAL
      Serial.printf("VFOA: %13sMHz - Band: %s  Mode: %s  DataMode: %s  Filter: %s  Source: BLE %d, USBHost %d, BTClassic %d\n", formatVFO(_freq), bands[band].band_name, \
          modeList[bands[band].mode_idx].mode_label, ModeStr[bands[band].datamode], FilStr[bands[band].filt], BLE_connected, USBH_connected, btConnected);
    #endif
    if (board_type == M5ATOMS3) {
      font_sz = 4;  // downsize for Atom
      y = 54;
    }

    #ifndef M5STAMPC3U
      int16_t color = TFT_WHITE;
      //M5.Lcd.fillRect(x, y, x1, y1, background_color);
      M5.Lcd.setTextDatum(MC_DATUM);
      M5.Lcd.setTextColor(background_color, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
      M5.Lcd.drawString(formatVFO(_prev_freq), (int)(M5.Lcd.width() / 2), y, font_sz);
      M5.Lcd.setTextColor(color, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
      M5.Lcd.drawString(formatVFO(_freq), (int)(M5.Lcd.width() / 2), y, font_sz);
    #endif // M5STAMPC3U
    _prev_freq = _freq;
  }
}

void display_Band(uint8_t _band, bool _force) {
  static uint8_t _prev_band = 255;
  int x = 8;
  int y = 150;
  int font_sz = 4;

  if (_band != _prev_band || _force) {
    // Update our outputs
    Band_Decode_Output(band, true);
    //Serial.printf("Band %s\n", bands[_band].band_name);

    #ifndef M5STAMPC3U
      if (board_type == M5ATOMS3) {
        font_sz = 4;  // downsize for Atom
        x = 1;
        y= 118;
      }
      #ifndef DBG_TO_LCD
        M5.Lcd.setTextDatum(ML_DATUM);
        M5.Lcd.setTextColor(background_color, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
        M5.Lcd.drawString(bands[_prev_band].band_name, x, y, font_sz);
        M5.Lcd.setTextColor(TFT_CYAN);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
        M5.Lcd.drawString(bands[_band].band_name, x, y, font_sz);
      #endif
    #endif

    if (frequency != 0) {
      if (!update_radio_settings_flag)
        #ifdef SD_CARD
          write_bands_data();  // save on band changes.  Other times would be good bu this catches the most.
        #else
        ; // nothing
        #endif
      else                   // by this time we should be stable after XVTR transitions
        ;//update_radio_settings_flag = false;
    }
    _prev_band = _band;
  }
}

void display_Grid(char _grid[], bool _force) {
  static char _last_grid[9] = {};
  int x = 8;
  int y = 184;
  int font_sz = 4;

  // call to convert the strings for Lat and long fronm CIV to floats and then caluclate grid
  if ((strcmp(_last_grid, _grid)) || _force) {
    //Serial.printf("Grid Square = %s\n",_grid);

    #ifndef M5STAMPC3U
      #ifndef DBG_TO_LCD   
        if (board_type == M5ATOMS3) {
          font_sz = 4;  // downsize for Atom
          y = 86;
          M5.Lcd.setTextDatum(MC_DATUM);
          M5.Lcd.setTextColor(background_color, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
          M5.Lcd.drawString(_last_grid, (int)(M5.Lcd.width() / 2), y, font_sz);
          M5.Lcd.setTextColor(TFT_DARKGREEN, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
          M5.Lcd.drawString(_grid, (int)(M5.Lcd.width() / 2), y, font_sz);
        } else {
          M5.Lcd.setTextDatum(ML_DATUM);
          M5.Lcd.setTextColor(background_color, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
          M5.Lcd.drawString(_last_grid, x, y, font_sz);
          M5.Lcd.setTextColor(TFT_GREEN, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
          M5.Lcd.drawString(_grid, x, y, font_sz);
        }
      #endif
    #endif   
    strcpy(_last_grid, _grid);
  }
}

#ifdef BTCLASSIC
void restart_BT(void) {
  if (btConnected) {
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

void restart_USBH(void) {
  if (!BT_enabled) {
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

void band_Selector(uint8_t _band_input_pattern, bool ext_input) {
  static uint8_t _band_input_pattern_last = 0;
  static bool XVTR_enabled_last;
  static uint8_t XVTR_band_before;
  uint8_t _input_band;
  bool change_band = false;
  static uint8_t last_band = 255;

  // if PC or radio side frequncy change then go to the band.  If wired or button, then use input pattern
  if (ext_input && !update_radio_settings_flag) {  // skip if freq change a result of wired input change
    if (band != last_band) {
      _input_band = band;
      change_band = true;
      last_band = band;
      DPRINTF("Band Selector External Input = "); DPRINTLN(bands[band].band_name);
    }
  }    
  else if (_band_input_pattern != _band_input_pattern_last)  // only do something if it is different
  {
    if (!XVTR_enabled_last && !XVTR_enabled) {  // capture last non-Xvtr band in use
      XVTR_band_before = band;                  // record the non-xvtr band before initial XVTR mode enabled.
    }
    DPRINTF("Band Selector Button, or Wired/Polled input = "); DPRINTLN(_band_input_pattern);
    for (_input_band = 0; _input_band < NUM_OF_BANDS; _input_band++) {
      //DPRINTF("Input search index = "); DPRINTLN(_input_band);
      //DPRINTF("Band Map Value = "); DPRINT(bands[_input_band].InputMap);
      //DPRINTF("    Input Band Value = "); DPRINTLN(_input_band);
      if (bands[_input_band].InputMap == _band_input_pattern) {
        DPRINTF("Band Selector MATCH = "); DPRINTLN(bands[_input_band].InputMap);         
        _band_input_pattern_last = _band_input_pattern;
        change_band = true;
        DPRINTF("Band Selector Source (wired or polled) Input Pattern = "); DPRINTLN(_band_input_pattern); 
        break;   // we have a match, use this as the target band
      }
    }
  }

  if (change_band) {
    DPRINTLNF("Band Selector Dummy Band 0 for break before make effect");
    PTT_Output(DUMMY, false);  // send PTT OFF with current band before changing to new band.
    // On the 8In/4Out Digital IO module, the inputs are in the middle of 2x 4.7K between 3.3V and GND.  1 is open, 0 is closed.
    // translate input band pattern to band index then send to band Decode output
  
    if (bands[_input_band].Xvtr_offset != 0) {  // Xvtr band
      DPRINTF("Xvtr Band Detected = "); DPRINTLN(bands[_input_band].band_name);
      XVTR_Band = _input_band;
      XVTR_enabled = true;
      #ifdef M5STAMPC3U
        band = _input_band;
      #endif
    } else {  // Not a Xvtr band
      DPRINTF("Not a Xvtr Band = "); DPRINTLN(bands[_input_band].band_name);
      band = _input_band;
      XVTR_Band = 0;
      XVTR_enabled = false;   // No match, do nothing
    }
  } else {
    //DPRINTF("Band Selector NO MATCH = "); DPRINTLN(bands[_input_band].InputMap);
    XVTR_enabled = false;   // No match, do nothing
  }

  if (change_band) {
    update_radio_settings_flag = true;
    
    DPRINTF("   Xvtr enabled = "); DPRINT(XVTR_enabled);
    DPRINTF("   Band = "); DPRINT(band);
    DPRINTF("   Xvtr Band = "); DPRINTLN(XVTR_Band);
    draw_new_screen();  // clears the update flag
    PTT_Output(band, false);
    
    // Band and Frequency are not yet changed.  Set split to off on band changes else weird things happen.  
    // Split will be reset after change is stabilized
    #ifndef M5STAMPC3U
      // If changing to a XVTR band, or a different one, update VFO to the last used on that band.   We only get band changes here, never the same band.
      if (XVTR_enabled) {  // set VFO and other values to last used for the target XVTR band
        SetSplit(XVTR_Band, true);  // force split off 
        if (ext_input)
          if (read_buffer[5] == 0x00)
            SetFreq(frequency, CIV_C_F25A_SEND);
          else
            SetFreq(frequency, CIV_C_F25B_SEND);
        else
          SetFreq(bands[XVTR_Band].VFO_last, CIV_C_F25A_SEND);  // This value always has Xvtr offset applied    
        vTaskDelay(10);
        processCatMessages();
        SetMode(XVTR_Band);
        vTaskDelay(10);
        SetPre(XVTR_Band);
        vTaskDelay(10);
        SetAttn(XVTR_Band);
        vTaskDelay(10);
        SetRFPwr(XVTR_Band);
        vTaskDelay(10);
        //SetSplit(XVTR_Band, 0);
        //vTaskDelay(10);
        SetAGC(XVTR_Band);
        vTaskDelay(10);
      }
      else
      // Restore the IF to last used values
      //if ((ext_input && !XVTR_enabled) || (XVTR_enabled_last && !XVTR_enabled)) {  // This will have Xvtr offset = 0
        // band is still Xvtr band until the radio actually changes frequency
      {

        if (ext_input)
          XVTR_band_before = band;  // if sent from a PC then use that band, not the last non-Xvtr band
        SetSplit(XVTR_band_before, true);  // force split off 
        //if (!ext_input)
        SetFreq(bands[XVTR_band_before].VFO_last, CIV_C_F25A_SEND);  // set radio to that last non-XVTR band used.
        vTaskDelay(10);
        processCatMessages();
        SetMode(XVTR_band_before);
        vTaskDelay(10);
        SetPre(XVTR_band_before);
        vTaskDelay(10);
        SetAttn(XVTR_band_before);
        vTaskDelay(10);
        SetRFPwr(XVTR_band_before);
        vTaskDelay(10);
        //SetSplit(XVTR_band_before, 0);
        //vTaskDelay(10);
        SetAGC(XVTR_band_before);
        vTaskDelay(10);
      }

      if (update_radio_settings_flag == true) {
        vTaskDelay(300);  // Give some time for the radio to change bands
        #ifdef BTCLASSIC
          if (btConnected) SerialBT.flush();
        #endif
        #ifdef BLE
            //if (BLE_connected) xxxxx.flush();
        #endif
        #ifdef USBHOST
          if (USBH_connected) SerialHost.flush();
        #endif
      }
      
      #ifdef SD_Card
        write_bands_data();                         // capture all data to SD before XVTR transition
      #endif
      update_radio_settings_flag = false;
    #endif

    // now the band and freq should be updated
    DPRINTF("Current Band = ");DPRINT(bands[band].band_name);
    DPRINTF("   Last VFO = "); DPRINT(bands[band].VFO_last);
    DPRINTF("   Last Xvtr VFO = "); DPRINT(bands[XVTR_Band].VFO_last);
    DPRINTF("   Band Input Pattern = "); DPRINT(_band_input_pattern, HEX);
    DPRINTF("   PTT = "); DPRINTLN(PTT);
    
    XVTR_enabled_last = XVTR_enabled;
  }
}

// Length is 5 or 6 depending if < 10GHz band  followed by 5 or 6 BCD encoded frequency bytes
// vfo_dec[] holds the frequency result to send out
// returns length;
uint8_t formatFreq(uint64_t vfo, uint8_t vfo_dec[]) {
  //static uint8_t vfo_dec[7] = {};  // hold 6 or 7 bytes (length + 5 or 6 for frequency, bcd encoded bytes)
  uint8_t len;

  if (vfo < 10000000000LL) {
    len = 5;
    vfo_dec[6] = (uint8_t)0x00;  // set to 0, unused < 10Ghz
    for (uint8_t i = 0; i < len; i++) {
      uint64_t x = vfo % 100;
      vfo_dec[i] = bcdByteEncode(static_cast<uint8_t>(x));
      vfo = vfo / 100;
    }
    //Serial.printf(" VFO: < 10G Bands = Reversed hex to DEC byte %02X %02X %02X %02X %02X %02X\n", vfo_dec[0], vfo_dec[1], vfo_dec[2], vfo_dec[3], vfo_dec[4], vfo_dec[5]);
  } else {
    len = 6;
    for (uint8_t i = 0; i < len; i++) {
      uint64_t x = vfo % 100;
      vfo_dec[i] = bcdByteEncode(static_cast<uint8_t>(x));
      vfo = vfo / 100;
    }
    //Serial.printf(" VFO: > 10G Bands = Reversed hex to DEC byte %02X %02X %02Xpass_PC_to_radio %02X %02X %02X %02X\n", vfo_dec[0], vfo_dec[1], vfo_dec[2], vfo_dec[3], vfo_dec[4], vfo_dec[5], vfo_dec[6]);
  }
  return len;  // 5 or 6
}

// Send new frequency to radio, radio will change bands as needed.
// ToDo:  Radio mode and other settings are not touched so stay the same as the last band used.  We are only changing the frequency, nothing else.
//        Need to save mode, filter and other stuff to return each band to the last way it was used.
void SetFreq(uint64_t Freq, uint8_t cmd) {
  uint8_t vfo_dec[7] = {};

  if (XVTR_enabled) {
    Freq -= bands[XVTR_Band].Xvtr_offset;
    //Serial.printf("SetFreq: Xvtr Offset applied - IF freq %llu  band %s  offset %llu\n", Freq, bands[XVTR_Band].band_name, bands[XVTR_Band].Xvtr_offset);
  }

  uint8_t len = formatFreq(Freq, vfo_dec);  // Convert to BCD string
  //Serial.printf("SetFreq: Radio Freq = %llu  To radio (5 or 6 bytes) in BCD: %02X %02X %02X %02X %02X (%02X)\n", Freq, vfo_dec[0], vfo_dec[1], vfo_dec[2], vfo_dec[3], vfo_dec[4], vfo_dec[5]);
  //#ifndef PC_PASSTHROUGH
    sendCatRequest(cmd, vfo_dec, len);
  //#endif
}

void refesh_display(void) {
  display_Time(UTC, false);
  display_Freq(frequency, false);
  display_PTT(PTT, false);
  display_Band(band, false);  // true means draw the icon regardless of state
  display_Xvtr(XVTR_enabled, false);
  display_Grid(Grid_Square, false);
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
void app_setup(void) {
  //Serial.printf("Begin App Setup, battery level = %d\n", M5.Power.getBatteryLevel());
  //M5.Power.setPowerVin(1);
  PC_to_Radio_Msg_Sent = false;
  #ifndef M5STAMPC3U
    M5.Lcd.setBrightness(brightness);  // 0-255.  burns more power at full, but works in daylight decently
    //M5.Lcd.drawString(title, 5, 5, 4);
  #endif

  #ifdef USBHOST
    int count_usb = 0;
    while (count_usb < 60 && USBHost_ready == 2)  // 0 = not mounted.  1 = mounted, 2 = system not initialized
    {
      count_usb++;
      delay(100);
      DPRINTF("Waiting for USB Initialization -  Retry count = ");
      DPRINT(count_usb);
      DPRINTF("   USB mount status = ");
      DPRINTLN(USBHost_ready);
    }
    DPRINTF("USB mount status = ");
    DPRINTLN(USBHost_ready);
    USBHost_ready = 1;
    USBH_connected = 1;
  #endif

  #if defined ( SD_CARD )
    DPRINTLNF("Looking for SD Card to try update and read config");
    if (SD.begin(SD_SPI_CS_PIN, SPI, SPI_FREQ)) {
      if (SD.exists("/config.ini")) {
        File root = SD.open("/");
        printDirectory(root, 0);  // look what is on the SD card
        root.close();
        #ifdef CLEAN_SD_DB_FILE
          write_bands_data();   // used when the data structure has changed, force an overwrite with data from memory
        #endif
        UpdateFromFS(SD);                 // if there is an update, do it.  Otherwise read config file.
        read_bands_data();                // overwrite default bands table with last known values saved on SD card
        uint16_t lines = read_SD_Card();  // get line count
        Serial.printf("Setup: config.ini line count is %d\n", lines);
        //printLineN(lines);
      } else {
        Serial.println(F("Setup: cannot open config.ini"));
      }
    } else {
      Serial.println(F("Setup: Cannot start SD card subsystem"));
    }
  #else
    Serial.println(F("Setup: SD Card feature not enabled"));
  #endif

    Serial.print(F("Updated bd_address:"));
    for (int z = 0; z < 6; z++)
      Serial.print(bd_address[z], HEX);
    Serial.println("");

  #ifdef IO_MODULE
    //Module_4in_8out_Output_test();
    //vTaskDelay(500);
    Module_4in_8out_setup();  //Set up our IO modules comms on I2C
  #endif
  
  #ifdef XVBOX_PLCC
    Core_CPU_IO_Setup();  // Set up the PLCC module for 3 Band + 1 PTT out and 1 PTT in
  #endif

  #ifdef MODULE_4RELAY_13_2
    Module_4_Relay_setup();   // Setup the stacking 4 channel relay module on i2c bus
  #endif

  #ifdef RELAY4_UNIT
    Unit_RELAY4_setup();
  #endif

  #ifdef EXT_IO2_UNIT
    Unit_EXTIO2_setup();
  #endif
  
  #ifdef M5STAMPC3U // For 705 Xvtr box controller
    vTaskDelay(100); 
    CPU_C3U_IO_Setup();
    vTaskDelay(100); 
    uint8_t inp = M5STAMPC3U_Input_scan();
    DPRINTF("CPU Input IO Setup Complete - Pattern = ");  DPRINTLN(inp);
    vTaskDelay(100); 
    inp = M5STAMPC3U_Input_scan();
    DPRINTF("CPU Input IO Setup Complete - Pattern 2 = ");  DPRINTLN(inp);
    
    MCP23017_IO_setup();
    DPRINTLNF("MCP23017 IO Expansion Modules Setup Complete");

    Band_Decode_Output(DUMMY, true);
    GPIO_PTT_Out(DECODE_DUMMY_PTT, true);   //initialize the PTT states.  Required since the ports are not all zero in RX but mixed state
    vTaskDelay(700); 
    
    // ESP32 native method to get CPU temp
    //ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor_config, &temp_handle));
    // Enable temperature sensor
    //ESP_ERROR_CHECK(temperature_sensor_enable(temp_handle));
    // Get converted sensor data
    // Disable the temperature sensor if it is not needed and save the power
    //ESP_ERROR_CHECK(temperature_sensor_disable(temp_handle));

    #ifdef INA226_I2C
      uint8_t lctr = 0;
      while (lctr < 4)
      {
        uint8_t r = INA.begin();
        lctr++;
        if (r) 
          break;
        Serial.println("**Error: Failed to connect to INA226 on i2c bus");
        delay(50);
      }
      
      INA.setMaxCurrentShunt(8.2, 0.00836);  // Solders about a 3/8" piece of resistor lead across the 10 ohm shunt resistor supplied in my INA226 board
      // Did initial calculation which camne out at 0.0082ohms then tweaked the value to calibrate it to match my 5-digit Fluke DVM current reading.
      // Also connected the IN+ pin to the VBUS pin to masure voltage.   
      // This board is installed in series with the 12V front panel power switch as a high side current measurement.
    #endif  // INA226
    
    #ifdef SSD1306_OLED
      Serial.println(F("Start SSD1306 OLED display Init"));
      // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
      if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
      }
      // Show initial display buffer contents on the screen --
      // the library initializes this with an Adafruit splash screen.
      display.display();
      delay(500);
      // Clear the buffer
      display.clearDisplay();
      // Draw a single pixel in white
      display.drawPixel(10, 10, SSD1306_WHITE);
      // Show the display buffer on the screen. You MUST call display() after
      // drawing commands to make them visible on screen!
      display.display();
    #endif // SSD1306_OLED
  #endif  // M5STAMPC3U
  
  draw_new_screen();

  usbh_setup();  // Just talk normal USB serial

  #ifdef BLE
    BLE_Setup();
    Scan_BLE_Servers();
  #endif

  #ifdef BTCLASSIC
    if (BT_enabled)
      restart_BT();
  //else
  #endif

  #ifndef M5STAMPC3U 
    poll_radio();  // do not send stuff to radio when a PC app is doing the same
  #endif

  DPRINTLNF("***Turn OFF scope data from radio");
  sendCatRequest(CIV_C_SCOPE_OFF, 0, 0);  // Turn Off scope data in case it is still on
  vTaskDelay(10);
  processCatMessages();

  // restart_USBH();
}

#ifdef M5STAMPC3U 
  #ifdef SSD1306_OLED
    void draw_PTT_icon(bool Tx_On) {
      display.setCursor(5, 4);
      display.setTextSize(1); // Draw 3X-scale text
      display.cp437(true);         // Use full 256 char 'Code Page 437' font
      if (Tx_On) {
        display.fillCircle(7, 7, 7, SSD1306_WHITE); // x, y, r, color
        display.setTextColor(SSD1306_BLACK);
        display.print("T");
      }
      else { 
        display.fillCircle(7, 7, 7, SSD1306_BLACK); // x, y, r, color
        display.drawCircle(7, 7, 7, SSD1306_WHITE); // x, y, r, color
        display.setTextColor(SSD1306_WHITE);
        display.print("R");
      } 
      display.display();
    }

    void draw_Xvtr_icon(bool active) {
      display.setCursor(5, 22);
      display.setTextSize(1); // Draw 3X-scale text
      display.cp437(true);         // Use full 256 char 'Code Page 437' font
      if (active) {
        display.fillRoundRect(0, 18, 14, 14, 3, SSD1306_WHITE);  // x0, y0, w, h, r, color  
        display.setTextColor(SSD1306_BLACK);
      }
      else
      {
        display.fillRoundRect(0, 18, 14, 14, 3, SSD1306_BLACK);  // x0, y0, w, h, r, color  
        display.drawRoundRect(0, 18, 14, 14, 3, SSD1306_WHITE);  // x0, y0, w, h, r, color
        display.setTextColor(SSD1306_WHITE);
      }
      display.print("X");
      display.display();
    }
  #endif  // SSD1306_OLED
#endif   // M5STAMPC3U

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// called by main USBHost comms loop
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void app_loop(void) {
  uint8_t decode_PTT;
  static uint8_t decode_PTT_last = 0;
  static uint32_t last_input_poll = 0;
  static uint32_t last_ptt_input_poll = 0;
  static uint32_t last_disp_info = 0;
  uint8_t decode_Band;
  static uint8_t decode_Band_last = 0;
  uint8_t decode_in;
  static uint8_t xvtr_band_select = 0;  // rotate through a few transverter bands.  Temp until we get a select window
  int btn_state = 0;
  static uint8_t screen = 0;

  #ifndef M5STAMPC3U
    M5.update();
    
    if (board_type == M5ATOMS3) {
      //btn_state = M5.BtnA.wasClicked() ? 2 : M5.BtnA.wasPressed() ? 3 : 0;
      btn_state = M5.BtnA.wasPressed() ? 3 : 0;
      if (btn_state) {
        BtnC_pressed = true;  // on Atoms3 translate to single screen button for Xvtr selection
        btn_state = 0;
      }
    }

    if (BtnA_pressed) {
      BtnA_pressed = false;
      #ifdef XVBOX
        // select band 222
        if (band != BAND_1_25M)
          band_Selector(3, false);
        else
          band_Selector(0, false);
      #else
        DPRINTLNF("BtnA pressed - Switch to BT mode");
        #ifdef BTCLASSIC
          DPRINTLNF("Switch to BT mode");
          BT_enabled = true;  // allows operaor to turn on BT if BT feature is active
          restart_BT_flag = true;
        #endif
      #endif
    }

    if (BtnB_pressed) {
      BtnB_pressed = false;
      #ifdef XVBOX
        // select band 903
        if (band != BAND_33cm)
          band_Selector(5, false);
        else
          band_Selector(0, false);
      #else
        radio_address = 0;
        DPRINTLNF("BtnB pressed: Scan for new radio address");
        get_new_address_flag = true;
      #endif
    }

    if (BtnC_pressed) {
      BtnC_pressed = false;
      #ifdef XVBOX
        // select band 1296
        if (band != BAND_23cm)
          band_Selector(6, false);
        else
          band_Selector(0, false);
      #else
        // Since the first version won't have USB Host (unreliable so far) reuse the button for a single Xvtr band for now
        #ifdef USBHOST
          DPRINTLNF("BtnC pressed - Switch to USB Host mode");
          restart_USBH_flag = true;
        #else
          if (XVTR)  // Btn used for USB or Xvtr for now  - Emulate the wired input for now
          {
            xvtr_band_select++;
            DPRINTLNF("BtnC pressed - Select a Xvtr band - index = ");
            DPRINTLN(xvtr_band_select);
            switch (xvtr_band_select)  // index our way through a curated list.
            {
              case 1: band_Selector(DECODE_INPUT_BAND222, false); break;
              case 2: band_Selector(DECODE_INPUT_BAND902, false); break;
              case 3: band_Selector(DECODE_INPUT_BAND1296, false); break;
              //case 4: band_Selector(DECODE_INPUT_BAND10G); break;
              case 4: band_Selector(0, false);  // fall thru to reset counter to non-Xvtr band
              default: xvtr_band_select = 0; break;
            }
          }
        #endif
      #endif
    }  // end Btn C

    processCatMessages();  // look for delayed or unsolicited messages from radio

    refesh_display();

    Get_Radio_address();  // can autodiscover CI-V address if not predefined.
  
    poll_radio();  // do not send stuff to radio when a PC app is doing the same

    #ifdef USBHOST
      if (restart_USBH_flag) {
        restart_USBH();
        restart_USBH_flag = false;
      }
    #endif

    #ifdef BLE
      //BLE_loop();  // calling from the main loop for now.
    #endif

    #ifdef BTCLASSIC
      bt_loop();  // handle all BT serial messaging in place of the USB host serial
      if (restart_BT_flag) {
        restart_BT();
        restart_BT_flag = false;
      }
    #endif
  #else   // Xvtr box controller
    float volts = 0.0;
    float current = 0.0;
    static uint8_t loop_ctr = 0;
    static uint8_t info_screen = 0;
    char Rx[3] = "Rx";
    char Tx[3] = "Tx";
    static uint32_t PTT_Sequencer_delay_timer = false;
    float temp_celsius = 0.0;

    #define SHOW_INFO 1

    if (millis() > last_disp_info + 250) { 
      temp_celsius = temperatureRead();
      
      // print out debug on serial if enabled
      if (loop_ctr == 4  && SHOW_INFO) {
        if (band == 1)  //  any band < 144MHz is represented as Band 1 but can be any band (AM) through 6M, ie HF/6M.
          DPRINTF("HF/6M");
        else
          DPRINT(bands[band].band_name);
        DPRINTF(" \t");

        if (decode_PTT_last)
          DPRINTF(Tx);
        else 
          DPRINTF(Rx);
        DPRINTF("\t");
        #ifdef INA226_I2C
          volts = INA.getBusVoltage();
          DPRINT(volts, 3);
          DPRINTF("V   \t");
          DPRINT(INA.getShuntVoltage_mV(), 3);
          DPRINTF("mV(shunt)   \t");
          
          current = INA.getCurrent_mA();
          current /= 1000;
          DPRINT(current , 3);
          DPRINTF("A   \t");
          
          DPRINT(INA.getPower_mW()/1000, 3);
          DPRINTF("W   \t");

          DPRINT(temp_celsius, 1);
          DPRINTLNF("C");
        #endif
      }

      // Show on OLED display
      // Rotate the info on the small OLED screen to keep font sizes larger
      // paint the status icons for every screen
      #ifdef SSD1306_OLED
        if (loop_ctr == 4) {
          display.cp437(true);         // Use full 256 char 'Code Page 437' font
          display.setCursor(20, 6);  // set up for Text Info
          display.setTextSize(3); // Draw 3X-scale text
          display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // Draw 'inverse' text
          
          // Update screen estate with band, current and volts in large text rotating every 1 second
          if (info_screen == 0) {    // only change info on full 1 second
            display.fillRect(33, 0, 94, 32, SSD1306_BLACK); // x, y, w, h, color
            if (band == 1)  // < 144MHz
              display.print("HF/6M");
            else
              display.print(bands[band].band_name);
            display.display();   // Update display
          }

          if (info_screen == 1) {  // only change info on full 1 second
            display.fillRect(33, 0, 94, 32, SSD1306_BLACK); // x, y, w, h, color
            display.print(volts);
            display.print("V");
            display.display();   // Update display
          }

          if (info_screen == 2) {  // only change info on full 1 second
            display.fillRect(33, 0, 94, 32, SSD1306_BLACK); // x, y, w, h, color
            display.print(current);
            display.print("A");
            display.display();   // Update display
          }

          if (info_screen == 3) {  // only change info on full 1 second
            display.fillRect(33, 0, 94, 32, SSD1306_BLACK); // x, y, w, h, color
            display.print(temp_celsius);
            display.print("C");
            display.display();   // Update display
          }

          info_screen += 1;  // rotate screen every 1 second
          if (info_screen > 3) 
            info_screen = 0;  // loop around the screens each second
        }
          
        draw_PTT_icon(decode_PTT_last);  // update TX and Xvtr status icons
        draw_Xvtr_icon(XVTR_Band);
      #endif  // 
  
      loop_ctr += 1;
      if (loop_ctr > 4) 
        loop_ctr = 0;  // loop update every 1 second

      last_disp_info = millis();
    }
  #endif  // M5STAMPC3U

  // scan our input sources for wired PTT and band change - only 1 module type at a time for now
  #if defined ( IO_MODULE ) || defined (XVBOX_PLCC ) || defined ( EXT_IO2_UNIT ) || defined ( M5STAMPC3U )
    // Have observed the app loop time to generally be < 10ms so go fast
    uint8_t poll_interval = 8;  //  time in ms for wired we can scan it much faster since it is not querying the radio over a BT connection
    if (millis() > last_input_poll + poll_interval) {
      // Process the band input and PTT input pins
      #if defined ( IO_MODULE )
        decode_in = Module_4in_8out_Input_scan();  // Has 4 digital inputs, using for PTT and band input
      #elif defined ( EXT_IO2_UNIT )
        decode_in = Unit_EXTIO2_Input_scan();     // Has 8 I/O ports, using lower 4 for band and PTT input
      #elif defined ( M5STAMPC3U )
        decode_in = M5STAMPC3U_Input_scan();     // Has 8 I/O ports, using lower 4 for band and PTT input
      #elif defined ( XVBOX_PLCC )
        decode_in = CPU_Input_scan();     // Has 3 Band Out, 1 PTT Out and 1 PTT in, no band in 
      #endif
      
      //DPRINT(decode_in,BIN); DPRINT(":");
      
      decode_PTT = (~decode_in & 0x08) >> 3;  //extract 4th bit
      decode_Band = (~decode_in & 0x07);      // extract the lower 3 of 4 input pins for band select.

      if (decode_Band != decode_Band_last)  // skip if nothing changed on the wired inputs
      { // band changed, process 
        DPRINTF("New Band Input Pattern detected "); DPRINTLN(decode_Band);
        band_Selector(decode_Band, false);  // converts input pattern to band (real or virtual Xvtr band)
        decode_Band_last = decode_Band;
      }

      // 4th pin is wired PTT from radio.
      // extract the 4th input to pass on PTT through selected band's IO pin.
      if ((decode_PTT != decode_PTT_last) && use_wired_PTT)  // only call when the state changes
      { 
        #ifdef M5STAMPC3U  // for Xvtr box only
          if (decode_PTT) {  // RX -> TX
            DPRINTLNF("PTT Sequence Delay Started BLOCK IF path");
            Band_Decode_Output(band, false);  // false forces IF Switch bits 0-2 to 1s (OFF state).  This turns the IF switch RF path off in the Xvtr Box for sequencing.
            //  Set IF switch delay timer
            PTT_Sequencer_delay_timer = millis();  // set up for 20ms delay
          } else {   // TX -> Rx
            PTT_Sequencer_delay_timer = 0;  // turn off time in RX
            Band_Decode_Output(band, true);  // turn IF OFF switch during transition from TX to RX to mute receiver
          }
        #endif

        DPRINTLNF("Operating PTT I/O");
        PTT_Output(band,  decode_PTT);
        decode_PTT_last = decode_PTT;
      }
      last_input_poll = millis();
    }
    
    #ifdef M5STAMPC3U  // for Xvtr box only
      // if in TX mode and PTT still applied, after 1st 20ms expires turn ON the IF Switch to let RF flow
      if (decode_PTT_last && PTT_Sequencer_delay_timer && millis() > (PTT_Sequencer_delay_timer + PTT_DELAY)) {
        DPRINTLNF("PTT Sequence Delay complete, set normal IF path");
        Band_Decode_Output(band, true);  // restore normal IF switch path in the Xv Box after PTT_DELAY ms
        PTT_Sequencer_delay_timer = 0;  // turn off timer after delay
      }
    #endif

  #endif
}
