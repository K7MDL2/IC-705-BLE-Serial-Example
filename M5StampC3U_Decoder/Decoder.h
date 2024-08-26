#ifndef DECODER_H_
#define DECODER_H_

#include <stdint.h>

void Band_Decode_Output(uint8_t band);
void GPIO_Out(uint8_t pattern);
void PTT_Output(uint8_t band, bool PTT_state);
void GPIO_PTT_Out(uint8_t pattern, bool PTT_state);
void Module_4in_8out_Output_test();
uint8_t Module_4in_8out_Input_scan(void);
void  Module_4in_8out_setup();

enum band_idx { BAND_AM,
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
// These are 8 MOSFET outputs on the 4-In/8-Out module
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

// BAND DECODE INPUT PINS
// Assign your pins of choice.  Use a number or one of the existing #define number names
// Make sure they are not monitored by the code as a button or other use like an encoder.
// If not used set to GPIO_PIN_NOT_USED since there is no pin 255.
#define BAND_DECODE_INPUT_PIN_0        GPIO_MOD_I_PIN_0      // bit 0
#define BAND_DECODE_INPUT_PIN_1        GPIO_MOD_I_PIN_1      // bit 1
#define BAND_DECODE_INPUT_PIN_2        GPIO_MOD_I_PIN_2      // bit 2
#define BAND_DECODE_INPUT_PIN_3        GPIO_MOD_I_PIN_3      // bit 3

// BAND DECODE OUTPUT PINS
// Assign your pins of choice.  Use a number or one of the existing #define number names
// Make sure they are not monitored by the code as a button or other use like an encoder.
// If not used set to GPIO_PIN_NOT_USED since there is no pin 255.
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
#define DECODE_BANDAM       (0x00)   //160M 
#define DECODE_BAND160M     (0x00)   //160M 
#define DECODE_BAND80M      (0x00)    //80M
#define DECODE_BAND60M      (0x00)    //60M
#define DECODE_BAND40M      (0x00)    //40M
#define DECODE_BAND30M      (0x00)    //30M
#define DECODE_BAND20M      (0x00)    //20M
#define DECODE_BAND17M      (0x00)    //17M      
#define DECODE_BAND15M      (0x00)    //15M
#define DECODE_BAND12M      (0x00)    //12M
#define DECODE_BAND10M      (0x00)    //10M
#define DECODE_BAND6M       (0x00)    //6M
#define DECODE_BANDFM       (0x00)    //6M
#define DECODE_BANDAIR      (0x00)    //6M
//#define DECODE_BAND70       (0x00)    //70MHz
#define DECODE_BAND144      (0x00)    //2M
#define DECODE_BAND222      (0x00)    //222
#define DECODE_BAND432      (0x00)    //432
#define DECODE_BAND902      (0x01)    //902
#define DECODE_BAND1296     (0x02)    //1296
#define DECODE_BAND2400     (0x04)    //2400
#define DECODE_BAND3400     (0x00)    //3400
#define DECODE_BAND5760     (0x00)    //5760M
#define DECODE_BAND10G      (0x00)    //10.368.1G
#define DECODE_BAND24G      (0x00)    //24.192G
#define DECODE_BAND47G      (0x00)    //47.1G
#define DECODE_BAND76G      (0x00)    //76.1G
#define DECODE_BAND122G     (0x00)    //122G
#define DECODE_B_GENERAL    (0x00)     // Non-Ham Band

// Band Decoder Output patterns for PTT routing.
// BAn example would be the BCD pattern following the Elecraft K3 HF-TRN table.  5 bits are used. Bit 4 =1 is VHF+ group
// here we have only 1 4-IN/8-Out module installed configured as 4x decode outputs and 4 PTT outputs for PTT routing to 4 amps/xvtrs
// Here the PTT lines are on the upper half of the group of 8 module outputs so we set values on the upper nibble, the lower will be ignored.
// if not used enter 0x00 
#define DECODE_BANDAM_PTT       (0x00)   //160M_PTT 
#define DECODE_BAND160M_PTT     (0x00)   //160M_PTT 
#define DECODE_BAND80M_PTT      (0x00)    //80M_PTT
#define DECODE_BAND60M_PTT      (0x00)    //60M_PTT
#define DECODE_BAND40M_PTT      (0x00)    //40M_PTT
#define DECODE_BAND30M_PTT      (0x00)    //30M_PTT
#define DECODE_BAND20M_PTT      (0x00)    //20M_PTT
#define DECODE_BAND17M_PTT      (0x00)    //17M_PTT      
#define DECODE_BAND15M_PTT      (0x00)    //15M_PTT
#define DECODE_BAND12M_PTT      (0x00)    //12M_PTT
#define DECODE_BAND10M_PTT      (0x00)    //10M_PTT
#define DECODE_BAND6M_PTT       (0x00)    //6M_PTT
#define DECODE_BANDFM_PTT       (0x00)    //6M_PTT
#define DECODE_BANDAIR_PTT      (0x00)    //6M_PTT
//#define DECODE_BAND70_PTT    (0x00)    //70M_PTTHz
#define DECODE_BAND144_PTT      (0x00)    //2M_PTT
#define DECODE_BAND222_PTT      (0x00)    //222_PTT
#define DECODE_BAND432_PTT      (0x00)    //432_PTT
#define DECODE_BAND902_PTT      (0x10)    //902_PTT
#define DECODE_BAND1296_PTT     (0x20)    //1296_PTT
#define DECODE_BAND2400_PTT     (0x40)    //2400_PTT
#define DECODE_BAND3400_PTT     (0x00)    //3400_PTT
#define DECODE_BAND5760_PTT     (0x00)    //5760_PTT
#define DECODE_BAND10G_PTT      (0x00)    //10.368.1G_PTT
#define DECODE_BAND24G_PTT      (0x00)    //24.192G_PTT
#define DECODE_BAND47G_PTT      (0x00)    //47.1G_PTT
#define DECODE_BAND76G_PTT      (0x00)    //76.1G_PTT
#define DECODE_BAND122G_PTT     (0x00)    //122G_PTT
#define DECODE_B_GENERAL_PTT    (0x00)     // Non-Ham Band


#endif