#ifndef DECODER_H_
#define DECODER_H_

#include <stdint.h>
#include "M5Stack_CI-V_Band_Decoder.h"

void Band_Decode_Output(uint8_t band);
void GPIO_Out(uint16_t pattern);
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
// 8 outputs are defined here for stacking 2 4-port modules
#define GPIO_MOD_RY_PIN_0    0   // Mod1 0-3 are module 1 i2c addr 0x26
#define GPIO_MOD_RY_PIN_1    1
#define GPIO_MOD_RY_PIN_2    2
#define GPIO_MOD_RY_PIN_3    3
#define GPIO_MOD_RY_PIN_4    4   // 4-7 are on module 2 if used, addr 0xxxx - reassign addr from default of 0x26 before stacking.
#define GPIO_MOD_RY_PIN_5    5  // if using the ext.IO i2c extender unit it is default of 0x27 so choose wisely
#define GPIO_MOD_RY_PIN_6    6
#define GPIO_MOD_RY_PIN_7    7
#define GPIO_MOD_RY_PIN_8    0   // Mod 2 0-3 are module 1 i2c addr 0x27
#define GPIO_MOD_RY_PIN_9    1
#define GPIO_MOD_RY_PIN_10   2
#define GPIO_MOD_RY_PIN_11   3
#define GPIO_MOD_RY_PIN_12   4   // 4-7 are on module 2 if used, addr 0xxxx - reassign addr from default of 0x26 before stacking.
#define GPIO_MOD_RY_PIN_13   5  // if using the ext.IO i2c extender unit it is default of 0x27 so choose wisely
#define GPIO_MOD_RY_PIN_14   6
#define GPIO_MOD_RY_PIN_15   7

// These are 8 MOSFET outputs (_MO_PIN_x) on the 4-In/8-Out module
#define GPIO_MOD_MO_PIN_0    0
#define GPIO_MOD_MO_PIN_1    1
#define GPIO_MOD_MO_PIN_2    2
#define GPIO_MOD_MO_PIN_3    3
#define GPIO_MOD_MO_PIN_4    4
#define GPIO_MOD_MO_PIN_5    5
#define GPIO_MOD_MO_PIN_6    6
#define GPIO_MOD_MO_PIN_7    7
#define GPIO_MOD_MO_PIN_8    0
#define GPIO_MOD_MO_PIN_9    1
#define GPIO_MOD_MO_PIN_10   2
#define GPIO_MOD_MO_PIN_11   3
#define GPIO_MOD_MO_PIN_12   4
#define GPIO_MOD_MO_PIN_13   5
#define GPIO_MOD_MO_PIN_14   6
#define GPIO_MOD_MO_PIN_15   7

// These are 4 digital inputs (_MI_PIN_x) on the 4-In/8-Out module - 4 per module
#define GPIO_MOD_MI_PIN_0    0  // 1st Module
#define GPIO_MOD_MI_PIN_1    1
#define GPIO_MOD_MI_PIN_2    2
#define GPIO_MOD_MI_PIN_3    3
#define GPIO_MOD_MI_PIN_4    0   // 2nd module
#define GPIO_MOD_MI_PIN_5    1
#define GPIO_MOD_MI_PIN_6    2
#define GPIO_MOD_MI_PIN_7    3


// These are the 8 IO (_IO_x) pins on the EXT.IO2 UNIT.  Lower 4 will be inputs, upper outputs
#define GPIO_MOD_IO_PIN_0    0
#define GPIO_MOD_IO_PIN_1    1
#define GPIO_MOD_IO_PIN_2    2
#define GPIO_MOD_IO_PIN_3    3
#define GPIO_MOD_IO_PIN_4    4
#define GPIO_MOD_IO_PIN_5    5
#define GPIO_MOD_IO_PIN_6    6
#define GPIO_MOD_IO_PIN_7    7
#define GPIO_MOD_IO_PIN_8    0  // 2nd module
#define GPIO_MOD_IO_PIN_9    1
#define GPIO_MOD_IO_PIN_10   2
#define GPIO_MOD_IO_PIN_11   3
#define GPIO_MOD_IO_PIN_12   4
#define GPIO_MOD_IO_PIN_13   5
#define GPIO_MOD_IO_PIN_14   6
#define GPIO_MOD_IO_PIN_15   7

