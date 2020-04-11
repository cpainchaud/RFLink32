//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-40 Mebus                                            ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding Mebus weatherstation outdoor sensors
 * It concerns Mebus sensors that are not following the Cresta (Hideki) protocol
 * Also sold as Stacja Pogody WS-9941-M
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
 * Decodes signals from a Mebus Weatherstation outdoor unit, (58 pulses, 28 bits, 433 MHz).
 * Mebus Message Format: 
 * AAAA BBBB BBBB CCCC CCCC CCCC DDEF
 *
 * A = Checksum value    AAAA=((BBBB+BBBB+CCCC+CCCC+DDEF)-1)&0x0f
 * B = Changes after each reset, no change during normal operation. (Device 'Session' ID)
 * C = Temperature (21.5 degrees is shown as decimal value 215, minus values have the high bit set and need to be subtracted from a base value of 4096)
 * D = Channel number 1/2/3
 * E = Always 1
 * F = 0 when "normal" data transmit, 1 when "requested" data transmit (TX button press)
 *
 * 20;DE;DEBUG;Pulses=58;Pulses(uSec)=525,1800,350,1800,350,4275,350,1800,350,4275,350,4275,350,4275,350,1800,350,4250,350,4275,350,1800,350,4250,350,1800,350,1800,350,1800,350,1800,350,4275,350,4275,350,4250,350,1800,350,1800,350,1800,350,4275,350,4250,350,1800,350,4275,350,4275,350,4250,350;
 * 20;80;DEBUG;Pulses=58;Pulses(uSec)=450,4450,375,4450,375,4450,375,4450,375,1875,375,4450,375,4450,375,1875,375,1875,375,4425,375,4425,375,4425,375,1875,375,1875,375,1875,375,4425,375,1875,375,1875,375,1875,375,1875,375,4450,375,4450,375,1875,375,1875,375,1875,375,4450,375,4425,375,1875,375;
 * 20;81;Mebus;ID=6701;TEMP=010c;
 \*********************************************************************************************/
// ==================================================================================
// MEBUS bit packets
// 0000 1101 1001 0000 1100 1000 0111
// 0100 1101 1001 0000 1101 1100 0110
// 0100 1101 1001 0000 1100 1101 0110
// 1001 1101 1001 0000 1101 1100 1011
// 1011 1101 1001 0001 0000 1111 0110    27.1
// 0010 0110 1110 0000 0000 1001 0110    0.9
// 0011 0110 1110 1111 1000 0011 0110    -12.5 (1111 1000 0011=3971, 4096-3971=125
//                  |----------|----------> temperature 0 - 51.1
//                |-|---------------------> set when minus temperatures -51.2 - 0
// ==================================================================================
#define MEBUS_PULSECOUNT 58

#define MEBUS_MIDHI 550 / RAWSIGNAL_SAMPLE_RATE
#define MEBUS_PULSEMIN 1500 / RAWSIGNAL_SAMPLE_RATE
#define MEBUS_PULSEMINMAX 2100 / RAWSIGNAL_SAMPLE_RATE
#define MEBUS_PULSEMAXMIN 3400 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_040
#include "../4_Display.h"

boolean Plugin_040(byte function, char *string)
{
   if (RawSignal.Number != MEBUS_PULSECOUNT)
      return false;

   unsigned long bitstream = 0L;
   unsigned int temperature = 0;
   byte data[7];
   byte checksum = 0;
   byte rc = 0;
   byte channel = 0;
   //==================================================================================
   // Get all 28 bits
   //==================================================================================
   for (byte x = 2; x <= MEBUS_PULSECOUNT - 2; x += 2)
   {
      if (RawSignal.Pulses[x + 1] > MEBUS_MIDHI)
         return false; // make sure inbetween pulses are not too long

      bitstream <<= 1; // Always shift
      if (RawSignal.Pulses[x] > MEBUS_PULSEMAXMIN)
         bitstream |= 0x1;
      else
      {
         if (RawSignal.Pulses[x] > MEBUS_PULSEMINMAX)
            return false; // invalid pulse length
         if (RawSignal.Pulses[x] < MEBUS_PULSEMIN)
            return false; // invalid pulse length

         // bitstream |= 0x0;
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
   if ((SignalHash != SignalHashPrevious) || (RepeatingTimer + 150 < millis() && SignalCRC != bitstream) || (SignalCRC != bitstream))
      SignalCRC = bitstream; // not seen the RF packet recently
   else
      return true; // already seen the RF packet recently
   //==================================================================================
   // Prepare nibbles from bit stream
   //==================================================================================
   for (byte i = 0; i < 7; i++)
   {
      data[i] = ((bitstream >> (24 - (4 * i))) & 0xF);
      if (i > 0)
         checksum += data[i];
   }
   //==================================================================================
   // Perform checksum calculations
   //==================================================================================
   checksum = (checksum - 1) & 0xF;
   if (checksum != data[0])
      return false;
   //==================================================================================
   // Now process the various sensor types
   //==================================================================================
   rc = (data[1] << 4) | data[2];
   channel = (data[6]) >> 2;
   temperature = (data[3] << 8) | (data[4] << 4) | data[5];
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
   //==================================================================================
   display_Header();
   display_Name(PSTR("Mebus"));
   char c_ID[4];
   sprintf(c_ID, "%02x%02x", rc, channel);
   display_IDc(c_ID);
   display_TEMP(temperature);
   display_Footer();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // PLUGIN_040
