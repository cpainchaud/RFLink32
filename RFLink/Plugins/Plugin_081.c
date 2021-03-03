//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                     Plugin-081 Mertik Maxitrol                                    ##
//#######################################################################################################
/*********************************************************************************************\
 * This Plugin takes care of reception of Mertik Maxitrol / DRU for fireplaces
 * PCB markings: G6R H4TB / G6R 4HT.
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
 *
 * 0001100101101001011001101
 *   ----------------------- data bits (10=1 01=0)
 * --                        preamble, always 00?   
 * Shortened: (10=1 01=0)
 * 01100101101001011001101
 * 0 1 0 0 1 1 0 0 1 0 1 1 
 * 
 * 010011001011  
 *         ----   command => 4 bits
 * --------       address => 8 bits 
 *
 * command bits:
 * 0111 7 off 
 * 0011 3 on  
 * 1011 b up
 * 1101 d down
 * 1000 8 stop
 * 1010 a go up
 * 1100 c go down
 *
 * Sample RF packet: 
 * Pulses=26;Pulses(uSec)=475,300,325,700,325,700,325,700,325,700,725,300,725,300,725,300,725,300,725,300,325,700,725,300,725;
 \*********************************************************************************************/
#define MAXITROL1_PLUGIN_ID 081
#define PLUGIN_DESC_081 "Mertik"

#define MAXITROL1_PULSECOUNT 46

#define MAXITROL1_MID 550
#define MAXITROL1_PULSEMINMAX_D 550
#define MAXITROL1_PULSEMAX_D 900

#ifdef PLUGIN_081
#include "../4_Display.h"

