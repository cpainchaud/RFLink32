//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                       Plugin-033: Conrad                                           ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding Conrad Pool Thermomether model 9771
 * 
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : StuntTeam 2015..2016
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical information:
 * Decodes signals from a Conrad Model 9771 Pool Thermometer, (80 pulses, 40 bits, 433 MHz).
 * Message Format: 
 * AAAAAAAA BBBBBBBB CCCCCCCCCC DD EEEE FFFF GGGG
 * 00000000 00010010 1100101101 01 1001 0001 1001
 *
 * A = Always 0 ?
 * B = Device id ?
 * C = Temperature digits 
 * D = Temperature ones
 * E = Temperature tens
 * F = Always 1?
 * G = Unknown
 *
 * Sample:
 * 20;8D;DEBUG;Pulses=80;Pulses(uSec)=1890,5760,1890,5730,1890,5760,1890,5730,1890,5760,1890,5760,1890,5760,1890,5760,1890,5760,1890,5760,1890,5760,5910,1830,1890,5640,1890,5760,5910,1830,1890,5640,5910,1830,1860,5640,5910,1830,1890,5610,5910,1830,5910,1830,5910,1830,1890,5400,5910,1830,1890,5610,1890,5760,5910,1830,5910,1830,1890,5520,1890,5760,5910,1860,1890,5610,1890,5760,1890,5760,5910,1830,1890,5610,5910,1830,5910,1830,1860,6990;
 \*********************************************************************************************/
#define CONRAD_PLUGIN_ID 033
#define PLUGIN_DESC_033 "Conrad"
#define CONRAD_PULSECOUNT 80

#define CONRAD_PULSEMAX_D 5000
#define CONRAD_PULSEMIN_D 2300

#ifdef PLUGIN_033
#include "../4_Display.h"

boolean Plugin_033(byte function, const char *string)
{
   if (RawSignal.Number != CONRAD_PULSECOUNT)
      return false;

   const long CONRAD_PULSEMAX = CONRAD_PULSEMAX_D / RawSignal.Multiply;
   const long CONRAD_PULSEMIN = CONRAD_PULSEMIN_D / RawSignal.Multiply;

   unsigned long bitstream = 0L;
   byte checksum = 0;
   byte bitcount = 0; // bit counter (counting first 8 bits that need
   unsigned int rc = 0;
   unsigned int temperature = 0;
   //==================================================================================
   // Get all 28 bits
   //==================================================================================
   for (byte x = 1; x <= CONRAD_PULSECOUNT - 1; x += 2)
   {
      if (RawSignal.Pulses[x] > CONRAD_PULSEMAX)
      {
         if (RawSignal.Pulses[x + 1] > CONRAD_PULSEMAX)
            if ((x + 1) < CONRAD_PULSECOUNT)
               return false; // invalid pulse length

         if (bitcount > 7)
         {
            bitstream <<= 1;
            bitstream |= 0x1;
         }
         else
            return false; // first 8 bits should all be zeros
         bitcount++;
      }
      else
      {
         if (RawSignal.Pulses[x] > CONRAD_PULSEMIN)
            return false; // invalid pulse length

         if (RawSignal.Pulses[x + 1] < CONRAD_PULSEMIN)
            return false; // invalid pulse length

         if (bitcount > 7)
            bitstream <<= 1;

         bitcount++;
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
   if ((SignalHash != SignalHashPrevious) || (RepeatingTimer + 500 < millis()) || (SignalCRC != bitstream))
      SignalCRC = bitstream; // not seen the RF packet recently
   else
      return true; // already seen the RF packet recently
   //==================================================================================
   // Perform checksum calculations
   //==================================================================================
   checksum = (bitstream >> 4) & 0xF;
   if (checksum != 0x1)
      return false;
   //==================================================================================
   // Now process the various sensor types
   //==================================================================================
   rc = ((bitstream >> 24) & 0xFF);
   temperature = ((bitstream >> 14) & 0x3FF);
   temperature = temperature - 500;
   //==================================================================================
   // Output
   //==================================================================================
   display_Header();
   display_Name(PSTR("Conrad"));
   display_IDn(rc, 4);
   display_TEMP(temperature);
   display_Footer();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // PLUGIN_033
