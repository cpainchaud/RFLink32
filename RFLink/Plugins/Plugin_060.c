//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                      Plugin-060 AlarmSensor                                       ##
//#######################################################################################################
/*********************************************************************************************\
 * This protocol provides support for some Alarm sensors that are part of a Varel/Chubb/Ajax alarm
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
 * Technical data:
 * Devices send 25 pulses, first pulse is part of the start bit. Remaining bits are Manchester encoded, 24 bits
 *
 * The PCB contains a Holtec HT12E Encoder chip
 * The PCB has two switch blocks: SW1 with switches 1-8  (Device code?)
 *                                SW2 with switches 1-4  (House code?)
 *
 * Sample:
 * 20;74;DEBUG;Pulses=26;Pulses(uSec)=425,425,800,875,350,875,350,875,350,875,350,875,350,875,350,875,350,400,800,875,350,400,825,875,350;
 * 1001101010101010 01100110
 * 10000000 1010
 \*********************************************************************************************/
#define ALARMPIRV0_PULSECOUNT 26

#define ALARMPIRV0_PULSEMID 700 / RAWSIGNAL_SAMPLE_RATE
#define ALARMPIRV0_PULSEMAX 1000 / RAWSIGNAL_SAMPLE_RATE
#define ALARMPIRV0_PULSESHORT 550 / RAWSIGNAL_SAMPLE_RATE
#define ALARMPIRV0_PULSEMIN 250 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_060
#include "../4_Misc.h"

boolean Plugin_060(byte function, char *string)
{
   if (RawSignal.Number != ALARMPIRV0_PULSECOUNT)
      return false;
   unsigned long bitstream = 0L;
   if (RawSignal.Pulses[1] > ALARMPIRV0_PULSESHORT)
      return false; // First pulse is start bit and should be short!
   for (byte x = 2; x < ALARMPIRV0_PULSECOUNT; x = x + 2)
   {
      if (RawSignal.Pulses[x] > ALARMPIRV0_PULSEMID)
      { // long pulse 800-875 (700-1000 accepted)
         if (RawSignal.Pulses[x] > ALARMPIRV0_PULSEMAX)
            return false; // pulse too long
         if (RawSignal.Pulses[x + 1] > ALARMPIRV0_PULSEMID)
            return false; // invalid manchester code
         bitstream = bitstream << 1;
      }
      else
      { // short pulse 350-425 (250-550 accepted)
         if (RawSignal.Pulses[x] < ALARMPIRV0_PULSEMIN)
            return false; // pulse too short
         if (RawSignal.Pulses[x + 1] < ALARMPIRV0_PULSEMID)
            return false; // invalid manchester code
         bitstream = (bitstream << 1) | 0x1;
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
   // Output
   // ----------------------------------
   display_Header();
   display_Name(PSTR("X10"));
   display_IDn((bitstream & 0xFFFF), 4); // "%S%04x"
   display_SWITCH(1U);
   display_CMD(false, true); // #ALL , #ON
   display_Footer();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // PLUGIN_060
