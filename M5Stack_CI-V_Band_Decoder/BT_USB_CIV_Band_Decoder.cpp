//
// BT_USB_CIV_Band_Decoder.cpp
//

#include <Update.h>
#include "FS.h"
#include <SPI.h>
#include <SD.h>
#include <stdint.h>
#include "CIV.h"
#include "M5Stack_CI-V_Band_Decoder.h"
#include "BLE.h"
#include "Decoder.h"
#include "time.h"
#include "TimeLib.h"

//#define SEE_RAW_RX // see raw hex messages from radio
//#define SEE_RAW_TX // see raw hex messages from radio

#define IC705 0xA4
#define IC905 0xAC

//extern bool XVTR_enabled;
void UpdateFromFS(fs::FS &fs);
void printDirectory(File dir, int numTabs);
extern struct cmdList cmd_List[];

/*  copy of struct here from header file for easy reference.
struct Bands {
  char band_name[6];    // Freindly name or label.  Default here but can be changed by user.
  uint64_t edge_lower;  // band edge limits for TX and for when to change to next band when tuning up or down.
  uint64_t edge_upper;
  uint64_t Xvtr_offset;  // Offset to add to radio frequency.
                         // When all is correct, it will be within the band limits and allow PTT and Band decoder outputs
  uint64_t VFO_last;     // store the last used frequency on each band.
                         // for XVTR bands subtract the LO offset and send the result to the radio
  uint8_t mode_idx;      // current mode stored as indexc to the modelist table.
  uint8_t filt;          // current fiult soreds in teh modelist table
  uint8_t datamode;
  uint8_t agc;            // store last agc.  Some radio/band/mode combos only have 1.
  uint8_t preamp;         // some bands there is no preamp (2.4G+ on 905).  Some radios/bands/modes combos have 1 preamp level, others have 2 levels.
  uint8_t atten;          // some bands there is no atten (some on 905).  Some radios/bands/mode combos have 1 atten level, others have more. 
};
*/

struct Bands bands[NUM_OF_BANDS] = {
  { "AM", 535000, 1705000, 0, 535000, 1, 1, 0, 1, 0, 0 },      // AM
  { "160M", 1800000, 2000000, 0, 1860000, 1, 1, 0, 1, 0, 0 },                    // 160m
  { "80M", 3500000, 4000000, 0, 3573000, 1, 1, 0, 1, 0, 0 },                     // 80m
  { "60M", 5351000, 5367000, 0, 5351000, 1, 1, 0, 1, 0, 0 },                     // 60m
  { "40M", 7000000, 7300000, 0, 7074000, 1, 1, 0, 1, 0, 0 },                     // 40m
  { "30M", 10100000, 10150000, 0, 10136000, 1, 1, 0, 1, 0, 0 },                  // 30m
  { "20M", 14000000, 14350000, 0, 14074000, 1, 1, 0, 1, 0, 0 },                  // 20m
  { "17M", 18068000, 18168000, 0, 18100000, 1, 1, 0, 1, 0, 0 },                  // 17m
  { "15M", 21000000, 21450000, 0, 21074000, 1, 1, 0, 1, 0, 0 },                  // 15m
  { "12M", 24890000, 24990000, 0, 24891500, 1, 1, 0, 1, 0, 0 },                  // 12m
  { "10M", 28000000, 29700000, 0, 28074000, 1, 1, 0, 1, 0, 0 },                  // 10m
  { "6M", 50000000, 54000000, 0, 50125000, 1, 1, 0, 1, 0, 0 },                   // 6m
  { "FM", 88000000, 108000000, 0, 95700000, 6, 1, 0, 1, 0, 0 },                  // FM
  { "Air", 118000000, 137000000, 0, 119200000, 2, 1, 0, 1, 0, 0 },               // AIR
  { "2M", 144000000, 148000000, 0, 144200000, 1, 1, 0, 1, 0, 0 },                // 2m
  { "1.25M", 222000000, 225000000, 194000000, 222100000, 1, 1, 0, 1, 0, 0 },     // 222
  { "70cm", 430000000, 450000000, 0, 432100000, 1, 1, 0, 1, 0, 0 },              // 430/440
  { "33cm", 902000000, 928000000, 758000000, 903100000, 1, 1, 0, 1, 0, 0 },      // 902
  { "23cm", 1240000000, 1300000000, 1152000000, 1296100000, 1, 1, 0, 1, 0, 0 },  // 1296Mhz
  { "13cm", 2300000000, 2450000000, 1870000000, 2304100000, 1, 1, 0, 1, 0, 0 },  // 2.3 and 2.4GHz
  { "9cm", 3300000000, 3500000000, 0, 3301000000, 1, 1, 0, 1, 0, 0 },            // 3.3GHz
  { "6cm", 5650000000, 5925000000, 0, 5760100000, 1, 1, 0, 1, 0, 0 },            // 5.7GHz
  { "3cm", 10000000000, 10500000000, 0, 10368100000, 1, 1, 0, 1, 0, 0 },         // 10GHz
  { "24G", 24000000000, 24002000000, 0, 24031000000, 1, 1, 0, 1, 0, 0 },         // 24GHz
  { "47G", 47000000000, 47002000000, 0, 47192100000, 1, 1, 0, 1, 0, 0 },         // 47GHz
  { "76G", 76000000000, 76002000000, 0, 76000000000, 1, 1, 0, 1, 0, 0 },         // 76GHz
  { "122G", 122000000000, 122002000000, 0, 122001000000, 1, 1, 0, 1, 0, 0 },     // 122GHz
  { "GENE", 0, 123000000000, 0, 432000000, 1, 1, 0, 1, 0, 0 }                    // 0 to 122GHz
};

