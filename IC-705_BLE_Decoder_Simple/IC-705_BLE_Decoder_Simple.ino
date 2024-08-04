/*
 *  IC-705_BLE_Decoder_Simple.ino
 *
 *  Central Mode (client) BLE UART for ESP32
 *
 *  Modifed by K7MDL Aug 2024 for BT connection to the IC-705 via BLE serial on a M5Stack Core3 and Core 3 SE.  Tested on a SE version
 * 
 *  This is simple band decoder that displays VFO and PTT state on the M5Core3 screen.  It also has Buttons enabled.
 *  If it is a CoreS3 then there should be 3 buttons drawn on the bottom of the screen. (Untested) It will display serial message when pressed or held.
 *  If it is a CoreS3 SE, then the lower black apron is touch enabled and will display serial message when pressed or held.
 *
 *  To use:
 *  1. Go to the IC-705 Pairing Reception menu
 *  2. Reset the M5Stack Core3
 *  3. It will autoconnect and display the name in the Pairing/Connect menu
 *
 ********************************   Below is the original code header/credits **************************************************  
 *
 * This sketch is a central mode (client) Nordic UART Service (NUS) that connects automatically to a peripheral (server)
 * Nordic UART Service. NUS is what most typical "blueart" servers emulate. This sketch will connect to your BLE uart
 * device in the same manner the nRF Connect app does.
 *
 * Once connected this sketch will switch notification on using BLE2902 for the charUUID_TX characteristic which is the
 * characteristic that our server is making data available on via notification. The data received from the server
 * characteristic charUUID_TX will be printed to Serial on this device. Every five seconds this device will send the
 * string "Time since boot: #" to the server characteristic charUUID_RX, this will make that data available in the BLE
 * uart and trigger a notifyCallback or similar depending on your BLE uart server setup.
 *
 *
 * A brief explanation of BLE client/server actions and rolls:
 *
 * Central Mode (client) - Connects to a peripheral (server).
 *   -Scans for devices and reads service UUID.
 *   -Connects to a server's address with the desired service UUID.
 *   -Checks for and makes a reference to one or more characteristic UUID in the current service.
 *   -The client can send data to the server by writing to this RX Characteristic.
 *   -If the client has enabled notifications for the TX characteristic, the server can send data to the client as
 *   notifications to that characteristic. This will trigger the notifyCallback function.
 *
 * Peripheral (server) - Accepts connections from a central mode device (client).
 *   -Advertises a service UUID.
 *   -Creates one or more characteristic for the advertised service UUID
 *   -Accepts connections from a client.
 *   -The server can send data to the client by writing to this TX Characteristic.
 *   -If the server has enabled notifications for the RX characteristic, the client can send data to the server as
 *   notifications to that characteristic. This the default function on most "Nordic UART Service" BLE uart sketches.
 *
 *
 * Copyright <2018> <Josh Campbell>
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright
 * notice and this permission notice shall be included in all copies or substantial portions of the Software. THE
 * SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * Based on the "BLE_Client" example by Neil Kolban:
 * https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLETests/Arduino/BLE_client/BLE_client.ino
 * With help from an example by Andreas Spiess:
 * https://github.com/SensorsIot/Bluetooth-BLE-on-Arduino-IDE/blob/master/Polar_Receiver/Polar_Receiver.ino
 * Nordic UART Service info:
 * https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v14.0.0%2Fble_sdk_app_nus_eval.html
 *
 */


#include "BLEDevice.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include <M5CoreS3.h>
//#include <M5Unified.h>



//#define M5BTNS
#ifdef M5BTNS
  #include <gob_unifiedButton.hpp>
  goblib::UnifiedButton unifiedButton;
#endif

// The remote Nordic UART service service we wish to connect to.
// This service exposes two characteristics: one for transmitting and one for receiving (as seen from the client).
static BLEUUID serviceUUID("14cf8001-1ec2-d408-1b04-2eb270f14203");

// The characteristics of the above service we are interested in.
// The client can send data to the server by writing to this characteristic.
static BLEUUID charUUID_RX("14cf8002-1ec2-d408-1b04-2eb270f14203");   // RX Characteristic

// If the client has enabled notifications for this characteristic,
// the server can send data to the client as notifications.
static BLEUUID charUUID_TX("14cf8002-1ec2-d408-1b04-2eb270f14203");   // TX Characteristic

