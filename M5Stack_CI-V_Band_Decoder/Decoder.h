#ifndef DECODER_H_
#define DECODER_H_

#include <stdint.h>
#include "M5Stack_CI-V_Band_Decoder.h"


void Band_Decode_Output(uint8_t band, bool IF_Switch_ON);
void GPIO_Out(uint16_t pattern, bool IF_Switch_ON);
void PTT_Output(uint8_t band, bool PTT_state);
void GPIO_PTT_Out(uint16_t pattern, bool PTT_state);

uint8_t Module_4in_8out_Input_scan(void);
uint8_t Unit_EXTIO2_Input_scan(void);
uint8_t M5STAMPC3U_Input_scan(void);

void Module_4in_8out_setup();
void Module_4_Relay_setup();
void Unit_EXTIO2_setup();
void Unit_RELAY4_setup();
void MCP23017_IO_setup();

enum band_idx { DUMMY,
                BAND_AM,
                BAND_160M,
                BAND_80M,
                BAND_60M,
                BAND_40M,
                BAND_30M,
                BAND_20M,
                BAND_17M,
                BAND_15M,
                BAND_12M,
                BAND_10M,
                BAND_6M,
                BAND_FM,
                BAND_AIR,
                BAND_2M,
                BAND_1_25M,
                BAND_70cm,
                BAND_33cm,
                BAND_23cm,
                BAND_13cm,
                BAND_9cm,
                BAND_6cm,
                BAND_3cm,
                BAND_24G,
                BAND_47G,
                BAND_76G,
                BAND_122G,
                B_GENERAL,
                NUM_OF_BANDS };

#define GPIO_PIN_NOT_USED   255  // use for any pin not in use below

// Make IO Pin assignments here. 

// These are 4 RELAY (_RY_PIN_x) outputs on the Module_4RELAY module
// 8 outputs are defined here for stacking 2 4-port modules, or 4 modules for 16 ports
#define GPIO_MOD1_RY_PIN_0    0   // Mod1 0-3 are module 1 i2c addr 0x26
#define GPIO_MOD1_RY_PIN_1    1
#define GPIO_MOD1_RY_PIN_2    2
#define GPIO_MOD1_RY_PIN_3    3
#define GPIO_MOD2_RY_PIN_4    0   // 4-7 are on module 2 if used, addr 0xxxx - reassign addr from default of 0x26 before stacking.
#define GPIO_MOD2_RY_PIN_5    1  // if using the ext.IO i2c extender unit it is default of 0x27 so choose wisely
#define GPIO_MOD2_RY_PIN_6    2
#define GPIO_MOD2_RY_PIN_7    3
#define GPIO_MOD3_RY_PIN_8    0   // Mod 2 0-3 are module 1 i2c addr 0x27
#define GPIO_MOD3_RY_PIN_9    1
#define GPIO_MOD3_RY_PIN_10   2
#define GPIO_MOD3_RY_PIN_11   3
#define GPIO_MOD4_RY_PIN_12   0   // 4-7 are on module 2 if used, addr 0xxxx - reassign addr from default of 0x26 before stacking.
#define GPIO_MOD4_RY_PIN_13   1  // if using the ext.IO i2c extender unit it is default of 0x27 so choose wisely
#define GPIO_MOD4_RY_PIN_14   2
#define GPIO_MOD4_RY_PIN_15   3

// These are 8 MOSFET outputs (_MO_PIN_x) on the 4-In/8-Out module
#define GPIO_MOD1_MO_PIN_0    0
#define GPIO_MOD1_MO_PIN_1    1
#define GPIO_MOD1_MO_PIN_2    2
#define GPIO_MOD1_MO_PIN_3    3
#define GPIO_MOD1_MO_PIN_4    4
#define GPIO_MOD1_MO_PIN_5    5
#define GPIO_MOD1_MO_PIN_6    6
#define GPIO_MOD1_MO_PIN_7    7
// pins for 2nd module if ever setup
#define GPIO_MOD2_MO_PIN_8    0
#define GPIO_MOD2_MO_PIN_9    1
#define GPIO_MOD2_MO_PIN_10   2
#define GPIO_MOD2_MO_PIN_11   3
#define GPIO_MOD2_MO_PIN_12   4
#define GPIO_MOD2_MO_PIN_13   5
#define GPIO_MOD2_MO_PIN_14   6
#define GPIO_MOD2_MO_PIN_15   7

// These are 4 digital inputs (_MI_PIN_x) on the 4-In/8-Out module - 4 per module
#define GPIO_MOD1_MI_PIN_0    0  // 1st Module
#define GPIO_MOD1_MI_PIN_1    1
#define GPIO_MOD1_MI_PIN_2    2
#define GPIO_MOD1_MI_PIN_3    3

#define GPIO_MOD2_MI_PIN_4    0   // 2nd module
#define GPIO_MOD3_MI_PIN_5    1
#define GPIO_MOD2_MI_PIN_6    2
#define GPIO_MOD2_MI_PIN_7    3


// These are the 8 IO (_IO_x) pins on the EXT.IO2 UNIT.  Lower 4 will be inputs, upper outputs
#define GPIO_MOD1_IO_PIN_0    0
#define GPIO_MOD1_IO_PIN_1    1
#define GPIO_MOD1_IO_PIN_2    2
#define GPIO_MOD1_IO_PIN_3    3
#define GPIO_MOD1_IO_PIN_4    4
#define GPIO_MOD1_IO_PIN_5    5
#define GPIO_MOD1_IO_PIN_6    6
#define GPIO_MOD1_IO_PIN_7    7

#define GPIO_MOD2_IO_PIN_8    0  // 2nd module
#define GPIO_MOD2_IO_PIN_9    1
#define GPIO_MOD2_IO_PIN_10   2
#define GPIO_MOD2_IO_PIN_11   3
#define GPIO_MOD2_IO_PIN_12   4
#define GPIO_MOD2_IO_PIN_13   5
#define GPIO_MOD2_IO_PIN_14   6
#define GPIO_MOD2_IO_PIN_15   7

// These are the 8 ports (PA0-7) configured for buffered outputs on the 1st of 2 MCP23017 Port Expander module
#define GPIO_MOD1_PE_PIN_0    0
#define GPIO_MOD1_PE_PIN_1    1
#define GPIO_MOD1_PE_PIN_2    2 // 222 Xvtr board PTT  (PTT)
#define GPIO_MOD1_PE_PIN_3    3 // 903 Xvtr board PTT  (PTT)
#define GPIO_MOD1_PE_PIN_4    4 // 1296 Xvtr board PTT (PTT)
#define GPIO_MOD1_PE_PIN_5    5
#define GPIO_MOD1_PE_PIN_6    6
#define GPIO_MOD1_PE_PIN_7    7

