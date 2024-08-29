#include "HWCDC.h"
/*
 *  IC705_BLE_Decoder_Simple.ino
 *
 *  Central Mode (client) BLE UART for ESP32 - Tested on CoreS3 SE
 *
 *  Modifed by K7MDL Aug 2024 for BLE Serial port (uart) connection to the IC-705 via BLE serial on a M5Stack Core3 and Core 3 SE.  Tested on a SE version
 * 
 *  This is a simplified example program showing a BLE connection to an IC-705 and getting CI-V frequency and PTT state.  
 *  The UI is simply a title, frequency, band number, and PTT status, with PTT (Rx/Tx) turning red during Tx.  
 *  Future example programs could show more info on the display, process button touches, and have configurable band decoder outputs
 *
 *********************************************** Usage *************************************************************************
 *
 *  To use:  
 *  1. On the IC-705 press menu
 *  2. Navigate to to Set -> Bluetooth Set -> <<Pairing Reception menu>>  
 *  3. The device will pair silently with the radio and display the given name in the Pairing Connect page.
 * 
 *  Once paired it will auto-reconnect as long as it is paired with the radio.
 *  The name shown in the Pairing/Connect menu is hard coded today.  See below byte arrays to edit.
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
#define BLE_
#ifdef BLE_

//#include "M5Stack_CI-V_Band_Decoder.h"
#include "DebugPrint.h"
#include "BLEDevice.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "BLEScan.h"
//#include "CIV.h"
#include "sdkconfig.h"
#if defined(CONFIG_BLUEDROID_ENABLED)
#include <esp_gap_ble_api.h>
#include <string>
#include "BLEAdvertisedDevice.h"
#include "BLEClient.h"
#include "RTOS.h"
#include "BLE.h"
//#include "BLEAddress.h"

//#define WATCH_BLE_SERIAL

// The remote Nordic UART service service we wish to connect to.
// This service exposes two characteristics: one for transmitting and one for receiving (as seen from the client).
static BLEUUID serviceUUID("14cf8001-1ec2-d408-1b04-2eb270f14203");

// The characteristics of the above service we are interested in.
// The client can send data to the server by writing to this characteristic.
static BLEUUID charUUID_RX("14cf8002-1ec2-d408-1b04-2eb270f14203");   // RX Characteristic

// If the client has enabled notifications for this characteristic,
// the server can send data to the client as notifications.
static BLEUUID charUUID_TX("14cf8002-1ec2-d408-1b04-2eb270f14203");   // TX Characteristic

// For testing I hard coded the pairing messages to the IC-705!

// UUID, 41 bytes total. Any less and will not get a reply from the Name msg 0x42.  It is too generated nunmber which iodeally should be unique on each device
// The radio uses the combination of UUID and Name to identity each device paired.
// Here is a generated UUID from https://www.uuidgenerator.net/version4
//                                        56a56730-                                     e8bc-                     4930-                     81b6-                     ef33e973842b
uint8_t CIV_ID0[] = {0xFE,0xF1,0x00,0x61, 0x35,0x36,0x41,0x35,0x36,0x37,0x33,0x30,0x2D, 0x45,0x38,0x42,0x43,0x2D, 0x34,0x39,0x30,0x30,0x2D, 0x38,0x31,0x42,0x36,0x2D, 0x45,0x46,0x33,0x33,0x45,0x39,0x37,0x33,0x38,0x34,0x32,0x42,0xFD};  // Send our UUID 

// name is "IC705 Decoder 04" 21 bytes total - YOu can change this but teh name must be exactly 16 bytes. Can pad with spaces
uint8_t CIV_ID1[] = {0xFE, 0xF1, 0x00, 0x62, 0x49, 0x43, 0x37, 0x30, 0x35, 0x2D, 0x44, 0x65, 0x63, 0x6F, 0x64, 0x65, 0x72, 0x2D, 0x30, 0x34, 0xFD};  // Send Name
// name is "IC705 BT Decoder"
//  uint8_t CIV_ID1[] = {0xFE, 0xF1, 0x00, 0x62, 0x49, 0x43, 0x37, 0x30, 0x35, 0x2D, 0x42, 0x54, 0x2D, 0x44, 0x65, 0x63, 0x6F, 0x64, 0x65, 0x72, 0xFD};  // Send Name

// ToDo: Make configurable the target radio's service and characteristic UUIDs. Only needed if future radios use differenmt UUIDs.  
// The RX and Tx UUIDs are the same number.  Unlike most devices, the CI-V bus is not full duplex, 
//    they share the same wire for Rx and TX and so it goes for the digital wire here.

#ifdef CORE3
extern HWCDC Serial;
#else
#include "M5Stack_CI-V_Band_Decoder.h"
#endif

int scanTime = 5; //In seconds
static BLEScan *pBLEScan = NULL;
static BLEClient *pClient = NULL;
static BLEAddress *pServerAddress = NULL;
//static boolean doConnect = false;
static bool connected = false;
static BLERemoteCharacteristic* pTXCharacteristic;
static BLERemoteCharacteristic* pRXCharacteristic;
//const uint64_t decMulti[]    = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};
static bool BT_ADDR_confirm = false;
static bool Name_confirm = false;
static bool Token_confirm = false;
static bool Pairing_Accepted = false;
static bool CIV_granted = false;
extern bool BLE_connected;
const uint8_t notificationOff[] = {0x0, 0x0};
const uint8_t notificationOn[] = {0x1, 0x0};
bool onoff = true;
extern uint8_t band;
extern uint8_t PTT;
extern uint8_t radio_address;
extern bool XVTR_enabled;
extern uint8_t  XVTR_Band;
extern uint64_t frequency;
extern uint8_t read_buffer[];   //Read buffer
extern bool BLE_buff_flag;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify);

//void printFrequency(void);

// The onDisconnect() callback function worked but am using pClient->isConnected() instead.
// It also arrives asynchronously which has to be handled as such.  Not using this now.
// To use these, uncomment the 
class MyClientCallback : public BLEClientCallbacks 
{
  void onConnect(BLEClient* pclient) {
      //doConnect = true;
      //BLE_connected = true;  // tracks state of BLE level connection
      DPRINTLNF("Now Connected to BLE server");
  }

  void onDisconnect(BLEClient* pclient) {
      connected = false; // tacks state of CIV connection
      doConnect = false; //  gateway to connect and pair processes.
      BLE_connected = false;  // tracks state of BLE level connection
      Name_confirm = Token_confirm = CIV_granted = false;  // reset 
      DPRINTLNF("Lost BLE server connection event flag set on Disconnect ");
      //Scan_BLE_Servers();
  }
};

// ----------------------------------------
//      Print the received frequency
// ----------------------------------------
/*
void printFrequency(const uint8_t freq[])
{
  if (BLE_connected) {
    frequency = 0;
    uint64_t mul = 1;
    //FE FE E0 42 03 <00 00 58 45 01 10> FD ic-905 6byes on 10G+ bands
    //FE FE E0 42 03 <00 00 58 45 01> FD    ic-820 and IC-705  5 bytes
    //FE FE 00 40 00 <00 60 06 14> FD       ic-732 4 bytes
    for (uint8_t i = 0; i < 5; i++) {
      if (freq[9 - i] == 0xFD) continue; //spike
      frequency += (freq[9 - i] >> 4) * decMulti[i * 2];
      frequency += (freq[9 - i] & 0x0F) * decMulti[i * 2 + 1];
    }
    DPRINTF("VFO:"); DPRINTLN(frequency); 
  }
}
*/
uint8_t *r = read_buffer;

