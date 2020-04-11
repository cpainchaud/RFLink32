//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-03: FA500R                                          ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of sending and receiving the Elro Flamingo FA500 protocol. 
 * Also works with compatible devices like the Mumbi M-FS300 and Silvercrest 91210/60494 RCS AAA3680, Unitec eim 821/art.48111
 * 
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : StuntTeam 2015..2016
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 ***********************************************************************************************
 * Incoming event: "FA500 <adres>,<On | Off>
 * Send          : "FA500Send <Adres>, <On | Off> 
 *
 * Address = A/B/C/D matching the remote control buttons.
 ***********************************************************************************************
 * Technical information:
 * The FA500R remote control sends 3 different protocols.
 * 4 x Method 1 - 28 bit code
 * 6 x method 2 - AC compatible 
 * 5 x method 3 - 24/12 bit code 
 * It appears that the FA500S is only capable to react to the first method. So this has to be used to switch sockets
 * Nodo can only distinguish the 3rd method properly. Method 1 and 2 have to be pulled apart first which
 * is done via plugin 001.
 *
 * Device types:
 * Elro FA500S Flamingo Switch 
 * Elro FA500DSS Flamingo Dimmer 
 * Elro FA500WD Flamingo Outdoor 
 * Elro FA500R Flamingo Remote Control 
 *
 * PCB Markings:
 *   50027.01B FLF-130105 
 * KT50039.01A FLF-13-06-03 Chip marking: S007V0.1 
 * KT50040.01A FLF-13-06-04
 *
 * http://forum.arduino.cc/index.php?topic=202556.0
 *
 * Sample:
 * 20;60;DEBUG;Pulses=24;Pulses(uSec)=325,800,275,800,825,225,275,800,275,825,275,800,825,225,275,800,825,225,275,800,275,800,275;
 * 20;61;DEBUG;Pulses=58;Pulses(uSec)=200,875,800,250,800,225,200,875,200,875,800,250,200,875,200,875,800,250,200,875,200,875,200,875,200,875,825,250,200,875,200,875,200,875,825,250,200,875,825,250,200,875,200,875,200,875,825,225,825,250,200,875,825,250,200,875,150;
 
 * 20;3E;DEBUG;Pulses=58;Pulses(uSec)=300,950,225,950,875,275,225,950,225,950,875,275,225,950,225,950,875,275,875,275,225,950,875,275,225,950,225,950,875,275,875,275,225,950,225,950,225,950,875,275,875,275,200,950,225,950,875,275,875,275,225,950,875,275,225,950,225;
 * 20;3F;DEBUG;Pulses=64;Pulses(uSec)=525,250,200,900,200,4900,200,900,200,900,875,275,225,950,225,950,875,275,225,950,225,950,875,275,875,275,225,950,900,250,225,950,225,950,875,250,875,275,225,950,225,950,225,950,900,275,875,275,225,950,225,950,875,275,875,275,225,950,875,250,225,950,225;
 * 20;40;DEBUG;Pulses=130;Pulses(uSec)=225,175,150,1250,150,200,150,1250,150,200,150,1250,150,200,150,1250,150,200,150,1275,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,1275,150,225,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,1275,150,225,150,1275,150,225,150,200,150,1300,150,1275,150,225,150,1275,150,225,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,1275,150,200,150;
 * 20;41;DEBUG;Pulses=126;Pulses(uSec)=225,200,125,1250,150,200,150,1250,150,175,150,1250,150,175,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,1300,150,225,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,1275,150,200,150,1275,150,225,150,200,150,1300,150,1275,150,225,150,1275,150,225,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,1275,150,200,150;
 * 20;42;DEBUG;Pulses=116;Pulses(uSec)=175,1275,150,1225,150,200,150,200,150,200,150,1250,150,1250,150,1300,150,1275,150,200,150,200,150,200,150,1275,150,1300,150,1300,150,1300,150,225,150,1300,150,225,150,225,150,1300,150,1300,150,1300,150,200,150,1300,150,200,150,1300,150,1275,150,225,150,1275,150,225,150,200,150,225,150,1300,150,1300,150,225,150,1300,150,200,150,200,150,200,150,1275,150,1300,150,1300,150,1300,150,200,150,200,150,200,150,1300,150,1300,150,1300,150,1300,150,200,150,200,150,1275,150,225,150,1275,150,1300,150;
 
 20;CC;DEBUG;Pulses=58;Pulses(uSec)=300,950,225,950,900,275,225,950,225,950,875,275,225,950,225,950,225,950,900,275,875,275,225,950,225,950,225,950,225,950,225,950,225,950,225,950,225,950,225,950,875,275,225,950,225,950,225,950,875,275,875,275,875,250,225,950,225;
