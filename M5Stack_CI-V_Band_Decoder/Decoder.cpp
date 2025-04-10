// Band Decoder Input and Output.  Customize the IO pin patterns for band
// Borrowed from my IC-905 Band Decoder dev project, applying it to the 4in/8out module example code.
#ifndef _DECODER_
#define _DECODER_

#include "Decoder.h"
#include <Wire.h>
#include "DebugPrint.h"
#include "M5Stack_CI-V_Band_Decoder.h"

#ifdef M5STAMPC3U
  #include <Adafruit_MCP23X08.h>
  #include <Adafruit_MCP23X17.h>
  Adafruit_MCP23X17 mcp0;  // 1 of 2 modules, addr = 0x20  (A0, A1, A2 jumpers shorted)
  Adafruit_MCP23X17 mcp1;  // 2 of 2 modules, addr = 0x21 (A0 open, A1 and A2 jumpers shorted)
  // uses i2c on I/O pins 0 and 1 (Port A on C3U) set up at init time.
  extern Adafruit_NeoPixel pixel;
#endif // M5STAMPC3U

#ifdef IO_MODULE
  #include "MODULE_4IN8OUT.h"
  MODULE_4IN8OUT module;
#endif // IO_MODULE

#ifdef MODULE_4RELAY_13_2
  #include "Module_4RELAY.h"
  MODULE_4RELAY RELAY;
#endif // MODULE_4RELAY_13_2

#ifdef EXT_IO2_UNIT
  #include "M5_EXTIO2.h"
  M5_EXTIO2 extio; //EXT.IO2 8 port GPIO extender unit
  extio_io_mode_t mode = DIGITAL_INPUT_MODE;
#endif // EXT_IO2_UNIT

#ifdef RELAY4_UNIT
  #include "Unit_4RELAY.h"
  UNIT_4RELAY relay;
#endif // RELAY4_UNIT

#ifdef RELAY2_UNIT
  #if defined ( CORE3 )   // Use Port A OK
    // Core 3 Port A
    #define pinR1 2   // 32 P.a  works on core 2, PB 26 works on M5Core
    #define pinR2 1   // 33 P.a  works on Core2.  Pb 36 does not work in M5Core
    // Core 3 Port B
    //#define pinR1 8   // 32 P.a  works on core 2, PB 26 works on M5Core
    //#define pinR2 9   // 33 P.a  works on Core2.  Pb 36 does not work in M5Core
    // Core 3 Port C
    //#define pinR1 18   // 32 P.a  works on core 2, PB 26 works on M5Core
    //#define pinR2 17   // 33 P.a  works on Core2.  Pb 36 does not work in M5Core

    #elif defined ( CORE2 )
    // Core 2 Port A
    #define pinR1 32   // 32 P.a  works on core 2, PB 26 works on M5Core
    #define pinR2 33   // 33 P.a  works on Core2.  Pb 36 does not work in M5Core
    // Core 2 Port B
    //#define pinR1 36   //  
    //#define pinR2 26   //
    // Core 2 Port C
    //#define pinR1 13   // 32 P.a  works on core 2, PB 26 works on M5Core
    //#define pinR2 14   // 33 P.a  works on Core2.  Pb 36 does not work in M5Core

    #else 
    // Use Port C preferred
    // Core basic Port Be
    //#define pinR1 36   // 32 P.a  works on core 2, PB 36 does not work on M5Core
    //#define pinR2 26   // 33 P.a  works on Core2.  Pb 26 works in M5Core
    // Core basic Port C
    #define pinR1 16   // 32 P.a  works on core 2, PB 26 works on M5Core
    #define pinR2 17   // 33 P.a  works on Core2.  Pb 36 does not work in M5Core
  #endif // CPU type
#endif // relay2

// Very basic - outputs a set pattern for each band.  Follows the Elecraft K3 pattern for combined HF and VHF used for transverters and antenna switching
// This may control an external band decoder that accepts wired inputs.  Other decoder outputs can be serial or ethernet
void Band_Decode_Output(uint8_t band, bool IF_Switch_ON) // pass IF_Switch_ON value to GPIO_out for sequencing usage
{
    // Convert frequency band to a parallel wire GPIO output pattern.
    // On an Elecraft K3 this is equivalent to the HF-TRN mode.  DigOut is used in combo with Band Decode BCD 0-3 pins.  
    // The pattern 0xYYXX where YY is 01 for VHF+ band group and 00 for HF band group.  XX is the band identifier witin each HF and VHF group.
    // Set your desired patterns in RadioConfig.h
    // ToDo: Eventually create a local UI screen to edit and monitor pin states

    DPRINTF("Band_Decode_Output: Band "); DPRINTLN(band);

    //GPIO_Out(ALL_OFF);  // turn off devices to effect break before make

    switch (band)  // Now set them to the desired ON state
    {   
        case  DUMMY     : GPIO_Out(0,              IF_Switch_ON);  break;   //Dummy Band
        case  BAND_AM   : GPIO_Out(DECODE_BANDAM,  IF_Switch_ON);  break;   //AM
        case  BAND_160M : GPIO_Out(DECODE_BAND160M,IF_Switch_ON);  break;   //160M 
        case  BAND_80M  : GPIO_Out(DECODE_BAND80M, IF_Switch_ON);  break;   //80M
        case  BAND_60M  : GPIO_Out(DECODE_BAND60M, IF_Switch_ON);  break;   //60M
        case  BAND_40M  : GPIO_Out(DECODE_BAND40M, IF_Switch_ON);  break;   //40M
        case  BAND_30M  : GPIO_Out(DECODE_BAND30M, IF_Switch_ON);  break;   //30M
        case  BAND_20M  : GPIO_Out(DECODE_BAND20M, IF_Switch_ON);  break;   //20M
        case  BAND_17M  : GPIO_Out(DECODE_BAND17M, IF_Switch_ON);  break;   //17M      
        case  BAND_15M  : GPIO_Out(DECODE_BAND15M, IF_Switch_ON);  break;   //15M
        case  BAND_12M  : GPIO_Out(DECODE_BAND12M, IF_Switch_ON);  break;   //12M
        case  BAND_10M  : GPIO_Out(DECODE_BAND10M, IF_Switch_ON);  break;   //10M
        case  BAND_6M   : GPIO_Out(DECODE_BAND6M,  IF_Switch_ON);  break;   //6M
        case  BAND_FM   : GPIO_Out(DECODE_BANDFM,  IF_Switch_ON);  break;   //6M
        case  BAND_AIR  : GPIO_Out(DECODE_BANDAIR, IF_Switch_ON);  break;   //6M
        
        //case BAND70   : GPIO_Out(0x01, IF_Switch_ON, IF_Switch_ON);  break;   //4M
        case  BAND_2M   : GPIO_Out(DECODE_BAND144, IF_Switch_ON);  break;   //2M
        case  BAND_1_25M: GPIO_Out(DECODE_BAND222, IF_Switch_ON);  break;   //222
        case  BAND_70cm : GPIO_Out(DECODE_BAND432, IF_Switch_ON);  break;   //432
        case  BAND_33cm : GPIO_Out(DECODE_BAND902, IF_Switch_ON);  break;   //902
        case  BAND_23cm : GPIO_Out(DECODE_BAND1296,IF_Switch_ON);  break;   //1296
        case  BAND_13cm : GPIO_Out(DECODE_BAND2400,IF_Switch_ON);  break;   //2400
        case  BAND_9cm  : GPIO_Out(DECODE_BAND3300,IF_Switch_ON);  break;   //3400
        case  BAND_6cm  : GPIO_Out(DECODE_BAND5760,IF_Switch_ON);  break;   //5760M
        case  BAND_3cm  : GPIO_Out(DECODE_BAND10G, IF_Switch_ON);  break;   //10.368.1G
        case  BAND_24G  : GPIO_Out(DECODE_BAND24G, IF_Switch_ON);  break;   //24.192G
        case  BAND_47G  : GPIO_Out(DECODE_BAND47G, IF_Switch_ON);  break;   //47.1G
        case  BAND_76G  : GPIO_Out(DECODE_BAND76G, IF_Switch_ON);  break;   //76.1G
        case  BAND_122G : GPIO_Out(DECODE_BAND122G, IF_Switch_ON); break;   //122G
        case  B_GENERAL : GPIO_Out(DECODE_B_GENERAL,IF_Switch_ON); break;   //122G
    }
}