// PB0-7 on module 1 are unbuffered
#define GPIO_MOD1_PE_PIN_8    8  // IF SW A (BAND)
#define GPIO_MOD1_PE_PIN_9    9  // IF SW B (BAND)
#define GPIO_MOD1_PE_PIN_10   10 // IF SW C (BAND)
#define GPIO_MOD1_PE_PIN_11   11 // Relay Module 2 - 12V power to 28VDC converter for 222 and 903 bands (BAND) 
#define GPIO_MOD1_PE_PIN_12   12 // Relay Module 2 - 28V to 222 amp (PTT)
#define GPIO_MOD1_PE_PIN_13   13 // Relay Module 2 - 28V to 903 Amp  (PTT)
#define GPIO_MOD1_PE_PIN_14   14 //  903 T/R coax switch (PTT)
#define GPIO_MOD1_PE_PIN_15   15 // 1296 T/R coax switch (PTT)

// PA0-7 on module 2 are buffered outputs for PTT outputs
#define GPIO_MOD2_PE_PIN_0    0 // HF/6M PTT
#define GPIO_MOD2_PE_PIN_1    1 // 144 PTT
#define GPIO_MOD2_PE_PIN_2    2 // 222 PTT
#define GPIO_MOD2_PE_PIN_3    3 // 430 PTT
#define GPIO_MOD2_PE_PIN_4    4 // 903 PTT
#define GPIO_MOD2_PE_PIN_5    5 // 1296 PTT
#define GPIO_MOD2_PE_PIN_6    6 // Spare
#define GPIO_MOD2_PE_PIN_7    7 // PTT to IF T/R relay

// PB0-7 on module 2  are unbuffered outputs for SP4T RF switch control and a bank of 4 relays that turn on 12V to each Xvtr and the 1296 12V PA
// The 28V to 222 and 903 PA units are on Module 1
#define GPIO_MOD2_PE_PIN_8    8  // SP4T switch (BAND)
#define GPIO_MOD2_PE_PIN_9    9  // SP4T switch (BAND)
#define GPIO_MOD2_PE_PIN_10   10 // SP4T switch (BAND)
#define GPIO_MOD2_PE_PIN_11   11 // SP4T switch (BAND)
#define GPIO_MOD2_PE_PIN_12   12 // Relay Module 1 relay 4 12V to 1296 Xvtr board (BAND)
#define GPIO_MOD2_PE_PIN_13   13 // Relay Module 1 relay 3 12v to 1296 PA (PTT)
#define GPIO_MOD2_PE_PIN_14   14 // Relay Module 1 relay 2 12V to 903 Xvtr board (BAND)
#define GPIO_MOD2_PE_PIN_15   15 // Relay Module 1 relay 1 12V to 222 Xvtr board (BAND)


//  These are IO pins on the C3U module intended for Band decode input and PTT input and IF switch control
//  PTT is the only IO pin that requires high speed scanning.  
//  Putting PTT on the CPU instead of the MCP23017 avoids putting high speed daa flow on the i2c bus which causes noise on the 222 band
//  Using the CPU IO lets the i2c bus be polled at low speed reducing radiated noise.

#define GPIO_C3U_BAND_0     10
#define GPIO_C3U_BAND_1     8
#define GPIO_C3U_BAND_2     7
#define GPIO_C3U_PTT        6
#define GPIO_C3U_BAND_3     5
#define GPIO_C3U_SPARE1     4
#define GPIO_C3U_SPARE2     3


// BAND DECODE INPUT (_INPUT_) PINS
// Assign your pins of choice.  Use a number or one of the existing #define number names
// Make sure they are not monitored by the code as a button or other use like an encoder.
// If not used set to GPIO_PIN_NOT_USED since there is no pin 255.
#ifndef M5STAMPC3U
  #define BAND_DECODE_INPUT_0        GPIO_MOD_MI_PIN_0      // bit 0   Band 0
  #define BAND_DECODE_INPUT_1        GPIO_MOD_MI_PIN_1      // bit 1   Band 1
  #define BAND_DECODE_INPUT_2        GPIO_MOD_MI_PIN_2      // bit 2   Band2
  #define BAND_DECODE_INPUT_3        GPIO_MOD_MI_PIN_3      // bit 3   PTT Input
  #define BAND_DECODE_INPUT_4        GPIO_PIN_NOT_USED      // bit 0
  #define BAND_DECODE_INPUT_5        GPIO_PIN_NOT_USED      // bit 1
  #define BAND_DECODE_INPUT_6        GPIO_PIN_NOT_USED      // bit 2
  #define BAND_DECODE_INPUT_7        GPIO_PIN_NOT_USED      // bit 3
#else  // Transverter Box  M5STAMP C3U controller PA0-7 on MCP23017 Module 1
  #define BAND_DECODE_INPUT_0        GPIO_C3U_BAND_0        // bit 0   Band 0
  #define BAND_DECODE_INPUT_1        GPIO_C3U_BAND_1        // bit 1   Band 1
  #define BAND_DECODE_INPUT_2        GPIO_C3U_BAND_2        // bit 2   Band 2
  #define BAND_DECODE_INPUT_3        GPIO_C3U_PTT           // bit 3   PTT Input
  #define BAND_DECODE_INPUT_4        GPIO_PIN_NOT_USED      // bit 0
  #define BAND_DECODE_INPUT_5        GPIO_PIN_NOT_USED      // bit 1
  #define BAND_DECODE_INPUT_6        GPIO_PIN_NOT_USED      // bit 2
  #define BAND_DECODE_INPUT_7        GPIO_PIN_NOT_USED      // bit 3
#endif

// BAND DECODE OUTPUT PINS
// Assign your pins of choice.  Use a number or one of the existing #define number names
// Make sure they are not monitored by the code as a button or other use like an encoder.
// If not used set to GPIO_PIN_NOT_USED since there is no pin 255.

