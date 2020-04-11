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
      display_CMD((bitstream >> 5) & B01, (bitstream >> 4) & B01); // #ALL , #ON
   display_Footer();
   // ----------------------------------
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // Plugin_004

#ifdef PLUGIN_TX_004
void AC_Send(unsigned long data, byte cmd);

boolean PluginTX_004(byte function, char *string)
{
   boolean success = false;
   //10;NewKaku;123456;3;ON;                   // ON, OFF, ALLON, ALLOFF, ALL 99, 99
   //10;NewKaku;0cac142;2;ON;
   //10;NewKaku;050515;f;OFF;
   //10;NewKaku;2100fed;1;ON;
   //10;NewKaku;000001;10;ON;
   //10;NewKaku;306070b;f;ON;
   //10;NewKaku;306070b;10;ON;
   //01234567890123456789012
   if (strncasecmp(InputBuffer_Serial + 3, "NEWKAKU;", 8) == 0)
   {
      byte x = 18; // pointer to the switch number
      if (InputBuffer_Serial[17] != ';')
      {
         if (InputBuffer_Serial[18] != ';')
         {
            return false;
         }
         else
         {
            x = 19;
         }
      }

      unsigned long bitstream = 0L;
      unsigned long tempaddress = 0L;
      byte cmd = 0;
      //byte c=0; // MRI commented
      byte Address = 0; // Address 1..16

      // -----
      InputBuffer_Serial[9] = 0x30; // Get NEWKAKU/AC main address part from hexadecimal value
      InputBuffer_Serial[10] = 0x78;
      InputBuffer_Serial[x - 1] = 0x00;
      tempaddress = str2int(InputBuffer_Serial + 9);
      // -----
      //while((c=InputBuffer_Serial[x++])!=';'){ // Address: 1 to 16
      //   if(c>='0' && c<='9'){Address=Address*10;Address=Address+c-'0';}
      //}
      InputBuffer_Serial[x - 2] = 0x30; // Get unit number from hexadecimal value
      InputBuffer_Serial[x - 1] = 0x78; // x points to the first character of the unit number
      if (InputBuffer_Serial[x + 1] == ';')
      {
         InputBuffer_Serial[x + 1] = 0x00;
         cmd = 2;
      }
      else
      {
         if (InputBuffer_Serial[x + 2] == ';')
         {
            InputBuffer_Serial[x + 2] = 0x00;
            cmd = 3;
         }
         else
         {
            return false;
         }
      }
      Address = str2int(InputBuffer_Serial + (x - 2)); // NewKAKU unit number
      if (Address > 16)
         return false; // invalid address
      Address--;       // 1 to 16 -> 0 to 15 (transmitted value is 1 less than shown values)
      x = x + cmd;     // point to on/off/dim command part
      // -----
      tempaddress = (tempaddress << 6) + Address; // Complete transmitted address
      // -----
      cmd = str2cmd(InputBuffer_Serial + x); // Get ON/OFF etc. command
      if (cmd == false)
      {                                         // Not a valid command received? ON/OFF/ALLON/ALLOFF
         cmd = str2int(InputBuffer_Serial + x); // get DIM value
      }
      // --------------- Prepare bitstream ------------
      bitstream = tempaddress & 0xFFFFFFCF; // adres geheel over nemen behalve de twee bits 5 en 6 die het schakel commando bevatten.

      // Dimming of groups is also possible but not supported yet!
      // when level=0 is it better to transmit just the off command ?

      if (cmd == VALUE_ON || cmd == VALUE_OFF)
      {
         bitstream |= (cmd == VALUE_ON) << 4; // bit-5 is the on/off command in the KAKU signal
         cmd = 0xff;
      }
      else if (cmd == VALUE_ALLON || cmd == VALUE_ALLOFF)
      {
         bitstream |= B1 << 5;                   // bit 5 is the group indicator
         bitstream |= (cmd == VALUE_ALLON) << 4; // bit-4 is the on/off indicator
         cmd = 0xff;
      }
      // bitstream now contains the AC/NewKAKU-bits that have to be transmitted
      // --------------- NEWKAKU SEND ------------
      AC_Send(bitstream, cmd);
      success = true;
   }
   // --------------------------------------
   return success;
}