void GPIO_Out(uint16_t pattern, bool IF_Switch_ON) // pass IF_Switch_ON value to GPIO_out for sequencing usage)
{
    if (!IF_Switch_ON) {
      pattern |= 0x0007;  // modify lower 3 bits to turn IF switch OFF (all 1s) for sequencing
      DPRINTLNF(" IF Switch Set to Block RF");
    } else {
      DPRINTLNF(" IF Switch allowed to PASS RF if configured to do so (last 3 bits)");
    }
    
    DPRINTF("  GPIO_Out: pattern:  DEC "); DPRINT(pattern);
    DPRINTF("  HEX "); DPRINT(pattern, HEX);
    DPRINTF("  Binary "); DPRINTLN(pattern, BIN);
    
    //#ifndef IO_MODULE
    //  DPRINTLNF("GPIO_Out: IO_MODULE **NOT** enabled");
      //return;
    //#endif

    // Enabled relay when any XVTR band active - test for now with 2 channel relay module
    #ifdef RELAY2_UNIT
      if (pattern) {
        digitalWrite(pinR2, HIGH); 
        DPRINTLNF("Relay+++++++++XVTR_ON");  
      }
      else {
        digitalWrite(pinR2, LOW);
        DPRINTLNF("Relay---------XVTR_OFF");  
      }
    #endif

    #ifdef IO_MODULE
      // mask each bit and apply the 1 or 0 to the assigned pin
      if (BAND_DECODE_OUTPUT_0 != GPIO_PIN_NOT_USED) module.setOutput(BAND_DECODE_OUTPUT_0, (pattern & 0x0001) ? 1 : 0);  // bit 0
      if (BAND_DECODE_OUTPUT_1 != GPIO_PIN_NOT_USED) module.setOutput(BAND_DECODE_OUTPUT_1, (pattern & 0x0002) ? 1 : 0);  // bit 1
      if (BAND_DECODE_OUTPUT_2 != GPIO_PIN_NOT_USED) module.setOutput(BAND_DECODE_OUTPUT_2, (pattern & 0x0004) ? 1 : 0);  // bit 2
      if (BAND_DECODE_OUTPUT_3 != GPIO_PIN_NOT_USED) module.setOutput(BAND_DECODE_OUTPUT_3, (pattern & 0x0008) ? 1 : 0);  // bit 3
      if (BAND_DECODE_OUTPUT_4 != GPIO_PIN_NOT_USED) module.setOutput(BAND_DECODE_OUTPUT_4, (pattern & 0x0010) ? 1 : 0);  // bit 4
      if (BAND_DECODE_OUTPUT_5 != GPIO_PIN_NOT_USED) module.setOutput(BAND_DECODE_OUTPUT_5, (pattern & 0x0020) ? 1 : 0);  // bit 5
      if (BAND_DECODE_OUTPUT_6 != GPIO_PIN_NOT_USED) module.setOutput(BAND_DECODE_OUTPUT_6, (pattern & 0x0040) ? 1 : 0);  // bit 6
      if (BAND_DECODE_OUTPUT_7 != GPIO_PIN_NOT_USED) module.setOutput(BAND_DECODE_OUTPUT_7, (pattern & 0x0080) ? 1 : 0);  // bit 7

      //   Add 8 more porsts here if a 2nd module added.  Pins are already defined as GPIO_MOD2_MI_PIN_XX 
    #endif

    #ifdef MODULE_4RELAY_13_2
      // mask each bit and apply the 1 or 0 to the assigned pin
      if (BAND_DECODE_OUTPUT_0 != GPIO_PIN_NOT_USED) RELAY.setRelay(BAND_DECODE_OUTPUT_0, (pattern & 0x0001) ? 1 : 0);  // bit 0
      if (BAND_DECODE_OUTPUT_1 != GPIO_PIN_NOT_USED) RELAY.setRelay(BAND_DECODE_OUTPUT_1, (pattern & 0x0002) ? 1 : 0);  // bit 1
      if (BAND_DECODE_OUTPUT_2 != GPIO_PIN_NOT_USED) RELAY.setRelay(BAND_DECODE_OUTPUT_2, (pattern & 0x0004) ? 1 : 0);  // bit 2
      if (BAND_DECODE_OUTPUT_3 != GPIO_PIN_NOT_USED) RELAY.setRelay(BAND_DECODE_OUTPUT_3, (pattern & 0x0008) ? 1 : 0);  // bit 3
      // for second relay.  Need to change to RELAY2.setRelay form to make this work.  That requires initializing a 2nd module
      if (BAND_DECODE_OUTPUT_4 != GPIO_PIN_NOT_USED) RELAY.setRelay(BAND_DECODE_OUTPUT_4, (pattern & 0x0010) ? 1 : 0);  // bit 4
      if (BAND_DECODE_OUTPUT_5 != GPIO_PIN_NOT_USED) RELAY.setRelay(BAND_DECODE_OUTPUT_5, (pattern & 0x0020) ? 1 : 0);  // bit 5
      if (BAND_DECODE_OUTPUT_6 != GPIO_PIN_NOT_USED) RELAY.setRelay(BAND_DECODE_OUTPUT_6, (pattern & 0x0040) ? 1 : 0);  // bit 6
      if (BAND_DECODE_OUTPUT_7 != GPIO_PIN_NOT_USED) RELAY.setRelay(BAND_DECODE_OUTPUT_7, (pattern & 0x0080) ? 1 : 0);  // bit 7
    #endif
    
    #ifdef EXT_IO2_UNIT
      // mask each bit and apply the 1 or 0 to the assigned pin
      //if (BAND_DECODE_OUTPUT_0 != GPIO_PIN_NOT_USED) {extio.setPinMode(BAND_DECODE_PTT_OUTPUT_0, (pattern & 0x0001) ? 1 : 0);}  // bit 0
      //if (BAND_DECODE_OUTPUT_1 != GPIO_PIN_NOT_USED) {extio.setPinMode(BAND_DECODE_PTT_OUTPUT_1, (pattern & 0x0002) ? 1 : 0);}  // bit 1
      //if (BAND_DECODE_OUTPUT_2 != GPIO_PIN_NOT_USED) {extio.setPinMode(BAND_DECODE_PTT_OUTPUT_2, (pattern & 0x0004) ? 1 : 0);}  // bit 2
      //if (BAND_DECODE_OUTPUT_3 != GPIO_PIN_NOT_USED) {extio.setPinMode(BAND_DECODE_PTT_OUTPUT_3, (pattern & 0x0008) ? 1 : 0);}  // bit 3
      if (BAND_DECODE_OUTPUT_4 != GPIO_PIN_NOT_USED) {extio.setDigitalOutput(BAND_DECODE_OUTPUT_4, (pattern & 0x0010) ? 1 : 0);}  // bit 4
      if (BAND_DECODE_OUTPUT_5 != GPIO_PIN_NOT_USED) {extio.setDigitalOutput(BAND_DECODE_OUTPUT_5, (pattern & 0x0020) ? 1 : 0);}  // bit 5
      if (BAND_DECODE_OUTPUT_6 != GPIO_PIN_NOT_USED) {extio.setDigitalOutput(BAND_DECODE_OUTPUT_6, (pattern & 0x0040) ? 1 : 0);}  // bit 6
      if (BAND_DECODE_OUTPUT_7 != GPIO_PIN_NOT_USED) {extio.setDigitalOutput(BAND_DECODE_OUTPUT_7, (pattern & 0x0080) ? 1 : 0);}  // bit 7
    #endif

    #ifdef RELAY4_UNIT
      // mask each bit and apply the 1 or 0 to the assigned pin
      if (BAND_DECODE_OUTPUT_0 != GPIO_PIN_NOT_USED) relay.relayWrite(BAND_DECODE_OUTPUT_0, (pattern & 0x0001) ? 1 : 0);  // bit 0
      if (BAND_DECODE_OUTPUT_1 != GPIO_PIN_NOT_USED) relay.relayWrite(BAND_DECODE_OUTPUT_1, (pattern & 0x0002) ? 1 : 0);  // bit 1
      if (BAND_DECODE_OUTPUT_2 != GPIO_PIN_NOT_USED) relay.relayWrite(BAND_DECODE_OUTPUT_2, (pattern & 0x0004) ? 1 : 0);  // bit 2
      if (BAND_DECODE_OUTPUT_3 != GPIO_PIN_NOT_USED) relay.relayWrite(BAND_DECODE_OUTPUT_3, (pattern & 0x0008) ? 1 : 0);  // bit 3
      //if (BAND_DECODE_OUTPUT_4 != GPIO_PIN_NOT_USED) relay.relayWrite(BAND_DECODE_OUTPUT_4, (pattern & 0x0010) ? 1 : 0);  // bit 4
      //if (BAND_DECODE_OUTPUT_5 != GPIO_PIN_NOT_USED) relay.relayWrite(BAND_DECODE_OUTPUT_5, (pattern & 0x0020) ? 1 : 0);  // bit 5
      //if (BAND_DECODE_OUTPUT_6 != GPIO_PIN_NOT_USED) relay.relayWrite(BAND_DECODE_OUTPUT_6, (pattern & 0x0040) ? 1 : 0);  // bit 6
      //if (BAND_DECODE_OUTPUT_7 != GPIO_PIN_NOT_USED) relay.relayWrite(BAND_DECODE_OUTPUT_7, (pattern & 0x0080) ? 1 : 0);  // bit 7
    #endif

    #ifdef M5STAMPC3U
      //
      // mask each bit and apply the 1 or 0 to the assigned pin

      // 41 input/output pins available - 16 per MCP23017 module and 7 more GPIO on the CPU for inputs andf IF SP6T switch control.
      // A virtual bank of pins is defines as generic BAND_DECODE_OUTPUT or BAND_XXX_PTT.  Physical pins are mapped into these.  
      // Some special handling is involved to inorporate the CPU IO pins into the bank which can be confusing.
   
      // CPU GPIO
      //   IO Mapping
      //   INPUT_6   pin 3     - unused IO line
      //   INPUT_5   pin 4     - unused IO line
      //   INPUT_4   pin 5     - unused IO line
      //   INPUT_3   pin 6     - PTT input from Radio
      //   INPUT_2   pin 7     - Band0 Decode Input 
      //   INPUT_1   pin 8     - Band1 Decode Input 
      //   INPUT_0   pin 10    - Band2 Decode Input 

      // 1st module PA0-7 are buffered outputs
      //   IO Mapping
      //   OUTPUT_0   pin 0    - SP6T IF switch control line A
      //   OUTPUT_1   pin 1    - SP6T IF switch control line B
      //   OUTPUT_2   pin 2    - SP6T IF switch control line C
      //   OUTPUT_3   pin 0    - 222 Xvtr PTT
      //   OUTPUT_4   pin 1    - 903 Xvtr PTT
      //   OUTPUT_5   pin 2    - 1296 Xvtr PTT
      //   OUTPUT_6   pin 2    - 1296 Xvtr PTT
      //   OUTPUT_7   pin 2    - 1296 Xvtr PTT
      
      // PB0-7 on 1st module are unbuffered outputs
      //   IO Mapping
      //   OUTPUT_8-13  pin 0-5 - spare
      //   OUTPUT_14    pin 6   - 903  RF Out T/R switch
      //   OUTPUT_15    pin 7   - 1296 RF Out T/R switch
      
      // PA0-7 on 2nd module are buffered outputs for band specific PTT outputs for amps.  These should be sequenced.  - 0x0800 to set all idle
      //   IO Mapping
      //   OUTPUT_8-13  pin 0-5 - PTT for 6 bands HF/50 thru 1296
      //   OUTPUT_14    pin 6   - PTT 12V to 1296 2W amp on TX
      //   OUTPUT_15    pin 7   - PTT out to TC board IF relay.  Switch this fast on incoming PTT change from Radio
      
      // PB0-7 on 2nd module are unbuffered TTL outputs.  - 0xF000 to set all to off
      //   IO Mapping
      //   OUTPUT_8     pin 0   - Port 1 (Xvtr mode) on SP4T coax switch These select IC-705 RF direct to antennas for HF/50, 144, 432 bands and the Xvtr box input
      //   OUTPUT_9     pin 1   - Port 2 (Xvtr mode) on SP4T coax switch
      //   OUTPUT_10    pin 2   - Port 3 (Xvtr mode) on SP4T coax switch
      //   OUTPUT_11    pin 3   - Port 4 (Xvtr mode) on SP4T coax switch
      //   OUTPUT_12    pin 4   - 12V Relay 1 - 1296 Xvtr Power
      //   OUTPUT_13    pin 5   - 12V Relay 2 - 903 Xvtr Power
      //   OUTPUT_14    pin 6   - 12V Relay 3 - 903 Amp Power
      //   OUTPUT_15    pin 7   - 12V Relay 4 - 222 Xvtr Power

      // send pattern (0xF8F3) before change to do a break before make effect

      if (BAND_DECODE_OUTPUT_0  != GPIO_PIN_NOT_USED) mcp0.digitalWrite(BAND_DECODE_OUTPUT_0,  (pattern & 0x0001) ? 1 : 0);  // bit 0   These are on the 1st MCP23017
      if (BAND_DECODE_OUTPUT_1  != GPIO_PIN_NOT_USED) mcp0.digitalWrite(BAND_DECODE_OUTPUT_1,  (pattern & 0x0002) ? 1 : 0);  // bit 1    
      if (BAND_DECODE_OUTPUT_2  != GPIO_PIN_NOT_USED) mcp0.digitalWrite(BAND_DECODE_OUTPUT_2,  (pattern & 0x0004) ? 1 : 0);  // bit 2
      if (BAND_DECODE_OUTPUT_3  != GPIO_PIN_NOT_USED) mcp0.digitalWrite(BAND_DECODE_OUTPUT_3,  (pattern & 0x0008) ? 1 : 0);  // bit 3   
      if (BAND_DECODE_OUTPUT_4  != GPIO_PIN_NOT_USED) mcp0.digitalWrite(BAND_DECODE_OUTPUT_4,  (pattern & 0x0010) ? 1 : 0);  // bit 4
      if (BAND_DECODE_OUTPUT_5  != GPIO_PIN_NOT_USED) mcp0.digitalWrite(BAND_DECODE_OUTPUT_5,  (pattern & 0x0020) ? 1 : 0);  // bit 5
      if (BAND_DECODE_OUTPUT_6  != GPIO_PIN_NOT_USED) mcp0.digitalWrite(BAND_DECODE_OUTPUT_6,  (pattern & 0x0040) ? 1 : 0);  // bit 6
      if (BAND_DECODE_OUTPUT_7  != GPIO_PIN_NOT_USED) mcp0.digitalWrite(BAND_DECODE_OUTPUT_7,  (pattern & 0x0080) ? 1 : 0);  // bit 7

      if (BAND_DECODE_OUTPUT_8  != GPIO_PIN_NOT_USED) mcp1.digitalWrite(BAND_DECODE_OUTPUT_8,  (pattern & 0x0100) ? 1 : 0);  // bit 0    2nd MCP23017 board
      if (BAND_DECODE_OUTPUT_9  != GPIO_PIN_NOT_USED) mcp1.digitalWrite(BAND_DECODE_OUTPUT_9,  (pattern & 0x0200) ? 1 : 0);  // bit 1
      if (BAND_DECODE_OUTPUT_10 != GPIO_PIN_NOT_USED) mcp1.digitalWrite(BAND_DECODE_OUTPUT_10, (pattern & 0x0400) ? 1 : 0);  // bit 2
      if (BAND_DECODE_OUTPUT_11 != GPIO_PIN_NOT_USED) mcp1.digitalWrite(BAND_DECODE_OUTPUT_11, (pattern & 0x0800) ? 1 : 0);  // bit 3
      if (BAND_DECODE_OUTPUT_12 != GPIO_PIN_NOT_USED) mcp1.digitalWrite(BAND_DECODE_OUTPUT_12, (pattern & 0x1000) ? 1 : 0);  // bit 4
      if (BAND_DECODE_OUTPUT_13 != GPIO_PIN_NOT_USED) mcp1.digitalWrite(BAND_DECODE_OUTPUT_13, (pattern & 0x2000) ? 1 : 0);  // bit 5
      if (BAND_DECODE_OUTPUT_14 != GPIO_PIN_NOT_USED) mcp1.digitalWrite(BAND_DECODE_OUTPUT_14, (pattern & 0x4000) ? 1 : 0);  // bit 6
      if (BAND_DECODE_OUTPUT_15 != GPIO_PIN_NOT_USED) mcp1.digitalWrite(BAND_DECODE_OUTPUT_15, (pattern & 0x8000) ? 1 : 0);  // bit 7
    #endif
}

