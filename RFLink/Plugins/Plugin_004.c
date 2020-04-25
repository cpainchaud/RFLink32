//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                       Plugin-04: NewKAKU                                          ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of receiving from and transmitting to "Klik-Aan-Klik-Uit" devices
 * working according to the learning code system. This protocol is also used by the Home Easy devices.
 * It includes direct DIM functionality.
 * 
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : StuntTeam 2015..2016
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 ***********************************************************************************************
 * Pulse (T) is 275us PDM
 * 0 = T,T,T,4T, 1 = T,4T,T,T, dim = T,T,T,T op bit 28
 *
 * From: Wieltje @ http://www.circuitsonline.net/forum/view/message/1181410#1181410 
 *       _   _
 * '0':	| |_| |____ (T,T,T,3T)
 *       _      _
 * '1':	| |____| |_ (T,3T,T,T)
 *       _   _
 * dim: | |_| |_    (T,T,T,T) 
 *
 * T = korte periode = 275 µs (of 375, werkt ook)
 * lange periode = 3 of 4*T (werkt ook allebei)
 *
 * | 00100011110100100010011010 |   0 |     1 |  0000 | 
 * | ID#                        | All | State | unit# | 
 *
 * NewKAKU supports:
 *   on/off       ---- 000x Off/On
 *   all on/off   ---- 001x AllOff/AllOn
 *   dim absolute xxxx 0110 Dim16        // dim on bit 27 + 4 extra bits for dim level
 *
 *  NewKAKU bitstream= (First sent) AA AAAAAAAA AAAAAAAA AAAAAAAACCUUUU(LLLL) -> A=KAKU_address, C=command, U=KAKU-Unit, L=extra dimlevel bits (optional)
 *
 * Sample RF packet:
 * 20;B8;NewKaku;ID=00c142;SWITCH=1;CMD=OFF;
 * 20;B9;DEBUG;Pulses=132;Pulses(uSec)=200,2550,150,200,125,1200,150,200,150,1200,125,1200,150,225,125,1200,125,225,125,200,150,1200,150,200,150,1200,150,1200,125,200,125,200,125,1225,125,1200,125,225,150,200,150,1200,150,1200,150,200,150,1200,150,225,125,200,150,1200,125,200,150,1200,125,200,150,1200,150,200,125,1225,150,200,125,1200,150,1200,125,225,125,200,125,1200,150,1200,125,225,125,200,125,1225,125,200,125,1225,125,200,125,1200,125,200,150,1225,125,1200,150,200,125,200,125,1200,125,200,150,1200,125,200,125,1200,150,200,125,1200,125,200,125,1200,150,200,125,1200,150,200,150,1200,125;
 * 20;06;DEBUG;Pulses=132;Pulses(uSec)=175,2575,150,200,150,1200,150,200,150,1200,150,1200,150,200,125,1200,150,200,125,200,150,1200,125,200,150,1200,150,1200,150,200,150,200,150,1225,150,1200,125,225,150,200,125,1200,150,1200,150,200,150,1200,150,200,150,200,125,1225,125,200,150,1200,125,200,150,1200,125,200,150,1200,150,200,150,1200,150,1200,125,200,150,200,125,1200,150,1200,125,225,150,200,125,1200,150,200,150,1200,150,200,150,1200,150,200,150,1225,125,1200,150,200,125,200,150,1200,150,200,125,1200,150,200,150,1200,150,200,150,1200,150,200,125,1225,125,200,125,1200,150,200,150,1200,150
 \*********************************************************************************************/
#define NewKAKU_PLUGIN_ID 004
#define NewKAKU_RawSignalLength 132            // regular KAKU packet length
#define NewKAKUdim_RawSignalLength 148         // KAKU packet length including DIM bits
#define NewKAKU_mT 650 / RAWSIGNAL_SAMPLE_RATE // us, approx. in between 1T and 4T

#ifdef PLUGIN_004
#include "../4_Display.h"