20;CD;DEBUG;Pulses=64;Pulses(uSec)=525,250,200,900,200,4900,225,900,200,925,875,275,225,950,225,950,875,275,225,950,225,950,225,950,900,275,875,275,225,950,225,950,225,950,225,950,225,950,225,950,225,950,225,950,225,950,875,275,225,950,225,950,225,950,875,275,875,275,900,250,225,950,225;
20;CE;DEBUG;Pulses=130;Pulses(uSec)=225,200,150,1250,150,200,150,1250,150,200,150,1250,150,200,150,1250,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,1275,150,200,150,200,150,1300,150,200,175,1300,150,200,150,1300,150,1275,150,200,150,1275,150,200,150,200,150,1300,150,1275,150,200,150,1275,150,200,150,200,150,1300,150,200,175,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,1275,150,200,150;
20;CF;DEBUG;Pulses=126;Pulses(uSec)=225,200,150,1250,150,200,150,1250,150,200,150,1250,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,1275,150,200,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,1275,150,200,150,1275,150,200,150,200,175,1300,150,1275,150,200,150,1300,150,200,150,200,150,1300,150,200,150,1300,150,200,150,1275,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,200,150,1300,150,1275,150,200,150;
20;D0;DEBUG;Pulses=116;Pulses(uSec)=275,1250,150,1250,150,200,150,200,150,200,150,1250,150,1250,150,1300,150,1300,150,200,150,200,150,200,150,1300,150,1275,150,1300,150,1300,150,200,150,1300,150,200,150,200,150,1300,150,1300,150,1275,150,200,150,1275,150,225,150,1275,150,1300,150,225,150,1300,150,200,150,200,150,225,150,1300,150,1300,150,200,175,1275,175,200,150,200,150,200,150,1300,150,1275,150,1300,150,1275,175,200,150,200,150,200,150,1300,150,1300,150,1300,150,1275,150,200,150,200,175,1300,150,200,150,1300,150,1300,150;
20;D1;HomeEasy;ID=7a75a347;SWITCH=0b;CMD=ALLON;
20;D2;DEBUG;Pulses=50;Pulses(uSec)=3200,875,300,875,300,875,300,875,300,875,300,875,300,875,300,875,300,875,300,875,300,875,300,900,275,875,300,875,300,875,925,250,300,875,300,875,300,875,925,250,300,875,925,250,300,875,300,875,300;
20;D3;Kaku;ID=5f;SWITCH=20;CMD=OFF;
20;D4;DEBUG;Pulses=50;Pulses(uSec)=3150,875,300,875,300,875,300,875,300,875,300,875,300,875,300,875,300,875,300,875,300,875,300,875,300,875,300,875,300,875,925,250,300,875,300,875,300,875,900,250,300,875,925,250,300,875,300,875,300;
20;D5;Kaku;ID=5f;SWITCH=20;CMD=OFF;