// This can pop up inthe middle of a read_buffer processing fucntion so use a semaphore and only copy the buffer if the process is done with the old one.
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) 
{
  uint8_t buf[64] = {};

  //Serial.printf("Callback Notify value = \n",isNotify);
  #ifdef WATCH_BLE_SERIAL
  DPRINTF("Callback length: "); DPRINTLN(length);
  DPRINTF("Notify callback - Data: ");
  #endif

  int i = 0;
  //   it is possble for several messages to be stacked up making length larger than the buffer.  
  //   Process the first and ignore the rest.
  //  Ideally we would parse several and put then into a queue
  //  Another option is to load up the read_buffer but that is not a circular buffer today

   if (length >= sizeof(buf))   // if length > buffer size will get a stack crash
    length = sizeof(buf) - 1;

  for (i; i < length; i++) 
  {
    // DPRINT((char)pData[i]);     // Print character to uart
    #ifdef WATCH_BLE_SERIAL
      DPRINT(pData[i], HEX);           // print raw data to uart
      DPRINTF(",");
    #endif
    
    if (pData[0] != 0xFE) {   
      break;
    }

    if (pData[0] == 0xFE && pData[1] == 0xFE)
        buf[i] = pData[i];  // copy into main read buffer

    if (pData[i] == 0xFD)
    {
      if (pData[1] == 0xF1 && pData[2] == 0x00)
      {
        switch (pData[3])
        {
          case 0x61:
            DPRINTLNF("Got BT_ADDR message confirmation, proceed");
            BT_ADDR_confirm = true;              
            break;
          case 0x62:            
            DPRINTLNF("Got NAME message confirmation, proceed");
            Name_confirm = true;
            break;
          case 0x63:
            DPRINTLNF("Got TOKEN message confirmation, proceed");
            Token_confirm = true;
            if (pData[4] == 0x01)
              Pairing_Accepted = true;  // Pairing action worked.  Once pair this wil return 0
            break;
          case 0x64:
            DPRINTLNF("CI-V bus ACCESS granted, proceed");
            CIV_granted = true;
            connected = true;
            break;
        }
        BLE_buff_flag = false;   // consuming functions will reset this once it is read and won't waste time reading stale data
        break;
      }

      buf[i+1] = 0;

      if (!BLE_buff_flag) {
        memcpy(r, buf, i+1);     // only move data into read_buffer if it has already been read by the main program.
        BLE_buff_flag = true;   // buffer reaady - consuming functions will reset this once it is read and won't waste time reading stale data
      }

      #ifdef WATCH_BLE_SERIAL
        DPRINTLNF("");
      #endif
    }
  } 
}

