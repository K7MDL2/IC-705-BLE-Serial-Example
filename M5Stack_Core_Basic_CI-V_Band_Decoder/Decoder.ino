// Band Decoder Input and Output.  Customize the IO pin patterns for band
// Borrowed from my IC-905 Band Decoder dev project, applying it to the 4in/8out module example code.
#ifndef _DECODER_
#define _DECODER_

#ifdef SKIP

#include "MODULE_4IN8OUT.h"
#include <Wire.h>
#include "CIV.h"

// Make IO Pin assignments here. 
// These are 8 MOSFET outputs on the 4-In/8-Out module
#define GPIO_PIN_NOT_USED   255
#define GPIO_MOD_O_PIN_0    0
#define GPIO_MOD_O_PIN_1    1
#define GPIO_MOD_O_PIN_2    2
#define GPIO_MOD_O_PIN_3    3
#define GPIO_MOD_O_PIN_4    4
#define GPIO_MOD_O_PIN_5    5
#define GPIO_MOD_O_PIN_6    6
#define GPIO_MOD_O_PIN_7    7

// These are te 4 input pins on the IO module
#define GPIO_MOD_I_PIN_0    0
#define GPIO_MOD_I_PIN_1    1
#define GPIO_MOD_I_PIN_2    2
#define GPIO_MOD_I_PIN_3    3


// BAND DECODE OUTPUT PINS
// Assign your pins of choice.  Use a number or one of the existing #define number names
// Make sure they are not monitored by the code as a button or other use like an encoder.
// If not used set to GPIO_PIN_NOT_USED since there is no pin 255.
#define GPIO_PIN_NOT_USED                255

#define BAND_DECODE_OUTPUT_PIN_0        GPIO_MOD_O_PIN_0      // bit 0
#define BAND_DECODE_OUTPUT_PIN_1        GPIO_MOD_O_PIN_1      // bit 1
#define BAND_DECODE_OUTPUT_PIN_2        GPIO_MOD_O_PIN_2      // bit 2
#define BAND_DECODE_OUTPUT_PIN_3        GPIO_MOD_O_PIN_3      // bit 3
#define BAND_DECODE_OUTPUT_PIN_4        GPIO_PIN_NOT_USED      // bit 4
#define BAND_DECODE_OUTPUT_PIN_5        GPIO_PIN_NOT_USED      // bit 5
#define BAND_DECODE_OUTPUT_PIN_6        GPIO_PIN_NOT_USED      // bit 6
#define BAND_DECODE_OUTPUT_PIN_7        GPIO_PIN_NOT_USED      // bit 7

// BAND DECODE PTT OUTPUT PINS
// Assign your pins of choice.  Use a number or one of the existing #define number names
// Make sure they are not monitored by the code as a button or other use like an encoder.
// If not used set to GPIO_PIN_NOT_USED since there is no pin 255.
#define BAND_DECODE_PTT_OUTPUT_PIN_0    GPIO_PIN_NOT_USED     // bit 0
#define BAND_DECODE_PTT_OUTPUT_PIN_1    GPIO_PIN_NOT_USED     // bit 1
#define BAND_DECODE_PTT_OUTPUT_PIN_2    GPIO_PIN_NOT_USED     // bit 2
#define BAND_DECODE_PTT_OUTPUT_PIN_3    GPIO_PIN_NOT_USED     // bit 3
#define BAND_DECODE_PTT_OUTPUT_PIN_4    GPIO_MOD_O_PIN_4     // bit 4
#define BAND_DECODE_PTT_OUTPUT_PIN_5    GPIO_MOD_O_PIN_5   // bit 5
#define BAND_DECODE_PTT_OUTPUT_PIN_6    GPIO_MOD_O_PIN_6   // bit 6
#define BAND_DECODE_PTT_OUTPUT_PIN_7    GPIO_MOD_O_PIN_7   // bit 7

