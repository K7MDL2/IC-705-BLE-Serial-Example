#ifndef CIVcmds_h
#define CIVcmds_h

#include "TimeLib.h"

// CIV related stuff
extern uint8_t UTC;    // 0 local time, 1 UTC time
extern void printFrequency(void);
extern uint8_t PTT;
static uint8_t TX_last = 0;
int hr_off;  // time offsets to apply to UTC time
int min_off;
int shift_dir;  // + or -
tmElements_t tm;
time_t prevDisplay = 0; // When the digital clock was displayed

//inline uint8_t bcdByte(const uint8_t x) const { return  (((x & 0xf0) >> 4) * 10) + (x & 0x0f); }
inline uint8_t bcdByte(const uint8_t x) { return  (((x & 0xf0) >> 4) * 10) + (x & 0x0f); }
// input is between 0 and 99 decimal.  output 0 to 0x99
//inline uint8_t bcdByteEncode(const uint8_t x) const { return ((x / 10) << 4) + (x % 10); }
inline uint8_t bcdByteEncode(const uint8_t x) { return ((x / 10) << 4) + (x % 10); }


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

struct cmdList cmd_List[End_of_Cmd_List] = {
    {CIV_C_F_SEND,          {1,0x00}},                      // send operating frequency to all
    {CIV_C_F1_SEND,         {1,0x05}},                      // send operating frequency to one
    {CIV_C_F_READ,          {1,0x03}},                      // read operating frequency
    {CIV_C_F26,        		  {1,0x26}},                 // read selected VFO m data, filt
    {CIV_C_F26A,        	  {2,0x26,0x00}},                 // read selected VFO m data, filt
    {CIV_C_F26B,       		  {2,0x26,0x01}},                 // read un- selected VFO m data, filt
    {CIV_C_F25A_SEND,       {8,0x25,0x00}},                 // set selected VFO frequency
    {CIV_C_F25B_SEND,       {8,0x25,0x01}},                 // set un-selected VFO frequency
    {CIV_C_MOD_READ,        {1,0x04}},               	    // read Modulation Mode in use
    {CIV_C_MOD_SET,         {3,0x00,0x00,0x00}},  		    // cmd 26 datafield template; selected VFO; mode, data on/off(0-1), filter (1-3);
    {CIV_C_MOD_SEND ,       {1,0x01}},                      // send Modulation Mode to all
    {CIV_C_MOD1_SEND,       {1,0x06}},                      // send Modulation Mode to one
    {CIV_C_MOD_USB_F1_SEND, {3,0x06,0x01,0x01}},            // send USB Filter 1 
    {CIV_C_MOD_USB_SEND,    {2,0x06,0x01}},                 // send USB Filter 1 
    {CIV_C_USB_D0_F2_SEND,  {5,0x26,0x00,0x01,0x00,0x02}},  // selected VFO; mod USB; Data OFF; RX_filter F2;
    {CIV_C_USB_D1_F2_SEND,  {5,0x26,0x00,0x01,0x01,0x02}},
    {CIV_C_LSB_D0_F2_SEND,  {5,0x26,0x00,0x00,0x00,0x02}},  // selected VFO; mod USB; Data OFF; RX_filter F2;
    {CIV_C_LSB_D1_F2_SEND,  {5,0x26,0x00,0x00,0x01,0x02}},  // selected VFO; mod USB; Data ON; RX_filter F2;
    {CIV_C_FM_D1_F1_SEND,   {5,0x26,0x00,0x05,0x01,0x01}},  // selected VFO; mod USB; Data ON; RX_filter F2;
    {CIV_C_ATTN_READ,   	  {1,0x11}},                  	// Attn read state
    {CIV_C_ATTN_OFF,   		  {2,0x11,0x00}},                 // Attn OFF
    {CIV_C_ATTN_ON,    		  {2,0x11,0x10}},                 // Attn 10dB (144, 432, 1200 bands only)
    {CIV_C_SPLIT_OFF_READ,  {2,0x0F,0x00}},                 // read Split OFF
    {CIV_C_SPLIT_ON_READ,   {2,0x0F,0x01}},                 // read split ON
    {CIV_C_SPLIT_OFF_SEND,  {2,0x0F,0x01}},                 // set split OFF
    {CIV_C_SPLIT_ON_SEND,   {2,0x0F,0x01}},                 // Set split ON
    {CIV_C_RFGAIN,          {2,0x14,0x02}},                 // send/read RF Gain
    {CIV_C_AFGAIN,          {2,0x14,0x01}},                 // send/read AF Gain
    {CIV_C_RFPOWER,         {2,0x14,0x0A}},                 // send/read selected bands RF power
    {CIV_C_S_MTR_LVL,       {2,0x15,0x02}},                 // send/read S-meter level (00 00 to 02 55)  00 00 = S0, 01 20 = S9, 02 41 = S9+60dB
    {CIV_C_PREAMP_READ,     {2,0x16,0x02}},             	// read preamp state
    {CIV_C_PREAMP_OFF,      {3,0x16,0x02,0x00}},            // send/read preamp 3rd byte is on or of for sending - 00 = OFF, 01 = ON
    {CIV_C_PREAMP_ON,       {3,0x16,0x02,0x01}},            // send/read preamp 3rd byte is on or of for sending - 00 = OFF, 01 = ON
    {CIV_C_AGC_READ,        {2,0x16,0x12}},                 // send/read AGC  01 = FAST, 02 = MID, 03 = SLOW
    {CIV_C_AGC_FAST,        {3,0x16,0x12,0x01}},            // send/read AGC  01 = FAST, 02 = MID, 03 = SLOW
    {CIV_C_AGC_MID,         {3,0x16,0x12,0x02}},            // send/read AGC  01 = FAST, 02 = MID, 03 = SLOW
    {CIV_C_AGC_SLOW,        {3,0x16,0x12,0x03}},            // send/read AGC  01 = FAST, 02 = MID, 03 = SLOW
    {CIV_C_CW_MSGS,         {1,0x17}},                      // Send CW messages see page 17 of prog manual for char table
    {CIV_C_BSTACK,          {2,0x1A,0x01}},                 // send/read BandStack contents - see page 19 of prog manual.  
                                                                    // data byte 1 0xyy = Freq band code
                                                                    // dat abyte 2 0xzz = register code 01, 02 or 03
                                                                    // to read 432 band stack register 1 use 0x1A,0x01,0x02,0x01
    {CIV_C_MY_POSIT_READ,   {2,0x23,0x00}},          	    // read my GPS Position
    {CIV_C_MY_POSIT_DATA,   {1,0x23}},          	    	  // read my GPS Position
    {CIV_C_RF_POW,          {2,0x14,0x0A}},            		// send / read max RF power setting (0..255 == 0 .. 100%)
    {CIV_C_TRX_ON_OFF,      {1,0x18}},                 		// switch radio ON/OFF
    {CIV_C_TRX_ID,          {2,0x19,0x00}},            		// ID query
    {CIV_C_TX,              {2,0x1C,0x00}},            		// query of TX-State 00=OFF, 01=ON
    // the following three commands don't fit for IC7100 !!!
    {CIV_C_DATE,            {4,0x1A,0x05,0x00,0x94}},  		// + 0x20 0x20 0x04 0x27 for 27.4.2020
    {CIV_C_TIME,            {4,0x1A,0x05,0x00,0x95}},  		// + 0x19 0x57 for 19:57
    //{CIV_C_UTC,             {4,0x1A,0x05,0x00,0x96}},  		// + 0x01,0x00,0x00 = +1h delta of UTC to MEZ
    {CIV_C_UTC_READ_905,    {4,0x1A,0x05,0x01,0x81}},     //  Get UTC Offset
    //{CIV_C_UTC_SEND,        {4,0x1A,0x05,0x00,0x96}},  		// + 0x01,0x00,0x00 = +1h delta of UTC to MEZ
    {CIV_C_UTC_READ_705,    {4,0x1A,0x05,0x01,0x70}},  		// + 0x01,0x00,0x00 = +1h delta of UTC to MEZ
    {CIV_C_DUPLEX_READ,		  {1,0x0C}},          	    	  // read Duplex Offset  - has 3 bytes frequency offset data
    {CIV_C_DUPLEX_SEND,		  {1,0x0D}},	          	    	// send Duplex Offset
    {CIV_C_RIT_XIT,			    {2,0x21,0x00}},          	    // read or send RIT/XIT Offset  - has 3 bytes frequency offset data  XIT and RIT share this Offset value
    {CIV_C_RIT_ON_OFF,		  {2,0x21,0x01}},	          	  // send or send RIT ON or Off status 00 = , 01 = t
    {CIV_C_XIT_ON_OFF,		  {2,0x21,0x02}},	          	  // send or send XIT Offset
    {CIV_C_RADIO_OFF,		    {2,0x18,0x00}},	          	  // Turn Off the radio
    {CIV_C_RADIO_ON,		    {2,0x18,0x01}}	          	  // Turn on the radio
};