void PTT_Output(uint8_t band, bool PTT_state)
{
    // Set your desired PTT pattern per band in RadioConfig.h
    // ToDo: Eventually create a local UI screen and SD card config file to edit and monitor pin states

    DPRINTF("PTT_Output: Band "); DPRINTLN(band);
    if (1) {
    //if (PTT_state) {  // We are going to TX
        switch (band)
        {
            case  DUMMY     : GPIO_PTT_Out(DECODE_DUMMY_PTT,    false);     break;   //Dummy Band
            case  BAND_AM   : GPIO_PTT_Out(DECODE_BANDAM_PTT,   PTT_state); break;   //AM
            case  BAND_160M : GPIO_PTT_Out(DECODE_BAND160M_PTT, PTT_state); break;   //160M 
            case  BAND_80M  : GPIO_PTT_Out(DECODE_BAND80M_PTT,  PTT_state); break;   //80M
            case  BAND_60M  : GPIO_PTT_Out(DECODE_BAND60M_PTT,  PTT_state); break;   //60M
            case  BAND_40M  : GPIO_PTT_Out(DECODE_BAND40M_PTT,  PTT_state); break;   //40M
            case  BAND_30M  : GPIO_PTT_Out(DECODE_BAND30M_PTT,  PTT_state); break;   //30M
            case  BAND_20M  : GPIO_PTT_Out(DECODE_BAND20M_PTT,  PTT_state); break;   //20M
            case  BAND_17M  : GPIO_PTT_Out(DECODE_BAND17M_PTT,  PTT_state); break;   //17M      
            case  BAND_15M  : GPIO_PTT_Out(DECODE_BAND15M_PTT,  PTT_state); break;   //15M
            case  BAND_12M  : GPIO_PTT_Out(DECODE_BAND12M_PTT,  PTT_state); break;   //12M
            case  BAND_10M  : GPIO_PTT_Out(DECODE_BAND10M_PTT,  PTT_state); break;   //10M
            case  BAND_6M   : GPIO_PTT_Out(DECODE_BAND6M_PTT,   PTT_state); break;   //6M
            case  BAND_FM   : GPIO_PTT_Out(DECODE_BANDFM_PTT,   PTT_state); break;   //6M
            case  BAND_AIR  : GPIO_PTT_Out(DECODE_BANDAIR_PTT,  PTT_state); break;   //6M
            //case BAND70   : GPIO_PTT_Out(0x01_PTT, PTT_state); break;   //6M
            case  BAND_2M   : GPIO_PTT_Out(DECODE_BAND144_PTT,  PTT_state); break;   //2M
            case  BAND_1_25M: GPIO_PTT_Out(DECODE_BAND222_PTT,  PTT_state); break;   //222
            case  BAND_70cm : GPIO_PTT_Out(DECODE_BAND432_PTT,  PTT_state); break;   //432
            case  BAND_33cm : GPIO_PTT_Out(DECODE_BAND902_PTT,  PTT_state); break;   //902
            case  BAND_23cm : GPIO_PTT_Out(DECODE_BAND1296_PTT, PTT_state); break;   //1296
            case  BAND_13cm : GPIO_PTT_Out(DECODE_BAND2400_PTT, PTT_state); break;   //2400
            case  BAND_9cm  : GPIO_PTT_Out(DECODE_BAND3300_PTT, PTT_state); break;   //3400
            case  BAND_6cm  : GPIO_PTT_Out(DECODE_BAND5760_PTT, PTT_state); break;   //5760M
            case  BAND_3cm  : GPIO_PTT_Out(DECODE_BAND10G_PTT,  PTT_state); break;   //10.368.1G
            case  BAND_24G  : GPIO_PTT_Out(DECODE_BAND24G_PTT,  PTT_state); break;   //24.192G
            case  BAND_47G  : GPIO_PTT_Out(DECODE_BAND47G_PTT,  PTT_state); break;   //47.1G
            case  BAND_76G  : GPIO_PTT_Out(DECODE_BAND76G_PTT,  PTT_state); break;   //76.1G
            case  BAND_122G : GPIO_PTT_Out(DECODE_BAND122G_PTT, PTT_state); break;   //122G
            case  B_GENERAL : GPIO_PTT_Out(DECODE_B_GENERAL_PTT,PTT_state); break;   //Any other band not in the list
        }
        #ifdef M5STAMPC3U
            DPRINTLNF("PTT_Output: Turning LED red for TX, setting PTT bits to TX");
            pixel.setPixelColor(0, pixel.Color(128, 0, 0));
            pixel.show(); 
        #endif
    } else {   // We are going back to RX
        #ifdef M5STAMPC3U
            DPRINTLNF("PTT_Output: Turning LED blue for RX and resetting PTT bits to RX state");
            GPIO_PTT_Out(DECODE_DUMMY_PTT, PTT_state);
            pixel.setPixelColor(0, pixel.Color(0, 0, 128));
            pixel.show(); 
        #endif
    }   
}