// *************************************************************************************************************
//
//   Below are fixed assignments.  User assignments are above
//
//**************************************************************************************************************
// Band Decode Output patterns.
// By default using BCD pattern following the Elecraft K3 HF-TRN table.  5 bits are used. Bit 4 =1 is VHF+ group
#define DECODE_BANDAM       (0x1F)   //160M 
#define DECODE_BAND160M     (0x01)   //160M 
#define DECODE_BAND80M      (0x02)    //80M
#define DECODE_BAND60M      (0x00)    //60M
#define DECODE_BAND40M      (0x03)    //40M
#define DECODE_BAND30M      (0x04)    //30M
#define DECODE_BAND20M      (0x05)    //20M
#define DECODE_BAND17M      (0x06)    //17M      
#define DECODE_BAND15M      (0x07)    //15M
#define DECODE_BAND12M      (0x08)    //12M
#define DECODE_BAND10M      (0x09)    //10M
#define DECODE_BAND6M       (0x0A)    //6M
#define DECODE_BANDFM       (0x1F)    //6M
#define DECODE_BANDAIR      (0x1F)    //6M
//#define DECODE_BAND70       (0x01)    //70MHz
#define DECODE_BAND144      (0x12)    //2M
#define DECODE_BAND222      (0x13)    //222
#define DECODE_BAND432      (0x14)    //432
#define DECODE_BAND902      (0x15)    //902
#define DECODE_BAND1296     (0x16)    //1296
#define DECODE_BAND2400     (0x17)    //2400
#define DECODE_BAND3400     (0x18)    //3400
#define DECODE_BAND5760     (0x19)    //5760M
#define DECODE_BAND10G      (0x1A)    //10.368.1G
#define DECODE_BAND24G      (0x1B)    //24.192G
#define DECODE_BAND47G      (0x1C)    //47.1G
#define DECODE_BAND76G      (0x1D)    //76.1G
#define DECODE_BAND122G     (0x1E)    //122G
#define DECODE_B_GENERAL    (0x1F)     // Non-Ham Band

// Band Decode Output patterns.
// By default using BCD pattern following the Elecraft K3 HF-TRN table.  5 bits are used. Bit 4 =1 is VHF+ group
#define DECODE_BANDAM_PTT       (0x1F)   //160M_PTT 
#define DECODE_BAND160M_PTT     (0x01)   //160M_PTT 
#define DECODE_BAND80M_PTT      (0x02)    //80M_PTT
#define DECODE_BAND60M_PTT      (0x00)    //60M_PTT
#define DECODE_BAND40M_PTT      (0x03)    //40M_PTT
#define DECODE_BAND30M_PTT      (0x04)    //30M_PTT
#define DECODE_BAND20M_PTT      (0x05)    //20M_PTT
#define DECODE_BAND17M_PTT      (0x06)    //17M_PTT      
#define DECODE_BAND15M_PTT      (0x07)    //15M_PTT
#define DECODE_BAND12M_PTT      (0x08)    //12M_PTT
#define DECODE_BAND10M_PTT      (0x09)    //10M_PTT
#define DECODE_BAND6M_PTT       (0x0A)    //6M_PTT
#define DECODE_BANDFM_PTT       (0x1F)    //6M_PTT
#define DECODE_BANDAIR_PTT      (0x1F)    //6M_PTT
//#define DECODE_BAND70       (0x01)    //70M_PTTHz
#define DECODE_BAND144_PTT      (0x12)    //2M_PTT
#define DECODE_BAND222_PTT      (0x13)    //222_PTT
#define DECODE_BAND432_PTT      (0x14)    //432_PTT
#define DECODE_BAND902_PTT      (0x15)    //902_PTT
#define DECODE_BAND1296_PTT     (0x16)    //1296_PTT
#define DECODE_BAND2400_PTT     (0x17)    //2400_PTT
#define DECODE_BAND3400_PTT     (0x18)    //3400_PTT
#define DECODE_BAND5760_PTT     (0x19)    //5760_PTT
#define DECODE_BAND10G_PTT      (0x1A)    //10.368.1G_PTT
#define DECODE_BAND24G_PTT      (0x1B)    //24.192G_PTT
#define DECODE_BAND47G_PTT      (0x1C)    //47.1G_PTT
#define DECODE_BAND76G_PTT      (0x1D)    //76.1G_PTT
#define DECODE_BAND122G_PTT     (0x1E)    //122G_PTT
#define DECODE_B_GENERAL_PTT    (0x1F)     // Non-Ham Band


