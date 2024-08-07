/*************************************************************************
      ICOM CI-V Band Decoder and PTT breakout
      K7MDL 8/2024

      BT Classic Serial or USB Host connection for IC-705
      USB for other models 
      Runs on M5Stack Core Basic, possible Core2. Core3 has issues with USB Host compatibility as of Aug 2024.
      
      For USB Host uses the M5Stack USB V1.2 module.        
      The DIP switches for SPI INT and SS need to be set to 5 and 35 for the Core Basic. 
      Core3 pins are different and set in usbh_helper.h

      This is based on a mege between 2 other works, the serial_host_bridge.inbo in Arduino examples, and
      T1 Interface V2.0 (12.09.2023) by Peter Jonas DL5DLA

      This is the main setup focused on USB Host setup then launches the app setup and loop
        which in turn starts up BT if enabled.  The eventual goal is to have both services
        standing by and able to use one, the other , or both.  Possible flipping both BT and USB around.

      For now this program does these main things:
        1. Pass through in both direction Radio CI-V data if a PC is connect
           This enables an optional USB PC connection for things like logging and WSJT-X).
        2. Monitor Radio Frequency (active VFO) and display it, adding any band offset the band decoder input or User Input selects
           This enables Transverter selection, enable a band specific amplifier, or change antenna relays, all per band.
        3. Monitor PTT and pass it through to a selected GPIO pin which in turn controls a port on the 4In/8Out M5Stack module.
           This enables PTT breakout per band
        4. Optionally connect to the radio by USB Host or by BT Classic SPP (IC-705 only so far)
        5. Radios tested so far - IC-705 and IC-905

        Down the road, probably on BLE mesh, this could bridge a radio to a PC completely by BLE.

        The code on ths page is almost cmplete the serial_host_bridge.ino example file.  

        I placed a hook into setup and loop for each architecture. I am only testing this on the M5Stack ESP32 models today 
          though it may be possble to run on other CPUs like the pico some day, so I left all that in place
          and put all the app code in other files.  There be need to ever edit this file.  Same for usbh_helper.h
          except you may need to edit the SPI pins for different model CPU such as the Core3 or a Core 2 maybe.

*********************************************************************
*********************************************************************
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 Copyright (c) 2019 Ha Thach for Adafruit Industries
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

/* This example demonstrates use of both device and host, where
 * - Device run on native usb controller (roothub port0)
 * - Host depending on MCUs run on either:
 *   - rp2040: bit-banging 2 GPIOs with the help of Pico-PIO-USB library (roothub port1)
 *   - samd21/51, nrf52840, esp32: using MAX3421e controller (host shield)
 *
 * Requirements:
 * - For rp2040:
 *   - [Pico-PIO-USB](https://github.com/sekigon-gonnoc/Pico-PIO-USB) library
 *   - 2 consecutive GPIOs: D+ is defined by PIN_USB_HOST_DP, D- = D+ +1
 *   - Provide VBus (5v) and GND for peripheral
 *   - CPU Speed must be either 120 or 240 Mhz. Selected via "Menu -> CPU Speed"
 * - For samd21/51, nrf52840, esp32:
 *   - Additional MAX2341e USB Host shield or featherwing is required
 *   - SPI instance, CS pin, INT pin are correctly configured in usbh_helper.h
 */

/* This example demonstrates use of Host Serial (CDC). SerialHost (declared below) is
 * an object to manage an CDC peripheral connected to our USB Host connector. This example
 * will forward all characters from Serial to SerialHost and vice versa.
 */

#define DEBUG_SERIAL   // shuts off USB host messages to the terminal during dev.

// nRF52 and ESP32 use freeRTOS, we may need to run USBhost.task() in its own rtos's thread.
// Since USBHost.task() will put loop() into dormant state and preUSBH_connectedvent followed code from running
// until there is USB host event.
#if defined(ARDUINO_NRF52_ADAFRUIT) || defined(ARDUINO_ARCH_ESP32)
  #define USE_FREERTOS
#endif

#include <Arduino.h>
#include <M5Stack.h>
#include "BluetoothSerial.h"
// USBHost is defined in usbh_helper.h
#include "usbh_helper.h"

