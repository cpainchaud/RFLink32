//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                     Plugin-064: F007_TH                                           ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin provides support for F007_TH sensor devices, sold under several brands :
 * Renkforce FT007TH, Neoteck (for WS-07), Ambient Weather F007TH, Frogit F007TH
 * And probably variants F007T (thermometer only) on F007PF (pool thermometer)
 * 
 * Author  : StormTeam 2020 - Cyril PAWELKO - Marc RIVES (aka Couin3)
 * Support : https://github.com/couin3/RFLink 
 * License : This code is free for use in any open source project when this header is included.
 *           Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Protocol information : 
 *      https://github.com/AMcAnerney/Arduino-F007th-Sketches/blob/master/Final%20version%20(with%20CC3000%20wireless%20upload)
 *      https://domoticproject.com/wp-content/uploads/2018/03/Weather-Sensor-RF-Protocols.pdf
 * 
 * Data is manchester encoded and sent 3 times an row, in a packet of about 292-314 pulses.
 * Short pulses are appro 480us and long pulses 960us. 
 * Last pulses is approx 7000ms, but it is truncated by plugin 001, which keeps only 111 packets, a bit more than a third of max pulses.
 * 
 * Data is not verified against checksum
 * 
 * Sample data:
 * 20;XX;DEBUG;Pulses=291;Pulses(uSec)=448,480,448,480,448,480,448,480,480,480,480,480,480,480,448,480,448,480,480,960,960,960,960,960,480,448,480,448,960,960,960,480,448,960,960,960,480,448,480,448,480,448,480,448,480,448,480,448,960,960,480,448,960,960,480,448,480,448,960,960,480,448,960,960,480,448,480,448,480,448,960,960,480,448,480,448,960,960,480,448,960,480,448,960,480,448,480,448,480,448,480,448,480,448,480,448,480,448,480,448,480,448,960,480,480,480,448,480,448,480,448,480,480,480,480,480,480,480,448,480,448,480,480,960,960,960,960,960,480,448,480,448,960,960,960,480,448,960,960,960,480,448,480,448,480,448,480,448,480,448,480,448,960,960,480,448,960,960,480,448,480,448,960,960,480,448,960,960,480,448,480,448,480,448,960,960,480,448,480,448,960,960,480,448,960,480,448,960,480,448,480,448,480,448,480,448,480,448,480,448,480,448,512,448,480,448,960,480,448,480,448,480,480,480,448,480,448,480,448,480,448,480,480,480,480,480,448,960,960,960,960,960,480,448,480,448,960,960,960,480,448,960,960,960,480,448,480,448,480,448,480,448,480,448,480,448,960,960,480,448,960,960,480,448,480,448,960,960,480,448,960,960,480,448,480,448,480,448,960,960,480,448,480,448,960,960,480,448,960,480,480;
 * -> 20;06;F007_TH;ID=45a02;TEMP=00d1;HUM=68;BAT=OK;
 * 
 * 20;XX;DEBUG;Pulses=291;Pulses(uSec)=448,480,448,480,448,480,448,480,448,480,480,480,480,480,480,480,448,480,448,960,960,960,960,960,480,448,480,448,960,960,960,960,960,960,960,480,448,480,480,960,480,448,480,448,480,448,480,448,960,960,960,960,480,448,480,448,480,448,960,960,960,960,480,448,480,448,480,448,480,448,960,480,480,480,448,480,480,480,448,960,960,480,448,960,480,448,960,480,480,480,480,960,480,448,480,448,480,448,480,448,960,480,448,480,448,480,448,480,448,480,448,480,480,480,480,480,480,480,480,480,448,960,960,960,960,960,480,448,480,448,960,960,960,960,960,960,960,480,480,480,448,960,480,448,512,448,480,448,480,448,960,960,960,960,480,448,480,448,480,448,960,960,960,960,480,448,480,448,480,448,480,448,960,480,480,480,480,480,448,480,480,960,960,480,480,960,512,448,960,480,480,480,448,960,480,448,480,448,480,448,480,448,960,480,480,480,448,480,448,480,448,480,448,480,480,480,480,480,448,480,480,480,480,960,960,960,960,960,480,448,480,448,960,960,960,960,960,960,960,480,448,480,480,960,480,448,480,448,480,448,480,448,960,960,960,960,480,448,480,448,480,448,960,960,960,960,480,448,512,448,480,448,480,448,960,480,480,480,448,480,480,480,480,960,960,480,448,960,480,448,960;
 * 20;02;F007_TH;ID=455c1;TEMP=00bf;HUM=62;BAT=OK;
 \*********************************************************************************************/