// Very basic - outputs a set pattern for each band.  Follows the Elecraft K3 patther for combined HF and VHF used for transverters and antenna switching
// This may control a external band decoder that accept wired inputs.  Other decoder outpout can be serial or ethernet
void Band_Decode_Output(uint8_t band)
{
    // Convert frequency band to a parallel wire GPIO output pattern.
    // On an Elecraft K3 this is equivalent to the HF-TRN mode.  DigOut is used in combo with Band Decode BCD 0-3 pins.  
    // The pattern 0xYYXX where YY is 01 for VHF+ band group and 00 for HF band group.  XX is the band identifier witin each HF and VHF group.
    // Set your desired patterns in RadioConfig.h
    // ToDo: Eventually create a local UI screen to edit and monitor pin states

    DPRINTF("Band_Decode_Output: Band: "); DPRINTLN(band);

    switch (band)
    {
        case  BAND_AM   : GPIO_Out(DECODE_BANDAM);   break;   //160M 
        case  BAND_160M : GPIO_Out(DECODE_BAND160M); break;   //160M 
        case  BAND_80M  : GPIO_Out(DECODE_BAND80M);  break;   //80M
        case  BAND_60M  : GPIO_Out(DECODE_BAND60M);  break;   //60M
        case  BAND_40M  : GPIO_Out(DECODE_BAND40M);  break;   //40M
        case  BAND_30M  : GPIO_Out(DECODE_BAND30M);  break;   //30M
        case  BAND_20M  : GPIO_Out(DECODE_BAND20M);  break;   //20M
        case  BAND_17M  : GPIO_Out(DECODE_BAND17M);  break;   //17M      
        case  BAND_15M  : GPIO_Out(DECODE_BAND15M);  break;   //15M
        case  BAND_12M  : GPIO_Out(DECODE_BAND12M);  break;   //12M
        case  BAND_10M  : GPIO_Out(DECODE_BAND10M);  break;   //10M
        case  BAND_6M   : GPIO_Out(DECODE_BAND6M);   break;   //6M
        case  BAND_FM   : GPIO_Out(DECODE_BANDFM);   break;   //6M
        case  BAND_AIR  : GPIO_Out(DECODE_BANDAIR);  break;   //6M
        
        //case BAND70   : GPIO_Out(0x01); break;   //6M
        case  BAND_2M   : GPIO_Out(DECODE_BAND144);  break;   //2M
        case  BAND_1_25M: GPIO_Out(DECODE_BAND222);  break;   //222
        case  BAND_70cm : GPIO_Out(DECODE_BAND432);  break;   //432
        case  BAND_33cm : GPIO_Out(DECODE_BAND902);  break;   //902
        case  BAND_23cm : GPIO_Out(DECODE_BAND1296); break;   //1296
        case  BAND_13cm : GPIO_Out(DECODE_BAND2400); break;   //2400
        case  BAND_9cm  : GPIO_Out(DECODE_BAND3400); break;   //3400
        case  BAND_6cm  : GPIO_Out(DECODE_BAND5760); break;   //5760M
        case  BAND_3cm  : GPIO_Out(DECODE_BAND10G);  break;   //10.368.1G
        case  BAND_24G  : GPIO_Out(DECODE_BAND24G);  break;   //24.192G
        case  BAND_47G  : GPIO_Out(DECODE_BAND47G);  break;   //47.1G
        case  BAND_76G  : GPIO_Out(DECODE_BAND76G);  break;   //76.1G
        case  BAND_122G : GPIO_Out(DECODE_BAND122G); break;   //122G
        case  B_GENERAL : GPIO_Out(DECODE_B_GENERAL);  break;   //122G
    }
}

