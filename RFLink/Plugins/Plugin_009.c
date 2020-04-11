//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-06: X10 RF                                          ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of sending and receiving the X10 RF protocol. 
 * 
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : StuntTeam 2015..2016
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 ***********************************************************************************************
 * Incoming event: "X10 <adres>, <On | Off>
 * Send          : "X10Send <Adres>, <On | Off> 
 *
 * Address = A1 - P16
 ***********************************************************************************************
 * Technical information:
 * RF packets are 68 bits long transferring 64 manchester encoded bits resulting in 32 bits / 4 bytes.
 *
 * address  address^  data     data^  
 * 01100000 10011111 00000000 11111111      609F00FF                                 
 * 10011111 01100000 11111111 00000000      9F60FF00
 * 
 * 4 bytes are transmitted, the second and fourth are the inverse of the first and third byte.
 * So the actual data is only 2 bytes
 * 
 * 01100000 00000000
 * AAAABBBB CDEDDDDD
 *
 * A = Housecode    0110 a  6    0111 b  7    0100 c  4    0101 d  5     1000 e  8       1001 f  9       1010 g  a       1011 h  b
 *                 1110 i  e    1111 j  f    1100 k  c    1101 l  d     0000 m  0       0001 n  1       0010 o  2       0011 p  3      
 * B = Unitcode 1-8 / 9-16 indicator
 * C = Group/Dimmer indicator
 * D = Unitcode 
 * E = on/off indicator
 *
 * on
 * 20;06;DEBUG;Pulses=68;Pulses(uSec)=3300,4225,400,375,400,1325,400,1325,400,1325,400,375,400,375,400,375,400,375,400,1325,400,375,400,375,400,375,400,1350,400,1350,375,1350,400,1350,400,375,400,375,400,375,400,1325,400,1325,400,375,400,375,400,375,400,1350,400,1325,400,1325,400,375,400,375,400,1325,400,1325,400,1325,400
 * off
 * 20;10;DEBUG;Pulses=68;Pulses(uSec)=3300,4225,400,375,400,1350,400,1350,400,1325,400,375,400,375,400,375,400,375,400,1325,400,375,400,375,400,375,400,1325,400,1325,400,1325,400,1325,400,375,400,375,400,1325,400,1350,400,1350,400,375,400,375,400,375,375,1350,400,1350,400,375,400,375,400,375,400,1325,400,1350,400,1325,400;
 * 20;20;DEBUG;Pulses=66;Pulses(uSec)=425,350,375,1300,375,1300,375,1350,375,375,375,1350,375,375,375,375,375,1350,375,375,375,375,375,375,400,1350,375,375,400,1350,375,1350,400,1325,400,375,400,375,400,375,400,375,400,375,400,375,400,375,400,375,400,1325,400,1325,400,1325,400,1325,400,1325,400,1350,375,1350,375;
 \*********************************************************************************************/
#define X10_PulseLength 66
#define X10_PULSEMID 600 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_009
#include "../4_Display.h"