int scanTime = 10; //In seconds
static BLEScan *pBLEScan;

static BLEAddress *pServerAddress = NULL;
static boolean doConnect = false;
static boolean connected = false;
static BLERemoteCharacteristic* pTXCharacteristic;
static BLERemoteCharacteristic* pRXCharacteristic;
uint8_t radio_address = 0xA4;   // A4 for IC-705, AC for IC-905
uint16_t  background_color = BLACK;
uint16_t  text_color = WHITE;
uint64_t frequency = 0;
const uint32_t decMulti[]    = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};
uint64_t prev_frequency = 0;
uint8_t   band = 254;
uint8_t PTT = 0;
uint8_t prev_PTT = 1;
bool BT_ADDR_confirm = false;
bool Name_confirm = false;
bool Token_confirm = false;
bool Pairing_Accepted = false;
bool CIV_granted = false;
bool BLE_connected = false;
const uint8_t notificationOff[] = {0x0, 0x0};
const uint8_t notificationOn[] = {0x1, 0x0};
bool onoff = true;

#define POLL_RADIO            30   // poll the radio for frequency and other parameters

#define NUM_OF_BANDS 13
const uint32_t bands[][2] = 
{
  {   1800,  2000 },  // 160m
  {   3500,  4000 },  // 80m
  {   5351,  5367 },  // 60m
  {   7000,  7300 },  // 40m
  {  10100, 10150 },  // 30m
  {  14000, 14350 },  // 20m
  {  18068, 18168 },  // 17m
  {  21000, 21450 },  // 15m
  {  24890, 24990 },  // 12m
  {  28000, 29700 },  // 10m
  {  50000, 54000 },  // 6m
  { 144000,148000 },  // 2m
  { 430000,450000 }   // UHF
};

// ----------------------------------------
//      Print the received frequency
// ----------------------------------------
void printFrequency(const uint8_t freq[])
{
  frequency = 0;
  //FE FE E0 42 03 <00 00 58 45 01> FD ic-820
  //FE FE 00 40 00 <00 60 06 14> FD ic-732
  for (uint8_t i = 0; i < 5; i++) {
    if (freq[9 - i] == 0xFD) continue; //spike
    frequency += (freq[9 - i] >> 4) * decMulti[i * 2];
    frequency += (freq[9 - i] & 0x0F) * decMulti[i * 2 + 1];
  }
}

//
//    formatVFO()
//
char* formatVFO(uint64_t vfo)
{
	static char vfo_str[20] = {""};
	//if (ModeOffset < -1 || ModeOffset > 1)
		//vfo += ModeOffset;  // Account for pitch offset when in CW mode, not others
	
	uint32_t MHz = (vfo/1000000 % 1000000);
	uint16_t Hz  = (vfo % 1000);
	uint16_t KHz = ((vfo % 1000000) - Hz)/1000;
	sprintf(vfo_str, "%lu.%03u.%03u", MHz, KHz, Hz);
	
	///sprintf(vfo_str, "%-13s", "012345.123.123");  // 999GHZ max  47G = 47000.000.000
	///DPRINT("New VFO: ");DPRINTLN(vfo_str);
	return vfo_str;
}


class MyClientCallback : public BLEClientCallbacks 
{
  void onConnect(BLEClient* pclient) {
      //doConnect = true;
      BLE_connected = true;  // tracks state of BLE level connection
      Serial.println("Connected to BLE server event");
  }

  void onDisconnect(BLEClient* pclient) {
      connected = false; // tacks state of CIV connection
      doConnect = true; //  gateway to connect and pair processes.
      BLE_connected = false;  // tracks state of BLE level connection
      Serial.println("Lost BLE server connection event flag set on Disconnect ");
      //Scan_BLE_Servers();
  }
};