#ifndef M5STAMPC3U
  // 8 are inputs, 8 are defined under PTT section, and remaining 16 are for general outputs
  #ifdef XVBOX  // Front end controller talking to Xvtr box embedded controller

    #define BAND_DECODE_OUTPUT_0        GPIO_MOD1_IO_PIN_0     // bit 0  0-2 are band decoder, 3 is ptt to Xvtr Box
    #define BAND_DECODE_OUTPUT_1        GPIO_MOD1_IO_PIN_1     // bit 1
    #define BAND_DECODE_OUTPUT_2        GPIO_MOD1_IO_PIN_2     // bit 2
    #define BAND_DECODE_OUTPUT_3        GPIO_PIN_NOT_USED      // bit 3
    #define BAND_DECODE_OUTPUT_4        GPIO_PIN_NOT_USED      // bit 4
    #define BAND_DECODE_OUTPUT_5        GPIO_PIN_NOT_USED      // bit 5
    #define BAND_DECODE_OUTPUT_6        GPIO_PIN_NOT_USED      // bit 6
    #define BAND_DECODE_OUTPUT_7        GPIO_PIN_NOT_USED      // bit 7
  #else
    #define BAND_DECODE_OUTPUT_0        GPIO_MOD1_IO_PIN_0     // bit 0  
    #define BAND_DECODE_OUTPUT_1        GPIO_MOD1_IO_PIN_1     // bit 1
    #define BAND_DECODE_OUTPUT_2        GPIO_MOD1_IO_PIN_2     // bit 2
    #define BAND_DECODE_OUTPUT_3        GPIO_MOD1_IO_PIN_3     // bit 3
    #define BAND_DECODE_OUTPUT_4        GPIO_PIN_NOT_USED      // bit 4
    #define BAND_DECODE_OUTPUT_5        GPIO_PIN_NOT_USED      // bit 5
    #define BAND_DECODE_OUTPUT_6        GPIO_PIN_NOT_USED      // bit 6
    #define BAND_DECODE_OUTPUT_7        GPIO_PIN_NOT_USED      // bit 7
  #endif

  // on 2nd module if present and setup
  #define BAND_DECODE_OUTPUT_8        GPIO_PIN_NOT_USED      // bit 0
  #define BAND_DECODE_OUTPUT_9        GPIO_PIN_NOT_USED      // bit 1
  #define BAND_DECODE_OUTPUT_10       GPIO_PIN_NOT_USED      // bit 2
  #define BAND_DECODE_OUTPUT_11       GPIO_PIN_NOT_USED      // bit 3
  #define BAND_DECODE_OUTPUT_12       GPIO_PIN_NOT_USED      // bit 4
  #define BAND_DECODE_OUTPUT_13       GPIO_PIN_NOT_USED      // bit 5
  #define BAND_DECODE_OUTPUT_14       GPIO_PIN_NOT_USED      // bit 6
  #define BAND_DECODE_OUTPUT_15       GPIO_PIN_NOT_USED      // bit 7

#else  // Transverter Box embedded controller config
  
  // PA0-7 Module 1 has 3 internal Xvtr PTT outputs.  Has ULN2803A buffer module.
  // PA0-7 Module 2 has 6 external PTT outputs and 1 PTT to the TC board.  Has ULN2803A buffer module.
  // Since they are all PTT they are mapped under the PTT section, not here

  // PA0-7 Module 2 are listed under PTT outputs, not here.
  // PB0-7 Module 1 has 3 SP6T IF switch control lines PB0-2. 
  // PB3 is future Relay 2 module Relay A - 12V power to 28V converer for 222 and 902 50W amps.
  // PB6-7 are 903 and 1297 T/R switches (controlled under PTT lists, not here).
 
  // There are 32 IO ports total between the 2x MCP23017 port expanders.  7 more on the CPU.  Any can be mapped in the IO bank.  
  // The configuration uses a 16 bit integer to keep things manageable, so there are only 16 band outputs and 16 PTT outputs.  
  // Pins can be assigned to any grouping but Module 1 pins must stay on outputs 0-7 and Modules 2 pins must stay in outputs 8-15
  
  // Module 1 grouping
  #define BAND_DECODE_OUTPUT_0        GPIO_MOD1_PE_PIN_8      // bit 0  Internal SP6T IF switch control A
  #define BAND_DECODE_OUTPUT_1        GPIO_MOD1_PE_PIN_9      // bit 1  Internal SP6T IF switch control B
  #define BAND_DECODE_OUTPUT_2        GPIO_MOD1_PE_PIN_10     // bit 2  Internal SP6T IF switch control C
  #define BAND_DECODE_OUTPUT_3        GPIO_MOD1_PE_PIN_11     // bit 3  Relay Module 2 - Relay #1 - 12V to 28V DC-DC converter power.  Turn on for 222 and 903 bands.
  #define BAND_DECODE_OUTPUT_4        GPIO_PIN_NOT_USED       // bit 4  Spare
  #define BAND_DECODE_OUTPUT_5        GPIO_PIN_NOT_USED       // bit 5  Spare
  #define BAND_DECODE_OUTPUT_6        GPIO_PIN_NOT_USED       // bit 6  Spare
  #define BAND_DECODE_OUTPUT_7        GPIO_PIN_NOT_USED       // bit 7  Spare
// Module 2 grouping
  #define BAND_DECODE_OUTPUT_8        GPIO_MOD2_PE_PIN_8      // bit 0  SP4T switch
  #define BAND_DECODE_OUTPUT_9        GPIO_MOD2_PE_PIN_9      // bit 1  SP4T switch
  #define BAND_DECODE_OUTPUT_10       GPIO_MOD2_PE_PIN_10     // bit 2  SP4T switch
  #define BAND_DECODE_OUTPUT_11       GPIO_MOD2_PE_PIN_11     // bit 3  SP4T switch
  #define BAND_DECODE_OUTPUT_12       GPIO_MOD2_PE_PIN_15     // bit 4  Relay Module 1 - 12V to 222 Xvtr
  #define BAND_DECODE_OUTPUT_13       GPIO_MOD2_PE_PIN_14     // bit 5  Relay Module 1 - 12V to 903 Xvtr 
  #define BAND_DECODE_OUTPUT_14       GPIO_PIN_NOT_USED       // bit 6  Spare (physical io pin 13 is used by 1296 PTT so not listed here in this logical channel mapping)
  #define BAND_DECODE_OUTPUT_15       GPIO_MOD2_PE_PIN_12     // bit 7  Relay Module 1 - 12V to 1296 XVtr
#endif