void GPIO_Out(uint8_t pattern)
{
    DPRINTF("GPIO_Out: pattern:  DEC "); DPRINT(pattern);
    DPRINTF("  HEX "); DPRINT(pattern, HEX);
    DPRINTF("  Binary "); DPRINTLN(pattern, BIN);

    // mask each bit and apply the 1 or 0 to the assigned pin
    if (BAND_DECODE_OUTPUT_PIN_0 != GPIO_PIN_NOT_USED) digitalWrite(BAND_DECODE_OUTPUT_PIN_0, pattern & 0x01);  // bit 0
    if (BAND_DECODE_OUTPUT_PIN_1 != GPIO_PIN_NOT_USED) digitalWrite(BAND_DECODE_OUTPUT_PIN_1, pattern & 0x02);  // bit 1
    if (BAND_DECODE_OUTPUT_PIN_2 != GPIO_PIN_NOT_USED) digitalWrite(BAND_DECODE_OUTPUT_PIN_2, pattern & 0x04);  // bit 2
    if (BAND_DECODE_OUTPUT_PIN_3 != GPIO_PIN_NOT_USED) digitalWrite(BAND_DECODE_OUTPUT_PIN_3, pattern & 0x08);  // bit 3
    if (BAND_DECODE_OUTPUT_PIN_4 != GPIO_PIN_NOT_USED) digitalWrite(BAND_DECODE_OUTPUT_PIN_4, pattern & 0x10);  // bit 4
    if (BAND_DECODE_OUTPUT_PIN_5 != GPIO_PIN_NOT_USED) digitalWrite(BAND_DECODE_OUTPUT_PIN_5, pattern & 0x20);  // bit 5
    if (BAND_DECODE_OUTPUT_PIN_6 != GPIO_PIN_NOT_USED) digitalWrite(BAND_DECODE_OUTPUT_PIN_6, pattern & 0x40);  // bit 6
    if (BAND_DECODE_OUTPUT_PIN_7 != GPIO_PIN_NOT_USED) digitalWrite(BAND_DECODE_OUTPUT_PIN_7, pattern & 0x80);  // bit 7
}

