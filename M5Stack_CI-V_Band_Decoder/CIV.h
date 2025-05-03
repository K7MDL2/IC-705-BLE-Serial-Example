// CIV.h

#ifndef CIV_h
#define CIV_h

#include <string>
#include "M5Stack_CI-V_Band_Decoder.h"

//inline uint8_t bcdByte(const uint8_t x) const { return  (((x & 0xf0) >> 4) * 10) + (x & 0x0f); }
inline uint8_t bcdByte(const uint8_t x) { return  (((x & 0xf0) >> 4) * 10) + (x & 0x0f); }
// input is between 0 and 99 decimal.  output 0 to 0x99
//inline uint8_t bcdByteEncode(const uint8_t x) const { return ((x / 10) << 4) + (x % 10); }
inline uint8_t bcdByteEncode(const uint8_t x) { return ((x / 10) << 4) + (x % 10); }

/* ========================================
 *  geo.c
 *
 *  Converts Lat and Long to extended maidenhead grid squares 
 *
 * ========================================
*/

/* geo.c Ham Radio functions for orientation on earth – Copyright 2010 This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either
* version 3, or (at your option) any later version.
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
* You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
/*
*
* Next the function that converts the position to the Maidenhead locator. It could easily have been written just to 
* accept two arguments for the position, but in this case the structure is passed by address to the function for evaluation. 
*
* The return value of the function is best provided by the calling routine, so the second argument is the address of a string
* (i.e. an array of type char) where the function will return the result. This is quite a bit more complicated that most
* languages which use dynamic memory allocation and garbage collection to allow functions to return strings.
*
* I use the return value here to indicate success or failure, but have no failure conditions. I suppose a latitude of less than -90 or more than +90 would suffice. For longitude I thing wrapping (e.g. 181E = 179W) is sufficient, which is what the function does.
*
*/

/* Global variables for handling GPS conversion to Maindernhead grid square  */
#define NMEA_MAX_LENGTH  (120)
#define GRIDSQUARE_LEN  (9u)

struct position {
    double latitude;
    double longitude;
};

static char Latitude[14];    /* size to hold longest values supplied by GPS, assuming 10 plus null for now   */
static char Longitude[14];
void CIV_Action(const uint8_t cmd_num, const uint8_t data_start_idx, const uint8_t data_len, const uint8_t msg_len, const uint8_t read_buffer[]);
void reverse(char *str, int len);
void ftoa(float n, char *res, int afterpoint);
int positionToMaidenhead(char m[]);
int Convert_to_MH(void);
void ConvertToMinutes(char _gps_msg[]);
void SetMode(uint8_t _band);
void SetAGC(uint8_t  _band);
void SetPre(uint8_t  _band);
void SetAttn(uint8_t  _band);
void SetSplit(uint8_t  _band);
void SetRFPwr(uint8_t  _band);

// command "body" of the CIV commands currently in use
// Add new command to this list.  Then add array values to the structure below.  The row is the same as the enum value here.
enum cmds {
    CIV_C_F_SEND,
    CIV_C_F1_SEND,
    CIV_C_F_READ,
    CIV_C_F26,
    CIV_C_F26A,
    CIV_C_F26B,
    CIV_C_F25A_SEND,
    CIV_C_F25B_SEND,
    CIV_C_MOD_READ,
    CIV_C_MOD_SET,
    CIV_C_MOD_SEND,
    CIV_C_MOD1_SEND,
    CIV_C_MOD_USB_F1_SEND,
    CIV_C_MOD_USB_SEND,
    CIV_C_USB_D0_F2_SEND, 
    CIV_C_USB_D1_F2_SEND, 
    CIV_C_LSB_D0_F2_SEND, 
    CIV_C_LSB_D1_F2_SEND, 
    CIV_C_FM_D1_F1_SEND, 
    CIV_C_ATTN_READ,
    CIV_C_ATTN_OFF,
    CIV_C_ATTN_ON,
    CIV_C_SPLIT_READ,
    CIV_C_SPLIT_OFF_SEND,
    CIV_C_SPLIT_ON_SEND,
    CIV_C_RFGAIN,
    CIV_C_AFGAIN,
    CIV_C_RFPOWER,
    CIV_C_S_MTR_LVL,
    CIV_C_PREAMP_READ,
    CIV_C_PREAMP_OFF,
    CIV_C_PREAMP_ON,
    CIV_C_PREAMP_ON2,
    CIV_C_AGC_READ,
    CIV_C_AGC_FAST,
    CIV_C_AGC_MID,
    CIV_C_AGC_SLOW,
    CIV_C_CW_MSGS,
    CIV_C_BSTACK,
    CIV_C_MY_POSIT_READ,
    CIV_C_MY_POSIT_DATA,
    CIV_C_TRX_ON_OFF,
    CIV_C_TRX_ID,
    CIV_C_TX,
    CIV_C_DATE,
    CIV_C_TIME,
    CIV_C_UTC_READ_905,
    CIV_C_UTC_READ_705,
    CIV_C_DUPLEX_READ,
    CIV_C_DUPLEX_SEND,
    CIV_C_RIT_XIT,
    CIV_C_RIT_ON_OFF,
    CIV_C_XIT_ON_OFF,
    CIV_C_RADIO_OFF,
    CIV_C_RADIO_ON,
    CIV_C_SCOPE_ON,
    CIV_C_SCOPE_OFF,
    CIV_C_SCOPE_ALL,
    CIV_R_NO_GOOD,
    CIV_R_GOOD,
    End_of_Cmd_List
};

// For CIV commands

#define MODES_NUM   16
#define CW          3
#define CW_R        7
#define USB         1
#define LSB         0     
#define RTTY        4     
#define RTTY_R      8     
#define AM          2
#define FM          5
#define DV          23
#define DD          34
#define ATV         35

#define AGC_SET_NUM 4
#define AGC_OFF     0       // Index to AGC Settings table
#define AGC_SLOW    3
#define AGC_MID     2
#define AGC_FAST    1

#define FILTER      4
#define FILT1       1
#define FILT2       2
#define FILT3       3
#define VFO_A       1
#define VFO_B       0

#define ATTN_OFF   0       // Bypass
#define ATTN_ON    1       // Turn relay on
#define PREAMP_OFF  0       // Bypass
#define PREAMP_ON   1       // Switch relay on

struct cmdList {
    cmds cmd;
    uint8_t cmdData[6];  // first byte is number of bytes in the command (sub commands)
};

struct Modes_List {
    uint8_t     mode_num;
    char        mode_label[8];
    uint8_t     filtx;             // bandwidth in HZ - look up matching width in Filter table when changing modes
    uint8_t     data;
};

// translation of the radio's general mode
const char ModeStr[3][11] = {
  "MODE_VOICE",
  "MODE_DATA",
  "MODE_NDEF"
};

const char AgcStr[4][6] = {
    {"AGC- "},  // 0 reserved for AGC OFF
    {"AGC-F"},  // 1
    {"AGC-M"},  // 2
    {"AGC-S"}   // 3
};

// clear text translation of the Filter setting
const char FilStr[4][5] = {
  "NDEF",
  "FIL1",   // 1 (1 .. 3 is according to ICOM's documentation)
  "FIL2",
  "FIL3"
};

// states of radio's DC-Power (on/Off State)
const char radioOnOffStr[6][13] = {
  "RADIO_OFF",
  "RADIO_ON",
  "RADIO_OFF_TR",     // transit from OFF to ON
  "RADIO_ON_TR",      // transit from ON to OFF
  "RADIO_NDEF",       // don't know
  "RADIO_TOGGLE"
};

#endif // CIV.h header file
