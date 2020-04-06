//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                      Plugin-061 AlarmSensor                                       ##
//#######################################################################################################
/*********************************************************************************************\
 * This protocol provides support for some chinese Alarm "gadgets" (Motion detectors and door/window contacts)
 * Note that these modules are reported as X10 switches to the Home Automation software so that they work correctly 
 *
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : StuntTeam 2015..2016
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Technical data:
 * Devices send 50 pulses. Manchester encoded, 25 bits
 *
 * PIR Motion detection unit:
 * PCB contains 2 chips: biss0001 & ev1527
 * BISS0001 = Micro Power PIR Motion Detector IC  
 * EV1527   = OTP Encoder, max. of 20 bits providing up to 1 million codes.
 *
 * Sample:
 * Pulses=50, Pulses(uSec)=900,225,200,850,225,850,825,225,225,850,225,850,825,225,225,850,200,850,225,850,825,225,225,850,825,225,225,850,825,225,825,225,825,225,825,225,825,225,825,225,825,225,225,850,225,850,825,225,225,
 * 100101100101100101011001100110101010101010010110 
 * 01101101 1101010000000110  6D D406
 
 * 20;9A;DEBUG;Pulses=50;Pulses(uSec)=475,925,400,950,1150,175,400,950,375,950,1125,200,1100,225,1100,250,1075,250,1075,275,1050,275,1050,275,1050,275,1050,275,275,1050,1050,275,300,1050,1050,275,300,1050,300,1050,1050,275,300,1050,275,1050,1050,275,275;
 * 010110010110101010101010101001100110010110010110
 * 00100111 1111110101001001  27 FD49
 * 11011000 0000001010110110  D8 02B6

 * 1975,275,900,250,225,975,250,975,250,975,225,975,900,250,900,250,900,250,250,950,225,975,900,250,225,950,225,975,250,950,225,975,900,250,900,250,900,250,250,950,900,250,250,950,225,950,925,250,250;
 * 101001010101101010010110010101011010100110010110
 * 001111000110111100010110 3C6F16
 * 110000111001000011101001 c390E9
 \*********************************************************************************************/
#define ALARMPIRV1_PULSECOUNT 50

#define ALARMPIRV1_PULSEMID 600 / RAWSIGNAL_SAMPLE_RATE
#define ALARMPIRV1_PULSEMAX 1300 / RAWSIGNAL_SAMPLE_RATE
#define ALARMPIRV1_PULSEMIN 150 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_061
#include "../4_Misc.h"

boolean Plugin_061(byte function, char *string)
{
   if (RawSignal.Number != ALARMPIRV1_PULSECOUNT)
      return false;
   if (RawSignal.Pulses[0] == 63)
      return false; // No need to test, packet for plugin 63
   unsigned long bitstream = 0L;
   unsigned long bitstream2 = 0L;
   //==================================================================================
   for (byte x = 2; x <= 48; x = x + 2)
   {
      if (RawSignal.Pulses[x] > ALARMPIRV1_PULSEMID)
      {
         if (RawSignal.Pulses[x] > ALARMPIRV1_PULSEMAX)
            return false; // pulse too long
         if (RawSignal.Pulses[x - 1] > ALARMPIRV1_PULSEMID)
            return false; // invalid pulse sequence 10/01
         bitstream = bitstream << 1;
      }
      else
      {
         if (RawSignal.Pulses[x] < ALARMPIRV1_PULSEMIN)
            return false; // pulse too short
         if (RawSignal.Pulses[x - 1] < ALARMPIRV1_PULSEMID)
            return false; // invalid pulse sequence 10/01
         bitstream = (bitstream << 1) | 0x1;
      }
   }
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================

   if ((SignalHash != SignalHashPrevious) || ((RepeatingTimer + 200) < millis()) || (SignalCRC != bitstream))
   {
      // not seen the RF packet recently
      if (bitstream == 0)
         return false;

      SignalCRC = bitstream;
   }
   else
   {
      // already seen the RF packet recently
      return true;
   }
   //==================================================================================
   bitstream2 = (bitstream) >> 16;
   if ((bitstream2) == 0xff)
   {
      if ((bitstream & 0xffff) == 0xff)
         return false;
   }
   //==================================================================================
   // Output
   // ----------------------------------
   display_Header();
   display_Name(PSTR("EV1527"));
   display_IDn(((bitstream >> 4) & 0x0FFFFF), 6); // "%S%06lx"
   display_SWITCH((bitstream & 0x0F));
   display_CMD(false, true); // #ALL , #ON
   display_Footer();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // Plugin_061