boolean Plugin_081(byte function, const char *string)
{
   if (RawSignal.Number != MAXITROL1_PULSECOUNT)
      return false;
   
   const long MAXITROL1_PULSEMINMAX = MAXITROL1_PULSEMINMAX_D / RawSignal.Multiply;
   const long MAXITROL1_PULSEMAX = MAXITROL1_PULSEMAX_D / RawSignal.Multiply;

   unsigned int bitstream = 0L;
   byte address = 0;
   byte command = 0;
   byte status = 0;
   //==================================================================================
   // Perform a pre sanity check
   //==================================================================================
   if (RawSignal.Pulses[1] > MAXITROL1_PULSEMINMAX)
      return false;
   if (RawSignal.Pulses[2] > MAXITROL1_MID)
      return false;
   //==================================================================================
   // Get all 22 bits
   //==================================================================================
   for (int x = 3; x < MAXITROL1_PULSECOUNT + 1; x += 2)
   {
      bitstream <<= 1; // Always shift

      if (RawSignal.Pulses[x] < MAXITROL1_PULSEMINMAX)
      {
         if (RawSignal.Pulses[x + 1] < MAXITROL1_MID)
            return false;
         // bitstream |= 0x0; // 0
      }
      else
      {
         if (RawSignal.Pulses[x] > MAXITROL1_PULSEMAX)
            return false;
         if (RawSignal.Pulses[x + 1] > MAXITROL1_MID)
            return false;
         bitstream |= 0x1; // 1
      }
   }
   //==================================================================================
   // Perform a quick sanity check
   //==================================================================================
   if (bitstream == 0) // && (bitstream2 == 0)
      return false;
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   if ((SignalHash != SignalHashPrevious) || ((RepeatingTimer + 500) < millis()) || (SignalCRC != bitstream))
      SignalCRC = bitstream; // not seen the RF packet recently
   else
      return true; // already seen the RF packet recently
   //==================================================================================
   command = (bitstream & 0x0F); // get address from pulses
   address = ((bitstream >> 4) & 0xFF);

   switch (command)
   {
   case 0xB:
      status = 1; // up
      break;
   case 0xD:
      status = 2; // down
      break;
   case 0x7:
      status = 3; // off
      break;
   case 0x3:
      status = 4; // on
      break;
   case 0x8:
      status = 5; // stop
      break;
   case 0xA:
      status = 6; // go up
      break;
   case 0xC:
      status = 7; // go down
      break;
   default:
      return false;
   }
   //==================================================================================
   // Output
   //==================================================================================
   display_Header();
   display_Name(PSTR("Mertik"));
   display_IDn(address, 2);
   display_SWITCH(status);
   display_Name(PSTR(";CMD="));
   switch (status)
   {
   case 1:
      display_Name(PSTR("UP"));
      break;
   case 2:
      display_Name(PSTR("DOWN"));
      break;
   case 3:
      display_Name(PSTR("OFF"));
      break;
   case 4:
      display_Name(PSTR("ON"));
      break;
   case 5:
      display_Name(PSTR("STOP"));
      break;
   case 6:
      display_Name(PSTR("GOUP"));
      break;
   case 7:
      display_Name(PSTR("GODOWN"));
      break;
   }
   display_Footer();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // PLUGIN_081

#ifdef PLUGIN_TX_081
boolean  PluginTX_081(byte function, const char *string)
{
   boolean success = false;
   unsigned long bitstream = 0L;
   //10;MERTIK;64;UP;
   //0123456789012345
   if (strncasecmp(InputBuffer_Serial + 3, "MERTIK;", 7) == 0)
   { // KAKU Command eg.
      if (InputBuffer_Serial[12] != ';')
         return false;
      unsigned int bitstream2 = 0; // holds last 8 bits

      InputBuffer_Serial[8] = 0x30;
      InputBuffer_Serial[9] = 0x78;                // Get address from hexadecimal value
      InputBuffer_Serial[12] = 0x00;               // Get address from hexadecimal value
      bitstream = str2int(InputBuffer_Serial + 8); // Address (first 16 bits)

      if (strcasecmp(InputBuffer_Serial + 13, "stop") == 0)
         bitstream2 = 0x8;
      else if (strcasecmp(InputBuffer_Serial + 13, "on") == 0)
         bitstream2 = 0x3;
      else if (strcasecmp(InputBuffer_Serial + 13, "off") == 0)
         bitstream2 = 0x7;
      else if (strcasecmp(InputBuffer_Serial + 13, "up") == 0)
         bitstream2 = 0xB;
      else if (strcasecmp(InputBuffer_Serial + 13, "down") == 0)
         bitstream2 = 0xD;
      else if (strcasecmp(InputBuffer_Serial + 13, "go_up") == 0)
         bitstream2 = 0xA;
      else if (strcasecmp(InputBuffer_Serial + 13, "go_down") == 0)
         bitstream2 = 0xC;
      if (bitstream2 == 0)
         return false;
      //-----------------------------------------------
      RawSignal.Multiply = 50;
      RawSignal.Repeats = 10;
      RawSignal.Delay = 20;
      RawSignal.Pulses[1] = PLUGIN_081_RFLOW / RawSignal.Multiply;
      RawSignal.Pulses[2] = PLUGIN_081_RFLOW / RawSignal.Multiply;
      for (byte x = 18; x >= 3; x = x - 2)
      {
         if ((bitstream & 1) == 1)
         {
            RawSignal.Pulses[x] = PLUGIN_081_RFLOW / RawSignal.Multiply;
            RawSignal.Pulses[x - 1] = PLUGIN_081_RFHIGH / RawSignal.Multiply;
         }
         else
         {
            RawSignal.Pulses[x] = PLUGIN_081_RFHIGH / RawSignal.Multiply;
            RawSignal.Pulses[x - 1] = PLUGIN_081_RFLOW / RawSignal.Multiply;
         }
         bitstream = bitstream >> 1;
      }
      for (byte x = 26; x >= 19; x = x - 2)
      {
         if ((bitstream2 & 1) == 1)
         {
            RawSignal.Pulses[x] = PLUGIN_081_RFLOW / RawSignal.Multiply;
            RawSignal.Pulses[x - 1] = PLUGIN_081_RFHIGH / RawSignal.Multiply;
         }
         else
         {
            RawSignal.Pulses[x] = PLUGIN_081_RFHIGH / RawSignal.Multiply;
            RawSignal.Pulses[x - 1] = PLUGIN_081_RFLOW / RawSignal.Multiply;
         }
         bitstream2 = bitstream2 >> 1;
      }
      RawSignal.Pulses[27] = PLUGIN_081_RFSTART / RawSignal.Multiply;
      RawSignal.Number = 27;
      RawSendRF();
      success = true;
      //-----------------------------------------------
   }
   return success;
}
#endif // PLUGIN_081