void GPIO_PTT_Out(uint16_t pattern, bool _PTT_state)
{   
    uint16_t PTT_state = _PTT_state ? 0xFFFF : 0;
    
    #ifdef M5STAMPC3U
      if (pattern == DECODE_DUMMY_PTT && _PTT_state == true)
        DPRINTF("  PTT state is RX");
    #endif 
    DPRINTF("  PTT state "); DPRINT(_PTT_state, BIN);
    DPRINTF("  PTT Output Binary "); DPRINT(pattern, BIN);
    DPRINTF("  PTT Output Hex "); DPRINTLN(pattern, HEX);
    
    //#ifndef IO_MODULE
    //  DPRINTLNF("  GPIO_PTT_Out: IO_MODULE **NOT** Enabled");
      //return;
    //#endif

    // Operate a relay anytime PTT is high. Probably not useful if have more than 1 PTT but is here for a test
    // This is a M5Stack 1 relay Unit.  The 1 and 2 relay units operate on a grove port (B or C) with simple digital IO.  
    // On a Core, Port A is the same pins as the rest of the internal i2c, operating a relay on them would screw up I2C likely.
    // The 4 relay unit is i2c addressed and normally plugs into Port A on the Corexx itself. 
    // I2C on Port B is a separate IC2 port for the Core2 and Core3
    // M5STAMPC3U config - ignore the above, it is all different :-)

    #if defined ( RELAY2_UNIT )
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
    
    //Serial.println((pattern & 0x0008 & PTT_state) ? 0 : 1);
    
    #ifdef IO_MODULE
      // mask each bit and apply the 1 or 0 to the assigned pin
      if (BAND_DECODE_PTT_OUTPUT_0  != GPIO_PIN_NOT_USED) {module.setOutput(BAND_DECODE_PTT_OUTPUT_0,  (pattern & 0x0001 & PTT_state) ? 1 : 0);}  // bit 0
      if (BAND_DECODE_PTT_OUTPUT_1  != GPIO_PIN_NOT_USED) {module.setOutput(BAND_DECODE_PTT_OUTPUT_1,  (pattern & 0x0002 & PTT_state) ? 1 : 0);}  // bit 1
      if (BAND_DECODE_PTT_OUTPUT_2  != GPIO_PIN_NOT_USED) {module.setOutput(BAND_DECODE_PTT_OUTPUT_2,  (pattern & 0x0004 & PTT_state) ? 1 : 0);}  // bit 2
      if (BAND_DECODE_PTT_OUTPUT_3  != GPIO_PIN_NOT_USED) {module.setOutput(BAND_DECODE_PTT_OUTPUT_3,  (pattern & 0x0008 & PTT_state) ? 1 : 0);}  // bit 3
      if (BAND_DECODE_PTT_OUTPUT_4  != GPIO_PIN_NOT_USED) {module.setOutput(BAND_DECODE_PTT_OUTPUT_4,  (pattern & 0x0010 & PTT_state) ? 1 : 0);}  // bit 4
      if (BAND_DECODE_PTT_OUTPUT_5  != GPIO_PIN_NOT_USED) {module.setOutput(BAND_DECODE_PTT_OUTPUT_5,  (pattern & 0x0020 & PTT_state) ? 1 : 0);}  // bit 5
      if (BAND_DECODE_PTT_OUTPUT_6  != GPIO_PIN_NOT_USED) {module.setOutput(BAND_DECODE_PTT_OUTPUT_6,  (pattern & 0x0040 & PTT_state) ? 1 : 0);}  // bit 6
      if (BAND_DECODE_PTT_OUTPUT_7  != GPIO_PIN_NOT_USED) {module.setOutput(BAND_DECODE_PTT_OUTPUT_7,  (pattern & 0x0080 & PTT_state) ? 1 : 0);}  // bit 7
      /*  Future 2nd module  Update device handle
      if (BAND_DECODE_PTT_OUTPUT_8  != GPIO_PIN_NOT_USED) {module2.setOutput(BAND_DECODE_PTT_OUTPUT_8,  (pattern & 0x0100 & PTT_state) ? 1 : 0);}  // bit 0 Bank or module 2
      if (BAND_DECODE_PTT_OUTPUT_9  != GPIO_PIN_NOT_USED) {module2.setOutput(BAND_DECODE_PTT_OUTPUT_9,  (pattern & 0x0200 & PTT_state) ? 1 : 0);}  // bit 1
      if (BAND_DECODE_PTT_OUTPUT_10 != GPIO_PIN_NOT_USED) {module2.setOutput(BAND_DECODE_PTT_OUTPUT_10, (pattern & 0x0400 & PTT_state) ? 1 : 0);}  // bit 2
      if (BAND_DECODE_PTT_OUTPUT_11 != GPIO_PIN_NOT_USED) {module2.setOutput(BAND_DECODE_PTT_OUTPUT_11, (pattern & 0x0800 & PTT_state) ? 1 : 0);}  // bit 3
      if (BAND_DECODE_PTT_OUTPUT_12 != GPIO_PIN_NOT_USED) {module2.setOutput(BAND_DECODE_PTT_OUTPUT_12, (pattern & 0x1000 & PTT_state) ? 1 : 0);}  // bit 4
      if (BAND_DECODE_PTT_OUTPUT_13 != GPIO_PIN_NOT_USED) {module2.setOutput(BAND_DECODE_PTT_OUTPUT_13, (pattern & 0x2000 & PTT_state) ? 1 : 0);}  // bit 5
      if (BAND_DECODE_PTT_OUTPUT_14 != GPIO_PIN_NOT_USED) {module2.setOutput(BAND_DECODE_PTT_OUTPUT_14, (pattern & 0x4000 & PTT_state) ? 1 : 0);}  // bit 6
      if (BAND_DECODE_PTT_OUTPUT_15 != GPIO_PIN_NOT_USED) {module2.setOutput(BAND_DECODE_PTT_OUTPUT_15, (pattern & 0x8000 & PTT_state) ? 1 : 0);}  // bit 7
      */
    #endif
    
    #ifdef MODULE_4RELAY_13_2
      // mask each bit and apply the 1 or 0 to the assigned pin
      if (BAND_DECODE_PTT_OUTPUT_0  != GPIO_PIN_NOT_USED) {RELAY.setRelay(BAND_DECODE_PTT_OUTPUT_0,  (pattern & 0x0001 & PTT_state) ? 1 : 0);}  // bit 0
      if (BAND_DECODE_PTT_OUTPUT_1  != GPIO_PIN_NOT_USED) {RELAY.setRelay(BAND_DECODE_PTT_OUTPUT_1,  (pattern & 0x0002 & PTT_state) ? 1 : 0);}  // bit 1
      if (BAND_DECODE_PTT_OUTPUT_2  != GPIO_PIN_NOT_USED) {RELAY.setRelay(BAND_DECODE_PTT_OUTPUT_2,  (pattern & 0x0004 & PTT_state) ? 1 : 0);}  // bit 2
      if (BAND_DECODE_PTT_OUTPUT_3  != GPIO_PIN_NOT_USED) {RELAY.setRelay(BAND_DECODE_PTT_OUTPUT_3,  (pattern & 0x0008 & PTT_state) ? 1 : 0);}  // bit 3
      /*   Future 2nd or more 4-Relay modules/
      if (BAND_DECODE_PTT_OUTPUT_4  != GPIO_PIN_NOT_USED) {RELAY2.setRelay(BAND_DECODE_PTT_OUTPUT_4,  (pattern & 0x0010 & PTT_state) ? 1 : 0);}  // bit 0
      if (BAND_DECODE_PTT_OUTPUT_5  != GPIO_PIN_NOT_USED) {RELAY2.setRelay(BAND_DECODE_PTT_OUTPUT_5,  (pattern & 0x0020 & PTT_state) ? 1 : 0);}  // bit 1
      if (BAND_DECODE_PTT_OUTPUT_6  != GPIO_PIN_NOT_USED) {RELAY2.setRelay(BAND_DECODE_PTT_OUTPUT_6,  (pattern & 0x0040 & PTT_state) ? 1 : 0);}  // bit 2
      if (BAND_DECODE_PTT_OUTPUT_7  != GPIO_PIN_NOT_USED) {RELAY2.setRelay(BAND_DECODE_PTT_OUTPUT_7,  (pattern & 0x0080 & PTT_state) ? 1 : 0);}  // bit 3

      if (BAND_DECODE_PTT_OUTPUT_8  != GPIO_PIN_NOT_USED) {RELAY3.setRelay(BAND_DECODE_PTT_OUTPUT_8,  (pattern & 0x0100 & PTT_state) ? 1 : 0);}  // bit 0  Module 3
      if (BAND_DECODE_PTT_OUTPUT_9  != GPIO_PIN_NOT_USED) {RELAY3.setRelay(BAND_DECODE_PTT_OUTPUT_9,  (pattern & 0x0200 & PTT_state) ? 1 : 0);}  // bit 1
      if (BAND_DECODE_PTT_OUTPUT_10 != GPIO_PIN_NOT_USED) {RELAY3.setRelay(BAND_DECODE_PTT_OUTPUT_10, (pattern & 0x0400 & PTT_state) ? 1 : 0);}  // bit 2
      if (BAND_DECODE_PTT_OUTPUT_11 != GPIO_PIN_NOT_USED) {RELAY3.setRelay(BAND_DECODE_PTT_OUTPUT_11, (pattern & 0x0800 & PTT_state) ? 1 : 0);}  // bit 3

      if (BAND_DECODE_PTT_OUTPUT_12 != GPIO_PIN_NOT_USED) {RELAY4.setRelay(BAND_DECODE_PTT_OUTPUT_12, (pattern & 0x1000 & PTT_state) ? 1 : 0);}  // bit 0
      if (BAND_DECODE_PTT_OUTPUT_13 != GPIO_PIN_NOT_USED) {RELAY4.setRelay(BAND_DECODE_PTT_OUTPUT_13, (pattern & 0x2000 & PTT_state) ? 1 : 0);}  // bit 1
      if (BAND_DECODE_PTT_OUTPUT_14 != GPIO_PIN_NOT_USED) {RELAY4.setRelay(BAND_DECODE_PTT_OUTPUT_14, (pattern & 0x4000 & PTT_state) ? 1 : 0);}  // bit 2
      if (BAND_DECODE_PTT_OUTPUT_15 != GPIO_PIN_NOT_USED) {RELAY4.setRelay(BAND_DECODE_PTT_OUTPUT_15, (pattern & 0x8000 & PTT_state) ? 1 : 0);}  // bit 3
      */
    #endif

    #ifdef EXT_IO2_UNIT
      // mask each bit and apply the 1 or 0 to the assigned pin
      //if (BAND_DECODE_PTT_OUTPUT_0 != GPIO_PIN_NOT_USED) {extio.setPinMode(BAND_DECODE_PTT_OUTPUT_0, (pattern & 0x0001 & PTT_state) ? 1 : 0);}  // bit 0
      //if (BAND_DECODE_PTT_OUTPUT_1 != GPIO_PIN_NOT_USED) {extio.setPinMode(BAND_DECODE_PTT_OUTPUT_1, (pattern & 0x0002 & PTT_state) ? 1 : 0);}  // bit 1
      //if (BAND_DECODE_PTT_OUTPUT_2 != GPIO_PIN_NOT_USED) {extio.setPinMode(BAND_DECODE_PTT_OUTPUT_2, (pattern & 0x0004 & PTT_state) ? 1 : 0);}  // bit 2
      //if (BAND_DECODE_PTT_OUTPUT_3 != GPIO_PIN_NOT_USED) {extio.setPinMode(BAND_DECODE_PTT_OUTPUT_3, (pattern & 0x0008 & PTT_state) ? 1 : 0);}  // bit 3
      if (BAND_DECODE_PTT_OUTPUT_4 != GPIO_PIN_NOT_USED) {extio.setDigitalOutput(BAND_DECODE_PTT_OUTPUT_4, (pattern & 0x0010 & PTT_state) ? 1 : 0);}  // bit 4
      if (BAND_DECODE_PTT_OUTPUT_5 != GPIO_PIN_NOT_USED) {extio.setDigitalOutput(BAND_DECODE_PTT_OUTPUT_5, (pattern & 0x0020 & PTT_state) ? 1 : 0);}  // bit 5
      if (BAND_DECODE_PTT_OUTPUT_6 != GPIO_PIN_NOT_USED) {extio.setDigitalOutput(BAND_DECODE_PTT_OUTPUT_6, (pattern & 0x0040 & PTT_state) ? 1 : 0);}  // bit 6
      if (BAND_DECODE_PTT_OUTPUT_7 != GPIO_PIN_NOT_USED) {extio.setDigitalOutput(BAND_DECODE_PTT_OUTPUT_7, (pattern & 0x0080 & PTT_state) ? 1 : 0);}  // bit 7
      // mask each bit and apply the 1 or 0 to the assigned pin
      
      /*  Future extra units
      //if (BAND_DECODE_PTT_OUTPUT_8 != GPIO_PIN_NOT_USED) {extio2.setPinMode(BAND_DECODE_PTT_OUTPUT_8, (pattern & 0x0100 & PTT_state) ? 1 : 0);}  // bit 0
      //if (BAND_DECODE_PTT_OUTPUT_9 != GPIO_PIN_NOT_USED) {extio2.setPinMode(BAND_DECODE_PTT_OUTPUT_9, (pattern & 0x0200 & PTT_state) ? 1 : 0);}  // bit 1
      //if (BAND_DECODE_PTT_OUTPUT_10 != GPIO_PIN_NOT_USED) {extio2.setPinMode(BAND_DECODE_PTT_OUTPUT_10, (pattern & 0x0400 & PTT_state) ? 1 : 0);}  // bit 2
      //if (BAND_DECODE_PTT_OUTPUT_11 != GPIO_PIN_NOT_USED) {extio2.setPinMode(BAND_DECODE_PTT_OUTPUT_11, (pattern & 0x0800 & PTT_state) ? 1 : 0);}  // bit 3
      if (BAND_DECODE_PTT_OUTPUT_12 != GPIO_PIN_NOT_USED) {extio2.setDigitalOutput(BAND_DECODE_PTT_OUTPUT_12, (pattern & 0x1000 & PTT_state) ? 1 : 0);}  // bit 4
      if (BAND_DECODE_PTT_OUTPUT_13 != GPIO_PIN_NOT_USED) {extio2.setDigitalOutput(BAND_DECODE_PTT_OUTPUT_13, (pattern & 0x2000 & PTT_state) ? 1 : 0);}  // bit 5
      if (BAND_DECODE_PTT_OUTPUT_14 != GPIO_PIN_NOT_USED) {extio2.setDigitalOutput(BAND_DECODE_PTT_OUTPUT_14, (pattern & 0x4000 & PTT_state) ? 1 : 0);}  // bit 6
      if (BAND_DECODE_PTT_OUTPUT_15 != GPIO_PIN_NOT_USED) {extio2.setDigitalOutput(BAND_DECODE_PTT_OUTPUT_15, (pattern & 0x8000 & PTT_state) ? 1 : 0);}  // bit 7
      */
    #endif

    #ifdef RELAY4_UNIT  // A Unit is a standalone i2c connected device, not the module.
      // mask each bit and apply the 1 or 0 to the assigned pin
      if (BAND_DECODE_PTT_OUTPUT_0 != GPIO_PIN_NOT_USED) relay.relayWrite(BAND_DECODE_PTT_OUTPUT_0, (pattern & 0x0001 & PTT_state) ? 1 : 0);  // bit 0
      if (BAND_DECODE_PTT_OUTPUT_1 != GPIO_PIN_NOT_USED) relay.relayWrite(BAND_DECODE_PTT_OUTPUT_1, (pattern & 0x0002 & PTT_state) ? 1 : 0);  // bit 1
      if (BAND_DECODE_PTT_OUTPUT_2 != GPIO_PIN_NOT_USED) relay.relayWrite(BAND_DECODE_PTT_OUTPUT_2, (pattern & 0x0004 & PTT_state) ? 1 : 0);  // bit 2
      if (BAND_DECODE_PTT_OUTPUT_3 != GPIO_PIN_NOT_USED) relay.relayWrite(BAND_DECODE_PTT_OUTPUT_3, (pattern & 0x0008 & PTT_state) ? 1 : 0);  // bit 3
      //if (BAND_DECODE_OUTPUT_4 != GPIO_PIN_NOT_USED) relay.relayWrite(BAND_DECODE_OUTPUT_4, (pattern & 0x0010) ? 1 : 0);  // bit 4
      //if (BAND_DECODE_OUTPUT_5 != GPIO_PIN_NOT_USED) relay.relayWrite(BAND_DECODE_OUTPUT_5, (pattern & 0x0020) ? 1 : 0);  // bit 5
      //if (BAND_DECODE_OUTPUT_6 != GPIO_PIN_NOT_USED) relay.relayWrite(BAND_DECODE_OUTPUT_6, (pattern & 0x0040) ? 1 : 0);  // bit 6
      //if (BAND_DECODE_OUTPUT_7 != GPIO_PIN_NOT_USED) relay.relayWrite(BAND_DECODE_OUTPUT_7, (pattern & 0x0080) ? 1 : 0);  // bit 7
      
      /*  Future extra device
      // mask each bit and apply the 1 or 0 to the assigned pin
      if (BAND_DECODE_PTT_OUTPUT_8 != GPIO_PIN_NOT_USED) relay2.relayWrite(BAND_DECODE_PTT_OUTPUT_8, (pattern & 0x0100 & PTT_state) ? 1 : 0);  // bit 0
      if (BAND_DECODE_PTT_OUTPUT_9 != GPIO_PIN_NOT_USED) relay2.relayWrite(BAND_DECODE_PTT_OUTPUT_9, (pattern & 0x0200 & PTT_state) ? 1 : 0);  // bit 1
      if (BAND_DECODE_PTT_OUTPUT_10 != GPIO_PIN_NOT_USED) relay2.relayWrite(BAND_DECODE_PTT_OUTPUT_10, (pattern & 0x0400 & PTT_state) ? 1 : 0);  // bit 2
      if (BAND_DECODE_PTT_OUTPUT_11 != GPIO_PIN_NOT_USED) relay2.relayWrite(BAND_DECODE_PTT_OUTPUT_11, (pattern & 0x0800 & PTT_state) ? 1 : 0);  // bit 3
      //if (BAND_DECODE_OUTPUT_12 != GPIO_PIN_NOT_USED) relay2.relayWrite(BAND_DECODE_OUTPUT_12, (pattern & 0x1000) ? 1 : 0);  // bit 4
      //if (BAND_DECODE_OUTPUT_13 != GPIO_PIN_NOT_USED) relay2.relayWrite(BAND_DECODE_OUTPUT_13, (pattern & 0x2000) ? 1 : 0);  // bit 5
      //if (BAND_DECODE_OUTPUT_14 != GPIO_PIN_NOT_USED) relay2.relayWrite(BAND_DECODE_OUTPUT_14, (pattern & 0x4000) ? 1 : 0);  // bit 6
      //if (BAND_DECODE_OUTPUT_15 != GPIO_PIN_NOT_USED) relay2.relayWrite(BAND_DECODE_OUTPUT_15, (pattern & 0x8000) ? 1 : 0);  // bit 7
      */
    #endif

    #ifdef M5STAMPC3U
      // mask each bit and apply the 1 or 0 to the assigned pin
      if (BAND_DECODE_PTT_OUTPUT_0  != GPIO_PIN_NOT_USED) mcp0.digitalWrite(BAND_DECODE_PTT_OUTPUT_0,  (pattern & 0x0001) ? 1 : 0);  // bit 0 HF/50
      if (BAND_DECODE_PTT_OUTPUT_1  != GPIO_PIN_NOT_USED) mcp0.digitalWrite(BAND_DECODE_PTT_OUTPUT_1,  (pattern & 0x0002) ? 1 : 0);  // bit 1 144
      if (BAND_DECODE_PTT_OUTPUT_2  != GPIO_PIN_NOT_USED) mcp0.digitalWrite(BAND_DECODE_PTT_OUTPUT_2,  (pattern & 0x0004) ? 1 : 0);  // bit 2 222
      if (BAND_DECODE_PTT_OUTPUT_3  != GPIO_PIN_NOT_USED) mcp0.digitalWrite(BAND_DECODE_PTT_OUTPUT_3,  (pattern & 0x0008) ? 1 : 0);  // bit 3 432
      
      if (BAND_DECODE_PTT_OUTPUT_4  != GPIO_PIN_NOT_USED) mcp0.digitalWrite(BAND_DECODE_PTT_OUTPUT_4,  (pattern & 0x0010) ? 1 : 0);  // bit 4 902/903
      if (BAND_DECODE_PTT_OUTPUT_5  != GPIO_PIN_NOT_USED) mcp0.digitalWrite(BAND_DECODE_PTT_OUTPUT_5,  (pattern & 0x0020) ? 1 : 0);  // bit 5 1296
      if (BAND_DECODE_PTT_OUTPUT_6  != GPIO_PIN_NOT_USED) mcp0.digitalWrite(BAND_DECODE_PTT_OUTPUT_6,  (pattern & 0x0040) ? 1 : 0);  // bit 6 not used
      if (BAND_DECODE_PTT_OUTPUT_7  != GPIO_PIN_NOT_USED) mcp0.digitalWrite(BAND_DECODE_PTT_OUTPUT_7,  (pattern & 0x0080) ? 1 : 0);  // bit 7 TxInhibit or not used
      
      // mask each bit and apply the 1 or 0 to the assigned pin
      if (BAND_DECODE_PTT_OUTPUT_8  != GPIO_PIN_NOT_USED) mcp1.digitalWrite(BAND_DECODE_PTT_OUTPUT_8,  (pattern & 0x0100) ? 1 : 0);  // bit 0 HF/50
      if (BAND_DECODE_PTT_OUTPUT_9  != GPIO_PIN_NOT_USED) mcp1.digitalWrite(BAND_DECODE_PTT_OUTPUT_9,  (pattern & 0x0200) ? 1 : 0);  // bit 1 144
      if (BAND_DECODE_PTT_OUTPUT_10 != GPIO_PIN_NOT_USED) mcp1.digitalWrite(BAND_DECODE_PTT_OUTPUT_10, (pattern & 0x0400) ? 1 : 0);  // bit 2 222
      if (BAND_DECODE_PTT_OUTPUT_11 != GPIO_PIN_NOT_USED) mcp1.digitalWrite(BAND_DECODE_PTT_OUTPUT_11, (pattern & 0x0800) ? 1 : 0);  // bit 3 432
      
      if (BAND_DECODE_PTT_OUTPUT_12 != GPIO_PIN_NOT_USED) mcp1.digitalWrite(BAND_DECODE_PTT_OUTPUT_12, (pattern & 0x1000) ? 1 : 0);  // bit 4 902/903
      if (BAND_DECODE_PTT_OUTPUT_13 != GPIO_PIN_NOT_USED) mcp1.digitalWrite(BAND_DECODE_PTT_OUTPUT_13, (pattern & 0x2000) ? 1 : 0);  // bit 5 1296
      if (BAND_DECODE_PTT_OUTPUT_14 != GPIO_PIN_NOT_USED) mcp1.digitalWrite(BAND_DECODE_PTT_OUTPUT_14, (pattern & 0x4000) ? 1 : 0);  // bit 6 1296 12V PA PTT
      if (BAND_DECODE_PTT_OUTPUT_15 != GPIO_PIN_NOT_USED) mcp1.digitalWrite(BAND_DECODE_PTT_OUTPUT_15, (pattern & 0x8000) ? 1 : 0);  // bit 7 PTT to TC board IF T/R relay
    #endif  
}