void PTT_Output(uint8_t band, uint8_t PTT_state)
{
    // Set your desired PTT pattern per band in RadioConfig.h
    // ToDo: Eventually create a local UI screen to edit and monitor pin states

    DPRINTF("Band_Decode_Output: Band: "); DPRINTLN(band);

    switch (band)
    {
        case  BAND_AM   : GPIO_PTT_Out(DECODE_BANDAM_PTT,   PTT_state); break;  //160M 
        case  BAND_160M : GPIO_PTT_Out(DECODE_BAND160M_PTT, PTT_state); break;  //160M 
        case  BAND_80M  : GPIO_PTT_Out(DECODE_BAND80M_PTT,  PTT_state); break;   //80M
        case  BAND_60M  : GPIO_PTT_Out(DECODE_BAND60M_PTT,  PTT_state); break;   //60M
        case  BAND_40M  : GPIO_PTT_Out(DECODE_BAND40M_PTT,  PTT_state); break;   //40M
        case  BAND_30M  : GPIO_PTT_Out(DECODE_BAND30M_PTT,  PTT_state); break;   //30M
        case  BAND_20M  : GPIO_PTT_Out(DECODE_BAND20M_PTT,  PTT_state); break;   //20M
        case  BAND_17M  : GPIO_PTT_Out(DECODE_BAND17M_PTT,  PTT_state); break;   //17M      
        case  BAND_15M  : GPIO_PTT_Out(DECODE_BAND15M_PTT,  PTT_state); break;   //15M
        case  BAND_12M  : GPIO_PTT_Out(DECODE_BAND12M_PTT,  PTT_state); break;   //12M
        case  BAND_10M  : GPIO_PTT_Out(DECODE_BAND10M_PTT,  PTT_state); break;   //10M
        case  BAND_6M   : GPIO_PTT_Out(DECODE_BAND6M_PTT,   PTT_state); break;    //6M
        case  BAND_FM   : GPIO_PTT_Out(DECODE_BANDFM_PTT,   PTT_state); break;    //6M
        case  BAND_AIR  : GPIO_PTT_Out(DECODE_BANDAIR_PTT,  PTT_state); break;    //6M
        //case BAND70   : GPIO_PTT_Out(0x01_PTT, PTT_state); break;   //6M
        case  BAND_2M   : GPIO_PTT_Out(DECODE_BAND144_PTT,  PTT_state); break;   //2M
        case  BAND_1_25M: GPIO_PTT_Out(DECODE_BAND222_PTT,  PTT_state); break;   //222
        case  BAND_70cm : GPIO_PTT_Out(DECODE_BAND432_PTT,  PTT_state); break;   //432
        case  BAND_33cm : GPIO_PTT_Out(DECODE_BAND902_PTT,  PTT_state); break;   //902
        case  BAND_23cm : GPIO_PTT_Out(DECODE_BAND1296_PTT, PTT_state); break;  //1296
        case  BAND_13cm : GPIO_PTT_Out(DECODE_BAND2400_PTT, PTT_state); break;  //2400
        case  BAND_9cm  : GPIO_PTT_Out(DECODE_BAND3400_PTT, PTT_state); break;  //3400
        case  BAND_6cm  : GPIO_PTT_Out(DECODE_BAND5760_PTT, PTT_state); break;  //5760M
        case  BAND_3cm  : GPIO_PTT_Out(DECODE_BAND10G_PTT,  PTT_state); break;   //10.368.1G
        case  BAND_24G  : GPIO_PTT_Out(DECODE_BAND24G_PTT,  PTT_state); break;   //24.192G
        case  BAND_47G  : GPIO_PTT_Out(DECODE_BAND47G_PTT,  PTT_state); break;   //47.1G
        case  BAND_76G  : GPIO_PTT_Out(DECODE_BAND76G_PTT,  PTT_state); break;   //76.1G
        case  BAND_122G : GPIO_PTT_Out(DECODE_BAND122G_PTT, PTT_state); break;  //122G
        case  B_GENERAL : GPIO_PTT_Out(DECODE_B_GENERAL_PTT,PTT_state); break;  //Any other band not in the list
        }
}

void GPIO_PTT_Out(uint8_t pattern, uint8_t PTT_state)
{
    DPRINTF("  PTT state "); DPRINT(PTT_state, BIN);
    DPRINTF("  PTT Output Binary "); DPRINTLN(pattern & PTT_state, BIN);

    // mask each bit and apply the 1 or 0 to the assigned pin
    if (BAND_DECODE_PTT_OUTPUT_PIN_0 != GPIO_PIN_NOT_USED) digitalWrite(BAND_DECODE_PTT_OUTPUT_PIN_0, pattern & 0x01 & PTT_state);  // bit 0
    if (BAND_DECODE_PTT_OUTPUT_PIN_1 != GPIO_PIN_NOT_USED) digitalWrite(BAND_DECODE_PTT_OUTPUT_PIN_1, pattern & 0x02 & PTT_state);  // bit 1
    if (BAND_DECODE_PTT_OUTPUT_PIN_2 != GPIO_PIN_NOT_USED) digitalWrite(BAND_DECODE_PTT_OUTPUT_PIN_2, pattern & 0x04 & PTT_state);  // bit 2
    if (BAND_DECODE_PTT_OUTPUT_PIN_3 != GPIO_PIN_NOT_USED) digitalWrite(BAND_DECODE_PTT_OUTPUT_PIN_3, pattern & 0x08 & PTT_state);  // bit 3
    if (BAND_DECODE_PTT_OUTPUT_PIN_4 != GPIO_PIN_NOT_USED) digitalWrite(BAND_DECODE_PTT_OUTPUT_PIN_4, pattern & 0x10 & PTT_state);  // bit 4
    if (BAND_DECODE_PTT_OUTPUT_PIN_5 != GPIO_PIN_NOT_USED) digitalWrite(BAND_DECODE_PTT_OUTPUT_PIN_5, pattern & 0x20 & PTT_state);  // bit 5
    if (BAND_DECODE_PTT_OUTPUT_PIN_6 != GPIO_PIN_NOT_USED) digitalWrite(BAND_DECODE_PTT_OUTPUT_PIN_6, pattern & 0x40 & PTT_state);  // bit 6
    if (BAND_DECODE_PTT_OUTPUT_PIN_7 != GPIO_PIN_NOT_USED) digitalWrite(BAND_DECODE_PTT_OUTPUT_PIN_7, pattern & 0x80 & PTT_state);  // bit 7
}

