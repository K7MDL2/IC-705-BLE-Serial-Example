/*********************************************************************
      ICOM CI-V BLE Band Decoder and PTT breakout - M5Stamp-C3U version (headless)
      K7MDL 8/2024

      BT BLE Serial connection for IC-705.   
      
      Runs on M5Stamp C3U

      Intended use scenario is to be a CI-V serial bus USB to Bluetooth BLE converter for non-BT enabled USB equipped ICOM radios

      For now this program does these main things:
        1. Pass through in both direction Radio CI-V data if a PC is connect
           This enables an optional USB PC connection for things like logging and WSJT-X).
        2. Monitor Radio Frequency (active VFO) and display it, adding any band offset the band decoder input or User Input selects
           This enables Transverter selection, enable a band specific amplifier, or change antenna relays, all per band.
        3. Monitor PTT and pass it through to a selected GPIO pin which in turn controls a port on the 4In/8Out M5Stack module.
           This enables PTT breakout per band
        4. Optionally connect to the radio by USB Host or by BT BLE (IC-705 only so far)
        5. Radios tested so far - IC-705 and IC-905
        6. i2c can be brought out on a connector to drive peripherals like relays

        Down the road, probably on BLE mesh, this could bridge a radio to a PC completely by BLE.

      
*********************************************************************/

#include "M5StampC3U_Decoder.h"
#include "DebugPrint.h"

// Language ID: English
#define LANGUAGE_ID 0x0409

#ifdef _PC_PASSTHRU   // This is unused code, saving for reference
// forward Serial <-> SerialHost
void forward_serial(void) {
  uint8_t buf[64];
  //Serial.print("^");
  // Serial -> SerialHost
  if (Serial.available()) {
    size_t count = Serial.read(buf, sizeof(buf));
    if (SerialHost && SerialHost.connected()) {
      SerialHost.write(buf, count);
      SerialHost.flush();
    }
  }

  // SerialHost -> Serial
  if (SerialHost.connected() && SerialHost.available()) {
    size_t count = SerialHost.read(buf, sizeof(buf));
    Serial.write(buf, count);
    Serial.flush();
  }
}
#endif

extern void   BLE_Setup();
extern void   BLE_loop();
extern void   app_loop();
extern void   Scan_BLE_Servers();
extern void   app_setup();
bool BtnA_pressed = false;
bool BtnB_pressed = false;
bool BtnC_pressed = false;
uint64_t frequency = 0;
bool USBH_connected = false;
bool restart_BT_flag = false;
//
//   Main Setup for ESP32
//
void setup() {

  Serial.begin(115200);
  while ( !Serial ) delay(10);   // wait for native usb
  
  app_setup();  // setup app stuff
  
  #ifdef BLE
    //BLE_Setup();
    //Scan_BLE_Servers();
  #endif

  Serial.println("Setup Done");
}

//*****************************************************************************
//
// Main Loop.  Call app_setup() to run main application
//
//*****************************************************************************

void loop() {
  
  static int32_t loop_time = 0;
  static int32_t loop_max_time = 0;
  int32_t loop_time_threshold = 30;
  static int32_t prev_loop_time = 0;

  loop_time = millis();  // watermark
  
  #ifdef _PC_PASSTHRU   // Unused code, save for reference or test
  // allow a PC to talk o teh radio and opposite.  Need debug shuto    ff typically
    forward_serial();
  #endif
  
  //Serial.print(".");  // causes crashes on PC due to too high data rate
  
  #ifdef BLE
    BLE_loop();
  #endif

    app_loop();   // call to application main loop - moved to FreeRTOS task
  
    // Measure our current and max loop times
  int32_t temp_time = millis() - loop_time;  // current loop duration

  if ((temp_time > loop_max_time) || temp_time > loop_time_threshold) {
    if (temp_time > loop_max_time)
      loop_max_time = temp_time;
    //Serial.print("!");   // Turn on to see RTOS scheduling time allocated visually
    if (loop_max_time > loop_time_threshold) {
      Serial.printf("! loop time > %d  current time = %d  max time seen %d\n", loop_time_threshold, temp_time, loop_max_time);
      //Serial.println(" App loop time > 500!");

      //if (loop_max_time > 3000 && SerialBT.isClosed() && SerialBT.isReady())
      //  restart_BT(); // try this as a USBHost lockup failover short of having the btn task
    }  //    delete and restart the app task, or even the USBHost task
    else {
        ;  // nothing
    }
  }
}