#ifndef M5STAMPC3U
  // BAND DECODE PTT OUTPUT PINS
  // Assign your pins of choice.  Use a number or one of the existing #define number names
  // Make sure they are not monitored by the code as a button or other use like an encoder.
  // If not used set to GPIO_PIN_NOT_USED since there is no pin 255.
  
  #ifdef XVBOX  // only 4 outputs used on the XVBOX config, 3 for band decode out, 1 for PTT out

    #define BAND_DECODE_PTT_OUTPUT_0    GPIO_PIN_NOT_USED     // bit 0
    #define BAND_DECODE_PTT_OUTPUT_1    GPIO_PIN_NOT_USED     // bit 1
    #define BAND_DECODE_PTT_OUTPUT_2    GPIO_PIN_NOT_USED     // bit 2
    #define BAND_DECODE_PTT_OUTPUT_3    GPIO_MOD1_IO_PIN_3    // bit 3
    #define BAND_DECODE_PTT_OUTPUT_4    GPIO_PIN_NOT_USED     // bit 4
    #define BAND_DECODE_PTT_OUTPUT_5    GPIO_PIN_NOT_USED     // bit 5
    #define BAND_DECODE_PTT_OUTPUT_6    GPIO_PIN_NOT_USED     // bit 6
    #define BAND_DECODE_PTT_OUTPUT_7    GPIO_PIN_NOT_USED     // bit 7
  #else  // first 4 are band decode, 2nd 4 are PTT outputs matching each band decoder output
    #define BAND_DECODE_PTT_OUTPUT_0    GPIO_PIN_NOT_USED     // bit 0 
    #define BAND_DECODE_PTT_OUTPUT_1    GPIO_PIN_NOT_USED     // bit 1
    #define BAND_DECODE_PTT_OUTPUT_2    GPIO_PIN_NOT_USED     // bit 2
    #define BAND_DECODE_PTT_OUTPUT_3    GPIO_PIN_NOT_USED     // bit 3
    #define BAND_DECODE_PTT_OUTPUT_4    GPIO_MOD1_IO_PIN_4     // bit 4
    #define BAND_DECODE_PTT_OUTPUT_5    GPIO_MOD1_IO_PIN_5     // bit 5
    #define BAND_DECODE_PTT_OUTPUT_6    GPIO_MOD1_IO_PIN_6     // bit 6
    #define BAND_DECODE_PTT_OUTPUT_7    GPIO_MOD1_IO_PIN_7     // bit 7
  #endif
  
  #define BAND_DECODE_PTT_OUTPUT_8    GPIO_PIN_NOT_USED     // bit 0 PB0-7 on Module 1
  #define BAND_DECODE_PTT_OUTPUT_9    GPIO_PIN_NOT_USED     // bit 1
  #define BAND_DECODE_PTT_OUTPUT_10   GPIO_PIN_NOT_USED     // bit 2
  #define BAND_DECODE_PTT_OUTPUT_11   GPIO_PIN_NOT_USED     // bit 3
  #define BAND_DECODE_PTT_OUTPUT_12   GPIO_PIN_NOT_USED     // bit 4
  #define BAND_DECODE_PTT_OUTPUT_13   GPIO_PIN_NOT_USED     // bit 5
  #define BAND_DECODE_PTT_OUTPUT_14   GPIO_PIN_NOT_USED     // bit 6
  #define BAND_DECODE_PTT_OUTPUT_15   GPIO_PIN_NOT_USED     // bit 7
#else  
  // Xvtr Box uses outputs on PA0-3 and PB6-7 on 1st module.  PA0-7 are buffered
  // PA0-7 on 2nd MCP23017 module for band specific PTT outputs for amp PTTs amnd is buffered
  // For the transverter internal PTT, there is 9V in RX state so a buffer is required.
  // Note: this is a mapping between virutal pins and physical pins.The actual pins can be assigned in any order *within* a module.
  // Pins mus be within MOD1 group (ie first buffer/IO board)
  #define BAND_DECODE_PTT_OUTPUT_0    GPIO_MOD1_PE_PIN_12    // bit 0  222 28V Amp via relay module #2 - Relay 2
  #define BAND_DECODE_PTT_OUTPUT_1    GPIO_MOD1_PE_PIN_13    // bit 1  903 28V Amp via relay module #2 - Relay 3
  #define BAND_DECODE_PTT_OUTPUT_2    GPIO_PIN_NOT_USED    // bit 2  12-28V DC DC Converter 12V supply
  #define BAND_DECODE_PTT_OUTPUT_3    GPIO_MOD1_PE_PIN_2     // bit 3  222 Xvtr Brd PTT       0 = TX    
  #define BAND_DECODE_PTT_OUTPUT_4    GPIO_MOD1_PE_PIN_3     // bit 4  902/903 Xvtr Brd PTT   0 = TX   
  #define BAND_DECODE_PTT_OUTPUT_5    GPIO_MOD1_PE_PIN_4     // bit 5  1296 Xvtr Brd PTT      0 = TX 
  #define BAND_DECODE_PTT_OUTPUT_6    GPIO_MOD1_PE_PIN_14    // bit 6  T/R switch, 903 RF Xvtr board output  1 = TX
  #define BAND_DECODE_PTT_OUTPUT_7    GPIO_MOD1_PE_PIN_15    // bit 7  T/R switch, 1296 RF Xvtr board output   1 = TX
  
  // 2nd module PA0-7 are PTT outputs on back panel to amps and/or antennas  These are buffered.  Pins must be in MOD 2 group
  #define BAND_DECODE_PTT_OUTPUT_8    GPIO_MOD2_PE_PIN_0     // bit 0  HF/50 PTT
  #define BAND_DECODE_PTT_OUTPUT_9    GPIO_MOD2_PE_PIN_1     // bit 1  144 PTT
  #define BAND_DECODE_PTT_OUTPUT_10   GPIO_MOD2_PE_PIN_2     // bit 2  222 PTT
  #define BAND_DECODE_PTT_OUTPUT_11   GPIO_MOD2_PE_PIN_3     // bit 3  432 PTT
  #define BAND_DECODE_PTT_OUTPUT_12   GPIO_MOD2_PE_PIN_4     // bit 4  902/903 PTT
  #define BAND_DECODE_PTT_OUTPUT_13   GPIO_MOD2_PE_PIN_5     // bit 5  1296 PTT
  #define BAND_DECODE_PTT_OUTPUT_14   GPIO_MOD2_PE_PIN_13    // bit 6  12V to 1296 PA on PTT
  #define BAND_DECODE_PTT_OUTPUT_15   GPIO_MOD2_PE_PIN_7     // bit 7  PTT to TC board IF input TR relay
#endif

// *************************************************************************************************************
//
//   Assign pin pattern to each band as needed
//
//**************************************************************************************************************

// BAND DECODE INPUT PATTERN MAPPING TO BAND
// Select band by wired input by mapping the input pattern to a band
// Set bands not selectable by wire pattern to 0xFF

