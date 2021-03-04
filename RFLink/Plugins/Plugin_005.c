//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                 Plugin-005: Intertek Eurodomest 972080                            ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of sending and receiving the Intertek Eurodomest 972080 protocol. 
 * 
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : StuntTeam 2015..2016
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 ***********************************************************************************************
 * Technical information:
 *
 * 0111 00011011 00011111 000 0
 * AAAA AAAAAAAA AAAAAAAA BBB C
 *   0000 00000011 10101010 101 1
 *   0000 00000011 10101010 111 0
 *
 * 0011 01101001 01101011 000 0  Eurodomest 1 on
 * 0011 01101001 01101011 000 1  Eurodomest 1 off
 * 0011 01101001 01101011 001 0  ED 2 on
 * 0011 01101001 01101011 001 1  ED 2 off
 * 0011 01101001 01101011 010 0  3 on 
 * 0011 01101001 01101011 010 1  3 off
 * 0011 01101001 01101011 100 0  4 on
 * 0011 01101001 01101011 100 1  4 off
 * 0011 01101001 01101011 110 1  all on 
 * 0011 01101001 01101011 111 0  all off
 *
 * 
 * A = ID (20 bits) 
 * B = UnitCode (3 bits)
 * C = switch code (ON/OFF) (1 bit)
 *
 * 20;2A;DEBUG;Pulses=50;Pulses(uSec)= 900,200,825,200,225,825,200,825,800,200,200,825,200,825,825,200,225,825,800,200,800,225,225,825,800,225,200,825,225,825,800,225,225,825,800,225,200,825,200,825,225,825,225,825,225,825,800,200,200;
 * 20;9D;DEBUG;Pulses=50;Pulses(uSec)=1250,200,750,175,200,750,200,750,750,200,200,750,200,750,750,200,200,750,750,200,750,200,200,750,750,200,200,750,200,750,750,200,200,750,750,200,200,750,200,750,750,200,750,200,750,200,750,200,200;
 \*********************************************************************************************/
#define EURODOMEST_PLUGIN_ID 005
#define PLUGIN_DESC_005 "Eurodomest"
#define EURODOMEST_PulseLength 50

#define EURODOMEST_PULSEMID_D 400
#define EURODOMEST_PULSEMIN_D 100
#define EURODOMEST_PULSEMAX_D 900

#ifdef PLUGIN_005
#include "../4_Display.h"