//**************************************************************************************
//
//        Connect to Server
//
//**************************************************************************************

bool connectToServer(BLEAddress pAddress) {
  DPRINTF("Establishing a connection to device address: ");
  DPRINTLN(pAddress.toString().c_str());

  pClient  = BLEDevice::createClient();
  //BLEClient*  pClient  = BLEDevice::createClient();
  DPRINTLNF(" - Created client");

  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  DPRINTLNF(" - Connected to server");
  pClient->setMTU(517); //set client to request maximum MTU from
						  //server (default is 23 otherwise)

  // Obtain a reference to the Nordic UART service on the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    DPRINTF("Failed to find Nordic UART service UUID: ");
    DPRINTLN(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  DPRINTF(" - Found our service ");
  //DPRINTLN(pClient->getPeerAddress());

  // Obtain a reference to the TX characteristic of the Nordic UART service on the remote BLE server.
  pTXCharacteristic = pRemoteService->getCharacteristic(charUUID_TX);
  if (pTXCharacteristic == nullptr) {
    DPRINTF("Failed to find TX characteristic UUID: ");
    DPRINTLN(charUUID_TX.toString().c_str());
    pClient->disconnect();
    return false;
  }
  DPRINTLNF(" - Remote BLE TX characteristic reference established");

  // Read the value of the TX characteristic.
  //if(pTXCharacteristic->canRead()) {
    std::string value = pTXCharacteristic->readValue();
    DPRINTF("The characteristic value is currently: ");
    DPRINTLN(value.c_str());
  //}

  // Obtain a reference to the RX characteristic of the Nordic UART service on the remote BLE server.
  pRXCharacteristic = pRemoteService->getCharacteristic(charUUID_RX);
  if (pRXCharacteristic == nullptr) {
    DPRINTF("Failed to find our characteristic UUID: ");
    DPRINTLN(charUUID_RX.toString().c_str());
    return false;
  }
  DPRINTLNF(" - Remote BLE RX characteristic reference established");

  // Read the value of the characteristic.
  if(pRXCharacteristic->canRead()) {
      value = pRXCharacteristic->readValue();
      DPRINTF("The characteristic value was: ");
      DPRINTLN(value.c_str());
  }

  if(pTXCharacteristic->canNotify())
      pTXCharacteristic->registerForNotify(notifyCallback);

  // UUID is 32 bytes, msg is 41 total. Any less and the reply for the Name msg will be suppressed. Weird, probably a radio bug.
  pRXCharacteristic->writeValue(CIV_ID0, sizeof(CIV_ID0));
  pRXCharacteristic->canNotify();
  vTaskDelay(20);

  // Device name must be 16 bytes exactly, msg total 21 bytes.
  pRXCharacteristic->writeValue(CIV_ID1, sizeof(CIV_ID1));
  pRXCharacteristic->canNotify();
  vTaskDelay(20);  // a small delay was required or this message would be missed (collision likely).

  // Send Token - A fixed value 9 bytes long total.
  uint8_t CIV_ID2[] = {0xFE, 0xF1, 0x00, 0x63, 0xEE, 0x39, 0x09, 0x10, 0xFD}; // Send Token
  pRXCharacteristic->writeValue(CIV_ID2, 9);
  pRXCharacteristic->canNotify();
  vTaskDelay(20);

// if pairing,  get reply with       0xFE, 0xF1, 0x00, 0x62, 0xFD        // no reply when reconnecting
// if pairing,  get reply with       0xFE, 0xF1, 0x00, 0x63, 0x01, 0xFD  // pair confirmed
// if already paired, get reply with 0xFE, 0xF1, 0x00, 0x63, 0x00, 0xFD  // already paired
// if all good, will get reply with  0xFE, 0xF1, 0x00, 0x64, 0xFD        // CI-V bus access granted

//String helloValue = "Hello Remote Server";
  //pRXCharacteristic->writeValue(helloValue.c_str(), helloValue.length());
  pTXCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
  //connected = true;
  return true;
}

void printDeviceAddress() {
  const uint8_t* point = esp_bt_dev_get_address();
 
  for (int i = 0; i < 6; i++) {
    char str[3];
 
    sprintf(str, "%02X", (int)point[i]);
    DPRINT(str);
 
    if (i < 5){
      DPRINTF(":");
    }
  }
  DPRINTLNF("");
}

/**************************************************************************************
   Scan for BLE servers and find the first one that advertises the Nordic UART service.
***************************************************************************************/
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    /**
        Called for each advertising BLE server.
    */
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      DPRINTF("BLE Advertised Device found - ");
      DPRINTLN(advertisedDevice.toString().c_str());

      // We have found a device, check to see if it contains the Nordic UART service.
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(serviceUUID)) {

        DPRINTLNF("Found a device with the desired ServiceUUID!");
        advertisedDevice.getScan()->stop();

        pServerAddress = new BLEAddress(advertisedDevice.getAddress());
        doConnect = true;  // Trigger to process server connection

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
  DPRINTF("Devices found: "); DPRINTLN(foundDevices.getCount());
  DPRINTLNF("Scan done!");
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
}

