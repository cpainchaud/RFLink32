//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                       Plugin-45 Auriol                                            ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding the Auriol protocol (Z31743) and other devices following the same protocol (Rubicson?)
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
 * Decodes signals from a Auriol Weatherstation outdoor unit, (32/36 bits, 433 MHz).
 * Auriol Message Format: 
 * 1101 0110 1000 0000 1101 1111 1111 0000
 * AAAA AAAA BCCC DDDD DDDD DDDD EEEE FFFG 
 *
 * A = Rolling Code, no change during normal operation. (Device 'Session' ID) (Might also be 4 bits RC and 4 bits for channel number)
 * B = Battery status, 1=OK, 0=LOW
 * C = Always 000
 * D = Temperature (21.5 degrees is shown as decimal value 215, minus values have the high bit set and need to be subtracted from a base value of 4096)
 * E = Unknown
 * F = Unknown
 * G = sum of all bits xored together
 * 
 * Sample:
 * 20;34;DEBUG;Pulses=66;Pulses(uSec)=325,3725,325,1825,325,1825,325,1825,325,3700,325,3700,325,3700,325,3700,325,3700,325,1850,300,1825,325,1850,325,1825,325,1850,325,1825,300,1825,325,3725,300,3725,325,1825,325,1825,300,3725,300,1850,325,3725,300,1850,325,3725,300,3700,300,3725,300,1825,325,3700,325,3700,300,3700,325,1825,325;
 * 20;0A;DEBUG;Pulses=66;Pulses(uSec)=325,1850,300,1850,300,3700,300,1850,300,1850,300,1850,325,1850,300,1850,325,3700,325,1850,300,1850,300,1825,325,1850,300,1850,325,1825,300,1850,325,3725,300,3700,325,1825,300,1850,325,3700,300,3725,300,3725,300,1850,300,1850,300,3725,325,3700,300,1850,300,1825,325,1850,300,3700,300,1850,325;
 \*********************************************************************************************/
#define AURIOL_PULSECOUNT 66

#ifdef PLUGIN_045
#include "../4_Misc.h"
boolean Plugin_045(byte function, char *string)
{
   if (RawSignal.Number != AURIOL_PULSECOUNT)
      return false;
   unsigned long bitstream = 0L;
   unsigned int temperature = 0;
   byte rc = 0;
   byte bat = 0;
   byte checksumcalc = 0;
   //==================================================================================
   if (RawSignal.Number == AURIOL_PULSECOUNT)
   {
      for (int x = 2; x < AURIOL_PULSECOUNT; x += 2)
      {
         if (RawSignal.Pulses[x + 1] * RawSignal.Multiply > 550)
            return false; // inbetween pulses should not exceed a length of 550
         if (RawSignal.Pulses[x] * RawSignal.Multiply > 3000)
         { // long bit = 1
            bitstream = (bitstream << 1) | 0x1;
         }
         else
         {
            if (RawSignal.Pulses[x] * RawSignal.Multiply < 1600)
               return false; // pulse length too short to be valid?
            if (RawSignal.Pulses[x] * RawSignal.Multiply > 2200)
               return false;              // pulse length between 2000 - 3000 is invalid
            bitstream = (bitstream << 1); // short bit = 0
         }
      }
   }
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   if ((SignalHash != SignalHashPrevious) || (RepeatingTimer + 1000 < millis()))
   {
      // not seen the RF packet recently
      if (bitstream == 0)
         return false; // Perform a sanity check
   }
   else
   {
      // already seen the RF packet recently
      return true;
   }
   //==================================================================================
   for (int i = 1; i < 32; i++)
   { // Perform a checksum calculation to make sure the received packet is a valid Auriol packet
      checksumcalc = checksumcalc ^ ((bitstream >> i) & 0x01);
   }
   if (checksumcalc != (bitstream & 0x01))
      return false;
   rc = (bitstream >> 20) & 0x07; // get 3 bits to perform another sanity check
   if (rc != 0)
      return false; // selected bits should always be 000
   //==================================================================================
   bat = (bitstream >> 23) & 0x01;         // get battery strength indicator
   temperature = (bitstream >> 8) & 0xfff; // get 12 temperature bits
   rc = (bitstream >> 24) & 0xff;          // get rolling code
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
   //==================================================================================
   // Output
   // ----------------------------------
   Serial.print("20;");
   PrintHexByte(PKSequenceNumber++);
   Serial.print(F(";Auriol;ID=00")); // Label
   PrintHexByte(rc);
   // ----------------------------------
   sprintf(pbuffer, ";TEMP=%04x;", temperature);
   Serial.print(pbuffer);
   if (bat == 0)
   { // battery status
      Serial.print(F("BAT=LOW;"));
   }
   else
   {
      Serial.print(F("BAT=OK;"));
   }
   Serial.println();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // PLUGIN_045