20;CC;DEBUG;Pulses=58;Pulses(uSec)=300,950,225,950,900,275,225,950,225,950,875,275,225,950,225,950,225,950,900,275,875,275,225,950,225,950,225,950,225,950,225,950,225,950,225,950,225,950,225,950,875,275,225,950,225,950,225,950,875,275,875,275,875,250,225,950,225;
20;86;DEBUG;Pulses=58;Pulses(uSec)=250,4500,225,800,225,800,825,200,225,800,225,800,825,200,225,800,225,800,825,200,825,200,225,800,825,200,225,825,225,800,825,200,825,200,225,800,225,825,225,800,825,200,825,200,225,800,225,825,825,200,825,200,225,800,825,200,225;
 300,4500,225,800,225,825,825,200,225,800,225,800,825,200,825,200,225,800,825,200,225,800,825,200,825,200,825,200,225,800,825,200,825,200,825,200,225,825,225,800,825,200,825,200,225,800,225,825,225,800,225,800,825,200,825,200,225;
00010011010111011100110000110
 0010010010100011111101011110 

 20;D5;DEBUG;Pulses=364;Pulses(uSec)=
 250,4875,200,900,200,900,875,275,200,900,200,900,875,275,200,900,875,250,225,900,875,275,875,250,225,900,200,925,200,925,200,900,875,250,850,250,225,925,875,250,875,250,225,900,850,275,200,900,875,250,225,900,875,250,875,250,225,900,
 200,4875,200,900,225,900,850,250,200,900,225,900,850,275,200,900,875,250,225,900,875,275,850,250,200,900,200,900,200,925,200,900,875,250,875,250,200,925,850,250,875,275,200,900,875,275,200,900,875,250,225,900,875,250,850,250,200,900,
 200,4875,200,900,225,900,875,275,200,925,200,900,875,275,200,900,875,250,225,900,875,275,850,275,200,925,200,900,225,925,200,900,875,250,875,250,225,925,875,250,875,250,225,900,875,275,200,900,850,275,200,900,875,250,850,250,200,900,
 200,4875,200,900,225,925,850,250,225,900,200,900,850,275,225,900,850,275,200,900,875,275,875,250,200,900,225,900,200,925,200,900,875,250,875,250,225,925,850,250,875,250,225,900,875,250,225,900,875,250,200,900,875,250,875,250,200,925,
 150,2650,150,200,150,1250,150,200,150,1250,150,200,150,1250,150,200,150,1250,150,200,150,1250,150,200,150,1250,150,200,150,1225,150,200,150,1225,150,200,150,1250,150,1225,150,200,150,200,150,1250,150,200,150,1250,150,200,150,1250,150,1225,150,225,150,1225,150,200,150,200,150,1250,150,1225,150,200,150,1225,150,200,150,200,150,1250,150,200,150,1250,150,200,150,1250,150,200,150,1225,150,200,150,1250,150,200,150,1225,150,200,150,1225,150,200,150,1250,150,200,150,1250,150,1225,150,200,150,200,150,1250,150,200,150,1250,150,200,150,1250,150,1225,150,200,150;

 \*********************************************************************************************/
#define FA500_PLUGIN_ID 012
#define FA500RM3_PulseLength 26
#define FA500RM1_PulseLength 58

#define FA500_PULSEMID 400 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_012
#include "../4_Display.h"

