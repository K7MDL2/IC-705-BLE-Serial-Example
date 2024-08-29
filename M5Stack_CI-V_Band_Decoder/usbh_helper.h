/*********************************************************************
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 Copyright (c) 2019 Ha Thach for Adafruit Industries
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#ifndef USBH_HELPER_H
#define USBH_HELPER_H

  #include <M5_Max3421E_Usb.h>

  #if defined(CFG_TUH_MAX3421) && CFG_TUH_MAX3421
    // USB Host using MAX3421E: SPI, CS, INT
    #include "SPI.h"

    // Default CS and INT are pin 5, 35, these are with G34 INT switch set on USB host board
    #ifdef CONFIG_IDF_TARGET_ESP32S3
        M5_USBH_Host USBHost(&SPI, 36, 37, 35, 1, 10);  // Core3   default INT switch G35 position
      //M5_USBH_Host USBHost(&SPI, 36, 37, 35, 1, 14);  // Core3 Alt INT  G34 position
    #elif defined ( ARDUINO_M5STACK_CORE2 ) || defined ( ARDUINO_M5STACK_Core2 )  // maybe library confusing CS pins with display in places.
        M5_USBH_Host USBHost(&SPI, 18, 23, 38, 33, 35);  // Core2 default, INT G35 position  SD card uses INT=4, display INT=5 TOUCh=39 
      //M5_USBH_Host USBHost(&SPI, 18, 23, 38, 33, 34);  // Corealt , INT G34 position
    #else
      //default pins
        M5_USBH_Host USBHost(&SPI, 18, 23, 19, 5, 35);  // Core basic default, INT G35 position
      //M5_USBH_Host USBHost(&SPI, 18, 23, 19, 5, 34);  // Core basic  match to your DIP USB module switches  34 is used for Mic data
    #endif
  #endif  // CFG_TUH_MAX3421
#endif  // USBH_HELPER_H
