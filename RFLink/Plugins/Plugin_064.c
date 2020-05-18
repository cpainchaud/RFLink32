//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                     Plugin-064: Atlantic PIR/ALARM                                ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin provides support for Atlantic and Visonic PIR/ALARM devices
 *
 * Author  : StormTeam 2020 - Cyril PAWELKO - Marc RIVES (aka Couin3)
 * Support : https://github.com/couin3/RFLink 
 * License : This code is free for use in any open source project when this header is included.
 *           Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Protocol information : 
 *      https://forum.arduino.cc/index.php?topic=289554.0
 *      Visonic Sensors Documentation Annex
 * 
 * Sample data:
 * 20;XX;DEBUG;Pulses=370;Pulses(uSec)=352,416,768,832,352,416,768,832,352,832,352,416,768,832,352,832,352,832,352,416,768,832,352,832,352,416,768,448,736,448,768,416,768,832,352,832,352,832,352,416,768,832,352,832,352,832,352,416,736,832,352,832,352,832,352,416,768,832,352,832,352,416,768,416,736,416,768,832,352,832,352,832,352,4160; 
 * Only tested with Visonic 433 Mhz (Europe) devices : MCT-302, K-980MCW
 * Not tested with real Atlantic devices. Feedback is welcome.
 \*********************************************************************************************/

#define ATLANTIC_PLUGIN_ID 064
#define PLUGIN_DESC_064 "Atlantic"
#define ATLANTIC_PULSECOUNT 74

#define ATLANTIC_PULSE_MID 600 / RAWSIGNAL_SAMPLE_RATE
#define ATLANTIC_PULSE_MIN 300 / RAWSIGNAL_SAMPLE_RATE
#define ATLANTIC_PULSE_MAX 900 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_064
#include "../4_Display.h"

boolean Plugin_064(byte function, char *string)
{
   if (RawSignal.Number != ATLANTIC_PULSECOUNT)
      return false;

   unsigned long bitstream = 0L; // Only the 32 first bits are processed

   //==================================================================================
   // Get first 32 bits : Sensor ID (24 bits) + 8 first bits of data
   // Bits are inverted !
   //==================================================================================

   for (byte x = 2; x <= 64; x += 2)
   {
      if (RawSignal.Pulses[x] > ATLANTIC_PULSE_MID)
      { // long pulse = 1
         if (RawSignal.Pulses[x] > ATLANTIC_PULSE_MAX)
            return false; // pulse too long
         if (RawSignal.Pulses[x + 1] > ATLANTIC_PULSE_MAX)
            return false; // invalid manchester code
         bitstream = (bitstream << 1) | 0x1;
      }
      else
      { // short pulse = 0
         if (RawSignal.Pulses[x] < ATLANTIC_PULSE_MIN)
            return false; // pulse too short
         if (RawSignal.Pulses[x + 1] < ATLANTIC_PULSE_MID)
            return false; // invalid manchester code
         bitstream = bitstream << 1;
      }
   }
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   if ((SignalHash != SignalHashPrevious) || ((RepeatingTimer) + 700 < millis()) || (SignalCRC != bitstream))
      SignalCRC = bitstream;
   else
      return true; // packet already seen
   //==================================================================================
   // Extract data
   //==================================================================================
   boolean alarm = (bitstream >> 6) & 0x01;
   unsigned long ID = (bitstream >> 8) & 0xFFFFFF;
   // ----------------------------------
   // Output
   // ----------------------------------
   display_Header();
   display_Name(PSTR("Atlantic"));
   display_IDn(ID, 6);
   display_SWITCH(1);
   display_CMD(CMD_Single, alarm ? CMD_On : CMD_Off);
   display_Footer();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // Plugin_064