// These are the 8 ports (PA0-7) configured for buffered inputs on the MCP23017 Port Expander modules
#define GPIO_MOD_PE1_PIN_0    0
#define GPIO_MOD_PE1_PIN_1    1
#define GPIO_MOD_PE1_PIN_2    2
#define GPIO_MOD_PE1_PIN_3    3
#define GPIO_MOD_PE1_PIN_4    4
#define GPIO_MOD_PE1_PIN_5    5
#define GPIO_MOD_PE1_PIN_6    6
#define GPIO_MOD_PE1_PIN_7    7

// Remaining 24 ports configured for outputs, some buffered
// PB0-7 on module 1
#define GPIO_MOD_PE1_PIN_8    8
#define GPIO_MOD_PE1_PIN_9    9
#define GPIO_MOD_PE1_PIN_10   10
#define GPIO_MOD_PE1_PIN_11   11
#define GPIO_MOD_PE1_PIN_12   12
#define GPIO_MOD_PE1_PIN_13   13
#define GPIO_MOD_PE1_PIN_14   14
#define GPIO_MOD_PE1_PIN_15   15

// PA0-7 on module 2 are buffered outputs
#define GPIO_MOD_PE2_PIN_0    0
#define GPIO_MOD_PE2_PIN_1    1
#define GPIO_MOD_PE2_PIN_2    2
#define GPIO_MOD_PE2_PIN_3    3
#define GPIO_MOD_PE2_PIN_4    4
#define GPIO_MOD_PE2_PIN_5    5
#define GPIO_MOD_PE2_PIN_6    6
#define GPIO_MOD_PE2_PIN_7    7

// PB0-7 on module 2
#define GPIO_MOD_PE2_PIN_8    8
#define GPIO_MOD_PE2_PIN_9    9
#define GPIO_MOD_PE2_PIN_10   10
#define GPIO_MOD_PE2_PIN_11   11
#define GPIO_MOD_PE2_PIN_12   12
#define GPIO_MOD_PE2_PIN_13   13
#define GPIO_MOD_PE2_PIN_14   14
#define GPIO_MOD_PE2_PIN_15   15

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
#else  // Transverter Box  PA0-7 on Module 1
  #define BAND_DECODE_INPUT_0        GPIO_MOD_PE1_PIN_0     // bit 0   Band 0
  #define BAND_DECODE_INPUT_1        GPIO_MOD_PE1_PIN_1     // bit 1   Band 1
  #define BAND_DECODE_INPUT_2        GPIO_MOD_PE1_PIN_2     // bit 2   Band 2
  #define BAND_DECODE_INPUT_3        GPIO_MOD_PE1_PIN_3     // bit 3   PTT Input
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
  #define BAND_DECODE_OUTPUT_0        GPIO_MOD_IO_PIN_0      // bit 0  PB module 1 fopr M5StampC3U
  #define BAND_DECODE_OUTPUT_1        GPIO_MOD_IO_PIN_1      // bit 1
  #define BAND_DECODE_OUTPUT_2        GPIO_MOD_IO_PIN_2      // bit 2
  #define BAND_DECODE_OUTPUT_3        GPIO_MOD_IO_PIN_3      // bit 3
  #define BAND_DECODE_OUTPUT_4        GPIO_PIN_NOT_USED      // bit 4
  #define BAND_DECODE_OUTPUT_5        GPIO_PIN_NOT_USED      // bit 5
  #define BAND_DECODE_OUTPUT_6        GPIO_PIN_NOT_USED      // bit 6
  #define BAND_DECODE_OUTPUT_7        GPIO_PIN_NOT_USED      // bit 7

  #define BAND_DECODE_OUTPUT_8        GPIO_PIN_NOT_USED      // bit 0 PA Module 2
  #define BAND_DECODE_OUTPUT_9        GPIO_PIN_NOT_USED      // bit 1
  #define BAND_DECODE_OUTPUT_10       GPIO_PIN_NOT_USED      // bit 2
  #define BAND_DECODE_OUTPUT_11       GPIO_PIN_NOT_USED      // bit 3
  #define BAND_DECODE_OUTPUT_12       GPIO_PIN_NOT_USED      // bit 4
  #define BAND_DECODE_OUTPUT_13       GPIO_PIN_NOT_USED      // bit 5
  #define BAND_DECODE_OUTPUT_14       GPIO_PIN_NOT_USED      // bit 6
  #define BAND_DECODE_OUTPUT_15       GPIO_PIN_NOT_USED      // bit 7
  #define BAND_DECODE_OUTPUT_16       GPIO_PIN_NOT_USED      // bit 0 PB Module 2