extern void app_loop(void);
extern void app_setup(void);

bool USBH_connected = false;

// CDC Host object
Adafruit_USBH_CDC SerialHost;

// Will eventually need a message broker that coordinates our radio requests with any from an externel PC.  
// This would require picking offany replies to our messages and not forward them t the PC 
//    to not interfere with WSJT-X by giving it unexpected reponses.

// forward Seral <-> SerialHost
void forward_serial(void) {
  uint8_t buf[64];

  #ifndef DEBUG_SERIAL
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
    
    // normally we want to pass on the radio to a [possibly conencted PC but for dev and debug we want ot shut this off
    // Also will want to share a common read buffer
   
      Serial.write(buf, count);
      Serial.flush();
  }
  #endif
}

#if defined(CFG_TUH_MAX3421) && CFG_TUH_MAX3421
//--------------------------------------------------------------------+
// Using Host shield MAX3421E controller
//--------------------------------------------------------------------+

#ifdef USE_FREERTOS

#ifdef ARDUINO_ARCH_ESP32
  #define USBH_STACK_SZ 2048
#else
  #define USBH_STACK_SZ 200
#endif

void usbhost_rtos_task(void *param) {
  (void) param;
  while (1) {
    USBHost.task();
  }
}
#endif

//
//   Main Setup for ESP32
//
void setup() {
  Serial.begin(115200);

  // init host stack on controller (rhport) 1
  USBHost.begin(1);

  // Initialize SerialHost
  SerialHost.begin(115200);

#ifdef USE_FREERTOS
  // Create a task to run USBHost.task() in background
  xTaskCreate(usbhost_rtos_task, "usbh", USBH_STACK_SZ, NULL, 3, NULL);
#endif
  
//  while ( !Serial ) delay(10);   // wait for native usb
  Serial.println("TinyUSB Host Serial Setup Done");

  app_setup();  // call to our ESP32 app setup stuff
}


//*****************************************************************************
//
// Main Loop.  Call app_setup() to run main application
//
//*****************************************************************************
void loop() {

  #ifndef USE_FREERTOS
    USBHost.task();
  #endif

  forward_serial();  // leave this heare
  app_loop();       // call to our application main loop
}

#elif defined(ARDUINO_ARCH_RP2040)
//--------------------------------------------------------------------+
// For RP2040 use both core0 for device stack, core1 for host stack
//--------------------------------------------------------------------+

//------------- Core0 -------------//
void setup() 
{
  Serial.begin(115200);
  // while ( !Serial ) delay(10);   // wait for native usb
  Serial.println("TinyUSB Host Serial Echo Example");
  app_setup();   // call to our app setup stuff
}

void loop() 
{  
  forward_serial();
  app_loop();   // call to application main loop
}

//------------- Core1 -------------//
void setup1() {
  // configure pio-usb: defined in usbh_helper.h
  rp2040_configure_pio_usb();

  // run host stack on controller (rhport) 1
  // Note: For rp2040 pico-pio-usb, calling USBHost.begin() on core1 will have most of the
  // host bit-banging processing works done in core1 to free up core0 for other works
  USBHost.begin(1);

  // Initialize SerialHost
  SerialHost.begin(115200);
}

void loop1() {
  USBHost.task();
}

#endif

void tuh_hid_report_sent_cb(uint8_t dev_addr, uint8_t idx,
                            uint8_t const* report, uint16_t len) {
}

//--------------------------------------------------------------------+
// TinyUSB Host callbacks
//--------------------------------------------------------------------+

// Invoked when a device with CDC interface is mounted
// idx is index of cdc interface in the internal pool.
void tuh_cdc_mount_cb(uint8_t idx) {
  // bind SerialHost object to this interface index
  SerialHost.mount(idx);
  Serial.println("****** SerialHost is connected to a new CDC device");
  USBH_connected = true;
  delay(1200);  // Delay first Tx or get a hang.
}

// Invoked when a device with CDC interface is unmounted
void tuh_cdc_umount_cb(uint8_t idx) {
  SerialHost.umount(idx);
  Serial.println("****** SerialHost is disconnected");
  USBH_connected = false;
}