char title[17] = "CIV Band Decoder";  // make exactly 16 chards if used as the BT device name
uint16_t baud_rate;                 //Current baud speed
uint32_t readtimeout = 10;          //Serial port read timeout
//uint8_t read_buffer[2048];   //Read buffer
//extern uint64_t frequency;                 //Current frequency in Hz
uint8_t band = B_GENERAL;
uint32_t timer;
uint16_t background_color = TFT_BLACK;
uint16_t text_color = TFT_WHITE;
bool PTT = false;
bool prev_PTT = true;
extern char Grid_Square[];
bool BLE_buff_flag = false;

uint8_t radio_address = RADIO_ADDR;  //Transceiver address.  0 allows auto-detect on first messages form radio
bool  auto_address = false;        // If true, detects new radio address on connection mode changes
                               // If false, then the last used address, or the preset address is used.
                               // If Search for Radio button pushed, then ignores this and looks for new address
                               //   then follows rules above when switch connections
bool  use_wired_PTT = true;        // Selects source of PTT, wired input or polled state from radio.  Wired is preferred, faster.
bool  XVTR = true;                 // Enables Xvtr support
  // Edit the bands table farther down the page to enter the fixed LO offset (in Hz) to add to radio dial
  // frequency for the transverter band of interest. Only 1 band supported at this point
uint8_t  XVTR_Band = 0;         // Xvtr band to display - temp until a band select menu is built
uint8_t  brightness = 130;      // 0-255
bool  XVTR_enabled = false;  // true when a transverter feature is active
uint8_t read_buffer[2048];   //Read buffer
uint8_t prev_band = 0xFF;
uint64_t prev_frequency = 0;
bool btConnected = false;
bool btPaired = false;
bool BLE_connected = false;
uint32_t temp_passkey;
uint16_t poll_radio_ptt = POLL_PTT_DEFAULT;  // can be changed with detected radio address.
static bool get_new_address_flag = false;
uint8_t UTC = 1;  // 0 local time, 1 UTC time
extern bool BtnA_pressed;
extern bool BtnB_pressed;
extern bool BtnC_pressed;
extern uint64_t frequency;
//extern void SendMessageBLE(std::string Message);
extern void SendMessageBLE(uint8_t Message[], uint8_t len);

// ######################################################################
// Enter the BD_ADDRESS of your IC-705. You can find it in the Bluetooth
// settings in section 'Bluetooth Device Information'

uint8_t bd_address[7] = { 0x30, 0x31, 0x7d, 0x33, 0xbb, 0x7f, 0x00 };  // Rick's 705
//uint8_t bd_address[7] = { 0x30, 0x31, 0x7d, 0xBA, 0x44, 0xF9, 0x00 };  // Mike's 705
// ######################################################################

