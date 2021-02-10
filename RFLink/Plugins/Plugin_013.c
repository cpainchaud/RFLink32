//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                       Plugin-013: Powerfix RCB-i 3600                             ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of sending and receiving the Powerfix RCB-i 3600 protocol
 * Works with: Powerfix RCB-i 3600 - 4 power outlets and a remote, Quigg GT7000
 * 
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : StuntTeam 2015..2016
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 ***********************************************************************************************
 * Technical information:
 * Partially based on http://wiki.pilight.org/doku.php/quigg_switch_gt_7000_v7_0
 *
 * The Powerfix/Chacon RF packets are 42 pulses long resulting in 20 bits data packets
 *
 * 0000 1000 0000 000 1 0 0 0 1 
 * AAAA AAAA AAAA BBC D E F G H
 *
 * A = 12 bits address (Chacon:0000 0000 0000 Powerfix:0000 1000 0000)
 * B = 2 bits Unit code (bits reversed)
 * C = Group Command (to all devices)
 * D = State (ON/OFF/DIM UP/DIM DOWN)
 * E = Dim command (1=dim/bright command)
 * F = always 0
 * G = Parity bit calculated over command bits (BCDEFH)
 * H = unknown, copy of D (state)
 *
 * -------------------------------------------
 * Powerfix:
 * 0000 1000 0000 000 1 0 0 0 1     1 on 
 * 0000 1000 0000 000 0 0 0 0 0     1 off 
 * 0000 1000 0000 100 1 0 0 1 1     2 on
 * 0000 1000 0000 100 0 0 0 1 0     2 off
 * 0000 1000 0000 010 1 0 0 0 0     3 on
 * 0000 1000 0000 010 0 0 0 0 1     3 off
 * 0000 1000 0000 110 1 0 0 1 0     4 on
 * 0000 1000 0000 110 0 0 0 1 1     4 off
 * 0000 1000 0000 111 0 0 0 0 1     all off
 * 0000 1000 0000 111 1 0 0 0 0     all on
 * 0000 1000 0000 111 1 1 0 1 0     dim
 * 0000 1000 0000 111 0 1 0 1 1     bright
 * -------------------------------------------
 * Chacon:
 * 0000 0000 0000 000 1 0 0 0 1		1	ON
 * 0000 0000 0000 000 0 0 0 0 0		1	OFF
 * 0000 0000 0000 100 1 0 0 1 1		2	ON
 * 0000 0000 0000 100 0 0 0 1 0		2	OFF
 * 0000 0000 0000 010 1 0 0 0 0		3	ON
 * 0000 0000 0000 010 0 0 0 0 1		3	OFF
 * 0000 0000 0000 110 1 0 0 1 0		4	ON
 * 0000 0000 0000 110 0 0 0 1 1		4	OFF
 * 0000 0000 0000 111 1 0 0 0 0		All	ON
 * 0000 0000 0000 111 0 0 0 0 1		All	OFF
 * 0000 0000 0000 111 1 1 0 1 0		All DIM
 * 0000 0000 0000 111 0 1 0 1 1		All Bright
 * -------------------------------------------
 * Sample Powerfix:
 * 20;10;DEBUG;Pulses=42;Pulses(uSec)=630,570,1230,540,1230,540,1230,540,1230,570,1230,540,1230,540,1230,540,1230,570,1230,540,1230,540,1230,540,1230,570,1200,540,1200,570,1170,1230,540,570,1200,540,1200,540,1170,1230,540,6990;
 * 
 * BUTTON: DIM
 * 20;05;DEBUG;Pulses=42;Pulses(uSec)=600,600,1250,625,1250,625,1225,625,1250,1275,575,600,1250,625,1225,625,1250,625,1225,625,1225,625,1225,625,1225,1300,575,1300,575,1300,575,1300,600,1300,550,625,1225,1300,550,625,1175;
 * 00101010110010101010101011010101010011001
 * BUTTON: BRIGHT
 * 20;12;DEBUG;Pulses=42;Pulses(uSec)=600,600,1250,600,1250,600,1250,600,1250,1275,575,625,1225,625,1225,625,1250,625,1225,625,1225,625,1225,625,1250,1300,550,1300,575,1300,550,625,1250,1300,550,625,1225,1300,575,1300,525;
 * 00101010110010101010101011010100110011010 
 * BUTTON: ALL OFF
 * 20;18;DEBUG;Pulses=42;Pulses(uSec)=600,600,1250,625,1225,625,1225,625,1250,1275,575,625,1225,625,1225,625,1250,625,1225,625,1225,625,1225,625,1225,1300,575,1300,550,1300,550,625,1250,650,1200,625,1225,650,1225,1300,500;
 * 00101010110010101010101011010100101010110
 * BUTTON: ALL ON
 * 20;15;DEBUG;Pulses=42;Pulses(uSec)=600,600,1250,600,1250,625,1225,625,1225,1275,575,625,1225,625,1225,625,1250,625,1225,650,1200,625,1225,625,1225,1300,550,1300,575,1300,550,1300,575,650,1225,625,1225,625,1225,625,1175;
 * 00101010110010101010101011010101001010101
 * BUTTON: 1 ON
 * 20;04;DEBUG;Pulses=42;Pulses(uSec)=600,600,1250,625,1225,625,1225,625,1225,1300,575,625,1225,625,1225,625,1250,625,1225,625,1225,625,1225,625,1225,625,1225,625,1225,625,1225,1300,575,625,1225,625,1225,625,1225,1300,500;
 * 0101010110010101010101010101011001010110
 * BUTTON: 1 OFF
 * 20;07;DEBUG;Pulses=42;Pulses(uSec)=600, 600,1250,600,1250,625,1250,625,1225,1275,575,625,1225,600,1250,625,1275,625,1225,625,1225,625,1225,625,1225,625,1225,625,1225,625,1225,625,1250,625,1225,625,1225,625,1225,625,1175;
 * 0101010110010101010101010101010101010101
 * BUTTON: 2 ON
 * 20;0A;DEBUG;Pulses=42;Pulses(uSec)=575,600,1250,625,1225,625,1225,625,1225,1300,550,625,1225,625,1225,625,1275,625,1225,625,1225,625,1225,625,1225,1300,550,625,1225,625,1225,1300,575,625,1225,625,1225,1300,575,1300,500;
 * 0101010110010101010101011001011001011010
 * BUTTON: 2 OFF
 * 20;0D;DEBUG;Pulses=42;Pulses(uSec)=600,600,1250,625,1225,625,1225,625,1250,1300,550,625,1225,625,1225,625,1250,625,1225,625,1225,625,1225,625,1250,1300,575,625,1225,625,
 1225,625,1250,625,1225,650,1225,1300,550,625,1175;
 * 0101010110010101010101011001010101011001
 \*********************************************************************************************/
