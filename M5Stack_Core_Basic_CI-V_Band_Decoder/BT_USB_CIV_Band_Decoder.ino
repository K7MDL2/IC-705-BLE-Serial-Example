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

      It is important now to set the "bd_address" in the next lines!
*************************************************************************/

#include <Arduino.h>
#include "BluetoothSerial.h"
#include <M5Stack.h>
#include "CIV.h"

bool BT_enabled = 0;  // configuration toggle between BT and USB
#define IC705 0xA4
#define IC905 0xAC
uint8_t radio_address = IC905;  //Transceiver address

// ######################################################################
// Enter the BD_ADDRESS of your IC-705. You can find it in the Bluetooth
// settings in section 'Bluetooth Device Information'

uint8_t bd_address[6] = { 0x30, 0x31, 0x7d, 0x33, 0xbb, 0x7f };
// ######################################################################

#define DATA_PIN 5  // GPIO18 input/output
#define TUNE_PIN 2  // GPIO26 (output)

#define CONTROLLER_ADDRESS 0xE0  //Controller address
#define BROADCAST_ADDRESS 0x00

#define START_BYTE 0xFE  // Start byte
#define STOP_BYTE 0xFD   // Stop byte

#define CMD_READ_FREQ 0x03  // Read operating frequency data

#define POLL_RADIO_PTT    47  // poll the radio for PTT status odd numbers to stagger them ia bit
#define POLL_RADIO_FREQ  218  // poll the radio for frequency
#define POLL_RADIO_UTC 65132  // poll radio for time and location

uint8_t UTC = 1;  // 0 local time, 1 UTC time
extern Adafruit_USBH_CDC SerialHost;
extern bool USBH_connected;
//#define SSP                           // use BT SSP - pair with a passkey
//#ifdef SSP
bool confirmRequestPending = true;
//#endif

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

// Function prototypes:
void configRadioBaud(uint16_t);
uint8_t readLine(void);
bool searchRadio();
//void sendCatRequest(uint8_t );
void sendCatRequest(const uint8_t cmd_num, const uint8_t Data[], const uint8_t Data_len);  // first byte in Data is length
void printFrequency(void);
void processCatMessages();
void sendBit(int);
void sendBand(byte);

const uint64_t decMulti[] = { 100000000000, 10000000000, 1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1 };

#define BAUD_RATES_SIZE 4
const uint16_t baudRates[BAUD_RATES_SIZE] = { 19200, 9600, 4800, 1200 };

char title[] = "CI-V Band Decoder";
void callback(esp_spp_cb_event_t, esp_spp_cb_param_t *);
uint16_t baud_rate;            //Current baud speed
uint32_t readtimeout;          //Serial port read timeout
uint8_t read_buffer[127];      //Read buffer
uint64_t frequency;            //Current frequency in Hz
uint8_t band = 254;
uint32_t timer;
uint16_t background_color = BLACK;
uint16_t text_color = WHITE;
uint8_t PTT = 0;
uint8_t prev_PTT = 1;

#define NUM_OF_BANDS 18
const uint32_t bands[][2] = {
  { 1800, 2000 },      // 160m
  { 3500, 4000 },      // 80m
  { 5351, 5367 },      // 60m
  { 7000, 7300 },      // 40m
  { 10100, 10150 },    // 30m
  { 14000, 14350 },    // 20m
  { 18068, 18168 },    // 17m
  { 21000, 21450 },    // 15m
  { 24890, 24990 },    // 12m
  { 28000, 29700 },    // 10m
  { 50000, 54000 },    // 6m
  { 144000, 148000 },  // 2m
  { 430000, 450000 },   // 430/440
  { 902000, 928000 },   // 902
  { 1240000, 1300000 },   // 1296Mhz
  { 2304000, 2450000 },   // 2.3 and 2.4GHz
  { 5650000, 5925000 },   // 5.7GHz
  { 10000000, 10500000 }   // 10GHz
};

String modes;

uint8_t prev_band = 0xFF;
uint64_t prev_frequency = 0;
bool btConnected = false;
bool btPaired = false;
uint32_t temp_passkey;

BluetoothSerial SerialBT;

// ------------------------------------------
//   Callback to get info about connection
// ------------------------------------------
void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {  // 34
    btConnected = true;
    btPaired = true;
    Serial.println("Client Connected");
  } else if (event == ESP_SPP_CLOSE_EVT)  // 27
  {
    btConnected = false;
    if (btPaired) Serial.println("Client disconnected");  // suppress when we are not paired yet
  }
}