#ifndef M5STAMPC3U
  // For normal usage with M5Stack CoreXX modules
  #ifdef XVBOX  // COnfig to talk to Xvtrt box controller

    #define DECODE_INPUT_DUMMY        (0xFF)    //Dummy Row
    #define DECODE_INPUT_BANDAM       (0x01)    //AM
    #define DECODE_INPUT_BAND160M     (0x01)    //160M 
    #define DECODE_INPUT_BAND80M      (0x01)    //80M
    #define DECODE_INPUT_BAND60M      (0x01)    //60M
    #define DECODE_INPUT_BAND40M      (0x01)    //40M
    #define DECODE_INPUT_BAND30M      (0x01)    //30M
    #define DECODE_INPUT_BAND20M      (0x01)    //20M
    #define DECODE_INPUT_BAND17M      (0x01)    //17M      
    #define DECODE_INPUT_BAND15M      (0x01)    //15M
    #define DECODE_INPUT_BAND12M      (0x01)    //12M
    #define DECODE_INPUT_BAND10M      (0x01)    //10M
    #define DECODE_INPUT_BANDFM       (0x01)    //FM Braodcast Band
    #define DECODE_INPUT_BANDAIR      (0x01)    //Aircraft Band
    //#define DECODE_INPUT_BAND70       (0x01)    //70MHz
    #define DECODE_INPUT_BAND6M       (0x01)    //6M
    #define DECODE_INPUT_BAND144      (0x02)    //2M
    #define DECODE_INPUT_BAND222      (0x03)    //222
    #define DECODE_INPUT_BAND432      (0x04)    //432
    #define DECODE_INPUT_BAND902      (0x05)    //902
    #define DECODE_INPUT_BAND1296     (0x06)    //1296
    #define DECODE_INPUT_BAND2400     (0x01)    //2400
    #define DECODE_INPUT_BAND3300     (0x01)    //3400
    #define DECODE_INPUT_BAND5760     (0x01)    //5760M
    #define DECODE_INPUT_BAND10G      (0x01)    //10.368.1G
    #define DECODE_INPUT_BAND24G      (0x01)    //24.192G
    #define DECODE_INPUT_BAND47G      (0x01)    //47.1G
    #define DECODE_INPUT_BAND76G      (0x01)    //76.1G
    #define DECODE_INPUT_BAND122G     (0x01)    //122G
    #define DECODE_INPUT_B_GENERAL    (0x01)     // Non-Ham Band
  #else  // default for 4 Xvtr, 903, 2304, 3300, 5760
    #define DECODE_INPUT_DUMMY        (0xFF)    //Dummy Row
    #define DECODE_INPUT_BANDAM       (0xFF)    //AM
    #define DECODE_INPUT_BAND160M     (0xFF)    //160M 
    #define DECODE_INPUT_BAND80M      (0xFF)    //80M
    #define DECODE_INPUT_BAND60M      (0xFF)    //60M
    #define DECODE_INPUT_BAND40M      (0xFF)    //40M
    #define DECODE_INPUT_BAND30M      (0xFF)    //30M
    #define DECODE_INPUT_BAND20M      (0xFF)    //20M
    #define DECODE_INPUT_BAND17M      (0xFF)    //17M      
    #define DECODE_INPUT_BAND15M      (0xFF)    //15M
    #define DECODE_INPUT_BAND12M      (0xFF)    //12M
    #define DECODE_INPUT_BAND10M      (0xFF)    //10M
    #define DECODE_INPUT_BANDFM       (0xFF)    //FM Braodcast Band
    #define DECODE_INPUT_BANDAIR      (0xFF)    //Aircraft Band
    //#define DECODE_INPUT_BAND70       (0xFF)    //70MHz
    #define DECODE_INPUT_BAND6M       (0xFF)    //6M
    #define DECODE_INPUT_BAND144      (0xFF)    //2M
    #define DECODE_INPUT_BAND222      (0xFF)    //222
    #define DECODE_INPUT_BAND432      (0xFF)    //432
    #define DECODE_INPUT_BAND902      (0x01)    //902
    #define DECODE_INPUT_BAND1296     (0xFF)    //1296
    #define DECODE_INPUT_BAND2400     (0x02)    //2400
    #define DECODE_INPUT_BAND3300     (0x04)    //3400
    #define DECODE_INPUT_BAND5760     (0x08)    //5760M
    #define DECODE_INPUT_BAND10G      (0xFF)    //10.368.1G
    #define DECODE_INPUT_BAND24G      (0xFF)    //24.192G
    #define DECODE_INPUT_BAND47G      (0xFF)    //47.1G
    #define DECODE_INPUT_BAND76G      (0xFF)    //76.1G
    #define DECODE_INPUT_BAND122G     (0xFF)    //122G
    #define DECODE_INPUT_B_GENERAL    (0xFF)     // Non-Ham Band
  #endif

#else  // For Xvtr Box with M5StampC3U
  #define DECODE_INPUT_DUMMY        (0xFF)    //Dummy Row
  #define DECODE_INPUT_BANDAM       (0x01)    //AM
  #define DECODE_INPUT_BAND160M     (0x01)    //160M 
  #define DECODE_INPUT_BAND80M      (0x01)    //80M
  #define DECODE_INPUT_BAND60M      (0x01)    //60M
  #define DECODE_INPUT_BAND40M      (0x01)    //40M
  #define DECODE_INPUT_BAND30M      (0x01)    //30M
  #define DECODE_INPUT_BAND20M      (0x01)    //20M
  #define DECODE_INPUT_BAND17M      (0x01)    //17M      
  #define DECODE_INPUT_BAND15M      (0x01)    //15M
  #define DECODE_INPUT_BAND12M      (0x01)    //12M
  #define DECODE_INPUT_BAND10M      (0x01)    //10M
  #define DECODE_INPUT_BANDFM       (0x01)    //FM Braodcast Band
  #define DECODE_INPUT_BANDAIR      (0x01)    //Aircraft Band
  //#define DECODE_INPUT_BAND70       (0xFF)    //70MHz
  #define DECODE_INPUT_BAND6M       (0x01)    //6M
  #define DECODE_INPUT_BAND144      (0x02)    //2M
  #define DECODE_INPUT_BAND222      (0x03)    //222
  #define DECODE_INPUT_BAND432      (0x04)    //432
  #define DECODE_INPUT_BAND902      (0x05)    //902
  #define DECODE_INPUT_BAND1296     (0x06)    //1296
  #define DECODE_INPUT_BAND2400     (0xFF)    //2400
  #define DECODE_INPUT_BAND3300     (0xFF)    //3400
  #define DECODE_INPUT_BAND5760     (0xFF)    //5760M
  #define DECODE_INPUT_BAND10G      (0xFF)    //10.368.1G
  #define DECODE_INPUT_BAND24G      (0xFF)    //24.192G
  #define DECODE_INPUT_BAND47G      (0xFF)    //47.1G
  #define DECODE_INPUT_BAND76G      (0xFF)    //76.1G
  #define DECODE_INPUT_BAND122G     (0xFF)    //122G
  #define DECODE_INPUT_B_GENERAL    (0xFF)     // Non-Ham Band
#endif