#ifdef USBHOST 
// CDC Host object
#include "M5_Max3421E_Usb.h"
extern Adafruit_USBH_CDC SerialHost;
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

#ifdef BLE           // can set to BT on or off at startup
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
      BLE_buff_flag = false;  // reset and wait for next new data to arrive
      for (int i=0; i < sizeof(read_buffer); i++) {
        byte = read_buffer[i];
        if (byte == 0xFF) continue;  //TODO skip to start byte instead

        read_buffer[counter++] = byte;
        if (STOP_BYTE == byte) break;

        if (counter >= sizeof(read_buffer)) return 0;
      }
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

  #ifdef PC_PASSTHROUGH
  // SerialHost or SerialBT -> Serial
  if (counter) {
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
byte getBand(uint64_t _freq) {
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
  if (Data_len != 0) { // copy in 1 or more data bytes, if any
    for (j = 0; j < Data_len; j++)  //pick up with value i
      req[msg_len++] = Data[j];
  }

  req[msg_len] = STOP_BYTE;
  req[msg_len+1] = 0;  // null terminate for printing or conversion to String type

  //#define SEE_RAW_TX  // an also be set at top of file
  #ifdef SEE_RAW_TX
    Serial.print(F("--> Tx Raw Msg: "));
    for (uint8_t k = 0; k <= msg_len; k++) {
      Serial.print(req[k], HEX);
      Serial.print(F(","));
    }
    Serial.print(F(" msg_len = "));
    Serial.print(msg_len+1);
    Serial.println(F(" END"));
  #endif

  //Serial.print(F("   BT connected = ")); Serial.print(btConnected);
  //Serial.print(F("   USBH connected = ")); Serial.println(USBH_connected);

  //#define RAWT  // for a more detailed look

  //if (btConnected && !SerialBT.isClosed() && SerialBT.connected())
  if (USBH_connected || BLE_connected || btConnected) {
    if (msg_len < sizeof(req) - 1) { // ensure our data is not longer than our buffer
      #ifdef SEE_RAWT
      DPRINTF("Send CI-V Msg: ");
      #endif
      
      #if defined ( BLE )
        SendMessageBLE(req, msg_len+1);
      #else
        for (uint8_t i = 0; i <= msg_len; i++) {
          #ifdef SEE_RAWT
            Serial.print(req[i], HEX);
          #endif

            #if defined ( USBHOST )
              if (!SerialHost.write(req[i])) 
                DPRINTLNF("sendCatRequest: Tx: error");
            #elif defined ( BTCLASSIC )
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
    }// if overflow 
  }  // if connected
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
void read_Frequency(uint8_t data_len) {       // This is the displayed frequency, before the radio input, which may have offset applied
  if (frequency > 0) {                         // store frequency per band before it maybe changes bands.  Required to change IF and restore direct after use as an IF.

    //Serial.printf("read_Frequency: Last Freq %-13llu\n", frequency);
    bands[band].VFO_last = frequency;       // store Xvtr or non-Xvtr band displayed frequency per band before it changes.
    prev_band = band;                       // store associated band index
  }                                         // if an Xvtr band, subtract the offset to get radio (IF) frequency
  
  frequency = 0;
  uint64_t mul = 1;
  // This frequency from this point is from radio so will never be the XVTR offset applied version of 'frequency'
  //FE FE E0 42 03 <00 00 58 45 01> FD ic-820  IC-705  5bytes, 10bcd digits
  //FE FE 00 40 00 <00 60 06 14> FD ic-732
  //FE FE E0 AC 03 <00 00 58 45 01 01> FD  IC-905 for 10G and up bands - 6bytes, 12 bcd digits
  // use the data length to loop an extra byte when needed for the IC905 on 10GHz bands and up
  for (uint8_t i = 5; i < 5 + data_len; i++) {
    if (read_buffer[i] == 0xFD) continue;  //spike
    frequency += (read_buffer[i] & 0x0F) * mul;
    mul *= 10;  // * decMulti[i * 2 + 1];
    frequency += (read_buffer[i] >> 4) * mul;
    mul *= 10;  //  * decMulti[i * 2];
  }

  if (XVTR_enabled)
    frequency += bands[XVTR_Band].Xvtr_offset;

  band = getBand(frequency);

  if (prev_band != band) {
    sendCatRequest(CIV_C_F26A, 0, 0);   // fire off a mode/filter request with a change in band.
    vTaskDelay(5);
    processCatMessages();
    sendCatRequest(CIV_C_AGC_READ, 0, 0);   // fire off agc update
    vTaskDelay(5);
    processCatMessages();
  }
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
  uint8_t data[20] = {};
  
  if (1)
  {
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
void bt_loop(void) {
  if (BT_enabled && !btConnected) {
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

    while (radio_address == 0x00 || radio_address == 0xFF || radio_address == 0xE0) {
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
//         Poll radio for PTT, frequency, and other modes.  BLE in particular onkly gets what and when you ask for it (so far)
// --------------------------------------------------------------------------
void poll_radio(void) {
  static uint32_t time_last_freq = millis();
  static uint32_t time_last_UTC = millis();
  static uint32_t time_last_ptt = millis();
  static uint32_t time_last_agc = millis();
  static uint32_t time_last_mode = millis();
  

  if (radio_address != 0x00 && radio_address != 0xFF && radio_address != 0xE0) {
    if (millis() >= time_last_freq + POLL_RADIO_FREQ)  // poll every X ms
    {
      sendCatRequest(CIV_C_F_READ, 0, 0);  // Get current VFO 
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

#ifdef SDCARD

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
  Serial.print(line_buffer);
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
  sprintf(vfo_str, "%lu.%03u.%02u", MHz, KHz, Hz);

  ///sprintf(vfo_str, "%-13s", "012345.123.123");  // 999GHZ max  47G = 47000.000.000
  ///DPRINT("New VFO: ");DPRINTLN(vfo_str);
  return vfo_str;
}

void draw_new_screen(void) {
  int16_t x = 46;  // start position
  int16_t y = 16;
  int16_t x1 = 300;  // end of a line
  int16_t y1 = 10;
  int16_t h = 20;
  int16_t color = TFT_YELLOW;
  int16_t font_sz = 4;  // font size
  DPRINTLNF("+++++++++draw new screen");

  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_YELLOW, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
  M5.Lcd.setTextDatum(MC_DATUM);
  //M5.Lcd.drawString("CI-V band Decoder", (int)(M5.Lcd.width() / 2), y, font_sz);
  M5.Lcd.drawString(title, (int)(M5.Lcd.width() / 2), y, font_sz);
  M5.Lcd.drawFastHLine(1, y + 13, 319, TFT_RED);  // separator below title
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.setTextColor(TFT_CYAN, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
#if (defined(BT_CLASSIC) || defined(BLE)) && defined(USBHOST)
  M5.Lcd.drawString("BT Mode       Search       USB Mode", (int)(M5.Lcd.width() / 2), 220, 2);
#elif (defined(BT_CLASSIC) || defined(BLE)) && !defined(USBHOST)
   M5.Lcd.drawString("                Search          XVTR ", (int)(M5.Lcd.width() / 2), 220, 2);
#elif defined(USBHOST)
   M5.Lcd.drawString("                Search       USB Mode", (int)(M5.Lcd.width() / 2), 220, 2);
#else
  if (XVTR)
    M5.Lcd.drawString("                Search         XVTR ", (int)(M5.Lcd.width() / 2), 220, 2);
  else
    M5.Lcd.drawString("                Search              ", (int)(M5.Lcd.width() / 2), 220, 2);
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
void display_Time(uint8_t _UTC, bool _force) {
  static uint32_t time_last_disp_UTC = millis();

  if ((millis() >= time_last_disp_UTC + POLL_RADIO_UTC) || _force) {
    char temp_t[15] = {};
    int x = 10;
    int x1 = 310;
    int y = 52;
    int font_sz = 4;

    M5.Lcd.setTextColor(TFT_LIGHTGREY, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535

    M5.Lcd.setTextDatum(ML_DATUM);  // x is left side
    sprintf(temp_t, "%02d/%02d/%02d", month(), day(), year());
    M5.Lcd.drawString(temp_t, x, y, font_sz);
    M5.Lcd.setTextDatum(MR_DATUM);  // x1 is right side
    sprintf(temp_t, "%02d:%02d:%02d", hour(), minute(), second());
    M5.Lcd.drawString(temp_t, x1, y, font_sz);
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

  M5.Lcd.setTextDatum(MR_DATUM);

  if (_band != _prev_band || _force) {
    //DPRINTF("XVTR ON = "); DPRINTLN(_band);

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

  M5.Lcd.setTextDatum(MR_DATUM);
  if (_PTT_state != _prev_PTT_state || _force) {
    #ifdef PRINT_PTT_TO_SERIAL
    Serial.print("*********************************************** PTT = ");
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
}

void display_Freq(uint64_t _freq, bool _force) {
  static uint64_t _prev_freq;
  int16_t x = 1;  // start position
  int16_t y = 104;
  int16_t color = TFT_WHITE;
  int16_t font_sz = 6;  // font size

  if ((_freq != _prev_freq && _freq != 0) || _force) {
    #ifdef PRINT_VFO_TO_SERIAL
    Serial.printf("VFOA: %13sMHz - Band: %s\n", formatVFO(_freq), bands[band].band_name);
    #endif
    //M5.Lcd.fillRect(x, y, x1, y1, background_color);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextColor(background_color, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
    M5.Lcd.drawString(formatVFO(_prev_freq), (int)(M5.Lcd.width() / 2), y, font_sz);
    M5.Lcd.setTextColor(color, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
    M5.Lcd.drawString(formatVFO(_freq), (int)(M5.Lcd.width() / 2), y, font_sz);
    _prev_freq = _freq;
  }
}

void display_Band(uint8_t _band, bool _force) {
  static uint8_t _prev_band = 255;
    int x = 8;
  int y = 150;
  int font_sz = 4;

  M5.Lcd.setTextDatum(ML_DATUM);
  if (_band != _prev_band || _force) {
    // Update our outputs
    Band_Decode_Output(band);
    //sendBand(band);   // change the IO pins to match band
    //Serial.printf("Band %s\n", bands[_band].band_name);
    M5.Lcd.setTextDatum(ML_DATUM);
    M5.Lcd.setTextColor(background_color, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
    M5.Lcd.drawString(bands[_prev_band].band_name, x, y, font_sz);
    M5.Lcd.setTextColor(TFT_CYAN);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
    M5.Lcd.drawString(bands[_band].band_name, x, y, font_sz);
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
    M5.Lcd.setTextDatum(ML_DATUM);
    M5.Lcd.setTextColor(background_color, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
    M5.Lcd.drawString(_grid, x, y, font_sz);
    M5.Lcd.setTextColor(TFT_GREEN, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
    M5.Lcd.drawString(_grid, x, y, font_sz);
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

void band_Selector(uint8_t _band_input_pattern) {
  static uint8_t _band_input_pattern_last = 0;
  static bool XVTR_enabled_last;
  static uint8_t XVTR_band_before;

  if (_band_input_pattern != _band_input_pattern_last)  // only do something if it is different
  {
    if (!XVTR_enabled_last && !XVTR_enabled) {   // capture last non-Xvtr band in use
      XVTR_band_before = band;   // record the non-xvtr band before initial XVTR mode enabled.  
    }

    PTT_Output(band, false);  // send PTT OFF with current band before changing to new band.
    // Inputs are in the middle of 2x 4.7K between 3.3V and GND.  1 is open, 0 is closed.
    // translate input band pattern to band index then send to band Decode output
    
    switch (_band_input_pattern)  // this could be BCD, parallel. For now assume 3 lines in paralle for 3 Xvtrs. Only have 4 inputs on module
    {                             // customize tis for your needs.  This is simple 3 Xvtrs, rig is used for IF only, no direct bands.  Only 3 inputs without going to BCD
      case 1:
        XVTR_Band = BAND_33cm;
        XVTR_enabled = true;
        break;
      case 2:
        XVTR_Band = BAND_23cm;
        XVTR_enabled = true;
        break;  // match up choice with the btnC choices and the output patterns for each band
      case 4:
        XVTR_Band = BAND_13cm;
        XVTR_enabled = true;
        break;
      default: XVTR_enabled = false;
    }
    Serial.printf("Band Selector Source (wired or polled) Input Pattern = %d, Xvtr enabled = %d\n", _band_input_pattern, XVTR_enabled);

    _band_input_pattern_last = _band_input_pattern;

    // Band and Frequency are not yet changed    
    // If changing to a XVTR band, or a different one, update VFO to the last used on that band.   We only get band changes here, vever the same band.
    if (XVTR_enabled) {    // set VFO and other values to last used for the target XVTR band
      SetFreq(bands[XVTR_Band].VFO_last);   // This value always has Xvtr offset applied
    }

    // If turning off Xvtr mode, then restore the IF to normal last used values
    if (XVTR_enabled_last && !XVTR_enabled) {   // This will have Xvtr offset = 0
      // band is still Xvtr band until the radio actually changes frequency
      frequency = bands[XVTR_band_before].VFO_last;  // use that band to get last VFO on that band
      band = getBand(frequency);  // get the non-XVtr band
      SetFreq(frequency);  // set radio to that last used.
    }

    // now the band and freq should be updated
    Serial.print(">>>> Last VFO = ");
    Serial.print(bands[band].VFO_last);
    Serial.print("   Last Xvtr VFO = ");
    Serial.print(bands[XVTR_Band].VFO_last);
    Serial.print("   Band In = ");
    Serial.print(_band_input_pattern, HEX);
    Serial.print("   PTT = ");
    Serial.println(PTT);   

    XVTR_enabled_last = XVTR_enabled;
  }
}

// Length is 5 or 6 depending if < 10GHz band  folowded by 5 or 6 BCD encoded frequency bytes
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
    Serial.printf(" VFO: < 10G Bands = Reversed hex to DEC byte %02X %02X %02X %02X %02X %02X\n", vfo_dec[0], vfo_dec[1], vfo_dec[2], vfo_dec[3], vfo_dec[4], vfo_dec[5]);
  } else {
    len = 6;
    for (uint8_t i = 0; i < len; i++) {
      uint64_t x = vfo % 100;
      vfo_dec[i] = bcdByteEncode(static_cast<uint8_t>(x));
      vfo = vfo / 100;
    }
    Serial.printf(" VFO: > 10G Bands = Reversed hex to DEC byte %02X %02X %02X %02X %02X %02X %02X\n", vfo_dec[0], vfo_dec[1], vfo_dec[2], vfo_dec[3], vfo_dec[4], vfo_dec[5], vfo_dec[6]);
  }
  return len;  // 5 or 6
}

// Send new frequency to radio, radio will change bands as needed. 
// ToDo:  Radio mode and other settings are not tocuhed so stay the same as the last band used.  We are only changing the frequency, nothing else.
//        Need to save mode, filter and other stuff to return each band to the last way it was used.
void SetFreq(uint64_t Freq) {
  uint8_t vfo_dec[7] = {};

  if (XVTR_enabled) {
    Freq -= bands[XVTR_Band].Xvtr_offset;
    //Serial.printf("SetFreq: Xvtr Offset applied - IF freq %llu  band %s  offset %llu\n", Freq, bands[XVTR_Band].band_name, bands[XVTR_Band].Xvtr_offset);
  }

  uint8_t len = formatFreq(Freq, vfo_dec);  // Convert to BCD string
  //Serial.printf("SetFreq: Radio Freq = %llu  To radio (5 or 6 bytes) in BCD: %02X %02X %02X %02X %02X (%02X)\n", Freq, vfo_dec[0], vfo_dec[1], vfo_dec[2], vfo_dec[3], vfo_dec[4], vfo_dec[5]);
  sendCatRequest(CIV_C_F1_SEND, vfo_dec, len);
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

  M5.Lcd.setBrightness(brightness);  // 0-255.  burns more power at full, but works in daylight decently
  //M5.Lcd.drawString(title, 5, 5, 4);

#ifdef IO_MODULE
  Module_4in_8out_setup();  //Set up our IO modules comms on I2C
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

  //Module_4in_8out_Output_test();
  vTaskDelay(500);

#ifdef SDCARD
  #if defined ( CORE3 ) || defined ( CORE2 )
  if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
    // Print a message if the SD card initialization fails or if the SD card does not exist.
    Serial.println("Card failed, or not present");
  } else {
  #else
  if (1) {
  #endif
    Serial.println("Looking for SD Card to try update and read config");
    File root = SD.open("/");
    printDirectory(root, 0);  // look what is on the SD card
    root.close();
    UpdateFromFS(SD);                 // if there is an update, do it.  Otherwise read config file.
    uint16_t lines = read_SD_Card();  // get line count
    Serial.printf("Setup: config.ini line count is %d\n", lines);
    Serial.print(F("Updated bd_address:"));
    for (int z = 0; z < 6; z++)
      Serial.print(bd_address[z], HEX);
    Serial.println("");
    //printLineN(lines);
  }
#endif

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
  // restart_USBH();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// called by main USBHost comms loop
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void app_loop(void) {
  uint8_t decode_PTT_temp;
  static uint8_t decode_PTT_temp_last = 0;
  static uint32_t last_input_poll = 0;
  uint8_t decode_Band_temp;
  static uint8_t decode_Band_temp_last = 0;
  uint8_t decode_in;
  static uint8_t xvtr_band_select = 0;  // rotate through a few transverter bands.  Temp until we get a select window

  M5.update();

  if (BtnA_pressed) {
    BtnA_pressed = false;
    Serial.println(F("BtnA pressed - Switch to BT mode"));
#ifdef BTCLASSIC
    Serial.println(F("Switch to BT mode"));
    BT_enabled = true;  // allows operaor to turn on BT if BT feature is active
    restart_BT_flag = true;
#endif
  }

  if (BtnB_pressed) {
    BtnB_pressed = false;
    radio_address = 0;
    Serial.print(F("BtnB pressed: Scan for new radio address"));
    get_new_address_flag = true;
  }

  if (BtnC_pressed) {
    BtnC_pressed = false;

// Since the first version won't have USB Host (unreliable so far) reuse the button for a single Xvtr band for now
#ifdef USBHOST
    Serial.print(F("BtnC pressed - Switch to USB Host mode"));
    restart_USBH_flag = true;
#else
    if (XVTR)  // Btn used for USB or Xvtr for now  - Emulate the wired input for now
    {
      xvtr_band_select++;
      Serial.print(F("BtnC pressed - Select a Xvtr band - index = "));
      Serial.println(xvtr_band_select);
      switch (xvtr_band_select)  // index our way through a curated list.
      {
        case 1: band_Selector(1); break;
        case 2: band_Selector(2); break;
        case 3: band_Selector(4); break;
        case 4: band_Selector(0);  // fall thru to reset counter
        default: xvtr_band_select = 0; break;
      }
    }
#endif
  }  // end Btn C

  processCatMessages();  // look for delayed or unsolicited messages from radio

  refesh_display();

  Get_Radio_address();  // can autodiscover CI-V address if not predefined.

#if defined ( PC_PASSTHROUGH  )
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
  poll_radio();  // do not send stuff to radio when a PC app is doing the same
#endif

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

#ifdef IO_MODULE
  if (millis() > last_input_poll + POLL_PTT_DEFAULT) {
    // Process the band input and PTT input pins
    decode_in = Module_4in_8out_Input_scan();
    decode_PTT_temp = (~decode_in & 0x08) >> 3;  //extract 4th bit
    decode_Band_temp = (~decode_in & 0x07);      // extract the lower 3 of 4 input pins for band select.

    // 4th pin is wired PTT from radio.
    // extract the 4rh input to pass on PTT through slected bands IO pin.
    if ((decode_PTT_temp != decode_PTT_temp_last) && use_wired_PTT)  // only call when the state changes
    {
      PTT_Output(band, decode_PTT_temp);  // should add debounce but cpu in the IO module seems to be doing that already well enough
      decode_PTT_temp_last = decode_PTT_temp;
    }

    if (decode_Band_temp != decode_Band_temp_last)  // skip if nothing changed on the wired inputs
    {
      band_Selector(decode_Band_temp);  // converts input patern to band (real or virtual Xvtr band)
      decode_Band_temp_last = decode_Band_temp;
    }

    last_input_poll = millis();
  }
#endif
}