boolean Plugin_012(byte function, char *string)
{
   if (RawSignal.Number != (FA500RM3_PulseLength) && RawSignal.Number != (FA500RM1_PulseLength))
      return false;

   byte type = 0; // 0=KAKU 1=ITK 2=PT2262
   byte housecode = 0;
   byte unitcode = 0;
   byte command = 0;
   unsigned long bitstream = 0L;
   unsigned long address = 0L;
   //==================================================================================
   // Get all 28 bits
   //==================================================================================
   if (RawSignal.Number == (FA500RM3_PulseLength))
   {
      // get all 26pulses =>24 manchester bits => 12 actual bits
      type = 0;
      for (byte x = 2; x <= FA500RM3_PulseLength - 2; x += 2)
      {                   // Method 3
         bitstream <<= 1; // Always shift
         if (RawSignal.Pulses[x] > FA500_PULSEMID)
            bitstream |= 0x1;
         // else
         //    bitstream |= 0x0;
      }
   }
   else
   {
      // get all 58pulses =>28 bits
      type = 1;
      for (byte x = 1; x <= FA500RM1_PulseLength - 2; x += 2)
      {                   // method 1
         bitstream <<= 1; // Always shift
         if (RawSignal.Pulses[x] > FA500_PULSEMID)
            bitstream |= 0x1;
         // else
         //    bitstream |= 0x0;
      }
   }
   //==================================================================================
   // perform sanity checks
   //==================================================================================
   if (bitstream == 0)
      return false; // no bits detected?
   
   if (type == 0)
   {
      housecode = (((bitstream) >> 8) & 0x0F);
      unitcode = ((bitstream >> 1) & 0x7F);
      if (unitcode != 0x0A)
      { // invalid housecode?
         return false;
      }
      address = housecode;
      command = (bitstream)&1;
   }
   else
   {
      // 001001 0010100011111101 0111 10
      // ^^^^^^                       ^^
      address = bitstream;
      address = address >> 22;
      address = address << 2;
      housecode = (bitstream)&3;
      address = address + housecode;
      // sort buttons based on first 6 bits and last 2 bits
      if (address == 0x26)
      { // A On/off
         housecode = 1;
      }
      else if (address == 0x25)
      { // B On/off
         housecode = 4;
      }
      else if (address == 0xE5)
      { // C On/off
         housecode = 5;
      }
      else if (address == 0x66)
      { // D On/off
         housecode = 0;
      }
      else
      {
         return false;
      }
      command = 2; // initialize to "unknown"
      // Trick: here we use the on/off command from the other packet type as it is not detected in the current packet, it was passed via Pluses[0] in plugin 1
      if (RawSignal.Pulses[0] * RawSignal.Multiply > 1000 && RawSignal.Pulses[0] * RawSignal.Multiply < 1400)
      {
         command = 0;
      }
      else if (RawSignal.Pulses[0] * RawSignal.Multiply > 100 && RawSignal.Pulses[0] * RawSignal.Multiply < 400)
      {
         command = 1;
      }
      address = bitstream;
   }
   if (command > 1)
   {
      //Serial.println("FA500R error3");
      return false;
   }
   if (housecode != 0x01 && housecode != 0x04 && housecode != 0x05 && housecode != 0x00)
   { // invalid button code?
      //Serial.println("FA500R error4");
      return false;
   }
   //==================================================================================
   //      if (housecode == 1) housecode = 0x41; // A 0001 0001010 0/1     08   A    1
   //      if (housecode == 4) housecode = 0x42; // B 0100 0001010 0/1     20   B    4
   //      if (housecode == 5) housecode = 0x43; // C 0101 0001010 0/1     28   C    5
   //      if (housecode == 0) housecode = 0x44; // D 0000 0001010 0/1     00   D    0
   //==================================================================================
   // Output
   //==================================================================================
   display_Header();
   display_Name(PSTR("FA500"));

   if (type == 0)
   {
      display_IDn(((unitcode << 8) | housecode), 8); // "%02x%02x"

      char c_SWITCH[4];
      sprintf(c_SWITCH, "%02x%02x", unitcode, housecode);
      display_SWITCHc(c_SWITCH); // "%02x%02x"
   }
   else
   {
      display_IDn(address, 8);   // "%S%08lx"
      display_SWITCH(housecode); // "%02x"
   }

   switch (command)
   {
   case 0x00:
   case 0x01:
      display_CMD(false, (command & B01)); // #ALL , #ON
      break;
   default:
      display_Name(PSTR(";CMD=UNKOWN"));
   }
   display_Footer();
   // ----------------------------------
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   return true;
}
#endif //PLUGIN_012

#ifdef PLUGIN_TX_012
void Flamingo_Send(int funitc, int fcmd);

