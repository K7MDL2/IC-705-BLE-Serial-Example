//  CIV.cpp

#include "TimeLib.h"
#include "time.h"
#include "M5Stack_CI-V_Band_Decoder.h"
#include "CIV.h"

// CIV related stuff
char Grid_Square[GRIDSQUARE_LEN];   /* 10 char grid square max to be used - store d here as last known good Grid Flash icon if no valid GPS input*/
extern uint8_t UTC;    // 0 local time, 1 UTC time
extern void read_Frequency(uint8_t data_len);
extern uint8_t PTT;
static uint8_t TX_last = 0;
int hr_off;  // time offsets to apply to UTC time
int min_off;
int shift_dir;  // + or -
tmElements_t tm;
time_t prevDisplay = 0; // When the digital clock was displayed
extern bool use_wired_PTT;
extern uint8_t band;
extern void PTT_Output(uint8_t band, uint8_t PTT_state);
struct position p[1] = {};

struct cmdList cmd_List[End_of_Cmd_List] = {
    {CIV_C_F_SEND,          {1,0x00}},                      // send operating frequency to all
    {CIV_C_F1_SEND,         {1,0x05}},                      // send operating frequency to one
    {CIV_C_F_READ,          {1,0x03}},                      // read operating frequency
    {CIV_C_F26,        		  {1,0x26}},                 // read selected VFO m data, filt
    {CIV_C_F26A,        	  {2,0x26,0x00}},                 // read selected VFO m data, filt
    {CIV_C_F26B,       		  {2,0x26,0x01}},                 // read un- selected VFO m data, filt
    {CIV_C_F25A_SEND,       {2,0x25,0x00}},                 // set selected VFO frequency
    {CIV_C_F25B_SEND,       {2,0x25,0x01}},                 // set un-selected VFO frequency
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

//---------------------------------------------------------------------------------------------------------

unsigned int hexToDec(String hexString) {
    hexString.reserve(2);
    unsigned int decValue = 0;
    unsigned int nextInt;
    for (unsigned int i = 0; i < hexString.length(); i++) {
        nextInt = int(hexString.charAt(i));
        if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
        if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
        if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
        nextInt = constrain((signed)nextInt, 0, 15);
        decValue = (decValue * 16) + nextInt;
    }
    return decValue;
}

// reverses a string 'str' of length 'len' 
void reverse(char *str, int len) 
{ 
    int i=0, j=len-1, temp; 
    while (i<j) 
    { 
        temp = str[i]; 
        str[i] = str[j]; 
        str[j] = temp; 
        i++; j--; 
    } 
} 
 // Converts a given integer x to string str[].  d is the number 
 // of digits required in output. If d is more than the number 
 // of digits in x, then 0s are added at the beginning. 
int intToStr(int x, char str[], int d) 
{ 
    int i = 0; 
    while (x) 
    { 
        str[i++] = (x%10) + '0'; 
        x = x/10; 
    } 
  
    // If number of digits required is more, then 
    // add 0s at the beginning 
    while (i < d) 
        str[i++] = '0'; 
  
    reverse(str, i); 
    str[i] = '\0'; 
    return i; 
} 