//--------------------------------------------                                                //
// BLE 
// SendMessage by BLE Slow in packets of 20 chars
// or fast in one long string.
// Fast can be used in IOS app BLESerial Pro
//------------------------------
void SendMessageBLE(uint8_t Message[], uint8_t len)
{
 if(connected) 
   {
    //if (Mem.UseBLELongString)                                                                 // If Fast transmission is possible
      pRXCharacteristic->writeValue(Message, len, false); //.c_str()); 
      pRXCharacteristic->canNotify();
      //delay(10);                                                                              // Bluetooth stack will go into congestion, if too many packets are sent
     
     /*} 
    else                                                                                       // Packets of max 20 bytes
     {   
      int parts = (Message.length()/20) + 1;
      for(int n=0;n<parts;n++)
        {   
         pRXCharacteristic->writeValue(Message.substr(n*20, 20).c_str()); 
         pRXCharacteristic->canNotify();
         //delay(40);                                                                           // Bluetooth stack will go into congestion, if too many packets are sent
        }
     }*/
   } 
}

void BLE_Setup(void)
{
  BLEDevice::init("");
  doConnect = false;
}

// If the flag "doConnect" is true then we have scanned for and found the desired
// BLE Server with which we wish to connect.  Now we connect to it.  Once we are
// connected we set the connected flag to be true.
void BLE_loop(void)
{
  static uint32_t time_freq = 0;
  static uint32_t time_tx = 0;

  if (doConnect == true) 
  {
    if (connectToServer(*pServerAddress)) 
    {
      //DPRINTLNF("We are now connected to the BLE Server.");
      // should have CIV results by now, see if we have access to the CI-V bus or not
      Serial.printf("doConnect1 - post CIV_Connect - connected = %d  Token = %d  CIV_Granted = %d  BLE_connected = %d\n", connected, Token_confirm, CIV_granted, BLE_connected);

      if (Token_confirm)
          ;

      if (Pairing_Accepted && Token_confirm)
          DPRINTLNF("Pairing_Accepted");  // only get this whe a pairing is request.  false on regular sign in

      if (!Pairing_Accepted && Token_confirm) // continue on
          DPRINTLNF("Paired already, continue to sign in");
      
      if (!BLE_connected && CIV_granted)
      {
        connected = true;   // tracks CIV layer state can be connected to BLE but not CIV (not paired for example)
        BLE_connected = true;  // tracks CIV layer more glabal than the connected var, almost redundant
        DPRINTLNF("BLE Connected");
      }
      onoff = true;  // want to read from the radio
      //   Can toggle notifications for the TX Characteristic on and off.
      //   Update the RX characteristic with the current time since boot string.
      if (onoff) {
        //DPRINTLNF("Notifications turned on");
        pTXCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
      } else {
        //DPRINTLNF("Notifications turned off");
        pTXCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOff, 2, true);
      }
    } 
    else 
    {
      DPRINTLNF("We have failed to connect to the server; there is nothin more we will do.");
      //draw_initial_screen();
      //M5.Lcd.setTextColor(text_color); //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535        
      //M5.Lcd.setCursor(5, 80); //Set the location of the cursor to the coordinates X and Y
      //M5.Lcd.printf("Failed Connection to Radio");
      connected = false;
    }
    doConnect = false;
  }

  if (pClient != nullptr && !pClient->isConnected())
  {
    if (BLE_connected)
      DPRINTLNF("BLE not connected to radio");   // detect if we are not connected
    //delay(100);
    connected = false;
    BLE_connected = false;
  }

  if(connected)
  {
    /*
    if (millis() >= time_freq + POLL_RADIO_FREQ)   // poll every X ms
    {
      //DPRINTLNF("Poll radio");
      uint8_t CIV_frequency[] = {0xFE, 0xFE, radio_address, 0xE0, 0x03, 0xFD};
      pRXCharacteristic->writeValue(CIV_frequency, sizeof(CIV_frequency), false);
      //pRXCharacteristic->writeValue(&cmds[CIV_C_F_READ].cmdData[1], cmds[CIV_C_F_READ].cmdData[0], true);
      pRXCharacteristic->canNotify();
      //delay(10);
      time_freq = millis();
    }
    
    if (millis() >= time_tx + POLL_PTT_DEFAULT)   // poll every X ms
    {
        //DPRINTLNF("Poll radio for TX/RX state");
      uint8_t CIV_TX[] = {0xFE, 0xFE, radio_address, 0xE0, 0x1C, 0x00, 0xFD};
      pRXCharacteristic->writeValue(CIV_TX, sizeof(CIV_TX), false);
      //pRXCharacteristic->writeValue(CIV_C_TX, cmds[CIV_C_TX].cmdData[0], true);
      pRXCharacteristic->canNotify();
      //delay(10);
      time_tx = millis();
    }
    */
  }// if connected
  else 
  {
    Scan_BLE_Servers();
  }
} // End of loop

#endif
#endif