// Band Decode Output pattern
// Example: For Core module with just the 4In/8out module, there are 8 output.  Use lowest half of the value.
#ifndef M5STAMPC3U
  #define DECODE_BAND_DUMMY   (0x0000)    //Dummy Row
  #define DECODE_BANDAM       (0x0001)    //AM
  #define DECODE_BAND160M     (0x0001)    //160M 
  #define DECODE_BAND80M      (0x0001)    //80M
  #define DECODE_BAND60M      (0x0001)    //60M
  #define DECODE_BAND40M      (0x0001)    //40M
  #define DECODE_BAND30M      (0x0001)    //30M
  #define DECODE_BAND20M      (0x0001)    //20M
  #define DECODE_BAND17M      (0x0001)    //17M      
  #define DECODE_BAND15M      (0x0001)    //15M
  #define DECODE_BAND12M      (0x0001)    //12M
  #define DECODE_BAND10M      (0x0001)    //10M
  #define DECODE_BANDFM       (0x0001)    //6M
  #define DECODE_BANDAIR      (0x0001)    //6M
  //#define DECODE_BAND70       (0x0000)    //70MHz
  #define DECODE_BAND6M       (0x0001)    //6M
  #define DECODE_BAND144      (0x0002)    //2M
  #define DECODE_BAND222      (0x0003)    //222
  #define DECODE_BAND432      (0x0004)    //432
  #define DECODE_BAND902      (0x0005)    //902
  #define DECODE_BAND1296     (0x0006)    //1296
  #define DECODE_BAND2400     (0x0000)    //2400
  #define DECODE_BAND3300     (0x0000)    //3400
  #define DECODE_BAND5760     (0x0000)    //5760M
  #define DECODE_BAND10G      (0x0000)    //10.368.1G
  #define DECODE_BAND24G      (0x0000)    //24.192G
  #define DECODE_BAND47G      (0x0000)    //47.1G
  #define DECODE_BAND76G      (0x0000)    //76.1G
  #define DECODE_BAND122G     (0x0000)    //122G
  #define DECODE_B_GENERAL    (0x0000)     // Non-Ham Band
#else // For Xvtr Box with M5StampC3U
//  In Module 2, upper byte, the upper 4 bits are the 12V power relays.   0 = on for each relay.
// on relay module Relay 1 is 1296 but is wired PB4.  Relay 2 is 903, PB-5, 222 is relay 4, PB-7
//  bit 7 1296 12V ON to 1296 Xvtr  - binary 0111 or 0x7xyz
//  bit 6 Spare - binary 1011 of 0xBxyz  
//  bit 5 903 12V ON to 903 Xvtr   - binary 1101 or 0xDxyz  (but also need 903 amp when installed)
//  bit 4 222 12V ON to 222 Xvtr    - binary 1110 or 0xExyz
//  All non-Xvtr bands set top-most nibble to 0xFxxx

//  The upper byte, lower 4 bits are the SP4T coax switch.   1 = on for each of 4 ports.
//  Example:  for HF/6M, we want the SP4T switch Port 2 (Xvtr Mode input path is on Port 1), rest are dont't care since only used on Xvtr bands) so set them to 0.
//  Set the upper half (2nd MCP23017 port expander - 8 available outputs 8-16 in our numbering) bits 0-3
//  Any Xvtr band (222, 903, 1296) would be Port 1  - 0x0001  or 1
//  HF/50 would be Port 2 (0x02 - 2nd bit or bit 1) - 0x0010  or 2
//  144 would be Port 3 (0x04 - 3rd bit or bit 2)   - 0x0100  or 4
//  432 would be Port 4 (0x08 - 4th bit or bit 3)   - 0x1000  or 8
//  If band undefined then default to 0x0002 or 2 to pass through anything to the 50/HF port

// Moved 4 input pins from the MCP23107 to the CPU GPIO to get them off the MCP23017 to reduce noise from 
//    i2c activity as a result of fast PTT scanning.

// **** Sequencing *****
// Rx->Tx - The IF switch should be shut off and a 20ms delay should be inserted RX->TX after the relays and PTT are done.  This ensure no hot switching.
// Tx->Rx - The IF switch should be shut off at the start and enabled at the end to prevent noise at the Radio from switching.  
//           Delays are not required but can be added befoe the IF switch turn back on.
// The IF switch action can be created by calling GPIO_Out twice, first with bits 0-2 forced to 0x7, then delay 20ms, then call again with normal pattern

// Lower Byte   Upper nibble always 0
// bit 7-4 are spare 

// Lower Byte lower nibble 
// bit 3 - 12V to DC DC converter for 222 and 903 28V 50W amps
// Lower byte, lowest bits are PTT out.  6 dedicated buffered 
// bits 2-0 are IF SP6T switch.  C, B, A
//          CBA
//    RF1 = 000  NC     0
//    RF2 = 001  NC     1
//    RF3 = 010  222    2  
//    RF4 = 011  NC     3  
//    RF5 = 100  903    4
//    RF6 = 101  1296   5
//    OFF = 110         6
//    OFF = 111         7

// Examples           relays   SP4T     zero   28V IF_Switch
// HF/6M    0xF20F   b'1111    0010     0000     1 111    turn off 12 to 28V converter
// 144      0xF40F   b'1111    0100     0000     1 111    turn off 12 to 28V converter
// 222      0xE102   b'1110    0001     0000     0 010    turn on 12 to 28V converter
// 432      0xF80F   b'1111    1000     0000     1 111    turn off 12 to 28V converter
// 903      0xD104   b'1101    0001     0000     0 100    turn on 12 to 28V converter
// 1296     0x710D   b'0111    0001     0000     1 101  - turn off 12 to 28V converter amp relay operated by PTT not here
// All other bands same as HF/6M.  0xF207  1111 0010 0000 1111

// 0xYYZZ where ZZ is typically 07 sets the IF SP6T switch to all off.  Only needed for Xvtr bands

  #define ALL_OFF (0xF00F)  // This pattern can be sent at start of a band change to create a break before make effect (not used yet)
  #define DECODE_BAND_DUMMY   (0xF20F)    //Dummy Row
  #define DECODE_BANDAM       (0xF20F)    //AM
  #define DECODE_BAND160M     (0xF20F)    //160M 
  #define DECODE_BAND80M      (0xF20F)    //80M
  #define DECODE_BAND60M      (0xF20F)    //60M
  #define DECODE_BAND40M      (0xF20F)    //40M
  #define DECODE_BAND30M      (0xF20F)    //30M
  #define DECODE_BAND20M      (0xF20F)    //20M
  #define DECODE_BAND17M      (0xF20F)    //17M      
  #define DECODE_BAND15M      (0xF20F)    //15M
  #define DECODE_BAND12M      (0xF20F)    //12M
  #define DECODE_BAND10M      (0xF20F)    //10M
  #define DECODE_BANDFM       (0xF20F)    //6M
  #define DECODE_BANDAIR      (0xF20F)    //6M
  //#define DECODE_BAND70       (0xF20F)    //70MHz
  #define DECODE_BAND6M       (0xF20F)    //6M
  #define DECODE_BAND144      (0xF40F)    //2M
  #define DECODE_BAND222      (0xE102)    //222     0xZZZY y=2 for RF3 on SP6T
  #define DECODE_BAND432      (0xF80F)    //432
  #define DECODE_BAND902      (0xD104)    //902     0xZZZY y=4 for RF5 on SP6T switch.   Turn on both 12V relays, relays 2 & 3 for 903 Xvtr and Amp.
  #define DECODE_BAND1296     (0x710D)    //1296    0xZZZY y=5 for RF6 on SP6T switch.
  #define DECODE_BAND2400     (0xF20F)    //2400
  #define DECODE_BAND3300     (0xF20F)    //3400
  #define DECODE_BAND5760     (0xF20F)    //5760M
  #define DECODE_BAND10G      (0xF20F)    //10.3681G
  #define DECODE_BAND24G      (0xF20F)    //24.192G
  #define DECODE_BAND47G      (0xF20F)    //47.1G
  #define DECODE_BAND76G      (0xF20F)    //76.1G
  #define DECODE_BAND122G     (0xF20F)    //122G
  #define DECODE_B_GENERAL    (0xF20F)    // Non-Ham Band