#define WATCH_SERIAL

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) 
{
  //Serial.println("Notify callback for TX characteristic received. Data:");
  for (int i = 0; i < length; i++) 
  {
    // Serial.print((char)pData[i]);     // Print character to uart
    #ifdef WATCH_SERIAL
      Serial.print(pData[i], HEX);           // print raw data to uart
      Serial.print(" ");
    #endif
    if (pData[i] == 0xFD)
    {
      #ifdef WATCH_SERIAL
        Serial.println();
      #endif

      if (1) //(!connected)
      {
        if (pData[1] == 0xF1 && pData[2] == 0x00)
        {
          switch (pData[3])
          {
            case 0x61:
              Serial.println("Got BT_ADDR message confirmation, proceed");
              BT_ADDR_confirm = true;              
              break;
            case 0x62:            
              Serial.println("Got NAME message confirmation, proceed");
              Name_confirm = true;
              break;
            case 0x63:
              Serial.println("Got TOKEN message confirmation, proceed");
              Token_confirm = true;
              if (pData[4] == 0x01)
                Pairing_Accepted = true;  // Pairing action worked.  Once pair this wil return 0
              break;
            case 0x64:
              Serial.println("CI-V bus ACCESS granted, proceed");
              CIV_granted = true;
              //connected = true;
              break;
          }
        }
      }

      if (connected)
      {
        switch (pData[4])
        {
          case 0x00:
          case 0x03:
          case 0x05:
            printFrequency(pData); // VFO frequency
            band = getBand(frequency/1000);
            break;
          case 0x1C:
            if (pData[5] == 0)  // RX/TX state message
              PTT = pData[6];
            break;
          default: break;
        }
      }
    }
  }
}

//inline uint8_t bcdByte(const uint8_t x) const { return  (((x & 0xf0) >> 4) * 10) + (x & 0x0f); }
inline uint8_t bcdByte(const uint8_t x) { return  (((x & 0xf0) >> 4) * 10) + (x & 0x0f); }
// input is between 0 and 99 decimal.  output 0 to 0x99
//inline uint8_t bcdByteEncode(const uint8_t x) const { return ((x / 10) << 4) + (x % 10); }
inline uint8_t bcdByteEncode(const uint8_t x) { return ((x / 10) << 4) + (x % 10); }

