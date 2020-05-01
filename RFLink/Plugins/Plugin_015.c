//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                     Plugin-015: HomeEasy EU                                       ##
//#######################################################################################################
/*********************************************************************************************\
 * Dit protocol zorgt voor ontvangst en verzending HomeEasy EU zenders
 * die werken volgens de automatische codering (Ontvangers met leer-knop)
 *
 * LET OP: GEEN SUPPORT VOOR DIRECTE DIMWAARDES!!!
 *
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : StuntTeam 2015..2016
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Technische informatie:
 * Analyses Home Easy Messages and convert these into an eventcode
 * Only new EU devices with automatic code system are supported
 * Only  On / Off status is decoded, no DIM values
 * Only tested with Home Easy HE300WEU transmitter, doorsensor and PIR sensor
 * Home Easy message structure, by analyzing bitpatterns so far ...
 * AAAAAAAAAAA BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB CCCC DD EE FFFFFF G
 * 11000111100 10111100011101110010001111100011 1100 10 11 000111 1  HE301EU ON 
 * 11000111100 10111100011101110010001111100011 1100 01 11 000111 1  HE301EU OFF
 * 11000111100                                  1001 01 11 001011    HE842/844 ON
 * 11000111100 01111010011100110010001011000111 1000 11 11 001011 0000000   HE842/844 OFF
 *                                              1000 10 11 000111    HE844 ALLON;
 *                                              1000 01 11 000111    HE844 ALLOFF;
 * 11000111100 01111000111101100110010011000111 1000 11 11 000111 0000000
 *  
 * A = Startbits/Preamble, 
 * B = Address, 32 bits
 * C = Unknown, Possibly: Device type 
 * D = Command, 1 bit only?
 * E = Group indicator
 * F = Channel  0-15
 * G = Stopbit
 *
 * SAMPLE:
 * Pulses=116;Pulses(uSec)=200,1175,125,1175,125,200,150,200,125,200,150,1175,150,1175,150,1175,150,1175,125,200,150,200,150,200,125,1175,150,1175,150,1175,125,1175,150,200,125,200,150,1175,125,1175,150,200,125,1175,125,1175,150,200,150,200,150,1175,150,200,150,1175,150,200,150,1175,150,200,150,200,125,1175,150,200,125,1175,150,1175,125,1175,150,200,125,200,125,200,150,200,125,1175,150,1175,150,1175,150,200,150,200,125,200,150,1175,150,1175,150,1175,150,1175,125,200,150,200,125,1175,125,200,125,1175,150,1150,125;
 * HE preamble: 11000111100 (63C) Address: 1111001101100101010010111000011 (79B2A5C3) Stopbits: 0 (0) Commands: 10001111001011 Command: 0 Channel: 1011 Group: 1
 * 20;04;HomeEasy;ID=7900b200;SWITCH=0b;CMD=ALLOFF;
 *
 * Preamble 200,1175,125,1175,125,200,150,200,125,200,150,1175,150,1175,150,1175,150,1175,125,200,150,200,
 * Address  150,200,125,1175,150,1175,150,1175,125,1175,150,200,125,200,150,1175,125,1175,150,200,125,1175,125,1175,150,200,150,200,150,1175,150,200,150,1175,150,200,150,1175,150,200,150,200,125,1175,150,200,125,1175,150,1175,125,1175,150,200,125,200,125,200,150,200,125,1175,150,1175,
 * Command  150,1175,150,200,150,200,125,200,150,1175,150,1175,150,1175,150,1175,125,200,150,200,125,1175,125,200,125,1175,150,1150,  - 125;
 \*********************************************************************************************/
#define HomeEasy_PLUGIN_ID 015
#define PLUGIN_DESC_015 "HomeEasy"
#define HomeEasy_PulseLength 116

#define HomeEasy_PULSEMID 500 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_015
#include "../4_Display.h"