#endif

// Band Decoder Output patterns for PTT routing.
// An example would be the BCD pattern following the Elecraft K3 HF-TRN table.  5 bits are used. Bit 4 =1 is VHF+ group
// here we have only 1 4-IN/8-Out module installed configured as 4x decode outputs and 4 PTT outputs for PTT routing to 4 amps/xvtrs
// Here the PTT lines are on the upper half of the group of 8 module outputs so we set values on the upper nibble, the lower will be ignored.
// if not used enter 0x00 
#ifndef M5STAMPC3U
  #ifdef XVBOX  // patterns sent to transverter box controller.  3 BCD band decode outputs and single PTT wire.  
                // Xvtr Box breaks out PTT in this case, not here as usual.  Single PTT on the 4th Output port so 0x0008

    #define DECODE_DUMMY_PTT        (0x0000)    //Dummy Row - acts as a reset as well.
    #define DECODE_BANDAM_PTT       (0x0000)    //AM_PTT 
    #define DECODE_BAND160M_PTT     (0x0008)    //160M_PTT 
    #define DECODE_BAND80M_PTT      (0x0008)    //80M_PTT
    #define DECODE_BAND60M_PTT      (0x0008)    //60M_PTT
    #define DECODE_BAND40M_PTT      (0x0008)    //40M_PTT
    #define DECODE_BAND30M_PTT      (0x0008)    //30M_PTT
    #define DECODE_BAND20M_PTT      (0x0008)    //20M_PTT
    #define DECODE_BAND17M_PTT      (0x0008)    //17M_PTT      
    #define DECODE_BAND15M_PTT      (0x0008)    //15M_PTT
    #define DECODE_BAND12M_PTT      (0x0008)    //12M_PTT
    #define DECODE_BAND10M_PTT      (0x0008)    //10M_PTT
    #define DECODE_BANDFM_PTT       (0x0000)    //6M_PTT
    #define DECODE_BANDAIR_PTT      (0x0000)    //6M_PTT
    //#define DECODE_BAND70_PTT    (0x0008)    //70M_PTTHz
    #define DECODE_BAND6M_PTT       (0x0008)    //6M_PTT
    #define DECODE_BAND144_PTT      (0x0008)    //2M_PTT
    #define DECODE_BAND222_PTT      (0x0008)    //222_PTT
    #define DECODE_BAND432_PTT      (0x0008)    //432_PTT
    #define DECODE_BAND902_PTT      (0x0008)    //902_PTT
    #define DECODE_BAND1296_PTT     (0x0008)    //1296_PTT  
    #define DECODE_BAND2400_PTT     (0x0008)    //2400_PTT
    #define DECODE_BAND3300_PTT     (0x0008)    //3400_PTT
    #define DECODE_BAND5760_PTT     (0x0008)    //5760_PTT
    #define DECODE_BAND10G_PTT      (0x0008)    //10.368.1G_PTT
    #define DECODE_BAND24G_PTT      (0x0008)    //24.192G_PTT
    #define DECODE_BAND47G_PTT      (0x0008)    //47.1G_PTT
    #define DECODE_BAND76G_PTT      (0x0008)    //76.1G_PTT
    #define DECODE_BAND122G_PTT     (0x0008)    //122G_PTT
    #define DECODE_B_GENERAL_PTT    (0x0000)     // Non-Ham Band

  #else // default pattern, no Xvtr box so break out PTT here
    #define DECODE_DUMMY_PTT        (0x0000)    //Dummy Row
    #define DECODE_BANDAM_PTT       (0x0000)   //16M_PTT 
    #define DECODE_BAND160M_PTT     (0x0000)   //160M_PTT 
    #define DECODE_BAND80M_PTT      (0x0000)    //80M_PTT
    #define DECODE_BAND60M_PTT      (0x0000)    //60M_PTT
    #define DECODE_BAND40M_PTT      (0x0000)    //40M_PTT
    #define DECODE_BAND30M_PTT      (0x0000)    //30M_PTT
    #define DECODE_BAND20M_PTT      (0x0000)    //20M_PTT
    #define DECODE_BAND17M_PTT      (0x0000)    //17M_PTT      
    #define DECODE_BAND15M_PTT      (0x0000)    //15M_PTT
    #define DECODE_BAND12M_PTT      (0x0000)    //12M_PTT
    #define DECODE_BAND10M_PTT      (0x0000)    //10M_PTT
    #define DECODE_BANDFM_PTT       (0x0000)    //6M_PTT
    #define DECODE_BANDAIR_PTT      (0x0000)    //6M_PTT
    //#define DECODE_BAND70_PTT    (0x0001)    //70M_PTTHz
    #define DECODE_BAND6M_PTT       (0x0000)    //6M_PTT
    #define DECODE_BAND144_PTT      (0x0000)    //2M_PTT
    #define DECODE_BAND222_PTT      (0x0000)    //222_PTT
    #define DECODE_BAND432_PTT      (0x0000)    //432_PTT
    #define DECODE_BAND902_PTT      (0x0001)    //902_PTT
    #define DECODE_BAND1296_PTT     (0x0000)    //1296_PTT
    #define DECODE_BAND2400_PTT     (0x0002)    //2400_PTT
    #define DECODE_BAND3300_PTT     (0x0004)    //3400_PTT
    #define DECODE_BAND5760_PTT     (0x0008)    //5760_PTT
    #define DECODE_BAND10G_PTT      (0x0000)    //10.368.1G_PTT
    #define DECODE_BAND24G_PTT      (0x0000)    //24.192G_PTT
    #define DECODE_BAND47G_PTT      (0x0000)    //47.1G_PTT
    #define DECODE_BAND76G_PTT      (0x0000)    //76.1G_PTT
    #define DECODE_BAND122G_PTT     (0x0000)    //122G_PTT
    #define DECODE_B_GENERAL_PTT    (0x0000)     // Non-Ham Band
  #endif