//**************************************************************************************
//
//        Connect to Server
//
//**************************************************************************************
bool connectToServer(BLEAddress pAddress) {
  Serial.print("Establishing a connection to device address: ");
  Serial.println(pAddress.toString().c_str());

  M5.Lcd.setTextSize(2); // to Set the size of text from 0 to 255
  M5.Lcd.setTextColor(text_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
  M5.Lcd.setCursor(5, 80); //Set the location of the cursor to the coordinates X and Y
  M5.Lcd.printf("Connecting to radio BLE..");

  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");
  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  Serial.println(" - Connected to server");
  pClient->setMTU(517); //set client to request maximum MTU from
						  //server (default is 23 otherwise)

  // Obtain a reference to the Nordic UART service on the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find Nordic UART service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service ");
  //Serial.println(pClient->getPeerAddress());

  // Obtain a reference to the TX characteristic of the Nordic UART service on the remote BLE server.
  pTXCharacteristic = pRemoteService->getCharacteristic(charUUID_TX);
  if (pTXCharacteristic == nullptr) {
    Serial.print("Failed to find TX characteristic UUID: ");
    Serial.println(charUUID_TX.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Remote BLE TX characteristic reference established");

  // Read the value of the TX characteristic.
  //if(pTXCharacteristic->canRead()) {
    std::string value = pTXCharacteristic->readValue();
    Serial.print("The characteristic value is currently: ");
    Serial.println(value.c_str());
  //}

  // Obtain a reference to the RX characteristic of the Nordic UART service on the remote BLE server.
  pRXCharacteristic = pRemoteService->getCharacteristic(charUUID_RX);
  if (pRXCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID_RX.toString().c_str());
    return false;
  }
  Serial.println(" - Remote BLE RX characteristic reference established");

  // Read the value of the characteristic.
  //if(pRXCharacteristic->canRead()) {
      value = pRXCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    //}

  if(pTXCharacteristic->canNotify())
      pTXCharacteristic->registerForNotify(notifyCallback);
 
  delay(200);
  CIV_Connect();
  return true;
}

//  send these messages to pair and/or log in to radio and get access to CIV bus
void CIV_Connect(void)
{
  Serial.println("Connecting to radio CIV bus ...");
 // For testing I hard coded the pairing messages to the IC-705!
  delay(50);

  // using BT_Address   BT_ADDRESS "48:27:E2:79:12:51"
  //const uint8_t* bt_addr = esp_bt_dev_get_address();  // can get own BT address this way.  Need colons?
  //uint8_t CIV_ID0[11] = {0xFE,0xF1,0x30,0x61,0x48,0x27,0xE2,0x79,0x45,0x51,0xFD};  // Send our BT_ADDR 
  uint8_t CIV_ID0[] = {0xFE,0xF1,0x00,0x61,0x34,0x38,0x32,0x037,0x45,0x32,0x37,0x39,0x31,0x32,0x35,0x31,0xFD};  // Send our BT_ADDR 
  //uint8_t CIV_ID0[] = {0xFE,0xF1,0x30,0x61,0x48,0x3A,0x27,0x3A,0xE2,0x3A,0x79,0x3A,0x45,0x3A,0x51,0xFD};  // Send our BT_ADDR 
  pRXCharacteristic->writeValue(CIV_ID0, sizeof(CIV_ID0));
  pRXCharacteristic->canNotify();
  delay(50);
  
  // name is "IC-705 Decoder 3"
  uint8_t CIV_ID1[] = {0xFE, 0xF1, 0x00, 0x62, 0x49, 0x43, 0x2D, 0x37, 0x30, 0x35, 0x20, 0x44, 0x65, 0x63, 0x6F, 0x64, 0x65, 0x72, 0x20, 0x33, 0xFD};  // Send Name
  pRXCharacteristic->writeValue(CIV_ID1, sizeof(CIV_ID1));
  pRXCharacteristic->canNotify();
  delay(50);  // a small delay was required or this message would be missed (collision likely).

  // Send Token
  uint8_t CIV_ID2[] = {0xFE, 0xF1, 0x00, 0x63, 0xEE, 0x39, 0x09, 0x10, 0xFD}; // Send Token
  pRXCharacteristic->writeValue(CIV_ID2, 9, true);
  pRXCharacteristic->canNotify();
  delay(50);

//pRXCharacteristic->canNotify();
  //String helloValue = "Hello Remote Server";
  //pRXCharacteristic->writeValue(helloValue.c_str(), helloValue.length());
  //connected = true;
  return true;
}

void printDeviceAddress() {
  const uint8_t* point = esp_bt_dev_get_address();
 
  for (int i = 0; i < 6; i++) {
    char str[3];
 
    sprintf(str, "%02X", (int)point[i]);
    Serial.print(str);
 
    if (i < 5){
      Serial.print(":");
    }
  }
  Serial.println("");
}

/**
   Scan for BLE servers and find the first one that advertises the Nordic UART service.
*/
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    /**
        Called for each advertising BLE server.
    */
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.print("BLE Advertised Device found - ");
      Serial.println(advertisedDevice.toString().c_str());

      // We have found a device, check to see if it contains the Nordic UART service.
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(serviceUUID)) {

        Serial.println("Found a device with the desired ServiceUUID!");
        advertisedDevice.getScan()->stop();
  
        pServerAddress = new BLEAddress(advertisedDevice.getAddress());
        doConnect = true;

      } // Found our server
    } // onResult
}; // MyAdvertisedDeviceCallbacks

void Scan_BLE_Servers(void)
{
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device. Specify that we want active scanning and start the
  // scan to run for 30 seconds
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449); // less or equal setInterval value
  pBLEScan->setActiveScan(true);
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  ////pBLEScan->start(scanTime, false);
  Serial.print("Devices found: "); Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
}