#else  // Transverter Box   
  // PA0-7 Module 1 are inputs.  PA0-7 Module 2 are PTT outputs.  PB0-7 on each module (16 total) for general outputs
  // There are 32 IO ports total between the 2x MCP23017 port expanders.
  // 8 are used as inputs, 8 ofr PTT outputs, 16 are other control outputs.  There are more ports on the CPU if needed
  #define BAND_DECODE_OUTPUT_0        GPIO_MOD_PE1_PIN_8      // bit 0  PB module 1 for M5StampC3U
  #define BAND_DECODE_OUTPUT_1        GPIO_MOD_PE1_PIN_9      // bit 1
  #define BAND_DECODE_OUTPUT_2        GPIO_MOD_PE1_PIN_10     // bit 2
  #define BAND_DECODE_OUTPUT_3        GPIO_PIN_NOT_USED       // bit 3  These (3-7) are used for internal PTT so handled by PTT function, not here
  #define BAND_DECODE_OUTPUT_4        GPIO_PIN_NOT_USED       // bit 4
  #define BAND_DECODE_OUTPUT_5        GPIO_PIN_NOT_USED       // bit 5
  #define BAND_DECODE_OUTPUT_6        GPIO_PIN_NOT_USED       // bit 6
  #define BAND_DECODE_OUTPUT_7        GPIO_PIN_NOT_USED       // bit 7

  #define BAND_DECODE_OUTPUT_8        GPIO_MOD_PE2_PIN_8      // bit 0  PB Module 2
  #define BAND_DECODE_OUTPUT_9        GPIO_MOD_PE2_PIN_9      // bit 1
  #define BAND_DECODE_OUTPUT_10       GPIO_MOD_PE2_PIN_10     // bit 2
  #define BAND_DECODE_OUTPUT_11       GPIO_MOD_PE2_PIN_11     // bit 3
  #define BAND_DECODE_OUTPUT_12       GPIO_MOD_PE2_PIN_12     // bit 4
  #define BAND_DECODE_OUTPUT_13       GPIO_MOD_PE2_PIN_13     // bit 5
  #define BAND_DECODE_OUTPUT_14       GPIO_MOD_PE1_PIN_14     // bit 6
  #define BAND_DECODE_OUTPUT_15       GPIO_MOD_PE1_PIN_15     // bit 7
#endif

#ifndef M5STAMPC3U
  // BAND DECODE PTT OUTPUT PINS
  // Assign your pins of choice.  Use a number or one of the existing #define number names
  // Make sure they are not monitored by the code as a button or other use like an encoder.
  // If not used set to GPIO_PIN_NOT_USED since there is no pin 255.
  #define BAND_DECODE_PTT_OUTPUT_0    GPIO_PIN_NOT_USED     // bit 0  PA0-7 Module 2
  #define BAND_DECODE_PTT_OUTPUT_1    GPIO_PIN_NOT_USED     // bit 1
  #define BAND_DECODE_PTT_OUTPUT_2    GPIO_PIN_NOT_USED     // bit 2
  #define BAND_DECODE_PTT_OUTPUT_3    GPIO_PIN_NOT_USED     // bit 3
  #define BAND_DECODE_PTT_OUTPUT_4    GPIO_MOD_IO_PIN_4     // bit 4
  #define BAND_DECODE_PTT_OUTPUT_5    GPIO_MOD_IO_PIN_5     // bit 5
  #define BAND_DECODE_PTT_OUTPUT_6    GPIO_MOD_IO_PIN_6     // bit 6
  #define BAND_DECODE_PTT_OUTPUT_7    GPIO_MOD_IO_PIN_7     // bit 7
  
  #define BAND_DECODE_PTT_OUTPUT_8    GPIO_PIN_NOT_USED     // bit 0 PB0-7 on Module 1
  #define BAND_DECODE_PTT_OUTPUT_9    GPIO_PIN_NOT_USED     // bit 1
  #define BAND_DECODE_PTT_OUTPUT_10   GPIO_PIN_NOT_USED     // bit 2
  #define BAND_DECODE_PTT_OUTPUT_11   GPIO_PIN_NOT_USED     // bit 3
  #define BAND_DECODE_PTT_OUTPUT_12   GPIO_PIN_NOT_USED     // bit 4
  #define BAND_DECODE_PTT_OUTPUT_13   GPIO_PIN_NOT_USED     // bit 5
  #define BAND_DECODE_PTT_OUTPUT_14   GPIO_PIN_NOT_USED     // bit 6
  #define BAND_DECODE_PTT_OUTPUT_15   GPIO_PIN_NOT_USED     // bit 7