boolean Plugin_005(byte function, const char *string)
{
   if (RawSignal.Number != EURODOMEST_PulseLength)
      return false;
   if (RawSignal.Pulses[0] == 63)
      return false; // No need to test, packet for plugin 63

   const long EURODOMEST_PULSEMID = EURODOMEST_PULSEMID_D / RawSignal.Multiply;
   const long EURODOMEST_PULSEMIN = EURODOMEST_PULSEMIN_D / RawSignal.Multiply;
   const long EURODOMEST_PULSEMAX = EURODOMEST_PULSEMAX_D / RawSignal.Multiply;

   unsigned long bitstream = 0;
   byte unitcode = 0;
   byte command = 0;
   unsigned int address = 0;
   //==================================================================================
   // Perform a pre sanity check
   //==================================================================================
   if (RawSignal.Pulses[49] > EURODOMEST_PULSEMID)
      return false; // last pulse needs to be short, otherwise no Eurodomest protocol
   //==================================================================================
   // Get all 24 bits
   //==================================================================================
   for (int x = 2; x < EURODOMEST_PulseLength; x += 2)
   {
      bitstream <<= 1; //Always shift
      if (RawSignal.Pulses[x] > EURODOMEST_PULSEMID)
      { // long pulse
         if (RawSignal.Pulses[x - 1] > EURODOMEST_PULSEMID)
            return false; // not a 01 or 10 transmission
         if (RawSignal.Pulses[x] > EURODOMEST_PULSEMAX)
            return false; // make sure the long pulse is within range
         bitstream |= 0x1;
      }
      else
      { // short pulse
         if (RawSignal.Pulses[x] < EURODOMEST_PULSEMIN)
            return false; // pulse too short to be Eurodomest
         if (RawSignal.Pulses[x - 1] < EURODOMEST_PULSEMID)
            return false; // not a 01 or 10 transmission
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
   if ((SignalHash != SignalHashPrevious) || (RepeatingTimer + 500 < millis()) || (SignalCRC != bitstream))
      SignalCRC = bitstream; // not seen the RF packet recently
   else
      return true; // already seen the RF packet recently
   //==================================================================================
   // Perform more sanity checks to prevent false positives
   //==================================================================================
   address = ((bitstream >> 4) & 0xFFFFF);
   if (address == 0)
      return false; // Address would never be 0
   if (address == 0xFFFFF)
      return false; // Address would never be FFFFF
   // ----------------------------------
   unitcode = ((bitstream >> 1) & 0x7);
   command = (bitstream & 0x01);
   if (unitcode == 0x3)
      return false; // invalid button code?
   if (unitcode == 0x4)
      unitcode--;
   // if (unitcode == 5) return false;
   // Note: unitcode 5 is present on the PCB
   // and working but not used on any remotes.
   // if (unitcode > 0x7) // Impossible by code design
   // return false; // invalid button code?
   //==================================================================================
   // Output
   // ----------------------------------
   display_Header();
   display_Name(PSTR("Eurodomest"));
   display_IDn(address, 6);  //"%S%06lx"
   display_SWITCH(unitcode); // %02x
   if (unitcode > 4)
      display_CMD(CMD_All, (command != 0x0)); // #ALL #ON 
   else
      display_CMD(CMD_Single, (command != 0X1)); // #ALL #ON
   display_Footer();

   // ----------------------------------
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif //PLUGIN_005

#ifdef PLUGIN_TX_005
void Eurodomest_Send(unsigned long address);

boolean PluginTX_005(byte function, const char *string)
{
   //10;EURODOMEST;03696b;0;ON;
   //012345678901234567890123456
   boolean success = false;
   if (strncasecmp(InputBuffer_Serial + 3, "EURODOMEST;", 11) == 0)
   { // KAKU Command eg.
      unsigned long bitstream = 0L;
      if (InputBuffer_Serial[20] != ';')
         return success;
      if (InputBuffer_Serial[22] != ';')
         return success;

      InputBuffer_Serial[12] = 0x30;
      InputBuffer_Serial[13] = 0x78;
      InputBuffer_Serial[20] = 0x00;
      bitstream = str2int(InputBuffer_Serial + 12); // Address
      InputBuffer_Serial[22] = 0x00;
      byte temp = str2int(InputBuffer_Serial + 21); // Button number
      bitstream = (bitstream) << 4;
      if (temp == 1)
         bitstream = bitstream + 0x02; // 0010
      if (temp == 2)
         bitstream = bitstream + 0x04; // 0100
      if (temp == 3)
         bitstream = bitstream + 0x08; // 1000
      if (temp == 6)
         bitstream = bitstream + 0x0d; // 1101
      if (temp == 7)
         bitstream = bitstream + 0x0f; // 1111
      if (temp > 7)
      {
         return success;
      }
      byte command = 0;
      command = str2cmd(InputBuffer_Serial + 23);
      if (command == VALUE_OFF)
      {
         bitstream = bitstream | 1;
      }
      Eurodomest_Send(bitstream); // the full bitstream to send
      success = true;
   }
   return success;
}

void Eurodomest_Send(unsigned long address)
{
   int fpulse = 296; // Pulse witdh in microseconds
   int fretrans = 7; // Number of code retransmissions
   uint32_t fdatabit;
   uint32_t fdatamask = 0x800000;
   uint32_t fsendbuff;

   for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++)
   {
      fsendbuff = address;
      // Send command
      for (int i = 0; i < 24; i++)
      { // Eurodomest packet is 24 bits
         // read data bit
         fdatabit = fsendbuff & fdatamask; // Get most left bit
         fsendbuff = (fsendbuff << 1);     // Shift left

         if (fdatabit != fdatamask)
         { // Write 0
            digitalWrite(TX_DATA, HIGH);
            delayMicroseconds(fpulse * 3);
            digitalWrite(TX_DATA, LOW);
            delayMicroseconds(fpulse * 1);
         }
         else
         { // Write 1
            digitalWrite(TX_DATA, HIGH);
            delayMicroseconds(fpulse * 1);
            digitalWrite(TX_DATA, LOW);
            delayMicroseconds(fpulse * 3);
         }
      }
      digitalWrite(TX_DATA, HIGH);
      delayMicroseconds(fpulse * 1);
      digitalWrite(TX_DATA, LOW); // and lower the signal
      delayMicroseconds(fpulse * 32);
   }
}
#endif //PLUGIN_TX_005