#define POWERFIX_PLUGIN_ID 013
#define PLUGIN_DESC_013 "Powerfix"
#define POWERFIX_PulseLength 42

#define POWEFIX_PULSEMID 900 / RAWSIGNAL_SAMPLE_RATE
#define POWEFIX_PULSEMIN 450 / RAWSIGNAL_SAMPLE_RATE
#define POWEFIX_PULSEMAX 1400 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_013
#include "../4_Display.h"

boolean Plugin_013(byte function, const char *string)
{
   if (RawSignal.Number != POWERFIX_PulseLength)
      return false;
   unsigned long bitstream = 0L;
   unsigned int address = 0;
   byte unitcode = 0;
   byte button = 0;
   byte command = 0;
   byte parity = 0;
   byte bitcount = 0;
   //==================================================================================
   // Perform a pre sanity check
   //==================================================================================
   if (RawSignal.Pulses[1] > POWEFIX_PULSEMID)
      return false; // start pulse must be short
   //==================================================================================
   // Get all 20 bits
   //==================================================================================
   for (byte x = 2; x < POWERFIX_PulseLength - 1; x += 2)
   {
      bitstream <<= 1; // Always shift
      if (RawSignal.Pulses[x] > POWEFIX_PULSEMID)
      {
         if (RawSignal.Pulses[x] > POWEFIX_PULSEMAX)
            return false; // Long pulse too long
         if (RawSignal.Pulses[x + 1] > POWEFIX_PULSEMID)
            return false; // pulse sequence check 01/10
         bitstream |= 0x1;
         if (bitcount > 11)
            parity = parity ^ 1;
      }
      else
      {
         if (RawSignal.Pulses[x] < POWEFIX_PULSEMIN)
            return false; // Short pulse too short
         if (RawSignal.Pulses[x + 1] < POWEFIX_PULSEMID)
            return false; // pulse sequence check 01/10
         // bitstream |= 0x0;
      }
      bitcount++;
   }
   //==================================================================================
   // Perform a quick sanity check
   //==================================================================================
   if (parity != 0)
      return false; // Parity check
   if (((bitstream)&0x4) == 4)
      return false; // Tested bit should always be zero
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   if (SignalHash != SignalHashPrevious || (RepeatingTimer < millis() + 1500) || SignalCRC != bitstream)
      SignalCRC = bitstream; // not seen the RF packet recently
   else
      return true; // already seen the RF packet recently
   //==================================================================================
   // Sort data
   address = ((bitstream) >> 8); // 12 bits address
   unitcode = ((bitstream >> 6) & 0x03);
   if (unitcode == 2)
      button = 1; // bits are simply reversed
   if (unitcode == 1)
      button = 2;
   if (unitcode == 3)
      button = 3;

   parity = ((bitstream)&0x3f);      // re-use parity variable
   command = ((parity) >> 4) & 0x01; // On/Off command
   if (((parity)&0x08) == 0x08)
   { // dim command
      command = command + 2;
   }
   else
   {
      if (((parity)&0x20) == 0x20)
      { // group command
         command = command + 4;
      }
   }
   //==================================================================================
   // Output
   //==================================================================================
   display_Header();
   display_Name(PSTR("Powerfix"));
   display_IDn(address, 4); //"%S%04x"
   display_SWITCH(button);  // "%02x"
   switch (command)
   {
   case 0x00:
   case 0x01:
   case 0x04:
   case 0x05:
      display_CMD(((command >> 3) & B01), (command & B01)); // #All #ON
      break;
   case 0x02:
      display_CMD(CMD_Single, CMD_Bright); // #All #ON
      break;
   case 0x03:
      display_CMD(CMD_Single, CMD_Dim); // #All #ON
      break;
   }
   display_Footer();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}