#else  
  // Xvtr Box uses outputs on PB3-7 on 1st module and buffered PA0-7 on 2nd MCP23017 module for band specific PTT outputs for amp PTTs

  // Xvtr Box uses ports 3-7 of 8 outputs on PB3-7 on 1st MCP23017 module for band specific internal PTT outputs for amp PTTs
  #define BAND_DECODE_PTT_OUTPUT_0    GPIO_PIN_NOT_USED      // bit 0  0, 1 and 2 used by band decoder for internal IF SP6T switch
  #define BAND_DECODE_PTT_OUTPUT_1    GPIO_PIN_NOT_USED      // bit 1  
  #define BAND_DECODE_PTT_OUTPUT_2    GPIO_PIN_NOT_USED      // bit 2  
  #define BAND_DECODE_PTT_OUTPUT_3    GPIO_MOD_PE1_PIN_11    // bit 3  222 Xvtr Brd PTT       0 = TX
  #define BAND_DECODE_PTT_OUTPUT_4    GPIO_MOD_PE1_PIN_12    // bit 4  902/903 Xvtr Brd PTT   0 = TX
  #define BAND_DECODE_PTT_OUTPUT_5    GPIO_MOD_PE1_PIN_13    // bit 5  1296 Xvtr Brd PTT      0 = TX
  #define BAND_DECODE_PTT_OUTPUT_6    GPIO_MOD_PE1_PIN_14    // bit 6  T/R switch, 903 RF Xvtr board output  1 = TX
  #define BAND_DECODE_PTT_OUTPUT_7    GPIO_MOD_PE1_PIN_15    // bit 7  T/R swtich, 1296 RF Xvtr board output   1 = TX
  
  // 2nd module PA0-7 are PTT outputs on back panel to amps and/or antennas
  #define BAND_DECODE_PTT_OUTPUT_8    GPIO_MOD_PE2_PIN_0     // bit 0  HF/50
  #define BAND_DECODE_PTT_OUTPUT_9    GPIO_MOD_PE2_PIN_1     // bit 1  144
  #define BAND_DECODE_PTT_OUTPUT_10   GPIO_MOD_PE2_PIN_2     // bit 2  222
  #define BAND_DECODE_PTT_OUTPUT_11   GPIO_MOD_PE2_PIN_3     // bit 3  432
  #define BAND_DECODE_PTT_OUTPUT_12   GPIO_MOD_PE2_PIN_4     // bit 4  902/903
  #define BAND_DECODE_PTT_OUTPUT_13   GPIO_MOD_PE2_PIN_5     // bit 5  1296
  #define BAND_DECODE_PTT_OUTPUT_14   GPIO_PIN_NOT_USED      // bit 6  spare, could use for antenna switching
  #define BAND_DECODE_PTT_OUTPUT_15   GPIO_MOD_PE2_PIN_7     // bit 7  spare
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
// Example: For Core module with jsut hte 4In/*out module, there are 8 output.  Use lowest half of the value.
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
//  Example:  for HF/6M, we want the SP4T switch Port 2 (Xvtr Mode input path is on Port 1), rest are dont't care since only used on Xvtr bands) so set them to 0.
//  Set the upper half (2nd MCP23017 port expander - 8 available outputs 8-16 in our numbering) bits 0-3

//  The upper byte, lower 4 bits are the SP4T coax switch.   1 = on for each of 4 ports.
//  Any Xvtr band (222, 903, 1296) would be Port 1  - 0x0001  or 1
//  HF/50 would be Port 2 (0x02 - 2nd bit or bit 1) - 0x0010  or 2
//  144 would be Port 3 (0x04 - 3rd bit or bit 2)   - 0x0100  or 4
//  432 would be Port 4 (0x08 - 4th bit or bit 3)   - 0x1000  or 8
//  If band undefined then default to 0x0002 or 2 to pass through anything to the 50/HF port