void AC_Send(unsigned long data, byte cmd)
{
   int fpulse = 260;  // Pulse width in microseconds
   int fretrans = 10; // Number of code retransmissions

   unsigned long bitstream = 0L;
   byte command;
   // prepare data to send
   for (unsigned short i = 0; i < 32; i++)
   { // reverse data bits
      bitstream <<= 1;
      bitstream |= (data & B1);
      data >>= 1;
   }
   if (cmd != 0xff)
   { // reverse dim bits
      for (unsigned short i = 0; i < 4; i++)
      {
         command <<= 1;
         command |= (cmd & B1);
         cmd >>= 1;
      }
   }
   // Prepare transmit
   digitalWrite(PIN_RF_RX_VCC, LOW);            // Turn off power to the RF receiver
   digitalWrite(PIN_RF_TX_VCC, HIGH);           // Enable the 433Mhz transmitter
   delayMicroseconds(TRANSMITTER_STABLE_DELAY); // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)
   // send bits
   for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++)
   {
      data = bitstream;
      if (cmd != 0xff)
         cmd = command;
      digitalWrite(PIN_RF_TX_DATA, HIGH);
      //delayMicroseconds(fpulse);  //335
      delayMicroseconds(335);
      digitalWrite(PIN_RF_TX_DATA, LOW);
      delayMicroseconds(fpulse * 10 + (fpulse >> 1)); //335*9=3015 //260*10=2600
      for (unsigned short i = 0; i < 32; i++)
      {
         if (i == 27 && cmd != 0xff)
         { // DIM command, send special DIM sequence TTTT replacing on/off bit
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse);
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse);
         }
         else
            switch (data & B1)
            {
            case 0:
               digitalWrite(PIN_RF_TX_DATA, HIGH);
               delayMicroseconds(fpulse);
               digitalWrite(PIN_RF_TX_DATA, LOW);
               delayMicroseconds(fpulse);
               digitalWrite(PIN_RF_TX_DATA, HIGH);
               delayMicroseconds(fpulse);
               digitalWrite(PIN_RF_TX_DATA, LOW);
               delayMicroseconds(fpulse * 5); // 335*3=1005 260*5=1300  260*4=1040
               break;
            case 1:
               digitalWrite(PIN_RF_TX_DATA, HIGH);
               delayMicroseconds(fpulse);
               digitalWrite(PIN_RF_TX_DATA, LOW);
               delayMicroseconds(fpulse * 5);
               digitalWrite(PIN_RF_TX_DATA, HIGH);
               delayMicroseconds(fpulse);
               digitalWrite(PIN_RF_TX_DATA, LOW);
               delayMicroseconds(fpulse);
               break;
            }
         //Next bit
         data >>= 1;
      }
      // send dim bits when needed
      if (cmd != 0xff)
      { // need to send DIM command bits
         for (unsigned short i = 0; i < 4; i++)
         { // 4 bits
            switch (cmd & B1)
            {
            case 0:
               digitalWrite(PIN_RF_TX_DATA, HIGH);
               delayMicroseconds(fpulse);
               digitalWrite(PIN_RF_TX_DATA, LOW);
               delayMicroseconds(fpulse);
               digitalWrite(PIN_RF_TX_DATA, HIGH);
               delayMicroseconds(fpulse);
               digitalWrite(PIN_RF_TX_DATA, LOW);
               delayMicroseconds(fpulse * 5); // 335*3=1005 260*5=1300
               break;
            case 1:
               digitalWrite(PIN_RF_TX_DATA, HIGH);
               delayMicroseconds(fpulse);
               digitalWrite(PIN_RF_TX_DATA, LOW);
               delayMicroseconds(fpulse * 5);
               digitalWrite(PIN_RF_TX_DATA, HIGH);
               delayMicroseconds(fpulse);
               digitalWrite(PIN_RF_TX_DATA, LOW);
               delayMicroseconds(fpulse);
               break;
            }
            //Next bit
            cmd >>= 1;
         }
      }
      //Send termination/synchronisation-signal. Total length: 32 periods
      digitalWrite(PIN_RF_TX_DATA, HIGH);
      delayMicroseconds(fpulse);
      digitalWrite(PIN_RF_TX_DATA, LOW);
      delayMicroseconds(fpulse * 40); //31*335=10385 40*260=10400
   }
   // End transmit
   delayMicroseconds(TRANSMITTER_STABLE_DELAY); // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)
   digitalWrite(PIN_RF_TX_VCC, LOW);            // Turn thew 433Mhz transmitter off
   digitalWrite(PIN_RF_RX_VCC, HIGH);           // Turn the 433Mhz receiver on
   RFLinkHW();
}
#endif // Plugin_TX_004