 // Converts a floating point number to string. 
void ftoa(float n, char *res, int afterpoint) 
{ 
    // Extract integer part 
    int ipart = (int)n; 
  
    // Extract floating part 
    float fpart = n - (float)ipart; 
  
    // convert integer part to string 
    int i = intToStr(ipart, res, 0); 
  
    // check for display option after point 
    if (afterpoint != 0) 
    { 
        res[i] = '.';  // add dot 
  
        // Get the value of fraction part upto given no. 
        // of points after dot. The third parameter is needed 
        // to handle cases like 233.007 
        fpart = fpart * pow(10, afterpoint); 
  
        intToStr((int)fpart, res + i + 1, afterpoint); 
    } 
}

/*
* The algorithm is fairly straightforward. The scaling array provides divisors to divide up the space into the required number of sections,
* which is 18 for for the field, 10 for the square, 24 for the subsquare, 10 for the extended square, then 24, then 10. 
* The limit is 6 pairs which is 2 more than is used even in the most detailed versions (8 characters in all). 
* The divisor is also used in the fmod function to narrow down the range to the next highest order square that we’re dividing up.
* The scaling array could be precalculated, but I figure the optimizing compiler would take care of that. 
* I also thought the values could be scaled up at the beginning of the function, then use integer arithmetic to do the conversion. 
* It might be a bit faster.
*
*/

/*
To run it “./geo lat long”, e.g. “./geo 43.999 -79.495” which yields FN03gx09
*/

/*  Parse the GPS NMEA ASCII GPGGA string for the time, latitude and longitude  */

//---------------------------------------------------------------------------------------------------------

int positionToMaidenhead(char m[])
{

    const int pairs=4;
    const double scaling[]={360.,360./18.,(360./18.)/10., \
    ((360./18.)/10.)/24.,(((360./18.)/10.)/24.)/10., \
    ((((360./18.)/10.)/24.)/10.)/24., \
    (((((360./18.)/10.)/24.)/10.)/24.)/10.};
    int i;
    int index;

    for (i=0;i<pairs;i++)
    {
        index = (int)floor(fmod((180.0+p->longitude), scaling[i])/scaling[i+1]); 
        m[i*2] = (i&1) ? 0x30+index : (i&2) ? 0x61+index : 0x41+index;
        index = (int)floor(fmod((90.0+p->latitude), (scaling[i]/2))/(scaling[i+1]/2));
        m[i*2+1] = (i&1) ? 0x30+index : (i&2) ? 0x61+index : 0x41+index;
    }
    m[pairs*2]=0;
    return 1;  // success
}

/*  Convert Alt and Lon to MH now */

int Convert_to_MH(void)
{
    char m[9];

   // if(GPS_Status == GPS_STATUS_LOCK_INVALID || msg_Complete == 0)   
   //     return 1;  /* if we are here with invalid data then exit.  LAt and LOnwill have text which cannot be computered of cour     */
    /*  Get from GPS Serial input later */
    p->latitude=atof(Latitude);
    p->longitude=atof(Longitude);

    if (positionToMaidenhead(m))
    {   
        strncpy(Grid_Square,m,8);
        return 1;  // Success               
    }
    else
    {
        strcpy(Grid_Square,"Invalid\0");
        return 0; // fail      /*  Can use later to skip displaying anything when have invalid or no GPS input   */
    }
}

void ConvertToMinutes(char _gps_msg[])
{
    double  longitude_d, longitude_m, latitude_d, latitude_m;
    char    ns='?', ew='?';
    char    Degrees[13];
    char    Minutes[13];
    char    tempstring[12];
    char *ptr = _gps_msg;

    strncpy(Latitude, ptr, 9);      
    strncpy(Degrees,Latitude,2);
    Degrees[2] = '\0';
    latitude_d = atoi(Degrees);
    strncpy(Minutes,&Latitude[2],7);
    Minutes[9] = '\0';
    latitude_m = atof(Minutes);
    latitude_m /= 60;
    latitude_d = latitude_d + latitude_m;    
    ftoa(latitude_d, tempstring, 6);  
  
    /*  use N/S to set neg or pos */ 
    ptr = strchr(ptr+1, ',');
    ns = ptr[1] == ',' ? '?' : ptr[1];  /*  get the north south letter */
    if(ns == 'S')
        sniprintf(Latitude, 11, "%c%s", '-', tempstring);       /*  the ftoa function does not handle neg numbers */
    else
        sniprintf(Latitude, 11, "%c%s", ' ', tempstring);    
  
    ptr = strchr(ptr+1, ',');
    strncpy(Longitude, ptr+1, 10);
    strncpy(Degrees,Longitude,3);
    Degrees[3] = '\0';
    longitude_d = atoi(Degrees);
    strncpy(Minutes,&Longitude[3],8);
    Minutes[9] = '\0';
    longitude_m = atof(Minutes);
    longitude_m /= 60;
    longitude_d = longitude_d + longitude_m;
    ftoa(longitude_d, tempstring, 9);
    
    ptr = strchr(ptr+1, ',');
    ew = ptr[1] == ',' ? '?' : ptr[1];
    
    /*  use E/W to set neg or pos */
    if(ew == 'W')
        sniprintf(Longitude, 13, "%c%s", '-', tempstring);        /*  the ftoa function does not handle neg numbers */    
    else
        sniprintf(Longitude, 13, "%c%s", ' ', tempstring); 
}        
 


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

void CIV_Action(const uint8_t cmd_num, const uint8_t data_start_idx, const uint8_t data_len, const uint8_t msg_len, const uint8_t rd_buffer[])
{ 
  
  #ifdef DBG_CIV1
  Serial.printf("cmd = %X  data_start_idx = %d  data_len = %d  rd_buffer:%X %X %X %X %X %X %X\n", cmd_List[cmd_num].cmdData[1], data_start_idx, data_len, rd_buffer[0], rd_buffer[1], rd_buffer[2], rd_buffer[3],rd_buffer[4], rd_buffer[5], rd_buffer[6]);
  #endif
  
  switch (cmd_num) 
  {
    case CIV_C_F_READ:
    case CIV_C_F_SEND:
    case CIV_C_F1_SEND:
          read_Frequency(data_len);
          break;

    case CIV_C_TX:  // Used to request RX TX status from radio
          PTT = rd_buffer[data_start_idx];
          if (TX_last != PTT)
          {
            if (!use_wired_PTT)        // normally the wired input will pass thru the PTT from radio hardware PTT. 
            PTT_Output(band, PTT);   //  If that is not available, then use the radio polled TX state .
            //Serial.printf("TX Status = %d\n", PTT);
            TX_last = PTT;
          }         
          break;                     // Call PTT output here rather than in teh main loop to avoid any loop delay time.

    case CIV_C_PREAMP_READ:
          //Serial.printf("Preamp = %d\n", rd_buffer[6]);
          break;

    case CIV_C_MOD_READ:
    case CIV_C_MOD1_SEND:
    case CIV_C_MOD_SEND:
          //Serial.printf("Mode, Len = %d\n", data_len);
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

            hr_off = bcdByte(rd_buffer[data_start_idx+0]); 
            min_off = bcdByte(rd_buffer[data_start_idx+1]); 
            shift_dir = bcdByte(rd_buffer[data_start_idx+2]);
            
            
            if (shift_dir) 
            {
              hr_off = hr_off * -1;  // invert  - used by UTC set function
              min_off = min_off * -1;  // invert  - used by UTC set function
            }
            //DPRINTF("UTC Offset: "); Serial.print(hr_off); DPRINTF(":");Serial.println(min_off);

            //get current time and correct or set time zone offset
            //setTime(_hr,_min,_sec,_day,_month,_yr);

          }  // UTC Offset
          break;

    case CIV_C_MY_POSIT_READ:
          {
            //Serial.printf("processCatMessages: Position, Len = %d\n", data_len);
            //uint8_t RX = CIVresultL.value;

            //DPRINTF("check_CIV: CI-V Returned MY POSITION and TIME: "); DPRINTLN(retValStr[CIVresultL.value]);
            //               pos                  1  2  3  4   5     6  7  8  9  10   11       12 13 14 15   16 17   18 19 20   21 22  23  24  25 26 27  term
            // FE.FE.E0.AC.  23.00.  datalen 27  47.46.92.50.  01.   01.22.01.98.70.  00.      00.15.59.00.  01.05.  00.00.07.  20.24. 07. 20. 23.32.45. FD
            //                               lat 4746.9250   01(N)     12201.9870    00(-1) long  155.900m alt  105deg   0.7km/h   2024   07  20  23:32:45 UTC
            // when using datafield, add 1 to prog guide index to account for first byte used as length counter - so 27 is 28 here.
            //DPRINTF("** Time from Radio is: ");
            
            int _hr = bcdByte(rd_buffer[data_start_idx+24]); //Serial.print(_hr); DPRINTF(":");
            int _min = bcdByte(rd_buffer[data_start_idx+25]); //Serial.print(_min);DPRINTF(":");
            int _sec = bcdByte(rd_buffer[data_start_idx+26]); //Serial.print(_sec);DPRINTF(" ");
            
            int _month = bcdByte(rd_buffer[data_start_idx+22]); //Serial.print(_month);DPRINTF(".");
            int _day = bcdByte(rd_buffer[data_start_idx+23]); //Serial.print(_day); DPRINTF(".");
            int _yr = bcdByte(rd_buffer[data_start_idx+21]); //Serial.print(_yr); // yr can be 4 or 2 digits  2024 or 24                    
            
            if (!UTC) 
            {
              setTime(_hr+hr_off,_min+min_off,_sec,_day,_month,_yr);  // correct to local time                      
              //Serial.printf("Local Time: %d:%d:%d  %d/%d/20%d\n",_hr+hr_off,_min+min_off,_sec,_month,_day,_yr); 
            }
            else
            {
              setTime(_hr,_min,_sec,_day,_month,_yr);  // display UTC time
              //Serial.printf("UTC Time: %d:%d:%d  %d/%d/20%d\n",_hr,_min,_sec,_month,_day,_yr); 
            }
            
            // Extract and Process Lat and Long for Maidenhead Grid Square stored in global Grid_Square[]
            //char GPS_Msg[NMEA_MAX_LENGTH] = {}; // {"4746.92382,N,12201.98606,W\0"};
            uint8_t i = 0;
            uint8_t k = 0;
            float lati_deg = 0.0;
            float longi_deg = 0.0;
            float lati_min = 0.0;
            float longi_min = 0.0;
            float lati = 0.0;
            float longi = 0.0;
            char temp_str[14];

            // reformat latitude
            // 47.46.93.10.  01.  
            lati_deg  = (float) bcdByte(rd_buffer[data_start_idx+k++]);         //  47 deg
            lati_min  = (float) bcdByte(rd_buffer[data_start_idx+k++]);         //  46. min
            lati_min +=  (float) bcdByte(rd_buffer[data_start_idx+k++]) /100;    //    .93
            lati_min += (float) bcdByte(rd_buffer[data_start_idx+k++]) /10000;  //    .0010  last nibble is always 0
            lati_min /= 60;
            lati = lati_deg+lati_min;
            ftoa(lati, temp_str, 6);   // convert to string now, ftoa cannot handle negative numbers
            // if 1 then North, else south will be negative
            if (!bcdByte(rd_buffer[data_start_idx+k++])) /*  use N/S to set neg or pos */              
              sniprintf(Latitude, 11, "%c%s", '-', temp_str);       /*  the ftoa function does not handle neg numbers */
            else
              sniprintf(Latitude, 11, "%c%s", ' ', temp_str);
            
            //Serial.printf("Latitude Converted to dd mm ss format: Deg=%f  min=%f  lat=%f  string=%s\n",lati_deg, lati_min, lati, Latitude);

            // Longitude  01.22.01.98.70.  00.
            longi_deg  = (float) bcdByte(rd_buffer[data_start_idx+k++])*100;     // 100  first nibble is always 0
            longi_deg += (float) bcdByte(rd_buffer[data_start_idx+k++]);         //  22 deg
            longi_min  = (float) bcdByte(rd_buffer[data_start_idx+k++]);         //  01 min
            longi_min += (float) bcdByte(rd_buffer[data_start_idx+k++]) /100;    //    .98
            longi_min += (float) bcdByte(rd_buffer[data_start_idx+k++]) /10000;  //    .0070
            longi_min /= 60;  // convert minutes to mm.mmm
            longi = longi_deg+longi_min;  // make 1 number
            ftoa(longi, temp_str, 9);   // convert to string now, ftoa cannot handle negative numbers
            
            // if 1 then E, else W will be negative
             if (!bcdByte(rd_buffer[data_start_idx+k++]))
              sniprintf(Longitude, 13, "%c%s", '-', temp_str);        /*  the ftoa function does not handle neg numbers */    
            else
              sniprintf(Longitude, 13, "%c%s", ' ', temp_str); 
            
            //Serial.printf("Longitude Converted to dd mm ss format:  deg=%f   min=%f   long=%f  string=%s  %s\n",longi_deg, longi_min, longi, Latitude, Longitude);

            // if using NMEA string then proved a formnatted string like belw and convert to minutes  
            //strcpy(GPS_Msg, "4746.92382,N,12201.98606,W\0"};   // test string
            //ConvertToMinutes(GPS_Msg);       
            // Here I directly converted to what Convert_to_MH wants
            Convert_to_MH();
            //Serial.printf("GPS Converted: Lat = %s  Long = %s  Grid Square is %s\n", Latitude, Longitude, Grid_Square);

            break;
          }  // MY Position

    default: DPRINTLNF("*** default action");
          //knowncommand = false;
  }
}