// Also in Module 2, the upper 4 bits are the 12V power relays.   0 = on for each relay.
//  bit 7 1296 12V ON to 1296 Xvtr  - binary 1110 or 0xExyz
//  bit 6 903 12V ON to 903 Xvtr    - binary 1011 of 0xBxyz  (but also need 903 amp when installed)
//  bit 5 903 12V ON to 9093 Amp on - binary 1101 or 0xDxyz  
//  bit5+6 for 903 Xvtr + Amp need  - binary 1001 or 0x9xyz
//  bit 4 222 12V ON to 222 Xvtr    - binary 0111 or 0x7xyz
//  All non-Xvtr bands set top mibble to 0xF

//  so upper byte for 222 band is 0111 0001 or 0x71

// Lower byte, lowest bits are PTT out.  6 dedicated buffered 
// bits 0-2 are IF SP6T switch.  A, B, C
//          CBA
//    RF1 = 000  NC
//    RF2 = 001  NC
//    RF3 = 010  222
//    RF4 = 011  NC
//    RF5 = 100  903
//    RF6 = 101  1296
//    OFF = 110  
//    OFF = 111

// bits 3 to 7 are used in PTT function so can set to 0, they wil lbe ignored here.

// so lower byte for 222 here is simply 0010 or 0x02
// Full value for 222 is then 0x7102

// Bits 7-3 of the lower byte is not used here, used by PTT so they are set to pin-not-used and thus value does not matter.
// 0xYYZZ where ZZ is typically 07 sets the IF SP6T switch to all off.  Only needed for Xvtr bands


  #define DECODE_BAND_DUMMY   (0xF207)    //Dummy Row
  #define DECODE_BANDAM       (0xF207)    //AM
  #define DECODE_BAND160M     (0xF207)    //160M 
  #define DECODE_BAND80M      (0xF207)    //80M
  #define DECODE_BAND60M      (0xF207)    //60M
  #define DECODE_BAND40M      (0xF207)    //40M
  #define DECODE_BAND30M      (0xF207)    //30M
  #define DECODE_BAND20M      (0xF207)    //20M
  #define DECODE_BAND17M      (0xF207)    //17M      
  #define DECODE_BAND15M      (0xF207)    //15M
  #define DECODE_BAND12M      (0xF207)    //12M
  #define DECODE_BAND10M      (0xF207)    //10M
  #define DECODE_BANDFM       (0xF207)    //6M
  #define DECODE_BANDAIR      (0xF207)    //6M
  //#define DECODE_BAND70       (0x0000)    //70MHz
  #define DECODE_BAND6M       (0xF207)    //6M
  #define DECODE_BAND144      (0xF407)    //2M
  #define DECODE_BAND222      (0x7102)    //222
  #define DECODE_BAND432      (0xF807)    //432
  #define DECODE_BAND902      (0x9104)    //902  Turn on both 12V relays, 2 & 3 for 903 Xvtr and Amp
  #define DECODE_BAND1296     (0xE105)    //1296
  #define DECODE_BAND2400     (0xF207)    //2400
  #define DECODE_BAND3300     (0xF207)    //3400
  #define DECODE_BAND5760     (0xF207)    //5760M
  #define DECODE_BAND10G      (0xF207)    //10.3681G
  #define DECODE_BAND24G      (0xF207)    //24.192G
  #define DECODE_BAND47G      (0xF207)    //47.1G
  #define DECODE_BAND76G      (0xF207)    //76.1G
  #define DECODE_BAND122G     (0xF207)    //122G
  #define DECODE_B_GENERAL    (0xF207)    // Non-Ham Band
#endif

