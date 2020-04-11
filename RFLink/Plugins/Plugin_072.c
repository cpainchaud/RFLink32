//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                 Plugin-072 Byron Wireless Doorbell                                ##
//#######################################################################################################
/*********************************************************************************************\
 * This Plugin takes care of reception And sending of the Byron SX doorbell
 *
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : Maurice Ruiter (Dodge) 2015..2016
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical information:
 * 26 pulses, manchester code, 12 bits
 *
 * 111111110001
 * AAAAAAAABBBB
 *
 * A = 8 bit Address  
 * B = chime number    
 *     Valid chime numbers: 1,2,6,9,a,c,d,e ?
 * ---------------------------------------------------------
 * 20;25;DEBUG;Pulses=26;Pulses(uSec)=275,250,250,525,250,225,250,525,250,225,250,525,525,225,250,525,525,225,250,225,250,225,250,525,525;
 * 20;F0;DEBUG;Pulses=511;Pulses(uSec)=450,225,575,200,575,225,575,200,575,200,575,200,575,225,575,550,250,200,575,200,575,200,575,550,250,2825,250,200,575,200,575,200,575,225,575,200,575,200,575,200,575,550,250,200,575,200,575,200,575,525,250,2825,250,200,575,200,575,200,575,225,575,200,575,200,575,200,575,550,250,225,575,200,575,225,575,550,250,2825,250,200,575,200,575,200,575,200,575,200,575,200,575,200,575,550,250,200,575,225,575,200,575,550,250,2850,250,200,575,200,575,200,575,200,575,200,575,200,575,200,575,550,250,225,575,200,575,200,575,550,250,2850,225,225,575,200,575,225,575,200,575,200,575,200,575,200,575,550,250,225,575,200,575,200,575,525,250,2825,250,225,575,200,575,200,575,200,575,200,575,200,575,200,575,550,250,200,575,200,575,225,575,525,250,2825,250,225,575,225,575,200,575,200,575,200,575,225,575,200,575,550,250,200,575,200,575,200,575,550,250,2825,250,200,575,200,575,200,575,225,575,225,575,200,575,200,575,550,250,200,575,200,575,200,575,525,250,2825,250,200,575,200,575,200,575,200,575,200,575,200,575,200,575,550,250,225,575,200,575,225,575,550,250,2825,250,200,575,200,575,200,575,200,575,200,575,225,575,200,575,550,250,200,575,225,575,200,575,550,250,2850,250,200,575,200,575,200,575,200,575,200,575,200,575,200,575,550,250,225,575,225,575,200,575,550,250,2850,250,200,575,200,575,225,575,200,575,200,575,200,575,200,575,550,250,225,575,200,575,200,575,550,250,2825,250,200,575,200,575,200,575,200,575,225,575,200,575,200,575,550,250,200,575,200,575,225,575,550,250,2825,250,200,575,225,575,200,575,200,575,200,575,225,575,200,575,525,250,200,575,200,575,200,575,550,250,2825,250,200,575,225,575,200,575,200,575,200,575,225,575,200,575,550,250,200,575,200,575,200,575,525,250,2825,250,200,575,200,575,225,575,200,575,200,575,200,575,225,575,550,250,200,575,225,575,225,575,525,250,2825,250,200,575,200,575,200,575,200,575,225,575,200,575,225,575,550,250,225,575,200,575,225,575,550,250,2825,250,200,575,200,575,200,575,200,575,200,575,200,575,225,575,525,250,225,575,200,575,225,575,550,250,2850,225,225,575,200,575,225,575,200,575,200,575,200,575,200,575,525;
 * 20;31;DEBUG;Pulses=511;Pulses(uSec)=450,550,250,550,250,550,250,550,250,525,250,525,250,550,250,550,250,225,575,200,575,200,575,550,250,2825,250,550,250,550,250,550,250,550,250,550,250,550,250,525,250,525,250,200,575,225,575,200,575,550,250,2825,250,525,250,550,250,550,250,550,250,550,250,550,250,550,250,550,250,200,575,200,575,200,575,525,250,2825,250,550,250,550,250,525,250,550,250,550,250,550,250,550,250,550,250,200,575,200,575,200,575,525,250,2825,250,550,250,525,250,525,250,525,250,550,250,550,250,550,250,550,250,225,575,200,575,200,575,525,250,2825,250,550,250,525,250,525,250,525,250,525,250,550,250,550,250,550,250,200,575,200,575,225,575,550,250,2825,225,550,250,525,250,525,250,550,250,550,250,525,250,525,250,525,250,225,575,200,575,225,575,550,250,2850,250,550,250,550,250,550,250,525,250,525,250,525,250,550,250,550,250,225,575,200,575,225,575,550,250,2850,225,550,250,550,250,550,250,550,250,525,250,525,250,525,250,550,250,200,575,200,575,200,575,525,250,2825,250,550,250,550,250,550,250,550,250,550,250,550,250,525,250,525,250,200,575,200,575,200,575,550,250,2825,250,525,250,550,250,550,250,550,250,550,250,550,250,550,250,550,250,200,575,200,575,200,575,550,250,2825,250,550,250,525,250,525,250,525,250,550,250,550,250,550,250,550,250,200,575,200,575,200,575,525,250,2825,250,550,250,550,250,550,250,525,250,550,250,550,250,550,250,550,250,200,575,200,575,225,575,525,250,2825,250,550,250,550,250,525,250,525,250,550,250,550,250,525,250,525,250,200,575,225,575,225,575,550,250,2850,250,550,250,550,250,525,250,525,250,525,250,550,250,550,250,550,250,200,575,225,575,200,575,550,250,2850,250,550,250,550,250,550,250,550,250,525,250,525,250,550,250,550,250,225,575,225,575,200,575,550,250,2825,250,550,250,550,250,550,250,550,250,550,250,550,250,550,250,525,250,200,575,200,575,200,575,550,250,2825,250,525,250,550,250,550,250,550,250,550,250,550,250,550,250,525,250,200,575,200,575,200,575,550,250,2825,250,525,250,550,250,550,250,550,250,550,250,550,250,550,250,550,250,200,575,200,575,200,575,525,250,2825,250,550,250,550,250,525,250,525,250,525,250,550,250,550,250,550;
 * 20;25;DEBUG;Pulses=26;Pulses(uSec)=250,550,250,550,250,550,250,550,250,550,250,550,250,525,250,525,250,200,575,225,575,200,575,550,250;
 \*********************************************************************************************/