boolean Plugin_004(byte function, char *string)
{
   // nieuwe KAKU bestaat altijd uit start bit + 32 bits + evt 4 dim bits. Ongelijk, dan geen NewKAKU
   if ((RawSignal.Number != NewKAKU_RawSignalLength) && (RawSignal.Number != NewKAKUdim_RawSignalLength))
      return false;
   if (RawSignal.Pulses[0] == 15)
      return true; // Home Easy, skip KAKU
   boolean Bit;
   int i;
   //int P0,P1,P2,P3;
   byte P0, P1, P2, P3;
   byte dim = 0;
   byte dimbitpresent = 0;
   unsigned long bitstream = 0L;

   // RawSignal.Pulses[1] startbit with duration of 1T => ignore
   // RawSignal.Pulses[2] long space after startbit with duration of 8T => ignore
   i = 3; // RawSignal.Pulses[3] is first pulse of a T,xT,T,xT combination
   do
   {
      P0 = RawSignal.Pulses[i];     // * RawSignal.Multiply;
      P1 = RawSignal.Pulses[i + 1]; // * RawSignal.Multiply;
      P2 = RawSignal.Pulses[i + 2]; // * RawSignal.Multiply;
      P3 = RawSignal.Pulses[i + 3]; // * RawSignal.Multiply;

      if (P0 < NewKAKU_mT && P1 < NewKAKU_mT && P2 < NewKAKU_mT && P3 > NewKAKU_mT)
      {
         Bit = 0; // T,T,T,4T
      }
      else if (P0 < NewKAKU_mT && P1 > NewKAKU_mT && P2 < NewKAKU_mT && P3 < NewKAKU_mT)
      {
         Bit = 1; // T,4T,T,T
      }
      else if (P0 < NewKAKU_mT && P1 < NewKAKU_mT && P2 < NewKAKU_mT && P3 < NewKAKU_mT)
      { // T,T,T,T should be on i=111 (bit 28)
         dimbitpresent = 1;
         if (RawSignal.Number != NewKAKUdim_RawSignalLength)
         { // dim set but no dim bits present => invalid signal
            return false;
         }
         //if (i != 111) return false;                           // not the right location for the dim bit indicator
      }
      else
      {
         return false; // Other pulse patterns are invalid within the AC KAKU signal.
      }
      if (i < 130)
      { // all bits that belong to the 32-bit pulse sequence (32bits * 4 positions per bit + pulse/space for startbit)
         bitstream = (bitstream << 1) | Bit;
      }
      else
      { // remaining 4 bits that set the dim level
         dim = (dim << 1) | Bit;
      }
      i += 4;                          // Next 4 pulses
   } while (i < RawSignal.Number - 2); //-2 to exclude the stopbit space/pulse
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   if ((SignalHash != SignalHashPrevious) || ((RepeatingTimer + 700) < millis()) || (SignalCRC != bitstream))
   { // 1000
      // not seen the RF packet recently
      //if ((SignalHashPrevious==14) && ((RepeatingTimer+2000)>millis()) ) {
      //   SignalHash=14;
      //   return true;                            // SignalHash 14 = HomeEasy, eg. cant switch KAKU after HE for 2 seconds
      //}
      if ((SignalHashPrevious == 11) && ((RepeatingTimer + 2000) > millis()))
      {
         SignalHash = 11;
         return true; // SignalHash 11 = FA500, eg. cant switch KAKU after FA500 for 2 seconds
      }

      SignalCRC = bitstream;
   }
   else
      return true; // already seen the RF packet recently
   //==================================================================================
   // Output
   //==================================================================================
   display_Header();
   display_Name(PSTR("NewKaku"));
   display_IDn(((bitstream >> 6) & 0xFFFFFFFF), 8); //"%S%08lx"
   char c_SWITCH[4];
   sprintf(c_SWITCH, "%1x", ((byte)(bitstream & 0x0f) + 1)); // No leading 0
   display_SWITCHc(c_SWITCH);

   if (i > 140 && dimbitpresent == 1)
      display_SET_LEVEL(dim); // Command and Dim part
   else
      display_CMD((CMD_Group)((bitstream >> 5) & B01), (CMD_OnOff)((bitstream >> 4) & B01)); // #ALL , #ON
   display_Footer();
   // ----------------------------------
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // Plugin_004

#ifdef PLUGIN_TX_004
#include "3_Serial.h"
#include "4_Display.h"

boolean PluginTX_004(byte function, char *string)
{
   // ON, OFF, ALLON, ALLOFF, ALL 99, 99
   //10;NewKaku;123456;3;ON;
   //10;NewKaku;0cac142;2;ON;
   //10;NewKaku;050515;f;OFF;
   //10;NewKaku;2100fed;1;ON;
   //10;NewKaku;000001;10;ON;
   //10;NewKaku;306070b;f;ON;
   //10;NewKaku;306070b;10;ON;
   //01234567890123456789012

   // Start message split
   // Serial.print("*** Spliting message ***\n");

   static const char c_delim[2] = ";";
   static char c_label[10];
   static char c_ID[10];
   static char c_Switch[10];
   static char c_Cmd[10];

   // 10
   char *ptr = strtok(InputBuffer_Serial, c_delim);
   if (ptr != NULL)
   {
      strcpy(c_label, "10");
      if (strncasecmp(ptr, c_label, strlen(c_label)) != 0)
         return false;
   }
   else
      return false;

   // Newkaku
   ptr = strtok(NULL, c_delim);
   if (ptr != NULL)
   {
      strcpy(c_label, "NEWKAKU");
      if (strncasecmp(ptr, c_label, strlen(c_label)) != 0)
         return false;
   }
   else
      return false;

   // ID
   ptr = strtok(NULL, c_delim);
   if (ptr != NULL)
   {
      strcpy(c_label, "ID=");
      if (strncasecmp(ptr, c_label, strlen(c_label)) == 0)
         ptr += strlen(c_label);

      if (strlen(ptr) > 8)
         return false;

      for (byte i = 0; i < strlen(ptr); i++)
         if (!isxdigit(ptr[i]))
            return false;

      strcpy(c_ID, ptr);
      c_ID[8] = 0;
   }
   else
      return false;

   // Switch
   ptr = strtok(NULL, c_delim);
   if (ptr != NULL)
   {
      strcpy(c_label, "SWITCH=");
      if (strncasecmp(ptr, c_label, strlen(c_label)) == 0)
         ptr += strlen(c_label);

      if (strlen(ptr) > 1)
         return false;

      for (byte i = 0; i < strlen(ptr); i++)
         if (!isxdigit(ptr[i]))
            return false;

      strcpy(c_Switch, ptr);
   }
   else
      return false;

   // Command
   ptr = strtok(NULL, c_delim);
   if (ptr != NULL)
   {
      strcpy(c_label, "SET_LEVEL=");
      if (strncasecmp(ptr, c_label, strlen(c_label)) == 0)
         ptr += strlen(c_label);

      strcpy(c_label, "CMD=");
      if (strncasecmp(ptr, c_label, strlen(c_label)) == 0)
         ptr += strlen(c_label);

      if (strlen(ptr) > 7)
         return false;

      for (byte i = 0; i < strlen(ptr); i++)
         if (!isalnum(ptr[i]))
            return false;

      strcpy(c_Cmd, ptr);
   }
   else
      return false;

   // End
   ptr = strtok(NULL, c_delim);
   if (ptr != NULL)
      return false;

   // Summary message split
   // Serial.print("*** Split OK ***\n");
   // Serial.println(c_ID);
   // Serial.println(c_Switch);
   // Serial.println(c_Cmd);

   // --------------- Prepare bitstream ------------
   // Dimming of groups is also possible but not supported yet!
   // when level=0 is it better to transmit just the off command ?
   // Serial.print("*** Creating bitstream ***\n");
   unsigned long bitstream = 0L; // 32 bits complete packet

   unsigned long ID_bitstream = 0L; // 26 bits Address
   ID_bitstream = strtoul(c_ID, NULL, HEX);
   ID_bitstream &= 0x03FFFFFF;
   // Serial.println("ID_bitstream");
   // Serial.println(ID_bitstream, HEX);

   bitstream = (ID_bitstream << 6); // 26 bits on top

   byte Switch_bitstream = 0; // 4 bits Unit
   Switch_bitstream = (byte)strtoul(c_Switch, NULL, HEX);
   Switch_bitstream--; // 1 to 16 -> 0 to 15 (displayed value is one more)
   if (Switch_bitstream > 0xF)
      return false; // invalid address
   // Serial.println("Switch_bitstream");
   // Serial.println(Switch_bitstream, HEX);

   bitstream |= Switch_bitstream; // Complete transmitted address
   bitstream &= 0xFFFFFFCF;       // Bit 4 and 5 are left for cmd

   byte Cmd_bitstream = 0x00; // 2 bits Command
   byte cmd = 0;
   cmd = str2cmd(c_Cmd); // Get ON/OFF etc. command
   if (cmd == false)     // Not a valid command received? ON/OFF/ALLON/ALLOFF
      cmd = (byte)strtoul(c_Cmd, NULL, HEX);
   // ON
   switch (cmd)
   {
   case VALUE_ON:
   case VALUE_ALLON:
      Cmd_bitstream |= B01;
      break;
   }
   // Group
   switch (cmd)
   {
   case VALUE_ALLON:
   case VALUE_ALLOFF:
      Cmd_bitstream |= B10;
      break;
   }
   // Dimmer
   switch (cmd)
   {
   case VALUE_ON:
   case VALUE_OFF:
   case VALUE_ALLON:
   case VALUE_ALLOFF:
      cmd = 0xFF;
      break;
   }
   // Serial.println("Cmd_bitstream");
   // Serial.println(Cmd_bitstream, HEX);

   bitstream |= (Cmd_bitstream << 4);

   // Serial.println("Complete bitstream");
   // Serial.println(bitstream, HEX);

   // Serial.println("cmd");
   // Serial.println(cmd, HEX);

   // bitstream now contains the AC/NewKAKU-bits that have to be transmitted
   // --------------- NEWKAKU SEND ------------

   AC_Send(bitstream, cmd);

   // --------------------------------------
   return true;
}


#endif // Plugin_TX_004