boolean Plugin_015(byte function, char *string)
{
   if (RawSignal.Number != HomeEasy_PulseLength)
      return false;
   unsigned long preamble = 0L;
   unsigned long address = 0L;
   unsigned long bitstream = 0L;
   byte rfbit = 0;
   byte command = 0;
   byte group = 0;
   byte channel = 0;
   byte type = 0;
   byte temp = 0;
   RawSignal.Pulses[0] = 0; // undo any Home Easy to Kaku blocking that might be active
   //==================================================================================
   // Get all 58? bits
   //==================================================================================
   //==================================================================================
   // convert pulses into bit sections (preamble, address, bitstream)
   //==================================================================================
   for (byte x = 1; x <= HomeEasy_PulseLength; x = x + 2)
   {
      if ((RawSignal.Pulses[x] < HomeEasy_PULSEMID) && (RawSignal.Pulses[x + 1] > HomeEasy_PULSEMID))
         rfbit = 1;
      else
         rfbit = 0;

      if (x <= 22)
         preamble = (preamble << 1) | rfbit; // 11 bits preamble
      if ((x >= 23) && (x <= 86))
         address = (address << 1) | rfbit; // 32 bits address
      if ((x >= 87) && (x <= 114))
         bitstream = (bitstream << 1) | rfbit; // 15 remaining bits
   }
   //==================================================================================
   // Perform a quick sanity check
   //==================================================================================
   // To prevent false positives make sure the preamble is correct, it should always be 0x63c
   // We compare only 10 bits to compensate for the first bit being seen incorrectly by some receiver modules
   if ((preamble & 0x3FF) != 0x23C)
      return false; // comparing 10 bits is enough to make sure the packet is valid
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   if (SignalHash != SignalHashPrevious || (RepeatingTimer < millis() + 500) || SignalCRC != bitstream)
      SignalCRC = bitstream; // not seen the RF packet recently
   else
      return true; // already seen the RF packet recently
   //==================================================================================
   type = ((bitstream >> 12) & 0x3); // 11b for HE301
   channel = (bitstream)&0x3F;
   if (type == 3)
   {                                      // HE301
      command = ((bitstream >> 8) & 0x1); // 0=on 1=off (both group and single device)
      group = ((bitstream >> 7) & 0x1);   // 1=group
   }
   else
   {                                   // HE800  21c7 = off 22c7=on
      temp = ((bitstream >> 8) & 0x7); // 1=group
      if (temp < 3)
         group = 1;
      command = ((bitstream >> 9) & 0x1); // 0=off 1=on
      if (group == 1)
         command = (~command) & 1; // reverse bit for group: 1=group off 0=group on
   }
   //==================================================================================
   // Output
   //==================================================================================
   display_Header();
   display_Name(PSTR("HomeEasy"));
   display_IDn(address, 8);                        //"%S%08lx"
   display_SWITCH(channel);                        // "%02x"
   display_CMD((group & B01), (!(command & B01))); // #All #ON
   display_Footer();
   // ----------------------------------
   RawSignal.Repeats = true;
   RawSignal.Number = 0;
   return true;
}
#endif // PLUGIN_015

#ifdef PLUGIN_TX_015
void HomeEasyEU_Send(unsigned long address, unsigned long command);

