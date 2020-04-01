//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                      Plugin-46 Auriol / Xiron                                     ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding the Auriol protocol for sensor type Z31055A-TX and Xiron 
 * Watshome YT6018-2 (From Fujian Youtong)
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
 * Technical Information:
 * Decodes signals from a Auriol and Xiron Weatherstation outdoor unit, (36 bits, 433 MHz).
 *
 * Auriol Message Format: 
 * 10100100 10 00 000011100110 1111 00000000  Temp = 23.60
 * AAAAAAAA BB CC DDDDDDDDDDDD EEEE FFFFFFFF
 * A = Rolling Code, no change during normal operation. (Device 'Session' ID) (Might also be 4 bits RC and 4 bits for channel number)
 * B = Battery status indicator on highest bit, 1=OK 0=LOW
 * C = Always 00 (Most likely channel number)
 * D = Temperature (12 bit, 21.5 degrees is shown as decimal value 215, minus values have the high bit set and need to be subtracted from a base value of 4096)
 * E = Always 1111 ?
 * F = Always 0000 0000 ?
 *
 * Xiron Message Format: 
 * 01101110 10 00 000011101101 1111 00110011
 * AAAAAAAA BB CC DDDDDDDDDDDD EEEE FFFFFFFF
 * ID       ?? Ch Temperature  ?    Humidity
 *
 * A = ID (Rolling Code, changes after battery replacement)
 * B = Battery status indicator on highest bit, 1=OK 0=LOW
 * C = Channel (1,2,3)
 * D = Temperature (12 bit, 21.5 degrees is shown as decimal value 215, minus values have the high bit set and need to be subtracted from a base value of 4096)
 * E = Always 1111 ?
 * F = Humidity
 *
 *
 * Sample:
 * 20;1F;DEBUG;Pulses=74;Pulses(uSec)=550,1575,525,675,525,1625,500,700,475,725,500,1675,500,700,500,725,475,1675,475,750,450,750,475,725,450,750,450,750,475,750,450,750,475,1675,450,1700,425,1700,450,750,450,750,450,1700,450,1700,450,775,450,1700,450,1700,450,1700,425,1700,425,775,450,775,450,775,425,775,425,775,425,775,450,775,425,775,425,
 \*********************************************************************************************/
#define PLUGIN_ID 46
#define AURIOLV2_PULSECOUNT 74

#ifdef PLUGIN_046
#include "../4_Misc.h"

boolean Plugin_046(byte function, char *string)
{
   if ((RawSignal.Number) != AURIOLV2_PULSECOUNT)
      return false;
   unsigned long bitstream1 = 0L;
   unsigned long bitstream2 = 0L;
   byte rc = 0;
   byte bat = 0;
   byte bat0 = 0;
   int temperature = 0;
   int humidity = 0;
   byte channel = 0;
   byte bitcounter = 1; // 1st bit skipped! (hence forced to 0)
   byte type = 0;
   //==================================================================================
   // get all the bits we need (36 bits)
   for (byte x = 4; x < (AURIOLV2_PULSECOUNT); x += 2)
   {
      if (RawSignal.Pulses[x + 1] * RawSignal.Multiply > 700)
         return false;
      if (RawSignal.Pulses[x] * RawSignal.Multiply > 1400)
      {
         if (RawSignal.Pulses[x] * RawSignal.Multiply > 2100)
            return false;
         if (bitcounter < 24)
         {
            bitstream1 = (bitstream1 << 1) | 0x1;
            bitcounter++; // only need to count the first 10 bits
         }
         else
         {
            bitstream2 = (bitstream2 << 1) | 0x1;
         }
      }
      else
      {
         if (RawSignal.Pulses[x] * RawSignal.Multiply > 1100)
            return false;
         if (RawSignal.Pulses[x] * RawSignal.Multiply < 500)
            return false;
         if (bitcounter < 24)
         {
            bitstream1 = (bitstream1 << 1);
            bitcounter++; // only need to count the first 10 bits
         }
         else
         {
            bitstream2 = (bitstream2 << 1);
         }
      }
   }

   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   if (((RepeatingTimer + 700) < millis()) || SignalCRC != bitstream1)
   {
      if (bitstream1 == 0)
         return false; // Perform sanity check
      if ((bitstream2 & 0xF00) != 0xF00)
         return false; // check if 'E' has all 4 bits set
      if ((bitstream2 & 0xfff) != 0xF00)
      {
         type = 1; // Xiron
         // if (RawSignal.Pulses[0] != PLUGIN_ID) {
         //   return false;                         // only accept plugin_001 translated Xiron packets
         // }
      }
      else
      {
         type = 0;                       // Auriol
         rc = (bitstream1 >> 12) & 0x07; // get 3 bits
         if (rc != 0)
            return false; // should always be '000'
      }
      //==================================================================================
      bat = (bitstream1 >> 15) & 0x01;  // get battery strength indicator
      bat0 = (bitstream1 >> 14) & 0x01; // get blank battery strength indicator
      if (bat0 != 0)
         return false;                          // blank bat must be 0
      rc = (bitstream1 >> 16) & 0xff;           // get rolling code
      channel = ((bitstream1 >> 12) & 0x3) + 1; // channel indicator
      if (channel > 3)
         return false;                  // channel out of range
      temperature = (bitstream1)&0xfff; // get 12 temperature bits
      if (temperature > 3000)
      {
         temperature = 4096 - temperature; // fix for minus temperatures
         if (temperature > 0x258)
            return false;                    // temperature out of range ( > -60.0 degrees)
         temperature = temperature | 0x8000; // turn highest bit on for minus values
      }
      else
      {
         if (temperature > 0x258)
            return false; // temperature out of range ( > 60.0 degrees)
      }
      if (type == 1)
      {
         humidity = (bitstream2)&0xff; // humidity
         if (humidity > 100)
            return false; // humidity out of range ( > 100)
      }
      //==================================================================================
      // Output
      // ----------------------------------

      display_Header();

      if (type == 0)
         display_Name(PSTR("Auriol V2"));
      else
         display_Name(PSTR("Xiron"));

      display_ID((rc << 8) | channel);
      display_TEMP(temperature);

      if (type == 1)
         display_HUM(humidity);
      display_BAT(bat);
      display_Footer();

      SignalCRC = bitstream1;
   }
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // PLUGIN_046