boolean PluginTX_012(byte function, char *string)
{
   boolean success = false;
   //10;FA500;001b523;D3;ON;
   //012345678901234567890123
   if (strncasecmp(InputBuffer_Serial + 3, "FA500;", 6) == 0)
   { // FA500 Command
      if (InputBuffer_Serial[16] != ';')
         return false;
      unsigned long bitstream = 0L;
      byte Home = 0;
      byte c;

      InputBuffer_Serial[7] = 0x30;
      InputBuffer_Serial[8] = 0x78;
      bitstream = str2int(InputBuffer_Serial + 7); // get address

      c = tolower(InputBuffer_Serial[17]); // 1..5
      if (c >= '1' && c <= '5')
      {
         Home = Home + c - '0';
      }

      c = 0;
      c |= str2cmd(InputBuffer_Serial + 20) == VALUE_OFF; // ON/OFF command
      Flamingo_Send(bitstream, c);
      success = true;
   }
   return success;
}

void Flamingo_Send(int fbutton, int fcmd)
{
   int fpulse = 350; // Pulse witdh in microseconds
   int fretrans = 9; // Number of code retransmissions
   uint32_t fdatabit;
   uint32_t fdatamask = 0x80000000;
   uint32_t fsendbuff;
   uint32_t fsendbuff1;
   uint32_t fsendbuff2;
   uint32_t fsendbuff3;
   uint32_t fsendbuff4;

   if (fcmd == 0)
   {                           // OFF
      fsendbuff1 = 0x24D319A0; // A Off
      fsendbuff2 = 0x246008E0;
      fsendbuff3 = 0x26BB9860;
      fsendbuff4 = 0x26D4BFA0;
      //fsendbuff1=0xD86E6650;
      //fsendbuff2=0xDABDF710;
      //fsendbuff3=0xDA42A790;
      //fsendbuff4=0xDA614050;
   }
   else
   {                           // ON
      fsendbuff1 = 0x2561B560; // A On
      fsendbuff2 = 0x24A3F5E0;
      fsendbuff3 = 0x27B27B60;
      fsendbuff4 = 0x24543A20;
      //fsendbuff1=0xD97A4A10;
      //fsendbuff2=0xDA9A8490;
      //fsendbuff3=0xDB58C5D0;
      //fsendbuff4=0xDBF40A90;
   }
   digitalWrite(PIN_RF_RX_VCC, LOW);            // Spanning naar de RF ontvanger uit om interferentie met de zender te voorkomen.
   digitalWrite(PIN_RF_TX_VCC, HIGH);           // zet de 433Mhz zender aan
   delayMicroseconds(TRANSMITTER_STABLE_DELAY); // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)

   for (int nRepeat = 0; nRepeat < fretrans; nRepeat++)
   {

      if (nRepeat % 4 == 0)
         fsendbuff = fsendbuff1;
      if (nRepeat % 4 == 1)
         fsendbuff = fsendbuff2;
      if (nRepeat % 4 == 2)
         fsendbuff = fsendbuff3;
      if (nRepeat % 4 == 3)
         fsendbuff = fsendbuff4;

      Serial.println(fsendbuff, HEX);

      // send SYNC 1P High, 15P low
      digitalWrite(PIN_RF_TX_DATA, HIGH);
      delayMicroseconds(fpulse * 1);
      digitalWrite(PIN_RF_TX_DATA, LOW);
      delayMicroseconds(fpulse * 15);
      // end send SYNC

      // Send command
      for (int i = 0; i < 28; i++)
      { // Flamingo command is only 28 bits
         // read data bit
         fdatabit = fsendbuff & fdatamask; // Get most left bit
         fsendbuff = (fsendbuff << 1);     // Shift left

         if (fdatabit != fdatamask)
         { // Write 0
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 1);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 3);
         }
         else
         { // Write 1
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 3);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 1);
         }
      }
      //digitalWrite(PIN_RF_TX_DATA, LOW);
      //delayMicroseconds(fpulse * 15);
   }
   delayMicroseconds(TRANSMITTER_STABLE_DELAY); // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)
   digitalWrite(PIN_RF_TX_VCC, LOW);            // zet de 433Mhz zender weer uit
   digitalWrite(PIN_RF_RX_VCC, HIGH);           // Spanning naar de RF ontvanger weer aan.
   RFLinkHW();
}
#endif //PLUGIN_TX_012