boolean PluginTX_015(byte function, char *string)
{
   boolean success = false;
   //10;HomeEasy;7900b200;b;ON;
   //10;HomeEasy;d900ba00;23;OFF;
   //10;HomeEasy;79b2a5c3;b;ON;
   //10;HomeEasy;7a7322c7;b;ON;
   //01234567890123456789012345
   if (strncasecmp(InputBuffer_Serial + 3, "HOMEEASY;", 9) == 0)
   { // KAKU Command eg.
      if (InputBuffer_Serial[20] != ';')
         return false;
      unsigned long bitstream = 0L;
      unsigned long commandcode = 0L;
      byte cmd = 0;
      byte group = 0;
      // ------------------------------
      InputBuffer_Serial[10] = 0x30;
      InputBuffer_Serial[11] = 0x78;
      InputBuffer_Serial[20] = 0x00;
      bitstream = str2int(InputBuffer_Serial + 10); // Get Address from hexadecimal value
      // ------------------------------
      InputBuffer_Serial[19] = 0x30;
      InputBuffer_Serial[20] = 0x78;                  // Get home from hexadecimal value
      commandcode = str2int(InputBuffer_Serial + 19); // Get Button number
      // ------------------------------
      if (InputBuffer_Serial[23] == ';')
      { // Get command
         cmd = str2cmd(InputBuffer_Serial + 24);
      }
      else
      {
         cmd = str2cmd(InputBuffer_Serial + 23);
      }
      if (cmd == VALUE_OFF)
         cmd = 0; // off
      if (cmd == VALUE_ON)
         cmd = 1; // on
      if (cmd == VALUE_ALLON)
      {
         cmd = 1;
         group = 1;
      } // allon
      if (cmd == VALUE_ALLOFF)
      {
         cmd = 0;
         group = 1;
      } // alloff
      // ------------------------------
      commandcode = commandcode & 0x3f; // get button number
      if (group == 1)
      {                                      // HE8xx: 21xx/22xx HE3xx: 31xx/32xx (off/on) (HE3xx code works for HE8xx)
         commandcode = commandcode | 0x30C0; // group
         if (cmd == 1)
         {
            commandcode = (commandcode & 0xfdff) | 0x200; // group on > 32cx
         }
         else
         {
            commandcode = commandcode | 0x100; // group off > 31cx
         }
      }
      else
      {                                      // HE8xx: 23cx/25cx HE3xx: 2D4x/2E4x (off/on) (HE3 code works for HE8xx)
         commandcode = commandcode | 0x2040; // non-group
         if (cmd == 1)
         {
            commandcode = (commandcode & 0xfdff) | 0xe00; // On > 2ECx
         }
         else
         {
            commandcode = commandcode | 0xd00; // Off > 2DCx
         }
      }
      //-----------------------------------------------
      HomeEasyEU_Send(bitstream, commandcode);
      success = true;
   }
   return success;
}

void HomeEasyEU_Send(unsigned long address, unsigned long command)
{
   int fpulse = 275; // Pulse witdh in microseconds
   int fretrans = 5; // Number of code retransmissions
   uint32_t fdatabit;
   uint32_t fdatamask = 0x80000000;
   uint32_t fsendbuff;

   for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++)
   {
      // -------------- Send Home Easy preamble (0x63c) - 11 bits
      fsendbuff = 0x63c;
      fdatamask = 0x400;
      for (int i = 0; i < 11; i++)
      { // Preamble
         // read data bit
         fdatabit = fsendbuff & fdatamask; // Get most left bit
         fsendbuff = (fsendbuff << 1);     // Shift left
         if (fdatabit != fdatamask)
         { // Write 0
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 1);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 1);
         }
         else
         { // Write 1
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 1);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 5);
         }
      }
      // -------------- Send Home Easy device Address
      fsendbuff = address;
      fdatamask = 0x80000000;
      // Send Address - 32 bits
      for (int i = 0; i < 32; i++)
      { //28;i++){
         // read data bit
         fdatabit = fsendbuff & fdatamask; // Get most left bit
         fsendbuff = (fsendbuff << 1);     // Shift left
         if (fdatabit != fdatamask)
         { // Write 0
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 1);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 1);
         }
         else
         { // Write 1
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 1);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 5);
         }
      }
      // -------------- Send Home Easy command bits - 14 bits
      fsendbuff = command; // 0xFF;
      fdatamask = 0x2000;
      for (int i = 0; i < 14; i++)
      {
         // read data bit
         fdatabit = fsendbuff & fdatamask; // Get most left bit
         fsendbuff = (fsendbuff << 1);     // Shift left
         if (fdatabit != fdatamask)
         { // Write 0
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 1);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 1);
         }
         else
         { // Write 1
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 1);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 5);
         }
      }
      // -------------- Send stop
      digitalWrite(PIN_RF_TX_DATA, HIGH);
      delayMicroseconds(fpulse * 1);
      digitalWrite(PIN_RF_TX_DATA, LOW); // and lower the signal
      delayMicroseconds(fpulse * 26);
   }
}
#endif // PLUGIN_TX_015