#define BYRON_PLUGIN_ID 72
#define BYRON_PULSECOUNT 26

#define BYRONSTART 3000
#define BYRONSPACE 250
#define BYRONLOW 350
#define BYRONHIGH 675

#ifdef PLUGIN_072
#include "../4_Display.h"

boolean Plugin_072(byte function, char *string)
{
   if (RawSignal.Number != BYRON_PULSECOUNT)
      return false;
   if (RawSignal.Pulses[0] != BYRON_PLUGIN_ID)
      return false; // only accept plugin1 translated packets
   if (RawSignal.Pulses[1] * RAWSIGNAL_SAMPLE_RATE > 425)
      return false; // first pulse is start bit and must be short

   unsigned long bitstream = 0L;
   //==================================================================================
   // Get all 12 bits
   //==================================================================================
   for (byte x = 2; x < BYRON_PULSECOUNT; x += 2)
   {
      bitstream <<= 1; // Always shift
      if (RawSignal.Pulses[x] * RAWSIGNAL_SAMPLE_RATE < 350)
      { // 200-275 (150-350 is accepted)
         if (RawSignal.Pulses[x] * RAWSIGNAL_SAMPLE_RATE < 150)
            return false; // pulse too short
         if (RawSignal.Pulses[x + 1] * RAWSIGNAL_SAMPLE_RATE < 350)
            return false; // bad manchester code
         // bitstream |= 0x0;
      }
      else
      { // 500-575 (450-650 is accepted)
         if (RawSignal.Pulses[x + 1] * RAWSIGNAL_SAMPLE_RATE > 450)
            return false; // bad manchester code
         if (RawSignal.Pulses[x] * RAWSIGNAL_SAMPLE_RATE < 450)
            return false; // pulse too short
         if (RawSignal.Pulses[x] * RAWSIGNAL_SAMPLE_RATE > 650)
            return false; // pulse too long
         bitstream |= 0x1;
      }
   }
   //==================================================================================
   // Perform a quick sanity check
   //==================================================================================
   if (bitstream == 0)
      return false;
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   if ((SignalHash != SignalHashPrevious) || ((RepeatingTimer + 1000) < millis()) || (SignalCRC != bitstream))
      SignalCRC = bitstream; // not seen the RF packet recently
   else
      return true; // already seen the RF packet recently
   //==================================================================================
   // Output
   //==================================================================================
   display_Header();
   display_Name(PSTR("Byron SX"));
   display_IDn(((bitstream >> 4) & 0xFF), 4);
   display_SWITCH(1);
   display_CMD(false, true);
   display_CHIME((bitstream & 0xF));
   display_Footer();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // PLUGIN_072

#ifdef PLUGIN_TX_072
boolean PluginTX_072(byte function, char *string)
{
   boolean success = false;
   //10;BYRON;112233;01;OFF;
   //01234567890123456789012
   if (strncasecmp(InputBuffer_Serial + 3, "BYRON;", 5) == 0)
   { // KAKU Command eg.
      if (InputBuffer_Serial[15] != ';')
         return false;

      InputBuffer_Serial[9] = '0';
      InputBuffer_Serial[10] = 'x';
      InputBuffer_Serial[15] = 0;
      unsigned int tempbyte1 = 0;
      tempbyte1 = str2int(InputBuffer_Serial + 9); // get parameter 1

      int tempbyte2 = 0;
      InputBuffer_Serial[14] = '0';
      InputBuffer_Serial[15] = 'x';
      InputBuffer_Serial[18] = 0;
      tempbyte2 = str2int(InputBuffer_Serial + 14); // get parameter 2
      //-----------------------------------------------
      unsigned long bitstream1 = tempbyte1; // address
      unsigned long bitstream = tempbyte2;  // ringtone

      RawSignal.Multiply = 50;
      RawSignal.Repeats = 20;
      RawSignal.Delay = 3; // 1 = 900 3=2825  5=  6= x
      RawSignal.Pulses[1] = BYRONLOW / RawSignal.Multiply;
      //RawSignal.Pulses[1]=BYRONSTART/RawSignal.Multiply;
      for (byte x = 17; x >= 2; x = x - 1)
      {
         if ((bitstream1 & 1) == 1)
            RawSignal.Pulses[x] = BYRONHIGH / RawSignal.Multiply;
         else
            RawSignal.Pulses[x] = BYRONLOW / RawSignal.Multiply;
         bitstream1 = bitstream1 >> 1;
      }
      for (byte x = 25; x >= 18; x = x - 1)
      {
         if ((bitstream & 1) == 1)
            RawSignal.Pulses[x] = BYRONHIGH / RawSignal.Multiply;
         else
            RawSignal.Pulses[x] = BYRONLOW / RawSignal.Multiply;
         bitstream = bitstream >> 1;
      }
      //RawSignal.Pulses[26]=BYRONSTART/RawSignal.Multiply;
      RawSignal.Pulses[26] = BYRONSPACE / RawSignal.Multiply;
      RawSignal.Number = 26;
      RawSendRF();
      RawSignal.Multiply = RAWSIGNAL_SAMPLE_RATE;
      success = true;
      //-----------------------------------------------
   }
   return success;
}
#endif // PLUGIN_TX_072