// ----------------------------------------
//      Connect to bluetooth
// ----------------------------------------
void configRadioBaud(uint16_t baudrate) {

  // Connect to client:
  //Serial.print("Connect to bluetooth client ...");
  if (BT_enabled) {
    if (btPaired && !btConnected) {
      btConnected = SerialBT.connect(bd_address, role);  // connect as master so the order of power up between this and the radio do not matter
      Serial.println("Trying to connect to Transceiver");
    }
    if (btPaired && btConnected)
      Serial.println("Transceiver paired and connected");
  }
}

// ----------------------------------------
//    Read incoming line from bluetooth
// ----------------------------------------
uint8_t readLine(void) {
  uint8_t byte;
  uint8_t counter = 0;
  uint32_t ed = readtimeout;  // not initialized!
  if (BT_enabled) {
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
  } else {
    while (USBH_connected) {
      while (!SerialHost.available()) {
        if (--ed == 0 || !USBH_connected) return 0;  // leave the loop if BT connection is lost
      }
      ed = readtimeout;
      byte = SerialHost.read();
      if (byte == 0xFF) continue;  //TODO skip to start byte instead

      read_buffer[counter++] = byte;
      if (STOP_BYTE == byte) break;

      if (counter >= sizeof(read_buffer)) return 0;
    }
  }
  return counter;
}