void Decoder_GPIO_Pin_Setup(void)
{
    // using 8 bits since the ouput pattern is 1 byte.  Can use thenm any way you want. 
    // The pins used here are defined in RadioConfig.  The one GPIO_SWx_PIN were designated as hardware switches in the Teensy SDR 
    // If using the Teensy SDR motherboard and you have physical switch hardware on any of these then you need to pick alernate pins.
    // Most pins are alrewady goiven a #define bname in RadioCOnfig, substitute the right ones in here.  Make sure they are free.

    // set up our Decoder output pins if enabled
    if (BAND_DECODE_OUTPUT_PIN_0 != GPIO_PIN_NOT_USED) pinMode(BAND_DECODE_OUTPUT_PIN_0, OUTPUT);  // bit 0
    if (BAND_DECODE_OUTPUT_PIN_1 != GPIO_PIN_NOT_USED) pinMode(BAND_DECODE_OUTPUT_PIN_1, OUTPUT);  // bit 1
    if (BAND_DECODE_OUTPUT_PIN_2 != GPIO_PIN_NOT_USED) pinMode(BAND_DECODE_OUTPUT_PIN_2, OUTPUT);  // bit 2
    if (BAND_DECODE_OUTPUT_PIN_3 != GPIO_PIN_NOT_USED) pinMode(BAND_DECODE_OUTPUT_PIN_3, OUTPUT);  // bit 3
    if (BAND_DECODE_OUTPUT_PIN_4 != GPIO_PIN_NOT_USED) pinMode(BAND_DECODE_OUTPUT_PIN_4, OUTPUT);  // bit 4
    if (BAND_DECODE_OUTPUT_PIN_5 != GPIO_PIN_NOT_USED) pinMode(BAND_DECODE_OUTPUT_PIN_5, OUTPUT);  // bit 5
    if (BAND_DECODE_OUTPUT_PIN_6 != GPIO_PIN_NOT_USED) pinMode(BAND_DECODE_OUTPUT_PIN_6, OUTPUT);  // bit 6
    if (BAND_DECODE_OUTPUT_PIN_7 != GPIO_PIN_NOT_USED) pinMode(BAND_DECODE_OUTPUT_PIN_7, OUTPUT);  // bit 7
    
    // set up our PTT breakout pins if enabled
    if (BAND_DECODE_PTT_OUTPUT_PIN_0 != GPIO_PIN_NOT_USED) pinMode(BAND_DECODE_PTT_OUTPUT_PIN_0, OUTPUT);  // bit 0
    if (BAND_DECODE_PTT_OUTPUT_PIN_1 != GPIO_PIN_NOT_USED) pinMode(BAND_DECODE_PTT_OUTPUT_PIN_1, OUTPUT);  // bit 1
    if (BAND_DECODE_PTT_OUTPUT_PIN_2 != GPIO_PIN_NOT_USED) pinMode(BAND_DECODE_PTT_OUTPUT_PIN_2, OUTPUT);  // bit 2
    if (BAND_DECODE_PTT_OUTPUT_PIN_3 != GPIO_PIN_NOT_USED) pinMode(BAND_DECODE_PTT_OUTPUT_PIN_3, OUTPUT);  // bit 3
    if (BAND_DECODE_PTT_OUTPUT_PIN_4 != GPIO_PIN_NOT_USED) pinMode(BAND_DECODE_PTT_OUTPUT_PIN_4, OUTPUT);  // bit 4
    if (BAND_DECODE_PTT_OUTPUT_PIN_5 != GPIO_PIN_NOT_USED) pinMode(BAND_DECODE_PTT_OUTPUT_PIN_5, OUTPUT);  // bit 5
    if (BAND_DECODE_PTT_OUTPUT_PIN_6 != GPIO_PIN_NOT_USED) pinMode(BAND_DECODE_PTT_OUTPUT_PIN_6, OUTPUT);  // bit 6
    if (BAND_DECODE_PTT_OUTPUT_PIN_7 != GPIO_PIN_NOT_USED) pinMode(BAND_DECODE_PTT_OUTPUT_PIN_7, OUTPUT);  // bit 7
     
    DPRINTLNF("Decoder_GPIO_Pin_Setup: Pin Mode Setup complete");
}

