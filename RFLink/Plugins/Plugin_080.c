//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                               Plugin-080 Flamingo FA20RF Rookmelder                                ##
//#######################################################################################################
/*********************************************************************************************\
 * Dit protocol zorgt voor ontvangst van Flamingo FA20RF rookmelder
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
 * The Flamingo FA20RF/FA21RF Smokedetector contains both a RF receiver and transmitter. 
 * Every unit has a unique ID. The detector has a "learn" button to learn the ID of other units. 
 * That is what links the units to each other. After linking multiple units, they all have the same ID!
 * Normally, one unit is used as master and the code of the master is learned to the slaves
 *
 * Attention:  The smoke detector gives an alarm as long as the message is transmitted
 *
 * Sample:
 * 20;32;DEBUG;Pulses=52;Pulses(uSec)=2500,800,650,1250,650,1250,650,1250,650,1250,650,1250,650,1275,650,1250,650,2550,650,1275,650,2550,650,1250,650,1250,650,2550,650,2550,650,1275,650,2550,
 * 650,2550,650,1275,650,2550,650,2550,650,1275,650,1275,650,2550,650,1200,650;
 * 000000010100110110110010 = 014DB2
 * 20;0C;DEBUG;Pulses=52;Pulses(uSec)=2500,825,625,2575,625,1275,625,1300,625,2575,625,1275,625,2575,625,2575,625,2575,625,2575,625,2575,625,2575,625,1275,625,1275,625,1275,625,2575,625,2575,
 * 625,2575,625,1275,625,2575,625,2575,625,1300,625,1275,625,2575,625,1225,625;
 * 100101111110001110110010 = 97E3B2
 * 20;0D;FA20RF;ID=97e3b2;SMOKEALERT=ON;
 *  
 * False positive:
 * 20;52;DEBUG;Pulses=52;Pulses(uSec)=420,1860,330,3810,360,3960,360,1950,390,1920,360,3960,360,3960,360,3960,390,3960,390,3960,390,3960,390,1920,390,1920,390,1920,390,1890,480,1800,390,3930,390,1920,390,1920,420,1920,390,1920,420,1890,450,1860,420,1890,390,3930,390,6990;
 \*********************************************************************************************/
#define FA20_PLUGIN_ID 080
#define PLUGIN_DESC_080 "FA20RF"

#define FA20_PULSECOUNT 50

#define FA20_MIDHI 1000 / RAWSIGNAL_SAMPLE_RATE
#define FA20_PULSEMIN 1000 / RAWSIGNAL_SAMPLE_RATE
#define FA20_PULSEMINMAX 1500 / RAWSIGNAL_SAMPLE_RATE
#define FA20_PULSEMAXMIN 2000 / RAWSIGNAL_SAMPLE_RATE
#define FA20_PULSEMAX 2800 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_080
#include "../4_Display.h"

boolean Plugin_080(byte function, char *string)
{
   if (RawSignal.Number != FA20_PULSECOUNT)
      return false;
      
   unsigned long bitstream = 0L;
   //==================================================================================
   // Get all 24 bits
   //==================================================================================
   for (byte x = 2; x < FA20_PULSECOUNT; x += 2)
   {
      if (RawSignal.Pulses[x - 1] > FA20_MIDHI)
         return false; // every preceding pulse must be below 1000!

      bitstream <<= 1;

      if (RawSignal.Pulses[x] > FA20_PULSEMAXMIN)
      { // long pulse
         if (RawSignal.Pulses[x] > FA20_PULSEMAX)
            return false; // long pulse too long
         bitstream |= 0x1;
      }
      else
      {
         if (RawSignal.Pulses[x] > FA20_PULSEMINMAX)
            return false; // short pulse too long
         if (RawSignal.Pulses[x] < FA20_PULSEMIN)
            return false; // short pulse too short
         // bitstream |= 0x0;
      }
   }
   //==================================================================================
   // Perform a quick sanity check
   //==================================================================================
   if (bitstream == 0)
      return false;
   if (bitstream == 0xFFFFFF)
      return false;
   if ((bitstream & 0xFFFF) == 0xFFFF)
      return false;
   //==================================================================================
   // Output
   //==================================================================================
   display_Header();
   display_Name(PSTR("FA20RF"));
   display_IDn(bitstream, 6);
   display_SMOKEALERT(SMOKE_On);
   display_Footer();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;     // do not process the packet any further
   return true;
}
#endif // PLUGIN_080

#ifdef PLUGIN_TX_080
#define FA20RFSTART 3000 // 8000
#define FA20RFSPACE 675  //  800
#define FA20RFLOW 1250   // 1300
#define FA20RFHIGH 2550  // 2600

boolean PluginTX_080(byte function, char *string)
{
   boolean success = false;
   //10;FA20RF;67f570;1;ON;
   //012345678901234567890
   unsigned long bitstream = 0;
   if (strncasecmp(InputBuffer_Serial + 3, "FA20RF;", 7) == 0)
   { // KAKU Command eg.
      if (InputBuffer_Serial[18] != ';')
         return false;
      InputBuffer_Serial[8] = 0x30;
      InputBuffer_Serial[9] = 0x78;
      InputBuffer_Serial[16] = 0;
      bitstream = str2int(InputBuffer_Serial + 8);
      byte cmd = str2cmd(InputBuffer_Serial + 19); // ON/OFF
      if (cmd != VALUE_ON)
         return true; // pretend command was ok but we dont have to send anything..
      // ---------- SMOKEALERT SEND -----------
      RawSignal.Multiply = 50;
      RawSignal.Repeats = 10;
      RawSignal.Delay = 20;
      RawSignal.Pulses[1] = FA20RFSTART / RawSignal.Multiply;
      //RawSignal.Pulses[2]=FA20RFSPACE/RawSignal.Multiply;
      //RawSignal.Pulses[3]=FA20RFSPACE/RawSignal.Multiply;
      RawSignal.Pulses[2] = (FA20RFSPACE + 125) / RawSignal.Multiply;
      RawSignal.Pulses[3] = (FA20RFSPACE + 25) / RawSignal.Multiply;
      for (byte x = 49; x >= 3; x = x - 2)
      {
         RawSignal.Pulses[x] = FA20RFSPACE / RawSignal.Multiply;
         if ((bitstream & 1) == 1)
            RawSignal.Pulses[x + 1] = FA20RFHIGH / RawSignal.Multiply;
         else
            RawSignal.Pulses[x + 1] = FA20RFLOW / RawSignal.Multiply;
         bitstream = bitstream >> 1;
      }
      RawSignal.Pulses[51] = FA20RFSPACE / RawSignal.Multiply;
      RawSignal.Pulses[52] = 0;
      RawSignal.Number = 52;
      RawSendRF();
      RawSignal.Multiply = RAWSIGNAL_SAMPLE_RATE; // restore setting
      success = true;
   }
   return success;
}
#endif // PLUGIN_080