#else  
  // For Xvtr Box with M5StampC3U

  // upper byte  when non-xvtr band in TX is HF/6M + logic 1 for the relay bit 14 so 0100 0001  or 0x41
  // bit 15  TC PTT                // logic 1 for any xvtr band, 0 for others
  
  // bit 14  1296 PA power relay on TX   // Logic 0 for TX on 1296, logic 1 for all other bands

  // bit 13  1296 PTT              // for  1296 PTT, b'1 0 10 0000 1010 0011 0xA0A3
  // bit 12  903 PTT               // for   903 PTT, b'1 1 01 0000 0101 0101 0xD055
  // bit 11  430 PTT               // for   430 PTT, b'0 1 00 1000 0000 0111 0x4807
  // bit 10  222 PTT               // for   222 PTT, b'1 1 00 0100 0000 1110 0xC40E
  // bit 9   144 PTT               // for   144 PTT, b'0 1 00 0010 0000 0111 0x4207
  // bit 8   HF/6M PTT             // for HF/6M PTT, b'0 1 00 0001 0000 0111 0x4107
  // Lower byte  when non-xvtr band in TX is HF/6M is 0100 0001 000 0111 or 0x4107

  // bit 7  1296 RF output T/R coax switch   1 = ON
  // bit 6  903  RF output T/R coax switch   1 = ON - Logic 1 and 0.   Check for pullup in the switch, if so can likely use open collecor like ULN2803
  // bit 5  PTT to Xvtr 1296 Brd 0 = TX  - This requires a open collector like ULN2803 - Xvtr PTT has +9V on it and connecting to 3.3V output puts Xvtr into Tx - sort of.  
  // bit 4  PTT to Xvtr 903 Brd  0 = TX  - This requires a open collector like ULN2803 - Xvtr PTT has +9V on it and connecting to 3.3V output puts Xvtr into Tx - sort of.  
  
  // bit 3  PTT to Xvtr 222 Brd  0 = TX  - This requires a open collector like ULN2803 - Xvtr PTT has +9V on it and connecting to 3.3V output puts Xvtr into Tx - sort of.  
  // bit 2  PTT 12V to 28V via relay for 28V converter - wire to Relay Module #2 relay 1 - power on for 222 and 903
  // bit 1  PTT 28V to 903 Amp via relay  - wire to Relay Module #2 relay 3
  // bit 0  PTT 28V to 222 Amp via relay  - wire to Relay Module #2 relay 2
 
 // all together a HF/6M pattern is 0x4107 for PTT
  
  // Example Module 1 values for any non Xvtr band 
  //  invert outputs when buffered
  //   bit 7 6   54 3   210
  //       0 0 | 00 0 | 111   = 0x07 for TX and RX
  // For Xvtr band 222
  //   bit 7 6   54 3   210   
  //       0 0 | 00 H | 11L   = 0x0E = PTT on TX, 0x0E = RX on 222
  // For Xvtr band 903
  //   bit 7 6   54 3   210
  //       0 H | 0H 0 | 1L1   = 0x55 = PTT on TX, 0x05 = RX on 903
  // For Xvtr band 1296
  //   bit 7 6   54 3   210
  //       H 0 | H0 0 | L11   = 0xA3 = PTT on TX, 0x03 = RX on 1296
  
  //   Ideally the 903/1296 T/R switches would be set before the PTT issued to the Xvtr board

  // 0x0138 passes PTT through on HF/6M PTT output jack on back panel for all bands on HF and 6M.  Also sets the intenral Xvtr PTTs to RX state.
  // Highest byte is PTT out pin
  #define DECODE_DUMMY_PTT        (0x4007)    //Dummy Row  This pattern is also used to set RX state.  All other rows are TX state only.  
                                              //The 8 relays are Logic 0 to turn them on so set those pins high in RX, Lo in TX
  #define DECODE_BANDAM_PTT       (0x4107)   //16M_PTT 
  #define DECODE_BAND160M_PTT     (0x4107)   //160M_PTT 
  #define DECODE_BAND80M_PTT      (0x4107)    //80M_PTT
  #define DECODE_BAND60M_PTT      (0x4107)    //60M_PTT
  #define DECODE_BAND40M_PTT      (0x4107)    //40M_PTT
  #define DECODE_BAND30M_PTT      (0x4107)    //30M_PTT
  #define DECODE_BAND20M_PTT      (0x4107)    //20M_PTT
  #define DECODE_BAND17M_PTT      (0x4107)    //17M_PTT      
  #define DECODE_BAND15M_PTT      (0x4107)    //15M_PTT
  #define DECODE_BAND12M_PTT      (0x4107)    //12M_PTT
  #define DECODE_BAND10M_PTT      (0x4107)    //10M_PTT
  #define DECODE_BANDFM_PTT       (0x4107)    //6M_PTT
  #define DECODE_BANDAIR_PTT      (0x4107)    //6M_PTT
  //#define DECODE_BAND70_PTT      (0xC107)    //70M_PTT
  #define DECODE_BAND6M_PTT       (0x4107)    //6M_PTT
  #define DECODE_BAND144_PTT      (0x4207)    //2M_PTT
  #define DECODE_BAND222_PTT      (0xC40E)    //222_PTT   Turn on highest bit for TC Board IF relay/attenuator
  #define DECODE_BAND432_PTT      (0x4807)    //432_PTT
  #define DECODE_BAND902_PTT      (0xD055)    //902_PTT   Turn on highest bit for TC Board IF relay/attenuator
  #define DECODE_BAND1296_PTT     (0xA0A3)    //1296_PTT  Turn on highest bit for TC Board IF relay/attenuator
  #define DECODE_BAND2400_PTT     (0x4107)    //2400_PTT
  #define DECODE_BAND3300_PTT     (0x4107)    //3400_PTT
  #define DECODE_BAND5760_PTT     (0x4107)    //5760_PTT
  #define DECODE_BAND10G_PTT      (0x4107)    //10.368.1G_PTT
  #define DECODE_BAND24G_PTT      (0x4107)    //24.192G_PTT
  #define DECODE_BAND47G_PTT      (0x4107)    //47.1G_PTT
  #define DECODE_BAND76G_PTT      (0x4107)    //76.1G_PTT
  #define DECODE_BAND122G_PTT     (0x4107)    //122G_PTT
  #define DECODE_B_GENERAL_PTT    (0x4107)    // Any other band not defined
#endif

#endif