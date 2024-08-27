// Band Decoder Input and Output.  Customize the IO pin patterns for band
// Borrowed from my IC-905 Band Decoder dev project, applying it to the 4in/8out module example code.
#ifndef _DECODER_
#define _DECODER_

#include "Decoder.h"
#include "MODULE_4IN8OUT.h"
#include <Wire.h>
#include "DebugPrint.h"
#include "M5Stack_CI-V_Band_Decoder.h"

MODULE_4IN8OUT module;

#ifdef RELAY2
  #if defined ( CORE3 )   // Use Port A OK
    // Core 3 Port A
    #define pinR1 2   // 32 P.a  works on core 2, PB 26 works on M5Core
    #define pinR2 1   // 33 P.a  works on Core2.  Pb 36 does not work in M5Core
    // Core 3 Port B
    //#define pinR1 9   // 32 P.a  works on core 2, PB 26 works on M5Core
    //#define pinR2 8   // 33 P.a  works on Core2.  Pb 36 does not work in M5Core
    // Core 3 Port C
    //#define pinR1 18   // 32 P.a  works on core 2, PB 26 works on M5Core
    //#define pinR2 17   // 33 P.a  works on Core2.  Pb 36 does not work in M5Core

    #elif defined ( CORE2 )
    // Core 2 Port A
    #define pinR1 32   // 32 P.a  works on core 2, PB 26 works on M5Core
    #define pinR2 33   // 33 P.a  works on Core2.  Pb 36 does not work in M5Core
    // Core 2 Port B
    //#define pinR1 26   // 32 P.a  works on core 2, PB 26 works on M5Core
    //#define pinR2 36   // 33 P.a  works on Core2.  Pb 36 does not work in M5Core
    // Core 2 Port C
    //#define pinR1 13   // 32 P.a  works on core 2, PB 26 works on M5Core
    //#define pinR2 14   // 33 P.a  works on Core2.  Pb 36 does not work in M5Core

    #else 
    // Use Port C preferred
    // Core basic Port Be
    //#define pinR1 26   // 32 P.a  works on core 2, PB 26 works on M5Core
    //#define pinR2 36   // 33 P.a  works on Core2.  Pb 36 does not work in M5Core
    // Core basic Port C
    #define pinR1 16   // 32 P.a  works on core 2, PB 26 works on M5Core
    #define pinR2 17   // 33 P.a  works on Core2.  Pb 36 does not work in M5Core
  #endif // CPU type
#endif // relay2

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
    
    //#ifndef IO_MODULE
    //  DPRINTLNF("GPIO_Out: IO_MODULE **NOT** enabled");
      //return;
    //#endif

    // Enabled relay when any XVTR band active - test for now with 2 channel relay module
    #if defined ( RELAY2 )
      if (pattern) {
        digitalWrite(pinR2, HIGH); 
        DPRINTLNF("Relay+++++++++XVTR_ON");  
      }
      else {
        digitalWrite(pinR2, LOW);
        DPRINTLNF("Relay---------XVTR_OFF");  
      }
    #endif

    //pattern = !pattern;

    // mask each bit and apply the 1 or 0 to the assigned pin
    if (BAND_DECODE_OUTPUT_PIN_0 != GPIO_PIN_NOT_USED) module.setOutput(BAND_DECODE_OUTPUT_PIN_0, (pattern & 0x01) ? 1 : 0);  // bit 0
    if (BAND_DECODE_OUTPUT_PIN_1 != GPIO_PIN_NOT_USED) module.setOutput(BAND_DECODE_OUTPUT_PIN_1, (pattern & 0x02) ? 1 : 0);  // bit 1
    if (BAND_DECODE_OUTPUT_PIN_2 != GPIO_PIN_NOT_USED) module.setOutput(BAND_DECODE_OUTPUT_PIN_2, (pattern & 0x04) ? 1 : 0);  // bit 2
    if (BAND_DECODE_OUTPUT_PIN_3 != GPIO_PIN_NOT_USED) module.setOutput(BAND_DECODE_OUTPUT_PIN_3, (pattern & 0x08) ? 1 : 0);  // bit 3
    if (BAND_DECODE_OUTPUT_PIN_4 != GPIO_PIN_NOT_USED) module.setOutput(BAND_DECODE_OUTPUT_PIN_4, (pattern & 0x10) ? 1 : 0);  // bit 4
    if (BAND_DECODE_OUTPUT_PIN_5 != GPIO_PIN_NOT_USED) module.setOutput(BAND_DECODE_OUTPUT_PIN_5, (pattern & 0x20) ? 1 : 0);  // bit 5
    if (BAND_DECODE_OUTPUT_PIN_6 != GPIO_PIN_NOT_USED) module.setOutput(BAND_DECODE_OUTPUT_PIN_6, (pattern & 0x40) ? 1 : 0);  // bit 6
    if (BAND_DECODE_OUTPUT_PIN_7 != GPIO_PIN_NOT_USED) module.setOutput(BAND_DECODE_OUTPUT_PIN_7, (pattern & 0x80) ? 1 : 0);  // bit 7
}

void PTT_Output(uint8_t band, bool PTT_state)
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