// Band Decoder Output patterns for PTT routing.
// BAn example would be the BCD pattern following the Elecraft K3 HF-TRN table.  5 bits are used. Bit 4 =1 is VHF+ group
// here we have only 1 4-IN/8-Out module installed configured as 4x decode outputs and 4 PTT outputs for PTT routing to 4 amps/xvtrs
// Here the PTT lines are on the upper half of the group of 8 module outputs so we set values on the upper nibble, the lower will be ignored.
// if not used enter 0x00 
#ifndef M5STAMPC3U
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
#else  
  // For Xvtr Box with M5StampC3U
  // bits 0-2 is IF switch for Xvtr bands, set in the Band Decode function, ignored here as pins are set to not used for PTT
  // bit 3  PTT to Xvtr 222 Brd  0 = TX  - This requires a open collector like ULN2803 - Xvtr PTT has +9V on it and connecting to 3.3V output puts Xvtr into Tx - sort of.  
  // bit 4  PTT to Xvtr 903 Brd  0 = TX  - Same as above
  // bit 5  PTT to Xvtr 1296 Brd 0 = TX  - same as above
  // bit 6  903  RF output T/R coax switch   1 = ON - Logic 1 and 0.   Check for pullup in the switch, if so can likely use open collecor like ULN2803
  // bit 7  1296 RF output T/R coax switch   1 = ON
  // Example for any non Xvtr band (bits 0-2 are ignored for PTT so set to 0)
  //   bit 7 6 5 4  3 210
  //       0 0 1 1  1 000   = 0x38 for TX and RX
  // For Xvtr band 222
  //   bit 7 6 5 4  3 210   
  //       0 0 1 1  L 000   = 0x30 = PTT on TX, 0x38 = RX on 222
  // For Xvtr band 903
  //   bit 7 6 5 4  3 210
  //       0 H 1 L  1 000   = 0x68 = PTT on TX, 0x38 = RX on 903
  // For Xvtr band 1296
  //   bit 7 6 5 4  3 210
  //       H 0 L 1  1 000   = 0x98 = PTT on TX, 0x38 = RX on 1296
  //   Ideally the 903/1296 T/R swtiches would be set before the PTT issued to the Xvtr board

  // 0x0138 passes PTT through on HF/6M PTT output jack on back panel for all bands on HF and 6M.  Also sets the intenral Xvtr PTTs to RX state.
  #define DECODE_DUMMY_PTT        (0x0038)    //Dummy Row  This pattern is also used to set RX state.  All other rows are TX state only.
  #define DECODE_BANDAM_PTT       (0x0138)   //16M_PTT 
  #define DECODE_BAND160M_PTT     (0x0138)   //160M_PTT 
  #define DECODE_BAND80M_PTT      (0x0138)    //80M_PTT
  #define DECODE_BAND60M_PTT      (0x0138)    //60M_PTT
  #define DECODE_BAND40M_PTT      (0x0138)    //40M_PTT
  #define DECODE_BAND30M_PTT      (0x0138)    //30M_PTT
  #define DECODE_BAND20M_PTT      (0x0138)    //20M_PTT
  #define DECODE_BAND17M_PTT      (0x0138)    //17M_PTT      
  #define DECODE_BAND15M_PTT      (0x0138)    //15M_PTT
  #define DECODE_BAND12M_PTT      (0x0138)    //12M_PTT
  #define DECODE_BAND10M_PTT      (0x0138)    //10M_PTT
  #define DECODE_BANDFM_PTT       (0x0138)    //6M_PTT
  #define DECODE_BANDAIR_PTT      (0x0138)    //6M_PTT
  //#define DECODE_BAND70_PTT      (0x0138)    //70M_PTTHz
  #define DECODE_BAND6M_PTT       (0x0138)    //6M_PTT
  #define DECODE_BAND144_PTT      (0x0238)    //2M_PTT
  #define DECODE_BAND222_PTT      (0x8430)    //222_PTT  Turn on highest bit for TC Board IF relay/attenuator
  #define DECODE_BAND432_PTT      (0x0838)    //432_PTT
  #define DECODE_BAND902_PTT      (0x9068)    //902_PTT  Turn on highest bit for TC Board IF relay/attenuator
  #define DECODE_BAND1296_PTT     (0xA098)    //1296_PTT  Turn on highest bit for TC Board IF relay/attenuator
  #define DECODE_BAND2400_PTT     (0x0138)    //2400_PTT
  #define DECODE_BAND3300_PTT     (0x0138)    //3400_PTT
  #define DECODE_BAND5760_PTT     (0x0138)    //5760_PTT
  #define DECODE_BAND10G_PTT      (0x0138)    //10.368.1G_PTT
  #define DECODE_BAND24G_PTT      (0x0138)    //24.192G_PTT
  #define DECODE_BAND47G_PTT      (0x0138)    //47.1G_PTT
  #define DECODE_BAND76G_PTT      (0x0138)    //76.1G_PTT
  #define DECODE_BAND122G_PTT     (0x0138)    //122G_PTT
  #define DECODE_B_GENERAL_PTT    (0x0138)    // Non-Ham Band
#endif

#endif