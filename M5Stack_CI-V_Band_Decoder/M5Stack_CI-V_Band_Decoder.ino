/*********************************************************************
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
        1. Pass through in both direction Radio CI-V data if a PC is connect - Not finished
           This enables an optional USB PC connection for things like logging and WSJT-X).
        2. Monitor Radio Frequency (active VFO) and display it, adding any band offset the band decoder input or User Input selects
           This enables Transverter selection, enable a band specific amplifier, or change antenna relays, all per band.
        3. Monitor PTT and pass it through to a selected GPIO pin which in turn controls a port on the 4In/8Out M5Stack module.
           This enables PTT breakout per band
        4. Optionally connect to the radio by USB Host or by BLE or BT Classic SPP (IC-705 only so far)
        5. Radios tested so far - IC-705 and IC-905
        6. Tested on M5Stack Core Basic, Core2 and CoreS3. 
        7. Can run with minor changes on M5StampC3U, M5StampS3, and M5AtomS3.

        Down the road, probably on BLE mesh, this could bridge a radio to a PC completely by BLE.

        The code on ths page is almost complete the serial_host_bridge.ino example file.  

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

// nRF52 and ESP32 use freeRTOS, we may need to run USBhost.task() in its own rtos's thread.
// Since USBHost.task() will put loop() into dormant state and prevent followed code from running
// until there is USB host event.

#include "M5Stack_CI-V_Band_Decoder.h"
#include "Decoder.h"
#include "DebugPrint.h"
#include "M5_Max3421E_Usb.h"

#if defined(ARDUINO_NRF52_ADAFRUIT) || defined(ARDUINO_ARCH_ESP32)
  #define USE_FREERTOS
#endif

// USBHost is defined in usbh_helper.h
#include "usbh_helper.h"

#ifdef USBHOST
// CDC Host object
Adafruit_USBH_CDC SerialHost;
#endif

// Language ID: English
#define LANGUAGE_ID 0x0409

typedef struct {
  tusb_desc_device_t desc_device;
  uint16_t manufacturer[32];
  uint16_t product[48];
  uint16_t serial[16];
  bool mounted;
} dev_info_t;

// CFG_TUH_DEVICE_MAX is defined by tusb_config header
dev_info_t dev_info[CFG_TUH_DEVICE_MAX] = { 0 };

#ifdef PC_PASSTHRU_USBHOST   // This is unused code, saving for reference
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

#if defined(CFG_TUH_MAX3421) && CFG_TUH_MAX3421
//--------------------------------------------------------------------+
// Using Host shield MAX3421E controller
//--------------------------------------------------------------------+
//#define CFG_TUD_COUNT 2
#endif

#ifdef USE_FREERTOS

#ifdef ARDUINO_ARCH_ESP32
  #define USBH_STACK_SZ 8000
#else
  #define USBH_STACK_SZ 200
#endif

TaskHandle_t xHandle = NULL;

void usbhost_rtos_task(void *param) {
 (void) param;
  while (1) {
    //Serial.print("+");
    USBHost.task(10, false);
    //vTaskDelay(10);
     // test for stack size
    uint32_t stack_sz;
    stack_sz = uxTaskGetStackHighWaterMark( NULL );
    if (stack_sz < 1000)
      Serial.printf("\n  #######   USB Host Loop: Stack Size Low Space Warning < 1000 words left free:  %lu\n",stack_sz);
  }
  vTaskDelete(NULL);
}

extern void app_loop();

void app_loop_rtos_task(void *param) {
 (void) param;
  while (1) {
    app_loop();
    //Serial.print("\nA");
    //vTaskDelay(12);
    // test for stack size
    uint32_t stack_sz;
    stack_sz = uxTaskGetStackHighWaterMark( NULL );
    if (stack_sz < 1000)
      Serial.printf("\n  ^^^^^^^^^^^  app_loop: Stack Size Low Space Warning < 1000 words left free:  %lu\n",stack_sz);
  }
  vTaskDelete(NULL);
}

#ifdef BLE
extern void BLE_loop();

void BLE_loop_rtos_task(void *param) {
 (void) param;
  while (1) {
    BLE_loop();
    //Serial.print("\nT");
    //vTaskDelay(12);
    // test for stack size
    uint32_t stack_sz;
    stack_sz = uxTaskGetStackHighWaterMark( NULL );
    if (stack_sz < 1000)
      Serial.printf("\n  ^^^^^^^^^^^  BLE_loop: Stack Size Low Space Warning < 1000 words left free:  %lu\n",stack_sz);
  }
  vTaskDelete(NULL);
}
#endif // BLE


void btn_loop_rtos_task(void *param) {
 (void) param;
  while (1) {
    chk_btns();
    //Serial.print("\nT");
    //vTaskDelay(12);
    // test for stack size
    uint32_t stack_sz;
    stack_sz = uxTaskGetStackHighWaterMark( NULL );
    if (stack_sz < 1000)
      Serial.printf("\n  ^^^^^^^^^^^  chk_btns: Stack Size Low Space Warning < 1000 words left free:  %lu\n",stack_sz);
  }
  vTaskDelete(NULL);
}


#endif  // FREERTOS

extern void   BLE_Setup();
extern void   Scan_BLE_Servers();
extern void   app_setup();
uint8_t USBHost_ready = 2;  // 0 = not mounted.  1 = mounted, 2 = system not initialized
bool USBH_connected = false;
bool restart_USBH_flag = false;
bool restart_BT_flag = false;
bool BtnA_pressed = false;
bool BtnB_pressed = false;
bool BtnC_pressed = false;
uint64_t frequency = 0;
uint8_t board_type = 0;

void chk_btns(void) {
  #if defined ( CONFIG_IDF_TARGET_ESP32S3 ) && !defined ( __M5GFX_M5ATOMDISPLAY__ )
    auto touchPoint = M5.Touch.getDetail(); 
    if (prev_state != touchPoint.state) {
        prev_state = touchPoint.state;
    }
    if ((M5.Display.height() > touchPoint.y &&
         touchPoint.y > M5.Display.height() - 40) &&
        touchPoint.state == m5::touch_state_t::touch_begin) {
        if (M5.Display.width() / 3 > touchPoint.x && touchPoint.x > 0){
            BtnA_pressed = true;
            Serial.println("3A");
        }
        if ((M5.Display.width() / 3) * 2 > touchPoint.x &&
            touchPoint.x > M5.Display.width() / 3) {
            BtnB_pressed = true;
            Serial.println("3B");
        }
        if (M5.Display.width() > touchPoint.x &&
            touchPoint.x > (M5.Display.width() / 3) * 2) {
            BtnC_pressed = true;
            Serial.println("3C");
        }
    }   

  #elif !defined ( CORE2LIB ) && ( defined ( ARDUINO_M5STACK_CORE2 ) || defined ( ARDUINO_M5STACK_Core2 ) )
    int state = M5.BtnA.wasHold() ? 1
            : M5.BtnA.wasClicked() ? 2
            : M5.BtnA.wasPressed() ? 3
            : M5.BtnA.wasReleased() ? 4
            : M5.BtnA.wasDecideClickCount() ? 5
            : 0;
    if (state == 3 && !BtnA_pressed) {  // Only process once the main loop is done
      BtnA_pressed = true;
      Serial.print("2A, state = "); Serial.println(state);
    }
    
    state = M5.BtnB.wasHold() ? 1
          : M5.BtnB.wasClicked() ? 2
          : M5.BtnB.wasPressed() ? 3
          : M5.BtnB.wasReleased() ? 4
          : M5.BtnB.wasDecideClickCount() ? 5
          : 0;
    if (state == 3 && !BtnB_pressed) {
      BtnB_pressed = true;
      Serial.print("2B, state = "); Serial.println(state);
    }

    state = M5.BtnC.wasHold() ? 1
          : M5.BtnC.wasClicked() ? 2
          : M5.BtnC.wasPressed() ? 3
          : M5.BtnC.wasReleased() ? 4
          : M5.BtnC.wasDecideClickCount() ? 5
          : 0;
    if (state == 3 & !BtnC_pressed) {
      BtnC_pressed = true;
      Serial.print("2C, state = "); Serial.println(state);
    }
   
  #else
    if (//M5.BtnA.wasReleased() ||
     M5.BtnA.pressedFor(100, 3000)) {
        BtnA_pressed = true;
        Serial.println("A");
    } else if (//M5.BtnB.wasReleased() || 
    M5.BtnB.pressedFor(100, 3000)) {
        BtnB_pressed = true;
        Serial.println("B");
    } else if (//M5.BtnC.wasReleased() || 
    M5.BtnC.pressedFor(100, 3000)) {
        BtnC_pressed = true;
        Serial.println("C");
    }
  #endif
  }
//
//   Main Setup for ESP32
//
void setup() {
  Serial.begin(115200);
  while ( !Serial ) delay(10);   // wait for native usb

  #ifdef CONFIG_IDF_TARGET_ESP32S3
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Power.begin();
    #ifdef ATOMS3
      Serial.println("ATOMS3 defined")
      M5.Lcd.setBrightness(30);   // 0- 255
    #endif
    board_type = M5.getBoard();
    if (board_type == M5ATOMS3) {
      Serial.println("AtomS3 ext i2c pins defined");
      Wire.begin(2,1);   // M5AtomS3 external i2c
      M5.Lcd.setRotation(1);  // 0 to 3 rotate, 4 to 7 reverse and rotate.
    } else {
      Serial.println("CoreS3 or CoreS3SE ext i2C pins defined");
      Wire.begin(12,11);   // CoreS3 and ?StampC3U?
    }
    //M5.Touch.begin();
    auto ms = millis();
    if (M5.Touch.isEnabled())
      M5.Touch.update(ms);
    //M5.Power.setExtOutput(true);  // .powerModeSet(POWER_MODE_USB_IN_BUS_OUT);
    //SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);

  #elif defined ( ARDUINO_M5STACK_CORE2 ) || defined ( ARDUINO_M5STACK_Core2 )
    Serial.println("Core2 defined");
    Wire.begin(21,22);
    //SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
    
    #ifdef CORE2LIB
    // M5Core2.h stuff    USB Host sort of works, Touch Buttons not so much.
    M5.begin();
    M5.Axp.begin();
    Serial.print("Power Status 0=external, 1=internal 2=max  "); Serial.println(M5.Axp.isACIN() ? 0 : 1);
    M5.Touch.begin();
    //if (M5.Touch.isEnabled())
    M5.Touch.update();
    #else
    //// M5Unified.h stuff =  kills USB Host though, makes the touch buttons work.
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Power.begin();
    auto ms = millis();
    if (M5.Touch.isEnabled()) {
    M5.Touch.update(ms);
    }
    #endif

  #else
    Serial.println("Core Basic defined");
    M5.begin(); //(true, false, true, true);   // 2nd arg is enable SD card, off now.
    Wire.begin(21,22);
    //SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
  #endif
    
  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
  //SPI.setFrequency(SPI_FREQ);
  
  #ifdef USBHOST  
  // init host stack on controller (rhport) 1
  USBHost.begin(1);
  // Initialize SerialHost
  SerialHost.begin(115200);
  #endif

  #ifdef USE_FREERTOS
    #ifdef USBHOST
      // Create a task to run USBHost.task() in background
      xTaskCreatePinnedToCore(usbhost_rtos_task, "usbh", USBH_STACK_SZ, NULL, 4, NULL, 0);
      
      Serial.printf("USB pre-start status = %d\n", USBHost_ready);
      Serial.printf("   USBH_connected = %d\n",USBH_connected);
      int count = 0;
        while (USBHost_ready == 2 && count < 200)  // 0 of nothing, 1 for device connected. value started at 2 so we know init is done.
        {
          delay(10);
          Serial.print(count);
          count++;
        }
      Serial.printf("USB post-start status = %d\n", USBHost_ready);
      Serial.printf("   USBH_connected = %d\n", USBH_connected);
      
      app_setup();  // setup app stuff
      //xTaskCreatePinnedToCore(app_loop_rtos_task, "app", 8000, NULL, 3, &xHandle, 1);
      //xTaskCreate(app_loop_rtos_task, "app", 8000, NULL, 3, &xHandle); 
      //xTaskCreate(BLE_loop_rtos_task, "BLE", 8000, NULL, 2, &xHandle);   // callback on server ID not working inside a task
      //xTaskCreate(btn_loop_rtos_task, "btns", 8000, NULL, 2, &xHandle);   // callback on server ID not working inside a task
    #endif
  #endif

  #ifndef USBHOST
    app_setup();  // setup app stuff
  #endif
  
  #ifdef BLE
    BLE_Setup();
    Scan_BLE_Servers();
  #endif

  Serial.println("TinyUSB Host Serial Setup Done");
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
  
  M5.update();

  #ifndef USE_FREERTOS
    //USBHost.task(10,false);
    //USBHost.task();
  #endif

  chk_btns();
  
  #ifdef _PC_PASSTHRU   // Unused code, save for reference or test
  // allow a PC to talk o teh radio and opposite.  Need debug shuto    ff typically
    forward_serial();
  #endif
  
  //Serial.print(".");   causes crashes on PC due to too high data rate
  
  #ifdef BLE
    BLE_loop();
  #endif

  //#ifdef USBHOST
    app_loop();   // call to application main loop - moved to FreeRTOS task
  // #endif
  
    // Measure our current and max loop times
  int32_t temp_time = millis() - loop_time;  // current loop duration

  if ((temp_time > loop_max_time) || temp_time > loop_time_threshold) {
    if (temp_time > loop_max_time)
      loop_max_time = temp_time;
    //Serial.print("!");   // Turn on to see RTOS scheduling time allocated visually
    if (loop_max_time > loop_time_threshold) {
      //Serial.printf("! loop time > %d  current time = %d  max time seen %d\n", loop_time_threshold, temp_time, loop_max_time);
      //Serial.println(" App loop time > 500!");
      M5.Lcd.setTextDatum(ML_DATUM);
      M5.Lcd.setTextColor(WHITE, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
      M5.Lcd.drawString("!", 0, 0, 2);
      //if (loop_max_time > 3000 && SerialBT.isClosed() && SerialBT.isReady())
      //  restart_BT(); // try this as a USBHost lockup failover short of having the btn task
    }  //    delete and restart the app task, or even the USBHost task
    else {
      M5.Lcd.setTextDatum(ML_DATUM);                            // erase the marker
      M5.Lcd.setTextColor(background_color, background_color);  //Set the color of the text from 0 to 65535, and the background color behind it 0 to 65535
      M5.Lcd.drawString("!", 0, 0, 2);
    }
  }
  //uint32_t stack_sz;
  //stack_sz = uxTaskGetStackHighWaterMark( NULL );
  //if (stack_sz < 1000)
    //Serial.printf("\n  #######   App Loop: Stack Size Low Space Warning < 1000 words left free:  %lu\n",stack_sz);
}

void tuh_hid_report_sent_cb(uint8_t dev_addr, uint8_t idx,
                            uint8_t const* report, uint16_t len) {
}

//--------------------------------------------------------------------+
// TinyUSB Host callbacks
//--------------------------------------------------------------------+

// Invoked when a device with CDC interface is mounted
// idx is index of cdc interface in the internal pool.
void tuh_cdc_mount_cb(uint8_t idx) {
  // bind SerialHost object to this interface 
  #ifdef USBHOST
  SerialHost.mount(idx);
  #endif
  Serial.println("\n****** SerialHost is connected to a new CDC device");
  USBH_connected = true;
  frequency = 0;
  //BT_enabled= false;
  delay(1200);  // Delay first Tx or get a hang.
  //SerialHost.flush();
}

// Invoked when a device with CDC interface is unmounted
void tuh_cdc_umount_cb(uint8_t idx) {
  #ifdef USBHOST
  SerialHost.umount(idx);
  #endif
  Serial.println("\n****** SerialHost is disconnected");
  USBH_connected = false;
  frequency = 0;  // force the screen to update if reconnecting to same frequency
  //btConnected = false;
  //BT_enabled = true;
  ESP.restart();
}


#ifndef EXTRAS
//--------------------------------------------------------------------+
// TinyUSB Host callbacks
//--------------------------------------------------------------------+

void print_device_descriptor(tuh_xfer_t *xfer);

void utf16_to_utf8(uint16_t *temp_buf, size_t buf_len);

void print_lsusb(void) {
  bool no_device = true;
  for (uint8_t daddr = 1; daddr < CFG_TUH_DEVICE_MAX + 1; daddr++) {
    // TODO can use tuh_mounted(daddr), but tinyusb has an bug
    // use local connected flag instead
    dev_info_t *dev = &dev_info[daddr - 1];
    if (dev->mounted) {
      Serial.printf("Device %u: ID %04x:%04x %s %s\r\n", daddr,
                    dev->desc_device.idVendor, dev->desc_device.idProduct,
                    (char *) dev->manufacturer, (char *) dev->product);

      no_device = false;
    }
  }

  if (no_device) {
    Serial.println("No device connected (except hub)");  
  }
  USBHost_ready = (uint8_t) no_device;  // flag for setup to wait for USB Host to be ready.
}

//void tuh_hid_report_sent_cb(uint8_t dev_addr, uint8_t idx,
//                            uint8_t const* report, uint16_t len) {
//}

// Invoked when device is mounted (configured)
void tuh_mount_cb(uint8_t daddr) {
  Serial.printf("Device attached, address = %d\r\n", daddr);

  dev_info_t *dev = &dev_info[daddr - 1];
  dev->mounted = true;
  USBHost_ready = 1;  // flag for setup to wait for USB Host to be ready.
  
  // Get Device Descriptor
  tuh_descriptor_get_device(daddr, &dev->desc_device, 18, print_device_descriptor, 0);
}

/// Invoked when device is unmounted (bus reset/unplugged)
void tuh_umount_cb(uint8_t daddr) {
  Serial.printf("Device removed, address = %d\r\n", daddr);
  dev_info_t *dev = &dev_info[daddr - 1];
  dev->mounted = false;

  // print device summary
  print_lsusb();
}

void print_device_descriptor(tuh_xfer_t *xfer) {
  if (XFER_RESULT_SUCCESS != xfer->result) {
    Serial.printf("Failed to get device descriptor\r\n");
    return;
  }

  uint8_t const daddr = xfer->daddr;
  dev_info_t *dev = &dev_info[daddr - 1];
  tusb_desc_device_t *desc = &dev->desc_device;

  Serial.printf("Device %u: ID %04x:%04x\r\n", daddr, desc->idVendor, desc->idProduct);
  Serial.printf("Device Descriptor:\r\n");
  Serial.printf("  bLength             %u\r\n"     , desc->bLength);
  Serial.printf("  bDescriptorType     %u\r\n"     , desc->bDescriptorType);
  Serial.printf("  bcdUSB              %04x\r\n"   , desc->bcdUSB);
  Serial.printf("  bDeviceClass        %u\r\n"     , desc->bDeviceClass);
  Serial.printf("  bDeviceSubClass     %u\r\n"     , desc->bDeviceSubClass);
  Serial.printf("  bDeviceProtocol     %u\r\n"     , desc->bDeviceProtocol);
  Serial.printf("  bMaxPacketSize0     %u\r\n"     , desc->bMaxPacketSize0);
  Serial.printf("  idVendor            0x%04x\r\n" , desc->idVendor);
  Serial.printf("  idProduct           0x%04x\r\n" , desc->idProduct);
  Serial.printf("  bcdDevice           %04x\r\n"   , desc->bcdDevice);

  // Get String descriptor using Sync API
  Serial.printf("  iManufacturer       %u     ", desc->iManufacturer);
  if (XFER_RESULT_SUCCESS ==
      tuh_descriptor_get_manufacturer_string_sync(daddr, LANGUAGE_ID, dev->manufacturer, sizeof(dev->manufacturer))) {
    utf16_to_utf8(dev->manufacturer, sizeof(dev->manufacturer));
    Serial.printf((char *) dev->manufacturer);
  }
  Serial.printf("\r\n");

  Serial.printf("  iProduct            %u     ", desc->iProduct);
  if (XFER_RESULT_SUCCESS ==
      tuh_descriptor_get_product_string_sync(daddr, LANGUAGE_ID, dev->product, sizeof(dev->product))) {
    utf16_to_utf8(dev->product, sizeof(dev->product));
    Serial.printf((char *) dev->product);
  }
  Serial.printf("\r\n");

  Serial.printf("  iSerialNumber       %u     ", desc->iSerialNumber);
  if (XFER_RESULT_SUCCESS ==
      tuh_descriptor_get_serial_string_sync(daddr, LANGUAGE_ID, dev->serial, sizeof(dev->serial))) {
    utf16_to_utf8(dev->serial, sizeof(dev->serial));
    Serial.printf((char *) dev->serial);
  }
  Serial.printf("\r\n");

  Serial.printf("  bNumConfigurations  %u\r\n", desc->bNumConfigurations);

  // print device summary
  print_lsusb();
}

//--------------------------------------------------------------------+
// String Descriptor Helper
//--------------------------------------------------------------------+

static void _convert_utf16le_to_utf8(const uint16_t *utf16, size_t utf16_len, uint8_t *utf8, size_t utf8_len) {
  // TODO: Check for runover.
  (void) utf8_len;
  // Get the UTF-16 length out of the data itself.

  for (size_t i = 0; i < utf16_len; i++) {
    uint16_t chr = utf16[i];
    if (chr < 0x80) {
      *utf8++ = chr & 0xff;
    } else if (chr < 0x800) {
      *utf8++ = (uint8_t) (0xC0 | (chr >> 6 & 0x1F));
      *utf8++ = (uint8_t) (0x80 | (chr >> 0 & 0x3F));
    } else {
      // TODO: Verify surrogate.
      *utf8++ = (uint8_t) (0xE0 | (chr >> 12 & 0x0F));
      *utf8++ = (uint8_t) (0x80 | (chr >> 6 & 0x3F));
      *utf8++ = (uint8_t) (0x80 | (chr >> 0 & 0x3F));
    }
    // TODO: Handle UTF-16 code points that take two entries.
  }
}

// Count how many bytes a utf-16-le encoded string will take in utf-8.
static int _count_utf8_bytes(const uint16_t *buf, size_t len) {
  size_t total_bytes = 0;
  for (size_t i = 0; i < len; i++) {
    uint16_t chr = buf[i];
    if (chr < 0x80) {
      total_bytes += 1;
    } else if (chr < 0x800) {
      total_bytes += 2;
    } else {
      total_bytes += 3;
    }
    // TODO: Handle UTF-16 code points that take two entries.
  }
  return total_bytes;
}

void utf16_to_utf8(uint16_t *temp_buf, size_t buf_len) {
  size_t utf16_len = ((temp_buf[0] & 0xff) - 2) / sizeof(uint16_t);
  size_t utf8_len = _count_utf8_bytes(temp_buf + 1, utf16_len);

  _convert_utf16le_to_utf8(temp_buf + 1, utf16_len, (uint8_t *) temp_buf, buf_len);
  ((uint8_t *) temp_buf)[utf8_len] = '\0';
}

#endif  // extras
