//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                    Plugin-047: Auriol v4                                          ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding the Auriol protocol for sensor type Auriol HG02832
 * 
 * Author  (original)  : StormTeam 2020 - Marc RIVES (aka Couin3)
 * Support (original)  : https://github.com/couin3/RFLink 

 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical Information:
 * Decodes signals from a Auriol outdoor unit, (4 sync + 40 bits, 433 MHz).
 *
 * Auriol Message Format: 
 * 00111010 00111010 0000 000011110000 10101111
 * AAAAAAAA BBBBBBBB CCDD EEEEEEEEEEEE FFFFFFFF
 * ID       Hum      B.Ch Temp         CRC
 * 
 * A = Rolling Code,
 * B = Humidity 
 * C = Battery status indicator on highest bit, 1=OK 0=LOW
 * D = Channel
 * E = Temperature (12 bit, 21.5 degrees is shown as decimal value 215, minus values have the high bit set and need to be subtracted from a base value of 4096)
 * F = CRC
 * 
 *
 * Sample:
 * 20;1F;DEBUG;Pulses=88;Pulses(uSec)=384,768,896,768,896,768,896,768,288,544,288,544,640,192,640,192,640,192,288,544,640,192,288,544,288,544,288,544,640,192,640,192,640,192,288,544,640,192,288,544,288,544,288,544,288,544,288,544,288,544,288,544,288,544,288,544,640,192,640,192,640,192,640,192,288,544,288,544,288,544,288,544,640,192,288,544,640,192,288,544,640,192,640,192,640,192,640,1952
 \*********************************************************************************************/
#define AURIOLV4_PLUGIN_ID 047
#define PLUGIN_DESC_047 "Auriol V4"
#define AURIOLV4_PULSECOUNT 88

#define AURIOLV4_MIDLO 128 / RAWSIGNAL_SAMPLE_RATE
#define AURIOLV4_MIDHI 672 / RAWSIGNAL_SAMPLE_RATE

#define AURIOLV4_PULSEMIN 224 / RAWSIGNAL_SAMPLE_RATE
#define AURIOLV4_PULSEMINMAX 352 / RAWSIGNAL_SAMPLE_RATE
#define AURIOLV4_PULSEMAXMIN 576 / RAWSIGNAL_SAMPLE_RATE
#define AURIOLV4_PULSEMAX 768 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_047
#include "../4_Display.h"
#include "../7_Utils.h"

boolean Plugin_047(byte function, char *string)
{
   if ((RawSignal.Number < AURIOLV4_PULSECOUNT - 4) || (RawSignal.Number > AURIOLV4_PULSECOUNT + 2))
      return false;

   unsigned long bitstream = 0L; // holds first 8x4=32 bits
   uint8_t checksum = 0;         // holds last  2x4=8 bits
   byte bitcounter = 0;          // counts number of received bits (converted from pulses)
   uint8_t checksumcalc = 0;
   byte rc = 0;
   byte bat = 0;
   byte bat0 = 0;
   int temperature = 0;
   byte humidity = 0;
   byte channel = 0;
   //==================================================================================
   // Get all 40 bits
   //==================================================================================
   for (byte x = 1; x < RawSignal.Number; x += 2)
   {
      if ((RawSignal.Pulses[x + 1] < AURIOLV4_MIDLO) || (RawSignal.Pulses[x + 1] > AURIOLV4_MIDHI))
      {
         if (bitcounter == 0) // Possible (4) Sync bits
            continue;

         if (bitcounter != 39) // in between pulse check
            return false;
      }

      if (RawSignal.Pulses[x] > AURIOLV4_PULSEMAXMIN)
      {
         if (RawSignal.Pulses[x] > AURIOLV4_PULSEMAX)
            return false;

         if (bitcounter < 32)
         {
            bitstream <<= 1;
            bitstream |= 0x1;
         }
         else
         {
            checksum <<= 1;
            checksum |= 0x1;
         }
      }
      else
      {
         if (RawSignal.Pulses[x] < AURIOLV4_PULSEMIN)
            return false;

         if (RawSignal.Pulses[x] > AURIOLV4_PULSEMINMAX)
            return false;

         if (bitcounter < 32)
            bitstream <<= 1;
         else
            checksum <<= 1;
      }
      bitcounter++; // only need to count the first 40 bits
      if (bitcounter > 39)
         break;
   }
   //==================================================================================
   // Perform a quick sanity check
   //==================================================================================
   if (bitcounter != 40)
      return false;
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
   // Source : https://github.com/merbanan/rtl_433/blob/master/src/devices/auriol_hg02832.c
   //
   // They tried to implement CRC-8 poly 0x31, but (accidentally?) reset the key every new byte.
   // (equivalent key stream is 7a 3d 86 43 b9 c4 62 31 repeated 4 times.)
   //
   for (byte c = 0; c < 4; c++)
      checksumcalc ^= ((bitstream >> (8 * c)) & 0xFF);

   if (checksum != crc8(&checksumcalc, 1, 0x31, 0x53))
      return false;
   //==================================================================================
   // now process the various sensor types
   //==================================================================================
   rc = (bitstream >> 24) & 0xFF;    // get rolling code
   bat = !((bitstream >> 15) & 0x1); // get battery strength indicator
   bat0 = (bitstream >> 14) & 0x1;   // get blank battery strength indicator
   if (bat0 != 0)
      return false; // blank bat must be 0

   channel = ((bitstream >> 12) & 0x3) + 1; // channel indicator

   temperature = (bitstream & 0xFFF); // get 12 temperature bits
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

   humidity = (bitstream >> 16) & 0xFF;
   if (humidity > 100)
      return false; // humidity out of range ( > 100)
   //==================================================================================
   // Output
   //==================================================================================
   display_Header();
   display_Name(PSTR("Auriol V4"));
   char c_ID[4];
   sprintf(c_ID, "%02X%02X", rc, channel);
   display_IDc(c_ID);
   display_TEMP(temperature);
   display_HUM(humidity, HUM_HEX);
   display_BAT(bat);
   display_Footer();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // PLUGIN_047