void GPIO_PTT_Out(uint8_t pattern, bool _PTT_state)
{   
    uint8_t PTT_state = _PTT_state ? 0xFF : 0;

    DPRINTF("  PTT state "); DPRINT(_PTT_state, BIN);
    DPRINTF("  PTT Output Binary "); DPRINTLN(pattern, BIN);

    //#ifndef IO_MODULE
    //  DPRINTLNF("  GPIO_PTT_Out: IO_MODULE **NOT** Enabled");
      //return;
    //#endif

    // Operate a relay anytime PT is high. Probably not useful if have more than 1 PTT but is here for a test
    // This is a M5Stack 1 rleay Unit.  The 1 and 2 relay units oeprate on a grove port (B or C) with simple digital IO.  
    // On a Care, Port A is the same pins as the rest of the internal i2c, operating a relay on them would screw up I2C likely.
    // The 4 relay unit is i2c addressed and normally plugs into Port A on the Corexx itself. 
    // I2C on Port B is a separate IC2 port for the Core2 and Core3

    #if defined ( RELAY2 )
      if (PTT_state && pattern) {
        digitalWrite(pinR1, HIGH); 
        DPRINTLNF("Relay+++++++++PTT");
      }
      else {
        digitalWrite(pinR1, LOW);
        DPRINTLNF("Relay---------PTT");
      }
    #endif

    //PTT_state = !PTT_state;  // Invert  PTT 1 = TX, IO needs 0 to gnd for TX.
    
    //Serial.println((pattern & 0x10 & PTT_state) ? 0 : 1);
    
    // mask each bit and apply the 1 or 0 to the assigned pin
    if (BAND_DECODE_PTT_OUTPUT_PIN_0 != GPIO_PIN_NOT_USED) {module.setOutput(BAND_DECODE_PTT_OUTPUT_PIN_0, (pattern & 0x01 & PTT_state) ? 1 : 0);}  // bit 0
    if (BAND_DECODE_PTT_OUTPUT_PIN_1 != GPIO_PIN_NOT_USED) {module.setOutput(BAND_DECODE_PTT_OUTPUT_PIN_1, (pattern & 0x02 & PTT_state) ? 1 : 1);}  // bit 1
    if (BAND_DECODE_PTT_OUTPUT_PIN_2 != GPIO_PIN_NOT_USED) {module.setOutput(BAND_DECODE_PTT_OUTPUT_PIN_2, (pattern & 0x04 & PTT_state) ? 1 : 0);}  // bit 2
    if (BAND_DECODE_PTT_OUTPUT_PIN_3 != GPIO_PIN_NOT_USED) {module.setOutput(BAND_DECODE_PTT_OUTPUT_PIN_3, (pattern & 0x08 & PTT_state) ? 1 : 0);}  // bit 3
    if (BAND_DECODE_PTT_OUTPUT_PIN_4 != GPIO_PIN_NOT_USED) {module.setOutput(BAND_DECODE_PTT_OUTPUT_PIN_4, (pattern & 0x10 & PTT_state) ? 1 : 0);}  // bit 4
    if (BAND_DECODE_PTT_OUTPUT_PIN_5 != GPIO_PIN_NOT_USED) {module.setOutput(BAND_DECODE_PTT_OUTPUT_PIN_5, (pattern & 0x20 & PTT_state) ? 1 : 0);}  // bit 5
    if (BAND_DECODE_PTT_OUTPUT_PIN_6 != GPIO_PIN_NOT_USED) {module.setOutput(BAND_DECODE_PTT_OUTPUT_PIN_6, (pattern & 0x40 & PTT_state) ? 1 : 0);}  // bit 6
    if (BAND_DECODE_PTT_OUTPUT_PIN_7 != GPIO_PIN_NOT_USED) {module.setOutput(BAND_DECODE_PTT_OUTPUT_PIN_7, (pattern & 0x80 & PTT_state) ? 1 : 0);}  // bit 7
}

/*  Not used for the 4-In/8-Out module, all are fixed direction
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
*/

//MODULE_4IN8OUT module;  // done in main ino.   Be sure toi run Wire(21,22); before this in setup(0)
void  Module_4in_8out_setup()
{
    uint8_t counter = 0;

    #if defined ( RELAY2 )
      // For the 1 or 2-Relay modules if used.  Plug into Port C.  Port B only 1 pin 26 worked con the Core Basic.  
      // Port A pins are internal i2c and would conflict with relay usage
      pinMode(pinR1, OUTPUT);  // Set pin to output mode.
      pinMode(pinR2, OUTPUT);  // Set pin to output mode.
    #endif

    #ifdef CONFIG_IDF_TARGET_ESP32S3
      Serial.println("Decoder: CoreS3 i2c pins used");
      while (!module.begin(&Wire1, 12, 11, MODULE_4IN8OUT_ADDR) && counter < 4) {  // for cores3
    #else
      while (!module.begin(&Wire, 21, 22, MODULE_4IN8OUT_ADDR) && counter < 4) {  //for core basic
    #endif
      Serial.println("4IN8OUT INIT ERROR, Check Module is plugged in tight!");
        //M5.Lcd.drawString("4IN8OUT INIT ERROR", 5, 20, 4);
        //M5.Lcd.drawString("Check IO module is plugged in!", 5, 40, 4);
        delay(10);
        counter++;
      }
  if (counter < 4)
      Serial.println("4IN8OUT INIT Success");
}

long interval = 0;
bool level    = false;


//.setOutput(BAND_DECODE_PTT_OUTPUT_PIN_0, pattern & 0x01 & PTT_state);
uint8_t Module_4in_8out_Input_scan(void) 
{
  uint8_t pattern = 0;
  pattern |= module.getInput(0) ; 
  pattern |= module.getInput(1) << 1;
  pattern |= module.getInput(2) << 2;
  pattern |= module.getInput(3) << 3;
  return pattern & 0x0F;
}

#endif  // DECODER FILE