//
//  CIV_Action - Takes action on a sucessfully parsed CIV command result from processmessages()
//  Add new CIV commands action here
//  uses the command index from te enum above.  
//
//  To add a new CIV command not in the list already,
//  1. Insert the cmd name (index numbmer) into the enum list any place.
//  2. Clone and edit a new command in the structure, inserted into the same place as the enun list
//  3. add the new switch case statement with the new enum index
//

//#define DBG_CIV1

void CIV_Action(const uint8_t cmd_num, const uint8_t data_start_idx, const uint8_t data_len, const uint8_t msg_len, const uint8_t read_buffer[])
{ 
  
  #ifdef DBG_CIV1
  Serial.printf("cmd = %X  data_start_idx = %d  data_len = %d\n", cmd_List[cmd_num].cmdData[1], data_start_idx, data_len);
  #endif
  
  switch (cmd_num) 
  {
    case CIV_C_F_READ:
    case CIV_C_F_SEND:
    case CIV_C_F1_SEND:
          printFrequency(data_len);
          break;

    case CIV_C_TX:  // Used to request RX TX status from radio
          PTT = read_buffer[6];
          if (TX_last != PTT)
          {
            Serial.printf("TX Status = %d\n", PTT);
            TX_last = PTT;
          }
          break;

    case CIV_C_PREAMP_READ:
          Serial.printf("Preamp = %d\n", read_buffer[6]);
          break;

    case CIV_C_MOD_READ:
    case CIV_C_MOD1_SEND:
    case CIV_C_MOD_SEND:
          Serial.printf("Mode, Len = %d\n", data_len);
          break;

    case CIV_C_UTC_READ_905:
    case CIV_C_UTC_READ_705: 
          {
            //Serial.printf("processCatMessages: UTC Offset, Len = %d\n", data_len);
            //               pos     sub cmd  len        1    2     3          term
            // FE.FE.E0.AC.  1A.05.  01.81.   3         07.  00.   01.         FD
            // FE.FE.E0.AC.  1A.05.           datalen   hr   min   1=minus 0=+
            //                                     offset time 00 00 to 14 00    Shift dir 00 + and 01 is -
            // when using datafield, add 1 to prog guide index to account for first byte used as length counter - so 3 is 4 here.

            hr_off = bcdByte(read_buffer[data_start_idx+0]); 
            min_off = bcdByte(read_buffer[data_start_idx+1]); 
            shift_dir = bcdByte(read_buffer[data_start_idx+2]);
            
            Serial.print("UTC Offset: "); 
            if (shift_dir) 
            {
              hr_off = hr_off * -1;  // invert  - used by UTC set function
              min_off = min_off * -1;  // invert  - used by UTC set function
            }
            Serial.print(hr_off); Serial.print(":");Serial.println(min_off);

            //get current time and correct or set time zone offset
            //setTime(_hr,_min,_sec,_day,_month,_yr);

          }  // UTC Offset
          break;

    case CIV_C_MY_POSIT_READ:
          {
            //Serial.printf("processCatMessages: Position, Len = %d\n", data_len);
            //uint8_t RX = CIVresultL.value;

            //DPRINTF("check_CIV: CI-V Returned MY POSITION and TIME: "); DPRINTLN(retValStr[CIVresultL.value]);
            //               pos                  1  2  3  4  5   6  7  8  9  10 11   12 13 14 15   16 17   18 19 20   21 22  23  24  25 26 27  term
            // FE.FE.E0.AC.  23.00.  datalen 27  47.46.92.50.01.  01.22.01.98.70.00.  00.15.59.00.  01.05.  00.00.07.  20.24. 07. 20. 23.32.45. FD
            //                                     47.46.925001 lat 122.01.987000 long  155.900m alt  105deg   0.7km/h   2024   07  20  23:32:45 UTC
            // when using datafield, add 1 to prog guide index to account for first byte used as length counter - so 27 is 28 here.
            Serial.print("** Time from Radio is: ");
            
            int _hr = bcdByte(read_buffer[data_start_idx+24]); //Serial.print(_hr); Serial.print(":");
            int _min = bcdByte(read_buffer[data_start_idx+25]); //Serial.print(_min);Serial.print(":");
            int _sec = bcdByte(read_buffer[data_start_idx+26]); //Serial.print(_sec);Serial.print(" ");
            
            int _month = bcdByte(read_buffer[data_start_idx+22]); //Serial.print(_month);Serial.print(".");
            int _day = bcdByte(read_buffer[data_start_idx+23]); //Serial.print(_day); Serial.print(".");
            int _yr = bcdByte(read_buffer[data_start_idx+21]); //Serial.print(_yr); // yr can be 4 or 2 digits  2024 or 24                    
            
            if (!UTC) 
            {
              setTime(_hr+hr_off,_min+min_off,_sec,_day,_month,_yr);  // correct to local time                      
              Serial.printf("Local Time: %d:%d:%d  %d/%d/20%d\n",_hr+hr_off,_min+min_off,_sec,_month,_day,_yr); 
            }
            else
            {
              Serial.printf("UTC Time: %d:%d:%d  %d/%d/20%d\n",_hr,_min,_sec,_month,_day,_yr); 
              setTime(_hr,_min,_sec,_day,_month,_yr);  // display UTC time
            }
            break;
          }  // MY Position

    default: Serial.println("*** default action");
          //knowncommand = false;
  }
}
#endif // CIV.ino header file