#endif //PLUGIN_013

#ifdef PLUGIN_TX_013
void Powerfix_Send(unsigned long bitstream);

boolean  PluginTX_013(byte function, const char *string)
{
   boolean success = false;
   //10;POWERFIX;000080;0;ON;
   //012345678901234567890123
   if (strncasecmp(InputBuffer_Serial + 3, "POWERFIX;", 9) == 0)
   {
      if (InputBuffer_Serial[18] != ';')
         return success;

      InputBuffer_Serial[10] = 0x30;
      InputBuffer_Serial[11] = 0x78;
      InputBuffer_Serial[18] = 0x00; // Get address from hexadecimal value

      unsigned long bitstream = 0L; // Main placeholder
      byte command = 0;
      byte c;
      // -------------------------------
      bitstream = str2int(InputBuffer_Serial + 10); // Address, first 12 bits of the 20 bits in total
      bitstream = (bitstream) << 8;                 // shift left so that we can add the 8 command bits
      // -------------------------------
      byte temp = str2int(InputBuffer_Serial + 19); // button/unit number (0..3)
      if (temp == 1)
         command = 0x82;
      if (temp == 2)
         command = 0x40;
      if (temp == 3)
         command = 0xc2;
      // -------------------------------
      c = 0;
      c = str2cmd(InputBuffer_Serial + 21); // ON/OFF command
      if (c == VALUE_ON)
      {
         command = command | 0x10; // turn "on" bit for on command
      }
      else
      {
         if (c == VALUE_ALLOFF)
         { // set "all off" bits
            command = 0xe0;
         }
         else if (c == VALUE_ALLON)
         {
            command = 0xf0; // set "all on" bits
         }
      }
      // not supported yet..
      // dim: command=0xfa
      // bright: command=0xea;
      // -------------------------------
      bitstream = bitstream + command;
      // -------------------------------
      Powerfix_Send(bitstream); // bitstream to send
      success = true;
   }
   return success;
}

#define PLUGIN_013_RFLOW 650
#define PLUGIN_013_RFHIGH 1300

void Powerfix_Send(unsigned long bitstream)
{
   RawSignal.Repeats = 7; // Number of RF packet retransmits
   RawSignal.Delay = 20;  // Delay between RF packets
   RawSignal.Number = 42; // Length

   uint32_t fdatabit;
   uint32_t fdatamask = 0x80000;
   byte parity = 1; // to calculate the parity bit
   // -------------------------------
   RawSignal.Pulses[1] = PLUGIN_013_RFLOW / RawSignal.Multiply; // start pulse
   for (byte i = 2; i < 40; i = i + 2)
   {                                    // address and command bits
      fdatabit = bitstream & fdatamask; // Get most left bit
      bitstream = (bitstream << 1);     // Shift left

      if (fdatabit != fdatamask)
      { // Write 0
         RawSignal.Pulses[i] = PLUGIN_013_RFLOW / RawSignal.Multiply;
         RawSignal.Pulses[i + 1] = PLUGIN_013_RFHIGH / RawSignal.Multiply;
      }
      else
      { // Write 1
         parity = parity ^ 1;
         RawSignal.Pulses[i] = PLUGIN_013_RFHIGH / RawSignal.Multiply;
         RawSignal.Pulses[i + 1] = PLUGIN_013_RFLOW / RawSignal.Multiply;
      }
   }
   // parity
   if (parity == 0)
   { // Write 0
      RawSignal.Pulses[40] = PLUGIN_013_RFLOW / RawSignal.Multiply;
      RawSignal.Pulses[41] = PLUGIN_013_RFHIGH / RawSignal.Multiply;
   }
   else
   { // Write 1
      RawSignal.Pulses[40] = PLUGIN_013_RFHIGH / RawSignal.Multiply;
      RawSignal.Pulses[41] = PLUGIN_013_RFLOW / RawSignal.Multiply;
   }
   RawSendRF();
}
#endif // PLUGIN_TX_013
