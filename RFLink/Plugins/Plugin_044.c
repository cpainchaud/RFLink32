//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                     Plugin-044: Auriol V3                                         ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding the Auriol protocol for sensor type Z32171A
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
 * Decodes signals from a Auriol Weatherstation outdoor unit, (40 bits, 433 MHz).
 *
 * Auriol Message Format: 
 * 1011 1111 1001 1010 0110 0001 1011 0100 1001 0001
 * B    F    9    A    6    1    B    4    9    1
 * AAAA AAAA BBBB CCDD EEEE EEEE EEEE FFFF FFFF GGHH
 *
 * A = ID?
 * B = Rolling code?
 * C = possibly battery indicator ?
 * D = trend (2 bits) indicating temp equal/up/down ?
 * E = Temperature => 0x61b  (0x61b-0x4c4)=0x157 *5)=0x6b3 /9)=0xBE => 0xBE = 190 decimal!
 * F = humidity: 49% 
 * G = ?
 * H = channel: 1 (2 bits)
 *
 * Sample:
 * 20;C2;DEBUG;Pulses=82;Pulses(uSec)=475,3850,450,1700,450,3825,450,3900,450,3725,450,3825,450,3825,450,3900,450,3725,450,1700,450,1700,450,3900,450,3725,450,1700,450,1700,450,1800,450,1625,450,3800,450,3825,450,1800,450,1625,450,1700,450,1700,450,1800,450,3725,450,3800,450,1700,450,1800,450,1625,450,3825,450,1700,450,3900,450,1625,450,1700,450,1700,450,3900,450,1625,450,1700,450,1700,450,3825,500;
 \*********************************************************************************************/
#define AURIOLV3_PLUGIN_ID 044
#define PLUGIN_DESC_044 "Auriol V3"
#define AURIOLV3_PULSECOUNT 82

#define AURIOLV3_MIDHI_D 650

#define AURIOLV3_PULSEMIN_D 1500
#define AURIOLV3_PULSEMINMAX_D 2000
#define AURIOLV3_PULSEMAXMIN_D 3500

#ifdef PLUGIN_044
#include "../4_Display.h"

boolean Plugin_044(byte function, const char *string)
{
   if (RawSignal.Number != AURIOLV3_PULSECOUNT)
      return false;

   const long AURIOLV3_MIDHI = AURIOLV3_MIDHI_D / RawSignal.Multiply;
   const long AURIOLV3_PULSEMIN = AURIOLV3_PULSEMIN_D / RawSignal.Multiply;
   const long AURIOLV3_PULSEMINMAX = AURIOLV3_PULSEMINMAX_D / RawSignal.Multiply;
   const long AURIOLV3_PULSEMAXMIN = AURIOLV3_PULSEMAXMIN_D / RawSignal.Multiply;

   unsigned long bitstream1 = 0L; // holds first 4x4=16 bits
   unsigned long bitstream2 = 0L; // holds last  6x4=24 bits
   byte bitcounter = 0;           // counts number of received bits (converted from pulses)
   byte rc = 0;
   byte channel = 0;
   unsigned long temperature = 0;
   byte humidity = 0;
   //==================================================================================
   // Get all 40 bits
   //==================================================================================
   for (byte x = 2; x < AURIOLV3_PULSECOUNT; x += 2)
   {
      if (RawSignal.Pulses[x + 1] * RawSignal.Multiply > AURIOLV3_MIDHI)
         return false;
      if (RawSignal.Pulses[x] > AURIOLV3_PULSEMAXMIN)
      {
         if (bitcounter < 16)
         {
            bitstream1 <<= 1;
            bitstream1 |= 0x1;
            bitcounter++; // only need to count the first 10 bits
         }
         else
         {
            bitstream2 <<= 1;
            bitstream2 |= 0x1;
         }
      }
      else
      {
         if (RawSignal.Pulses[x] > AURIOLV3_PULSEMINMAX)
            return false;
         if (RawSignal.Pulses[x] < AURIOLV3_PULSEMIN)
            return false;
         if (bitcounter < 16)
         {
            bitstream1 <<= 1;
            bitcounter++; // only need to count the first 10 bits
         }
         else
            bitstream2 <<= 1;
      }
   }
   //==================================================================================
   // Perform sanity checks and prevent repeating signals from showing up
   //==================================================================================
   if ((SignalHash != SignalHashPrevious) || (RepeatingTimer < millis()))
   {
      if (bitstream1 == 0)
         return false; // not seen the RF packet recently
      if (bitstream2 == 0)
         return false;
   }
   else
   {
      return true; // already seen the RF packet recently
   }
   //==================================================================================
   // now process sensor type
   //==================================================================================
   rc = (bitstream1 >> 8) & 0xFF;              // get rolling code
   temperature = ((bitstream2) >> 12) & 0xFFF; // get 12 temperature bits
   temperature = (temperature - 0x4c4) & 0xFFFF;
   temperature = (((temperature)*5) / 9) & 0xFFFF;
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
   humidity = (bitstream2 >> 4) & 0xff; // humidity
   channel = (bitstream2)&0x03;         // channel number
   //==================================================================================
   // Output
   //==================================================================================
   display_Header();
   display_Name(PSTR("Auriol V3"));
   char c_ID[5];
   sprintf(c_ID, "%02X%02X", rc, channel);
   display_IDc(c_ID);
   display_TEMP(temperature);
   display_HUM(humidity, HUM_HEX);
   display_Footer();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // PLUGIN_044