// ----------------------------------------
//    get band from frequency 
// ----------------------------------------
byte getBand(uint32_t freq)
{
  for(uint8_t i=0; i<NUM_OF_BANDS;i++) {
    if(freq >= bands[i][0] && freq <= bands[i][1] ) {
      if(i==NUM_OF_BANDS) return NUM_OF_BANDS;  // T1 tuner does not have different settings for 2m and UHF 
      return i+1;
    }
  }
  return 0xFF;  // no band for considered frequency found
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

void display_Band(uint8_t _band)
{
  static uint8_t prev_band = 255;
  
  if (_band != prev_band)
  {    
    Serial.printf("Band %d\n", band);

    //sendBand(band);   // change the IO pins to match band
    M5.Lcd.setTextColor(background_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
    M5.Lcd.setCursor(80, 120); //Set the location of the cursor to the coordinates X and Y
    M5.Lcd.printf("Band: %3d", prev_band);

    M5.Lcd.setTextColor(CYAN); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
    M5.Lcd.setCursor(80, 120); //Set the location of the cursor to the coordinates X and Y
    M5.Lcd.printf("Band: %3d", band);
    prev_band = _band;
  }
}

void display_Freq(uint64_t freq)
{
  static uint64_t prev_freq;
  if (freq != prev_freq)
  {    
    Serial.printf("VFOA: %13sMHz - Band: %d\n", formatVFO(freq), band);

    M5.Lcd.setTextSize(3); // to Set the size of text from 0 to 255

    M5.Lcd.setTextColor(background_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
    M5.Lcd.setCursor(20, 80); //Set the location of the cursor to the coordinates X and Y
    M5.Lcd.printf( "%13sMHz", formatVFO(prev_freq));

    M5.Lcd.setTextColor(text_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
    M5.Lcd.setCursor(20, 80); //Set the location of the cursor to the coordinates X and Y
    M5.Lcd.printf( "%13sMHz", formatVFO(freq));

    prev_freq = freq;
  }
}

//--------------------------------------------                                                //
// BLE 
// SendMessage by BLE Slow in packets of 20 chars
// or fast in one long string.
// Fast can be used in IOS app BLESerial Pro
//------------------------------
void SendMessageBLE(std::string Message)
{
 if(connected) 
   {
    if (1)
    //if (Mem.UseBLELongString)                                                                 // If Fast transmission is possible
     {
      pRXCharacteristic->writeValue(Message); 
      pRXCharacteristic->canNotify();
      delay(10);                                                                              // Bluetooth stack will go into congestion, if too many packets are sent
     } 
   else                                                                                       // Packets of max 20 bytes
     {   
      int parts = (Message.length()/20) + 1;
      for(int n=0;n<parts;n++)
        {   
         pRXCharacteristic->writeValue(Message.substr(n*20, 20)); 
         pRXCharacteristic->canNotify();
         delay(40);                                                                           // Bluetooth stack will go into congestion, if too many packets are sent
        }
     }
   } 
}

void draw_initial_screen(void)
{
  M5.Lcd.fillScreen(background_color);
  M5.Lcd.setTextSize(2); // to Set the size of text from 0 to 255
  M5.Lcd.setCursor(20, 30); //Set the location of the cursor to the coordinates X and Y
  M5.Lcd.setTextColor(text_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
  M5.Lcd.printf("IC-705 BLE Band Decoder");

  M5.Lcd.setTextColor(text_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
  M5.Lcd.setCursor(5, 80); //Set the location of the cursor to the coordinates X and Y
  M5.Lcd.printf("Connected to CI-V");
  
  delay(1000);
  
  M5.Lcd.fillScreen(background_color);
  M5.Lcd.setTextSize(2); // to Set the size of text from 0 to 255
  M5.Lcd.setCursor(20, 30); //Set the location of the cursor to the coordinates X and Y
  M5.Lcd.setTextColor(text_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
  M5.Lcd.printf("IC-705 BLE Band Decoder");
}

#ifdef M5BTNS
void check_M5Buttons(void)
{
  unifiedButton.update(); // Must be call after M5.update. (Changed to call after M5.update() since 0.1.0)

    // M5.BtnX can be used to obtain status
    if(M5.BtnA.wasHold())
    {
        Serial.println("Button A held");
    }
    else if(M5.BtnA.wasClicked())
    {
        Serial.println("Button A pressed");
    }

    if(M5.BtnB.wasHold())
    {
        Serial.println("Button B held");
    }
    else if(M5.BtnB.wasClicked())
    {
        Serial.println("Button B pressed");
    }

    if(M5.BtnC.wasHold())
    {
        Serial.println("Button C held");
    }
    else if(M5.BtnC.wasClicked())
    {
        Serial.println("Button C pressed");
    }

    // Drawing Buttons
    unifiedButton.draw();
}
#endif

void setup() {
  M5.begin();

  #ifdef M5BTNS
    unifiedButton.begin(&M5.Display);
  #endif
  
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting Arduino BLE Central Mode (Client) Nordic UART Service");
  M5.Lcd.fillScreen(background_color);
  M5.Lcd.setTextSize(2); // to Set the size of text from 0 to 255
  M5.Lcd.setCursor(20, 30); //Set the location of the cursor to the coordinates X and Y
  M5.Lcd.setTextColor(text_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
  M5.Lcd.printf("IC-705 BLE Band Decoder");
  
  BLEDevice::init("IC-705-BLE-Decoder");
  doConnect = true;
  Serial.println("Setup Done");
} // End of setup.cover

void loop() 
{
  static uint32_t time_last = millis();
  
  M5.update();
  
  #ifdef M5BTNS
    check_M5Buttons();
  #endif

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  M5.Lcd.setTextSize(2); // to Set the size of text from 0 to 255

  if (!BLE_connected || !connected)
  {        
      Serial.printf("doConnect1 - Lost connection, starting BLE scanner - connected = %d  Token = %d  CIV_Granted = %d  BLE_connected = %d\n", connected, Token_confirm, CIV_granted, BLE_connected);        
      Scan_BLE_Servers();
  }
  
  if (doConnect == true) 
  {
    Serial.printf("doConnect2 - calling connect-to-server - connected = %d  Token = %d  CIV_Granted = %d  BLE_connected = %d\n", connected, Token_confirm, CIV_granted, BLE_connected);

    if (connectToServer(*pServerAddress)) 
    {      
      Serial.printf("doConnect3 - post-connect to server - connected = %d  Token = %d  CIV_Granted = %d  BLE_connected = %d\n", connected, Token_confirm, CIV_granted, BLE_connected);
      
      if (Pairing_Accepted && Token_confirm)
          Serial.println("Pairing_Accepted");  // only get this whe a pairing is request.  false on regular sign in

      if (!Pairing_Accepted && Token_confirm) // continue on
          Serial.println("Paired already, continue to sign in");
    
      if (CIV_granted)   // pairing succeeded and singed in or already paired and singed in
      {
        Serial.println("Sign-in completed");
        connected = true;
        doConnect = false;
        Name_confirm = Token_confirm = CIV_granted = false;  // reset 
        draw_initial_screen();
      }
      else 
      {
        //connected = false;
        Serial.println("No Sign-In confirmation from Radio");
        M5.Lcd.setTextColor(text_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
        M5.Lcd.setCursor(5, 80); //Set the location of the cursor to the coordinates X and Y
        M5.Lcd.printf("No Sign-In confirmation from Radio");
        CIV_Connect();
      } 
    }
    else 
    {
      Serial.println("We have failed to connect to the server; there is nothing more we will do.");
      M5.Lcd.setTextColor(text_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
      M5.Lcd.setCursor(5, 80); //Set the location of the cursor to the coordinates X and Y
      M5.Lcd.printf("Failed Connection to Radio");
      doConnect = true;
      Scan_BLE_Servers();  // kick of the process from the start
    }
  }

  // If we are connected to a peer BLE Server perform the following actions every five seconds:
  //   Toggle notifications for the TX Characteristic on and off.
  //   Update the RX characteristic with the current time since boot string.
  if (connected) 
  {
    if (onoff) {
      //Serial.println("Notifications turned on");
      pTXCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
    } else {
      //Serial.println("Notifications turned off");
      pTXCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOff, 2, true);
    }
    // Toggle on/off value for notifications.
    //onoff = onoff ? 0 : 1;
    onoff = 1;

    if (millis() >= time_last + POLL_RADIO)   // poll every X ms
    {
      // Set the characteristic's value to be the array of bytes that is actually a string
      //String timeSinceBoot = "Time since boot: " + String(millis()/1000);
      //Serial.println("Poll radio");
      uint8_t CIV_frequency[] = {0xFE, 0xFE, radio_address, 0xE0, 0x03, 0xFD};
      pRXCharacteristic->writeValue(CIV_frequency, sizeof(CIV_frequency), true);
      pRXCharacteristic->canNotify();
      delay(10);

      //Serial.println("Poll radio for TX/RX state");
      uint8_t CIV_TX[] = {0xFE, 0xFE, radio_address, 0xE0, 0x1C, 0x00, 0xFD};
      pRXCharacteristic->writeValue(CIV_TX, sizeof(CIV_TX), true);
      pRXCharacteristic->canNotify();
      delay(10);

      //SendMessageBLE(CIV_frequency);
      //pRXCharacteristic->writeValue(timeSinceBoot.c_str(), timeSinceBoot.length());
      //pRXCharacteristic->canNotify();

      // Update the M5 screen
      display_Freq(frequency);
      display_PTT(PTT);
      display_Band(band);
      time_last = millis();
    }  // poll radio
  }// if connected

  //delay(20); // Delay five seconds between loops.

} // End of loop