#define F007_TH_PLUGIN_ID 036
#define PLUGIN_DESC_036 "F007_TH"
#define F007_TH_PULSECOUNT 111

#define F007_TH_PULSE_MID  650 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_036
#include "../4_Display.h"

boolean Plugin_036(byte function, char *string)
{
   if (RawSignal.Number != F007_TH_PULSECOUNT)
      return false;

   byte toggle = 1;
   byte pulsecounter = 2;                                            // Pulse counter
   int bitcounter = 0;                                               // Bits counter 
   unsigned long headerstream = 0L ;                                 // Must be 111 1111 1101 0100 0101 (9 1's + '01' + 0x45) -> 19 Bits
   unsigned long datastream = 0L;                                    // Only the 32 first bits are processed, not the checksum

   //==================================================================================
   // Get bits
   //==================================================================================

   while(bitcounter < 51)
   {
      bitcounter++;
      if (RawSignal.Pulses[pulsecounter] < F007_TH_PULSE_MID)
      {                                                              // Short pulse in second half-bit -> same value
         if (RawSignal.Pulses[pulsecounter+1] > F007_TH_PULSE_MID)   // First :Manchester check, the next pulse must alway be short
            return false;                                            // Invalid Manchester code
         pulsecounter += 2;                                          // Manchester OK -> don't toggle bit, move ahead to next half-bit
      }
      else
      {                                                              // Long pulse in second half-bit -> invert bit value
         toggle ^= 0x1;                                              // Invert bit value
         pulsecounter += 1;                                          // Move ahead to next pulse
      }
      if ((bitcounter <= 9 || bitcounter == 11) && toggle == 0)      // Quick sanity checks : Bits 1-9 and 11 must be 1
         return false;                                               // Invalid 0 in header (sometimes good data, but mised some bits)
      if (bitcounter == 19)                                          // Longer sanity check : Header must be 0x7FD45
      {
         if (headerstream != 0x3FEA2)
            return false;                                            // Invalid header
      }      
      if (bitcounter < 19)
      {                                                              // Store in header
         headerstream <<= 1;
         headerstream |= toggle;                                     
      }
      else
      {                                                              // Store data
         datastream <<= 1;
         datastream |= toggle;   
      }
   }

   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   if ((SignalHash != SignalHashPrevious) || ((RepeatingTimer) + 700 < millis()) || (SignalCRC != datastream))
      SignalCRC = datastream;
   else
      return true; // packet already seen

   //==================================================================================
   // Extract data
   //==================================================================================
   unsigned int  rolling = (datastream >> 24) & 0xFF;                // Used to build ID
   byte battery = ((datastream >> 23) & 0x01) ^0x1;                  // To be checked -> feedbacks are welcome
   byte channel = (datastream >> 20) & 0x07;                         // Used to build ID
   int temperature =  (datastream >> 8) & 0xFFF;                     // Temperature bits
   int realtemp = (temperature - 720)*5/9;                           // Temperature in 1/10th of celsius
   if (realtemp < 0)
      realtemp = -realtemp | 0x8000;                                 // Set high bit for negative temperatures
   byte hygro = (datastream) & 0xFF;                                 // Humidity
   unsigned long ID = (0x45 << 12) | (rolling << 4) | channel;       // ID 

   //==================================================================================
   // Output
   //==================================================================================
   display_Header();
   display_Name(PSTR("F007_TH"));
   display_IDn(ID, 4);
   display_TEMP(realtemp);
   display_HUM(hygro, HUM_HEX);
   display_BAT(battery);
   display_Footer();

   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // Plugin_036