//MODULE_4IN8OUT module;  // done in main ino

void  Module_4in_8out_setup() {

  return;
     //while (!module.begin(&Wire, 21, 22, MODULE_4IN8OUT_ADDR)) {  for core basic
    while (!module.begin(&Wire, 12, 11, MODULE_4IN8OUT_ADDR)) {  // fopr cores3
        DPRINTLNF("4IN8OUT INIT ERROR");
        M5.Lcd.println("4IN8OUT INIT ERROR");
       delay(1000);
    };
    DPRINTLNF("4IN8OUT INIT SUCCESS");
}

long interval = 0;
bool level    = false;

void Module_4in_8out_loop() 
{

  return;

    for (uint8_t i = 0; i < 4; i++) 
    {
        if (module.getInput(i) == 1) 
        {
        if(i){
                //M5.Lcd.fillRect(60 + 60 * i, 0, 25, 25, TFT_BLACK);
                //M5.Lcd.fillRect(60 + 60 * i, 0, 25, 25, TFT_BLUE);
            } else {
                //M5.Lcd.fillRect(60 + 60 * i, 0, 25, 25, TFT_BLACK);
                //M5.Lcd.drawRect(60 + 60 * i, 0, 25, 25, TFT_BLUE);
            }
            //M5.Lcd.drawString("IN" + String(i), 40 + 60 * i, 5);
        }
        //M5.Lcd.drawString("4IN8OUT MODULE", 60, 80, 4);
        //M5.Lcd.drawString("FW VERSION:" + String(module.getVersion()), 70, 120, 4);
        if (millis() - interval > 1000) 
        {
            interval = millis();
            level    = !level;
            for (uint8_t i = 0; i < 8; i++) {
              module.setOutput(i, level);
                if (level) {
                    //M5.Lcd.fillRect(20 + 35 * i, 200, 25, 25, TFT_BLACK);
                    //M5.Lcd.fillRect(20 + 35 * i, 200, 25, 25, TFT_GREEN);
                } else {
                    //M5.Lcd.fillRect(20 + 35 * i, 200, 25, 25, TFT_BLACK);
                    //M5.Lcd.drawRect(20 + 35 * i, 200, 25, 25, TFT_GREEN);
                }
                //M5.Lcd.drawString("OUT" + String(i), 18 + 35 * i, 180);
                //delay(50);
            }
        }
    } 
    M5.update();
}

#endif
#endif  // DECODER FILE