// ----------------------------------------
//       Get address of transceiver
// ----------------------------------------
bool searchRadio() {
  for (uint8_t baud = 0; baud < BAUD_RATES_SIZE; baud++) {
    configRadioBaud(baudRates[baud]);

    //Serial.println("Send out Frequency Request");
    sendCatRequest(CIV_C_F_READ, 0, 0);
    delay(10);
    //Serial.println("Look for response from radio");

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
byte getBand(uint32_t freq) {
  for (uint8_t i = 0; i < NUM_OF_BANDS; i++) {
    if (freq >= bands[i][0] && freq <= bands[i][1]) {
      if (i == NUM_OF_BANDS) return NUM_OF_BANDS;  // T1 tuner does not have different settings for 2m and UHF
      return i + 1;
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
  uint8_t req[12] = { START_BYTE, START_BYTE, radio_address, CONTROLLER_ADDRESS };

  //Serial.print("sendCatRequest: USBH_connected = "); Serial.println(USBH_connected);

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

//#define SEE_RAWT
#ifdef SEE_RAWT
  Serial.print("Tx Raw Msg: ");
  
  for (uint8_t k = 0; k <= msg_len; k++) {
    Serial.print(req[k], HEX); Serial.print(",");
  }
  Serial.print(" msg_len = "); Serial.print(msg_len); Serial.println(" END");
#endif

  if (BT_enabled && !SerialBT.isClosed() && SerialBT.connected()) 
  {
    if (msg_len < sizeof(req) - 1)  // ensure our data is not longer than our buffer
    {
#ifdef SEE_RAWT
      Serial.print("Snd BT Msg: ");
#endif
      for (uint8_t i = 0; i <= msg_len; i++) {
#ifdef SEE_RAWT
        Serial.print(req[i], HEX);
#endif
        if (!SerialBT.write(req[i]))
          Serial.println("sendCatRequest: BT tx: error");
#ifdef SEE_RAWT
        Serial.print(",");
#endif
      }
#ifdef SEE_RAWT
      Serial.print(" END TX MSG, msg_len = "); Serial.println(msg_len);
#endif
    } 
    else 
    {
      Serial.println("sendCatRequest: BT buffer overflow");
    }
  } 
  else if (USBH_connected) 
  {
    if (msg_len < sizeof(req) - 1)  // ensure our data is not longer than our buffer
    {
#ifdef SEE_RAWT
      Serial.print("Snd USB Host Msg: ");
#endif
      for (uint8_t i = 0; i <= msg_len; i++) 
      {
#ifdef SEE_RAWT
        Serial.print(req[i], HEX);
#endif
        if (!SerialHost.write(req[i]))
          Serial.println("sendCatRequest: USB Host tx: error");
#ifdef SEE_RAWT
        Serial.print(",");
#endif
      }
#ifdef SEE_RAWT
      Serial.print(" END TX MSG, msg_len = ");Serial.println(msg_len);
#endif
    } 
    else 
    {
      Serial.println("sendCatRequest: USB Host buffer overflow");
    }
  } else
    Serial.println(" No open BT or USB Host ports to send to");
}

// ----------------------------------------
//      Print the received frequency
// ----------------------------------------
void printFrequency(uint8_t data_len) {
  frequency = 0;
  //FE FE E0 42 03 <00 00 58 45 01> FD ic-820  IC-705  5bytes, 10bcd digits
  //FE FE 00 40 00 <00 60 06 14> FD ic-732
  //FE FE E0 AC 03 <00 00 58 45 01 01> FD  IC-905 for 10G and up bands - 6bytes, 12 bcd digits
  uint64_t mul = 1;

  // use the data length to loop an extra byte when needed for the IC905 on 10GHz bands and up
    for (uint8_t i = 5; i < 5 + data_len; i++) {   
      if (read_buffer[i] == 0xFD) continue;  //spike
      frequency += (read_buffer[i] & 0x0F) * mul; mul *= 10;    // * decMulti[i * 2 + 1];
      frequency += (read_buffer[i] >> 4) * mul; mul   *= 10;      //  * decMulti[i * 2];
    }
  Serial.printf("Freq %-11llu  datalen = %d\n", frequency, data_len);
}

#ifdef SSP

void BTConfirmRequestCallback(uint32_t numVal) {
  confirmRequestPending = true;
  Serial.print("Confirmation ID: ");
  Serial.println(numVal);
  M5.Lcd.setTextColor(text_color, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
  M5.Lcd.setTextSize(2);                              // to Set the size of text from 0 to 255
  M5.Lcd.setCursor(10, 90);                           //Set the location of the cursor to the coordinates X and Y
  M5.Lcd.printf(" Passkey is %d\n", numVal);
  temp_passkey = numVal;
  SerialBT.confirmReply(true);
}

void BTAuthCompleteCallback(bool success) {
  if (success) {
    Serial.println("Pairing success!!");
    btPaired = true;
    confirmRequestPending = false;
    M5.Lcd.setTextColor(background_color);            //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
    M5.Lcd.setTextSize(2);                            // to Set the size of text from 0 to 255
    M5.Lcd.setCursor(10, 90);                         //Set the location of the cursor to the coordinates X and Y
    M5.Lcd.printf(" Passkey is %d\n", temp_passkey);  // erase passkey from screen now that we are paired
  } else {
    Serial.println("Pairing failed, (Timed out or rejected by user!!");
    btPaired = false;
  }
}
#endif

// ----------------------------------------
//         sendBit
// ----------------------------------------
void sendBit(int bit) {

  digitalWrite(DATA_PIN, HIGH);
  if (bit != 0) {
    delay(4);
  } else {
    delayMicroseconds(1500);
  }

  digitalWrite(DATA_PIN, LOW);
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

  while ((BT_enabled && SerialBT.available()) || (!BT_enabled && SerialHost.available()))  // &&  USBH_connected))
  {
    bool knowncommand = true;
    int i;
    int msg_len;

    cmd_num = 255;
    match = 0;

    if ((msg_len = readLine()) > 0) {

//#define SEE_RAW
#ifdef SEE_RAW
      Serial.print("Rx Raw Msg: ");
      for (uint8_t k = 0; k < msg_len; k++) {
        Serial.print(read_buffer[k], HEX);
        Serial.print(",");
      }
      Serial.print(" msg_len = ");
      Serial.print(msg_len);
      Serial.println(" END");
#endif
      if (read_buffer[0] == START_BYTE && read_buffer[1] == START_BYTE) {
        if (read_buffer[3] == radio_address) {
          if (read_buffer[2] == CONTROLLER_ADDRESS || read_buffer[2] == BROADCAST_ADDRESS) {

            for (cmd_num = CIV_C_F_SEND; cmd_num < End_of_Cmd_List; cmd_num++)  // loop through the command list structure looking for a pattern match
            {
              //Serial.print("processCatMessageslist: list index = "); Serial.println(cmd_num);
              for (i = 1; i <= cmd_List[cmd_num].cmdData[0]; i++)  // start at the highest and search down. Break out if no match. Make it to the bottom and you have a match
              {

                //Serial.print("processCatMessages: byte index = "); Serial.println(i);
                //Serial.printf("processCatMessages: cmd_num=%d from radio, current byte from radio = %X  next byte=%X, on remote length=%d and cmd=%X\n",cmd_num, read_buffer[3+i], read_buffer[3+i+1], cmd_List[cmd_num].cmdData[0], cmd_List[cmd_num].cmdData[1]);
                if (cmd_List[cmd_num].cmdData[i] != read_buffer[3 + i]) {
                  //Serial.print("processCatMessages: Skip this one - Matched 1 element: look at next field, if any left. CMD Body Length = ");
                  //Serial.print(cmd_List[cmd_num].cmdData[0]); Serial.print(" CMD  = "); Serial.print(cmd_List[cmd_num].cmdData[i], HEX);Serial.print(" next RX byte = "); Serial.println(read_buffer[3+i+1],HEX);
                  match = 0;
                  break;
                }
                match++;
                //Serial.print("processCatMessages: Possible Match: Len = "); Serial.print(cmd_List[cmd_num].cmdData[0],DEC); Serial.print("  CMD1 = "); Serial.print(read_buffer[4],HEX);
                //Serial.print(" CMD2  = "); Serial.print(read_buffer[5],HEX); Serial.print(" Data1/Term  = "); Serial.println(read_buffer[6],HEX);
              }

              //if (read_buffer[3+i] == STOP_BYTE)  // if the next byte is not a stop byte then it is thge next cmd byte or maybe a data byte, depends on cmd length

              if (match && (match == cmd_List[cmd_num].cmdData[0])) {
                //Serial.print("processCatMessages:    FOUND MATCH: Len = "); Serial.print(cmd_List[cmd_num].cmdData[0],DEC); Serial.print("  CMD1 = "); Serial.print(read_buffer[4],HEX);
                //Serial.print(" CMD2  = "); Serial.print(read_buffer[5],HEX);  Serial.print(" Data1/Term  = "); Serial.print(read_buffer[6],HEX); Serial.print("  Message Length = "); Serial.println(msg_len);
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
              Serial.printf("processCatMessages: No message match found - cmd_num = %d  read_buffer[4 & 5] = %X %X\n", cmd_num, read_buffer[4], read_buffer[5]);
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
  digitalWrite(TUNE_PIN, HIGH);
  delay(500);
  digitalWrite(TUNE_PIN, LOW);

  // The ATU will pull the DATA_PIN line HIGH for 50ms

  previousTime = millis();
  //while(digitalRead(DATA_PIN) == LOW) {
  // Measure time the while loop is active, and jump out after maxWaiTime.
  // This esnures that the program does not lock in case the communication
  // with the ATU is temporarly broken

  //unsigned long currentTime = millis();
  //if (currentTime - previousTime > maxWaitTime) {
  //  Serial.println("Error: No positive pulse from T1 detected!");
  //  return;
  //}
  //}

  //while(digitalRead(DATA_PIN) == HIGH) {
  //}
  // Wait 10ms
  //delay(10);

  // and then send data on the DATA line
  //pinMode(DATA_PIN, OUTPUT);

  // 1 bits are HIGH for 4ms
  // 0 bits are HIGH for 1.5ms
  // Gap between digits is 1.5ms LOW

  // 1st bit
  sendBit(band & 8);
  sendBit(band & 4);
  sendBit(band & 2);
  sendBit(band & 1);

  // Leave the line LOW
  digitalWrite(DATA_PIN, LOW);

  // and switch it back to an input
  pinMode(DATA_PIN, INPUT);
}

void draw_new_screen(void)
{
  M5.Lcd.fillScreen(background_color);
  M5.Lcd.setTextSize(3); // to Set the size of text from 0 to 255
  M5.Lcd.setCursor(1, 20); //Set the location of the cursor to the coordinates X and Y
  M5.Lcd.setTextColor(YELLOW); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
  M5.Lcd.printf(title);    
  //M5.Lcd.drawString(title, 1, 20, text_color);
}
//
//    formatVFO()
//
char *formatVFO(uint64_t vfo) {
  static char vfo_str[20] = { "" };
  //if (ModeOffset < -1 || ModeOffset > 1)
  //vfo += ModeOffset;  // Account for pitch offset when in CW mode, not others

  uint32_t MHz = (vfo / 1000000 % 1000000);
  uint16_t Hz = (vfo % 1000);
  uint16_t KHz = ((vfo % 1000000) - Hz) / 1000;
  sprintf(vfo_str, "%lu.%03u.%03u", MHz, KHz, Hz);

  ///sprintf(vfo_str, "%-13s", "012345.123.123");  // 999GHZ max  47G = 47000.000.000
  ///DPRINT("New VFO: ");DPRINTLN(vfo_str);
  return vfo_str;
}

void bt_loop(void) {
  while (BT_enabled && !btConnected && btPaired) {
    M5.Lcd.fillScreen(background_color);
    M5.Lcd.setTextColor(text_color, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
    //M5.Lcd.drawString("Connecting ...", 40, 40, 2);
    M5.Lcd.setTextSize(2);     // to Set the size of text from 0 to 255
    M5.Lcd.setCursor(10, 60);  //Set the location of the cursor to the coordinates X and Y
    M5.Lcd.printf("Connecting ...");

    btConnected = SerialBT.connect(bd_address, role);
    if (btConnected) {
      Serial.println("Transceiver reconnected");

      M5.Lcd.setCursor(10, 60);  //Set the location of the cursor to the coordinates X and Y
      M5.Lcd.printf("Transceiver reconnected");

      delay(500);  // let the operator see something

      M5.Lcd.fillScreen(background_color);
      M5.Lcd.setTextSize(3);            // to Set the size of text from 0 to 255
      M5.Lcd.setCursor(1, 20);         //Set the location of the cursor to the coordinates X and Y
      M5.Lcd.setTextColor(YELLOW);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
      M5.Lcd.printf("IC-705 BT Band Decoder");
      prev_band = 255;
      prev_frequency = 0;  // cause the screen to refresh
      break;
    }
  }
}

void Get_Radio_address(void) {
  if ((BT_enabled && btConnected) || (USBH_connected && !BT_enabled))  // handle both USB and BT
  {
    while (radio_address == 0x00 || radio_address == 0xFF || radio_address == 0xE0) {
      if (!searchRadio()) {
        Serial.println("Radio not found");
      } else {
        Serial.print("Radio found at ");
        Serial.print(radio_address, HEX);
        Serial.println();
      }
    }
  }
}

// ----------------------------------------
//         Setup
// ----------------------------------------
void usbh_setup(void) {
  //Serial.begin(115200);

}

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

  M5.Lcd.setTextSize(2);     // to Set the size of text from 0 to 255
  M5.Lcd.setCursor(10, 60);  //Set the location of the cursor to the coordinates X and Y
  M5.Lcd.printf("Connecting ...");

  SerialBT.register_callback(callback);
  // Setup bluetooth as master:
  if (!SerialBT.begin("BT CI-V Decoder ", true))  // true = master  must be 16 characters exactly
  {
    Serial.println("An error occurred initializing Bluetooth");
    //abort();
  } else {
    Serial.println("Bluetooth initialized");
    Serial.println("The device started, now you can pair it with bluetooth!  Use Pairing Reception menu on the radio");
  }

  btConnected = SerialBT.connect(bd_address, role);
  delay(1000);  // give time for title screen
  Serial.print("btPaired = "); Serial.println(btPaired);
  Serial.print("btConnected = "); Serial.println(btConnected);
  if (btConnected) {
    btPaired = true;
    //M5.Lcd.fillScreen(background_color);
    M5.Lcd.setTextColor(background_color);
    M5.Lcd.setTextSize(2);            // to Set the size of text from 0 to 255
    M5.Lcd.setCursor(10, 60);         //Set the location of the cursor to the coordinates X and Y
    M5.Lcd.printf("Connecting ...");  // erase connecting
  } else
    Serial.println("Pair to Radio");
}

// -----------------------------------------------------------------------
//         Poll tadio for PTT and when CIV Echo Back is off, frequency
// --------------------------------------------------------------------------
void poll_radio(void) {
  static uint32_t time_last_freq = millis();
  static uint32_t time_last_UTC = millis();
  static uint32_t time_last_ptt = millis();

  if ((!BT_enabled && !USBH_connected) || (BT_enabled && !btConnected)) {
    Serial.println("poll_radio: No ports open, nothing to do");
    return;  // nothing to send to
  }

  if (radio_address != 0x00 && radio_address != 0xFF && radio_address != 0xE0) {
    if (millis() >= time_last_freq + POLL_RADIO_FREQ)  // poll every X ms
    {
      sendCatRequest(CIV_C_F_READ, 0, 0);  // Get TX status
      delay(2);
      processCatMessages();
      band = getBand(frequency / 1000);
      time_last_freq = millis();
    }

    if (millis() >= time_last_ptt + POLL_RADIO_PTT)  // poll every X ms
    {
      sendCatRequest(CIV_C_TX, 0, 0);  // Get TX status
      delay(2);
      processCatMessages();
      time_last_ptt = millis();
    }

    if (millis() >= time_last_UTC + POLL_RADIO_UTC)  // poll every X ms
    {
      if (radio_address == 0xAC)                   //905
        sendCatRequest(CIV_C_UTC_READ_905, 0, 0);  //CMD_READ_FREQ);
      else if (radio_address == 0xA4)              // 705
        sendCatRequest(CIV_C_UTC_READ_705, 0, 0);  //CMD_READ_FREQ);
      delay(5);
      processCatMessages();
      sendCatRequest(CIV_C_MY_POSIT_READ, 0, 0);  //CMD_READ_FREQ);
      delay(10);
      processCatMessages();
      time_last_UTC = millis();
    }
  }
}

void display_PTT(uint8_t PTT_state)
{
  static uint8_t prev_PTT_state = 1;
  char PTT_Tx[] = " Tx ";
  char PTT_Rx[] = " Rx ";

  if (PTT_state != prev_PTT_state)
  {
    Serial.print("*** PTT = ");Serial.println(PTT_state);

    M5.Lcd.setTextSize(3); // to Set the size of text from 0 to 255
    M5.Lcd.setCursor(230, 190); //Set the location of the cursor to the coordinates X and Y
    if (prev_PTT_state)
    {
      M5.Lcd.setTextColor(LIGHTGREY, BLACK); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
      M5.Lcd.printf("%s", PTT_Tx);
    }
    else 
    {
      M5.Lcd.setTextColor(WHITE, RED); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
      M5.Lcd.printf("%s", PTT_Rx);
    }

    M5.Lcd.setCursor(230, 190); //Set the location of the cursor to the coordinates X and Y
    if (PTT_state)
    {
      M5.Lcd.setTextColor(WHITE, RED); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
      M5.Lcd.printf("%s", PTT_Tx);
    }
    else 
    {
      M5.Lcd.setTextColor(LIGHTGREY, BLACK); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
      M5.Lcd.printf("%s", PTT_Rx);
    }
    prev_PTT_state = PTT_state;
  }
 }

void display_Freq(uint64_t freq)
{
  static uint64_t prev_freq;
  if (freq != prev_freq)
  {    
    Serial.printf("VFOA: %13sMHz - Band: %d\n", formatVFO(freq), band);
    Serial.println(freq);
    M5.Lcd.setTextSize(4); // to Set the size of text from 0 to 255

    M5.Lcd.setTextColor(background_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
    M5.Lcd.setCursor(1, 80); //Set the location of the cursor to the coordinates X and Y
    M5.Lcd.printf( "%13s", formatVFO(prev_freq));

    M5.Lcd.setTextColor(text_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
    M5.Lcd.setCursor(1, 80); //Set the location of the cursor to the coordinates X and Y
    M5.Lcd.printf( "%13s", formatVFO(freq));

    prev_freq = freq;
  }
}

void display_Band(uint8_t _band)
{
  static uint8_t prev_band = 255;
  
  if (_band != prev_band)
  {    
    
    //sendBand(band);   // change the IO pins to match band
    
    Serial.printf("Band %d\n", band);

    M5.Lcd.setTextSize(3); // to Set the size of text from 0 to 255
    M5.Lcd.setTextColor(background_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
    M5.Lcd.setCursor(70, 130); //Set the location of the cursor to the coordinates X and Y
    M5.Lcd.printf("Band: %3d", prev_band);

    M5.Lcd.setTextColor(CYAN); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
    M5.Lcd.setCursor(70, 130); //Set the location of the cursor to the coordinates X and Y
    M5.Lcd.printf("Band: %3d", band);
    prev_band = _band;
  }
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
  M5.begin();
  M5.Lcd.setBrightness(120);
  draw_new_screen();

  if (BT_enabled)
    BT_Setup();  // turn on BlueTooth Classic SPP client and connect to a radio server
  else
    usbh_setup();  // Just talk normal USB serial
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// called by main USBHost comms loop
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void app_loop(void) {

  //Serial.print("USBH_connected = "); Serial.println(USBH_connected);

  poll_radio();

  bt_loop();  // handle all BT serial messaging in place of the USB host serial

  Get_Radio_address();  // can autodiscuver CI-V address if not predefined.

  processCatMessages();  // look for delayed or unsolicited messages from radio

  display_Freq(frequency);
  display_PTT(PTT);
  display_Band(band);
  
}