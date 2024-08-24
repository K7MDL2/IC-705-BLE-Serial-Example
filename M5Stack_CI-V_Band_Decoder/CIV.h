// CIV.h

#ifndef CIV_h
#define CIV_h

#include <string>

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
    CIV_C_SPLIT_OFF_READ,
    CIV_C_SPLIT_ON_READ,
    CIV_C_SPLIT_OFF_SEND,
    CIV_C_SPLIT_ON_SEND,
    CIV_C_RFGAIN,
    CIV_C_AFGAIN,
    CIV_C_RFPOWER,
    CIV_C_S_MTR_LVL,
    CIV_C_PREAMP_READ,
    CIV_C_PREAMP_OFF,
    CIV_C_PREAMP_ON,
    CIV_C_AGC_READ,
    CIV_C_AGC_FAST,
    CIV_C_AGC_MID,
    CIV_C_AGC_SLOW,
    CIV_C_CW_MSGS,
    CIV_C_BSTACK,
    CIV_C_MY_POSIT_READ,
    CIV_C_MY_POSIT_DATA,
    CIV_C_RF_POW,
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
    End_of_Cmd_List
};

struct cmdList {
    cmds cmd;
    uint8_t cmdData[6];  // first byte is number of bytes in the command (sub commands)
};

#endif // CIV.h header file