void Module_4_Relay_setup()
{
  uint8_t counter = 0;

  #if defined ( RELAY2_UNIT )
    // For the 1 or 2-Relay modules if used.  Plug into Port C.  Port B only 1 pin 26 worked con the Core Basic.  
    // Port A pins are internal i2c and would conflict with relay usage
    pinMode(pinR1, OUTPUT);  // Set pin to output mode.
    pinMode(pinR2, OUTPUT);  // Set pin to output mode.
  #endif

  #ifdef MODULE_4RELAY_13_2
    #ifdef CONFIG_IDF_TARGET_ESP32S3
      DPRINTLNF("Decoder: CoreS3 i2c pins used for 4-Relay module setup");
      while (!RELAY.begin(&Wire1, MODULE_4RELAY_ADDR, 12, 11, 200000L) && counter < 4) {  //for core S3
    #else
      while (!RELAY.begin(&Wire, MODULE_4RELAY_ADDR, 21, 22, 200000L) && counter < 4) {  //for core basic core 2
    #endif
      DPRINTLNF("4-RELAY Module INIT ERROR, Check Module is plugged in tight!");
      delay(10);
      counter++;
    }
    if (counter < 4) {
      DPRINTLNF("4-RELAY Module INIT Success");
      Serial.printf("raw 8bit adc: %d\r\n", RELAY.getRawADC(ADC_8BIT));
      Serial.printf("raw 12bit adc: %d\r\n", RELAY.getRawADC(ADC_12BIT));
      Serial.printf("voltage: %fV\r\n", RELAY.getVoltage());
      RELAY.setAllRelay(true);
      vTaskDelay(700);
      RELAY.setAllRelay(false);
    }
  #endif
}

void Module_4in_8out_setup()
{
  uint8_t counter = 0;

  #if defined ( RELAY2_UNIT )
    // For the 1 or 2-Relay modules if used.  Plug into Port C.  Port B only 1 pin 26 worked con the Core Basic.  
    // Port A pins are internal i2c and would conflict with relay usage
    pinMode(pinR1, OUTPUT);  // Set pin to output mode.
    pinMode(pinR2, OUTPUT);  // Set pin to output mode.
  #endif

  #ifdef IO_MODULE
    #ifdef CONFIG_IDF_TARGET_ESP32S3
      DPRINTLNF("Decoder: CoreS3 i2c pins used");
      while (!module.begin(&Wire1, 12, 11, MODULE_4IN8OUT_ADDR) && counter < 4) {  // for cores3
    #else
      while (!module.begin(&Wire, 21, 22, MODULE_4IN8OUT_ADDR) && counter < 4) {  //for core basic
    #endif
        DPRINTLNF("4IN8OUT INIT ERROR, Check Module is plugged in tight!");
        delay(10);
        counter++;
      }
      if (counter < 4)
        DPRINTLNF("4IN8OUT INIT Success");
  #endif
}

void Unit_RELAY4_setup()
{
  uint8_t counter = 0;

  #ifdef RELAY4_UNIT
    #if defined ( CONFIG_IDF_TARGET_ESP32S3 )
      if (board_type == M5ATOMS3) {
        DPRINTLNF("Decoder: M5ATOMS3 Port A i2c pins used for i2c RELAY4_UNIT UNIT setup");
        //   Note 0x45 is also default for the 4IN8OUT module.  Reassign one of them
        while (!relay.begin(&Wire, 2, 1) && counter < 4) {  // for cores3 and AtomS3 ext i2c bus use Wire1?  addr = 0x26
          DPRINTLNF("RELAY4_UNIT INIT ERROR, Check Module is plugged in tight in Port A");
          delay(10);
          counter++;
        }
      } 
      else {
        DPRINTLNF("Decoder: CoreS3 Port A i2c pins used for i2c RELAY4_UNIT UNIT setup");
        while (!relay.begin(&Wire1, 12, 11) && counter < 4) {  // for cores3 - Wire1???? addr = 0x26
          DPRINTLNF("RELAY4_UNIT INIT ERROR, Check Module is plugged in tight in Port A");
          delay(10);
          counter++;
        }
      }
    #else
      while (!relay.begin(&Wire, 21, 22) && counter < 4) {  //for core basic   - core 2 wire 1??? addr = 0x26
        DPRINTLNF("RELAY4_UNIT INIT ERROR, Check Module is plugged in tight in Port A");
        delay(10);
        counter++;
      }
    #endif
        
      if (counter < 4) {
        DPRINTLNF("RELAY4_UNIT INIT Success");
        relay.Init(1); // Set the lamp and relay to synchronous mode(Async = 0, Sync = 1)
        relay.relayAll(1);
        vTaskDelay(1000);
        relay.relayAll(0);
      }
  #endif
}

void Unit_EXTIO2_setup()
{
  uint8_t counter = 0;

  #ifdef EXT_IO2_UNIT
    #if defined ( CONFIG_IDF_TARGET_ESP32S3 )
      if (board_type == M5ATOMS3) {
        DPRINTLNF("Decoder: M5ATOMS3 Port A i2c pins used for i2c EXTIO2 UNIT setup");
        //   Note 0x45 is also default for the 4IN8OUT module.  Reassign one of them
        while (!extio.begin(&Wire, 2, 1, 0x45) && counter < 4) {  // for cores3 and AtomS3 ext i2c bus use Wire1?
          DPRINTLNF("EXT_IO2 INIT ERROR, Check Module is plugged in tight in Port A");
          delay(10);
          counter++;
        }
      } else {
        DPRINTLNF("Decoder: CoreS3 Port A i2c pins used for i2c EXTIO2 UNIT setup");
        while (!extio.begin(&Wire1, 12, 11, 0x45) && counter < 4) {  // for cores3 - Wire1????
          DPRINTLNF("EXT_IO2 INIT ERROR, Check Module is plugged in tight in Port A");
          delay(10);
          counter++;
        }
      }
    #else
      while (!extio.begin(&Wire, 21, 22, 0x45) && counter < 4) {  //for core basic   - core 2 wire 1???
        DPRINTLNF("EXT_IO2 INIT ERROR, Check Module is plugged in tight in Port A");
        delay(10);
        counter++;
      }
    #endif
        
      if (counter < 4) {
        DPRINTLNF("EXT_IO2 INIT Success, Configure lower 4 pins as INPUT, upper 4 as OUTPUT");
        mode = DIGITAL_INPUT_MODE;
        extio.setPinMode(GPIO_MOD_IO_PIN_0, mode);
        extio.setPinMode(GPIO_MOD_IO_PIN_1, mode);
        extio.setPinMode(GPIO_MOD_IO_PIN_2, mode);
        extio.setPinMode(GPIO_MOD_IO_PIN_3, mode);
        mode = DIGITAL_OUTPUT_MODE;
        extio.setPinMode(GPIO_MOD_IO_PIN_4, mode);
        extio.setPinMode(GPIO_MOD_IO_PIN_5, mode);
        extio.setPinMode(GPIO_MOD_IO_PIN_6, mode);
        extio.setPinMode(GPIO_MOD_IO_PIN_7, mode);
      }
  #endif
}

uint8_t Unit_EXTIO2_Input_scan(void) 
{
  uint8_t pattern = 0;
 #if defined ( EXT_IO2_UNIT )
    pattern |= extio.getDigitalInput(0) ; 
    pattern |= extio.getDigitalInput(1) << 1;
    pattern |= extio.getDigitalInput(2) << 2;
    pattern |= extio.getDigitalInput(3) << 3;
 #endif
  return pattern & 0x0F;  // will leave the upper 4 GPIO pins for outputs
}

uint8_t Module_4in_8out_Input_scan(void) 
{
  uint8_t pattern = 0;
 #ifdef IO_MODULE
  pattern |= module.getInput(0) ; 
  pattern |= module.getInput(1) << 1;
  pattern |= module.getInput(2) << 2;
  pattern |= module.getInput(3) << 3;
 #endif
  return pattern & 0x0F;
}

#ifdef M5STAMPC3U
  
  uint8_t M5STAMPC3U_Input_scan(void) 
  {
    uint8_t pattern = 0;
      pattern |= digitalRead(BAND_DECODE_INPUT_0);       // BCD Band 0 
      pattern |= digitalRead(BAND_DECODE_INPUT_1) << 1;  // BCD Band 1
      pattern |= digitalRead(BAND_DECODE_INPUT_2) << 2;  // BCD Band 2 
      pattern |= digitalRead(BAND_DECODE_INPUT_3) << 3;  // PTT input
    return pattern & 0x0F;  // upper 4 GPIO pins spare for now
  }

  void MCP23017_IO_setup()
  {
    uint8_t counter = 0;
    
    DPRINTLNF("Decoder: M5 StampC3U i2c pins used for 2 MCP23017 16 port expander modules address 0x20 and 0x21");
    
    while (!mcp0.begin_I2C(0x20) && counter < 4) {  //for core S3
      DPRINTLNF("M5StampC3U i2c INIT ERROR, Check Modules are installed correctly, address 0x20");
      delay(10);
      counter++;
    }
    while (!mcp1.begin_I2C(0x21) && counter < 4) {  //for core S3
      DPRINTLNF("M5StampC3U i2c INIT ERROR, Check Modules are installed correctly, address 0x21");
      delay(10);
      counter++;
    }

    if (counter < 4) {
      DPRINTLNF("MCP23017 Module INIT Success");

      // set up pin directions
      int i;
      for (i = 0; i < 16; i++)   // PA0-7, PB0-7 1st Module
      {
        mcp0.pinMode(i, OUTPUT);   // PA0-7 outputs on 1st module (0-2 used for Xvtr board internal PTT )  rest are spare
      }

      // Module 2, all are outputs    
      for (i = 0; i < 16; i++)   // PA0-7 and PB0-7
      {
        mcp1.pinMode(i, OUTPUT);   
      }

      for (i = 3; i < 6; i++)   // C3U CPU GPIO pins 3,4,5 for IF SP6T Switch Control
      {
        pinMode(i, OUTPUT);   // CPU IO pins 3,4,5
      }

      for (i = 6; i < 9; i++)   // C3U CPU GPIO pins 6,7,8,10
      {
        pinMode(i, INPUT_PULLUP);   // CPU IO pins 6,7,8, 9 not used
      }
      pinMode(BAND_DECODE_INPUT_3, INPUT_PULLUP);   //  PTT IO pin on pin 10

      Band_Decode_Output(DUMMY, true);
      GPIO_PTT_Out(DECODE_DUMMY_PTT, true);   //initialize the PTT states.  Required since the ports are not all zero in RX but mixed state

      vTaskDelay(700);
    }
  }
#endif  // M5STAMPC3U

#endif  // DECODER FILE