boolean Plugin_009(byte function, char *string)
{
   if ((RawSignal.Number != (X10_PulseLength)) && (RawSignal.Number != (X10_PulseLength + 2)))
      return false;
   unsigned long bitstream = 0L;
   byte housecode = 0;
   byte unitcode = 0;
   byte command = 0;
   byte data[4];
   byte start = 0;
   if (RawSignal.Number == X10_PulseLength + 2)
   {
      if ((RawSignal.Pulses[1] * RawSignal.Multiply > 3000) && (RawSignal.Pulses[2] * RawSignal.Multiply > 3000))
         start = 2;
      else
         return false; // not an X10 packet
   }
   // get all 24 bits
   for (byte x = 2 + start; x < ((X10_PulseLength) + start); x += 2)
   {
      bitstream <<= 1; // Always shift
      if (RawSignal.Pulses[x] > X10_PULSEMID)
         bitstream |= 0x1;
      // else
      //    bitstream |= 0x0;
   }
   //==================================================================================
   // Perform a quick sanity check
   //==================================================================================
   if (bitstream == 0)
      return false;
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   if ((SignalHash != SignalHashPrevious) || (RepeatingTimer + 1000 < millis()) || (SignalCRC != bitstream))
      SignalCRC = bitstream; // not seen the RF packet recently
   else
      return true; // already seen the RF packet recently
   //==================================================================================
   // order received data
   data[0] = (bitstream >> 24) & 0xFF;
   data[1] = (bitstream >> 16) & 0xFF;
   data[2] = (bitstream >> 8) & 0xFF;
   data[3] = (bitstream) & 0xFF;
   // ----------------------------------
   // perform sanity checks
   data[1] ^= 0xFF;
   data[3] ^= 0xFF;
   if (data[0] != data[1])
      return false;
   if (data[2] != data[3])
      return false;
   // ----------------------------------
   data[1] = data[1] & 0x0F; // lower nibble only
   data[0] = data[0] & 0xF0; // upper nibble only

   housecode = 0;
   if (data[0] == 0x60)
      housecode = 0;
   if (data[0] == 0x70)
      housecode = 1;
   if (data[0] == 0x40)
      housecode = 2;
   if (data[0] == 0x50)
      housecode = 3;
   if (data[0] == 0x80)
      housecode = 4;
   if (data[0] == 0x90)
      housecode = 5;
   if (data[0] == 0xa0)
      housecode = 6;
   if (data[0] == 0xb0)
      housecode = 7;
   if (data[0] == 0xe0)
      housecode = 8;
   if (data[0] == 0xf0)
      housecode = 9;
   if (data[0] == 0xc0)
      housecode = 10;
   if (data[0] == 0xd0)
      housecode = 11;
   if (data[0] == 0x00)
      housecode = 12;
   if (data[0] == 0x10)
      housecode = 13;
   if (data[0] == 0x20)
      housecode = 14;
   if (data[0] == 0x30)
      housecode = 15;

   if (data[2] == 0x00)
   {
      unitcode = 1;
      command = 1;
   }
   if (data[2] == 0x20)
   {
      unitcode = 1;
      command = 0;
   }
   if (data[2] == 0x10)
   {
      unitcode = 2;
      command = 1;
   }
   if (data[2] == 0x30)
   {
      unitcode = 2;
      command = 0;
   }
   if (data[2] == 0x08)
   {
      unitcode = 3;
      command = 1;
   }
   if (data[2] == 0x28)
   {
      unitcode = 3;
      command = 0;
   }
   if (data[2] == 0x18)
   {
      unitcode = 4;
      command = 1;
   }
   if (data[2] == 0x38)
   {
      unitcode = 4;
      command = 0;
   }
   if (data[2] == 0x40)
   {
      unitcode = 5;
      command = 1;
   }
   if (data[2] == 0x60)
   {
      unitcode = 5;
      command = 0;
   }
   if (data[2] == 0x50)
   {
      unitcode = 6;
      command = 1;
   }
   if (data[2] == 0x70)
   {
      unitcode = 6;
      command = 0;
   }
   if (data[2] == 0x48)
   {
      unitcode = 7;
      command = 1;
   }
   if (data[2] == 0x68)
   {
      unitcode = 7;
      command = 0;
   }
   if (data[2] == 0x58)
   {
      unitcode = 8;
      command = 1;
   }
   if (data[2] == 0x78)
   {
      unitcode = 8;
      command = 0;
   }
   if (data[2] == 0x88)
   {
      unitcode = 0;
      command = 2;
   }
   if (data[2] == 0x98)
   {
      unitcode = 0;
      command = 3;
   }
   if (data[2] == 0x80)
   {
      unitcode = 0;
      command = 4;
   }
   if (data[2] == 0x90)
   {
      unitcode = 0;
      command = 5;
   }

   if ((data[1] == 0x04) && (command < 2))
   {
      unitcode = unitcode + 8;
   }
   //==================================================================================
   // ----------------------------------
   // All is OK, build event
   // ----------------------------------
   // Output
   // ----------------------------------
   display_Header();
   display_Name(PSTR("X10"));
   display_IDn((0x41 + housecode), 2); //"%S%02x"
   display_SWITCH(unitcode);

   switch (command)
   {
   case 0x00:
   case 0x01:
   case 0x04:
   case 0x05:
      display_CMD(((command >> 3) & B01), (command & B01));
      break;
   case 0x02:
   case 0x03:
      display_Name(PSTR(";CMD="));
      switch (command)
      {
      case 0x02:
         display_Name(PSTR("BRIGHT"));
         break;
      case 0x03:
         display_Name(PSTR("DIM"));
         break;
      }
      break;
   }
   display_Footer();
   // ----------------------------------
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif //PLUGIN_009

#ifdef PLUGIN_TX_009
void X10_Send(uint32_t address);

boolean PluginTX_009(byte function, char *string)
{
   boolean success = false;
   //10;X10;000041;1;OFF;
   //0123456789012345678
   // Hier aangekomen bevat string het volledige commando. Test als eerste of het opgegeven commando overeen komt
   if (strncasecmp(InputBuffer_Serial + 3, "X10;", 4) == 0)
   { // X10 Command eg.
      unsigned long bitstream = 0L;
      byte x = 14; // teller die wijst naar het te behandelen teken
      byte command = 0;
      byte Home = 0;    // Home A..P
      byte Address = 0; // Blyss subchannel 1..5
      byte c;
      uint32_t newadd = 0;

      InputBuffer_Serial[9] = 0x30;
      InputBuffer_Serial[10] = 0x78;
      InputBuffer_Serial[13] = 0;
      Home = str2int(InputBuffer_Serial + 9); // Home: A..P
      if (Home < 0x51)                        // take care of upper/lower case
         Home = Home - 'A';
      else if (Home < 0x71) // take care of upper/lower case
         Home = Home - 'a';
      else
      {
         return success; // invalid value
      }

      while ((c = tolower(InputBuffer_Serial[x++])) != ';')
      { // Address: 1 to 16
         if (c >= '0' && c <= '9')
         {
            Address = Address * 10;
            Address = Address + c - '0';
         }
      }

      if (Home == 0)
         c = 0x60;
      if (Home == 1)
         c = 0x70;
      if (Home == 2)
         c = 0x40;
      if (Home == 3)
         c = 0x50;
      if (Home == 4)
         c = 0x80;
      if (Home == 5)
         c = 0x90;
      if (Home == 6)
         c = 0xa0;
      if (Home == 7)
         c = 0xb0;
      if (Home == 8)
         c = 0xe0;
      if (Home == 9)
         c = 0xf0;
      if (Home == 10)
         c = 0xc0;
      if (Home == 11)
         c = 0xd0;
      if (Home == 12)
         c = 0x00;
      if (Home == 13)
         c = 0x10;
      if (Home == 14)
         c = 0x20;
      if (Home == 15)
         c = 0x30;
      if (Address > 7)
      {
         c = c + 4;
         Address = Address - 8;
      }
      // ---------------
      Home = str2cmd(InputBuffer_Serial + x);
      if (Home == 0)
      { // DIM/BRIGHT command
         if (strcasecmp(InputBuffer_Serial + x, "DIM") == 0)
         {
            command = 3;
         }
         else if (strcasecmp(InputBuffer_Serial + x, "BRIGHT") == 0)
         {
            command = 2;
         }
         c = c + 4;
      }
      else
      {
         if (Home == VALUE_ON)
         {
            command = 1;
         }
         else if (Home == VALUE_OFF)
         {
            command = 0;
         }
         else if (Home == VALUE_ALLOFF)
         {
            command = 4;
            c = c + 4;
         }
         else if (Home == VALUE_ALLON)
         {
            command = 5;
            c = c + 4;
         }
      }
      if (Address == 1 && command == 1)
         bitstream = 0x00;
      if (Address == 1 && command == 0)
         bitstream = 0x20;
      if (Address == 2 && command == 1)
         bitstream = 0x10;
      if (Address == 2 && command == 0)
         bitstream = 0x30;
      if (Address == 3 && command == 1)
         bitstream = 0x08;
      if (Address == 3 && command == 0)
         bitstream = 0x28;
      if (Address == 4 && command == 1)
         bitstream = 0x18;
      if (Address == 4 && command == 0)
         bitstream = 0x38;
      if (Address == 5 && command == 1)
         bitstream = 0x40;
      if (Address == 5 && command == 0)
         bitstream = 0x60;
      if (Address == 6 && command == 1)
         bitstream = 0x50;
      if (Address == 6 && command == 0)
         bitstream = 0x70;
      if (Address == 7 && command == 1)
         bitstream = 0x48;
      if (Address == 7 && command == 0)
         bitstream = 0x68;
      if (Address == 8 && command == 1)
         bitstream = 0x58;
      if (Address == 8 && command == 0)
         bitstream = 0x78;
      if (command == 2)
         bitstream = 0x88;
      if (command == 3)
         bitstream = 0x98;
      if (command == 4)
         bitstream = 0x80;
      if (command == 5)
         bitstream = 0x90;
      // -----------------------------
      newadd = bitstream << 8;
      bitstream = bitstream ^ 0xff;
      newadd = newadd + bitstream;
      bitstream = c ^ 0xff;
      bitstream = bitstream << 16;
      newadd = newadd + bitstream;
      bitstream = c;
      bitstream = bitstream << 24;
      newadd = newadd + bitstream;
      bitstream = newadd;
      // -----------------------------
      X10_Send(bitstream); // full bitstream to send
      success = true;
   }
   return success;
}

void X10_Send(uint32_t address)
{
   int fpulse = 375; // Pulse witdh in microseconds
   int fretrans = 4; // Number of code retransmissions
   uint32_t fdatabit;
   uint32_t fdatamask = 0x80000000;
   uint32_t fsendbuff;

   digitalWrite(PIN_RF_RX_VCC, LOW);            // Disable RF receiver
   digitalWrite(PIN_RF_TX_VCC, HIGH);           // Enable RF transmitter
   delayMicroseconds(TRANSMITTER_STABLE_DELAY); // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)

   for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++)
   {
      fsendbuff = address;

      // send SYNC 12P High, 10P low
      digitalWrite(PIN_RF_TX_DATA, HIGH);
      delayMicroseconds(fpulse * 12);
      digitalWrite(PIN_RF_TX_DATA, LOW);
      delayMicroseconds(fpulse * 10);
      // end send SYNC
      // Send command
      for (int i = 0; i < 32; i++)
      { // 32 bits
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
            delayMicroseconds(fpulse * 4);
         }
      }
      // Send Stop/delay
      digitalWrite(PIN_RF_TX_DATA, HIGH);
      delayMicroseconds(fpulse * 1);
      digitalWrite(PIN_RF_TX_DATA, LOW);
      delayMicroseconds(fpulse * 20);
   }
   delayMicroseconds(TRANSMITTER_STABLE_DELAY); // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)
   digitalWrite(PIN_RF_TX_VCC, LOW);            // Disable RF transmitter
   digitalWrite(PIN_RF_RX_VCC, HIGH);           // Enable RF receiver
   RFLinkHW();
   return;
}
#endif //PLUGIN_TX_009
