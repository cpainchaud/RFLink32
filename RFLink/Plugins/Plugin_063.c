//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                     Plugin-063: Oregon PIR/LED/ALARM                              ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin provides support for Oregon PIR/LED/ALARM devices
 *
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : StuntTeam 2015..2016
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Technical data:
 * Devices send 50/52 pulses using typical Oregon bit encoding
 *
 * Sample:
 * 570,390,570,360,570,360,540,360,540,360,570,360,570,330,570,330,570,330,570,330,570,330,570,360,540,360,540,360,1110,330,570,900,1110,900,540,330,570,330,1110,900,1110,360,540,360,540,900,1110,330,570,330,570,1620
 * 00101110 10001000 = 2E88
 \*********************************************************************************************/
#define OREGON_PLA_PLUGIN_ID 063
#define PLUGIN_DESC_063 "X10"

#define OREGON_PLA_PULSECOUNT 52

#ifdef PLUGIN_063
#include "../4_Display.h"

boolean Plugin_063(byte function, const char *string)
{
   if ((RawSignal.Number < OREGON_PLA_PULSECOUNT - 2) || (RawSignal.Number > OREGON_PLA_PULSECOUNT))
      return false;

   if (RawSignal.Pulses[0] != 63)
      return false; // Only accept RF packets converted by plugin 1
   RawSignal.Pulses[0] = 0;

   unsigned long bitstream = 0L;
   //==================================================================================
   // Perform a pre sanity check
   //==================================================================================
   for (byte x = 1; x < 28; x += 2)
   {
      if (RawSignal.Pulses[x] * RawSignal.Multiply > 600)
         return false;
      if (RawSignal.Pulses[x + 1] * RawSignal.Multiply > 600)
         return false;
   }
   //==================================================================================
   byte bits = 0;
   byte rfbit = 1;
   //==================================================================================
   // Get all 24 bits
   //==================================================================================
   for (byte x = 29; x <= RawSignal.Number; x++)
   {
      if (RawSignal.Pulses[x] * RawSignal.Multiply > 600)
      { // toggle bit value
         if (RawSignal.Pulses[x] * RawSignal.Multiply > 1600)
            break; // done..
         rfbit = (~rfbit) & 1;
         bitstream = (bitstream << 1) | rfbit;
         if (RawSignal.Pulses[x + 1] * RawSignal.Multiply < 600)
            x++;
      }
      else
      {
         bitstream = (bitstream << 1) | rfbit; // short pulse keep bit value
         if (RawSignal.Pulses[x + 1] * RawSignal.Multiply < 600)
            x++;
      }
   }
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   if ((SignalHash != SignalHashPrevious) || (RepeatingTimer + 2000 < millis()))
   {
      // not seen the RF packet recently
      if (bitstream == 0)
         return false;
   }
   else
   {
      // already seen the RF packet recently
      return true;
   }
   //==================================================================================
   // calculate sensor and channel
   bitstream = (bitstream) >> 4;
   bits = (bitstream)&0x3;
   bits = bits + 0x30; // signal 3 for plugin 63
   //==================================================================================
   // Output
   // ----------------------------------
   display_Header();
   display_Name(PSTR("X10"));
   display_IDn((bitstream & 0xFFFFFF), 6); // "%S%06lx"
   display_SWITCH(bits);                   // "%02x"
   display_CMD(CMD_Single, CMD_On);        // #ALL #ON
   display_Footer();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // Plugin_063
