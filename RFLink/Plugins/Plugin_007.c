//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                      Plugin-007 CONRAD RSL2                                       ##
//#######################################################################################################
/*********************************************************************************************\
 * This Plugin takes care of receiving of the Conrad RSL2 protocol
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
 * RF packets contain 66 pulses, 32 bits
 *
 * Conrad RSL2 Message Format: 
 * AABBCDDD EEEEEEEE EEEEEEEE EEEEEEEE
 *
 * A = always 10
 * B = Button code
 * C = on/off command (inverted for some buttons/groups)
 * D = group code
 * E = 24 bit address
 *
 * Details: http://www.mikrocontroller.net/topic/252895
 *
 * ConradSend address,switch,cmd;  => (16 bits,8 bits,on/off/allon/alloff)
 
 20;5B;DEBUG;Pulses=66;Pulses(uSec)=400,1200,350,1200,350,1200,350,1200,350,1225,350,1200,350,1200,350,1200,350,1200,350,1200,350,1200,350,1200,350,1200,350,1225,350,1200,350,1200,1200,350,350,1225,350,1200,1200,350,350,1200,350,1200,1200,350,350,1200,350,1200,350,1200,350,1200,350,1200,350,1200,350,1200,350,1200,350,1200,350;

 400,1200,350,1200,350,1200,350,1200,350,1225,350,1200,350,1200,350,1200,350,1200,350,1200,
 350,1200,350,1200,350,1200,350,1225,350,1200,350,1200,1200,350,350,1225,350,1200,1200,350,
 350,1200,350,1200,1200,350,350,1200,350,1200,350,1200,350,1200,350,1200,350,1200,350,1200,
 350,1200,350,1200,350;
 
20;3D;DEBUG;Pulses=68;Pulses(uSec)=600,6450,1290,330,390,1260,390,1260,360,1260,360,1260,390,1260,390,1260,1290,360,1260,360,1290,360,1290,330,1290,360,1260,360,1260,360,1290,330,1290,360,360,1260,360,1260,390,1260,390,1260,360,1260,1290,360,1260,360,360,1260,390,1260,360,1260,360,1260,360,1260,390,1260,1290,360,1290,360,1260,360,390,6990;
20;3E;DEBUG;Pulses=68;Pulses(uSec)=720,6450,1290,330,390,1260,360,1260,360,1260,390,1260,390,1260,360,1260,1260,360,1290,330,1290,330,1290,360,1260,360,1260,360,1290,330,1290,360,1290,360,360,1260,360,1260,390,1260,360,1260,360,1260,1260,360,1290,330,390,1260,360,1260,360,1260,360,1260,390,1260,390,1260,1260,360,1260,360,1290,330,390,6990;
20;3F;DEBUG;Pulses=68;Pulses(uSec)=630,6450,1290,360,360,1260,360,1260,390,1260,390,1260,360,1260,360,1260,1290,330,1290,330,1290,360,1260,360,1290,330,1290,330,1290,360,1260,360,1260,360,390,1260,390,1260,360,1260,360,1260,360,1260,1290,330,1290,360,360,1260,360,1260,390,1260,390,1260,360,1260,360,1260,1290,330,1290,330,1290,360,360,6990;

 \*********************************************************************************************/
#define CONRADRSL2_PULSECOUNT 66
#define CONRADRSL2_PULSEMID 600 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_007
#include "../4_Misc.h"

boolean Plugin_007(byte function, char *string)
{
   if ((RawSignal.Number != CONRADRSL2_PULSECOUNT) && (RawSignal.Number != CONRADRSL2_PULSECOUNT + 2))
      return false;
   unsigned long bitstream = 0L;
   byte checksum = 0;
   byte command = 0;
   byte button = 0;
   byte group = 0;
   byte action = 0;
   byte start = 0;
   if (RawSignal.Number == CONRADRSL2_PULSECOUNT + 2)
   {
      start = 2;
   }
   //==================================================================================
   // get bits
   for (byte x = 1 + start; x < RawSignal.Number - 2; x = x + 2)
   {
      if (RawSignal.Pulses[x] > CONRADRSL2_PULSEMID)
      {
         if (RawSignal.Pulses[x + 1] > CONRADRSL2_PULSEMID)
            return false;                    // manchester check
         bitstream = (bitstream << 1) | 0x1; // 1
      }
      else
      {
         if (RawSignal.Pulses[x + 1] < CONRADRSL2_PULSEMID)
            return false;              // manchester check
         bitstream = (bitstream << 1); // 0
      }
   }
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   if (SignalHash != SignalHashPrevious || RepeatingTimer < millis())
   {
      // not seen the RF packet recently
      if (bitstream == 0)
         return false; // sanity check
   }
   else
   {
      // already seen the RF packet recently
      return true;
   }
   //==================================================================================
   // all bits received, make sure checksum is okay
   //==================================================================================
   checksum = ((bitstream >> 30) & B11); // first two bits should always be '10'
   if (checksum != B10)
      return false;
   //==================================================================================
   command = ((bitstream >> 24) & 0xFF); // 10100011
   // ----- check for possible valid values
   if (command < 0x81)
      return false;
   if (command > 0xBE)
      return false;
   byte temp = (command & 0xF);
   if (temp == 0x07 || temp == 0x0b || temp == 0x0f)
      return false;
   if (command == 0x83 || command == 0x86 || command == 0x89 || command == 0x8c)
      return false;
   if (command == 0x91 || command == 0x94 || command == 0x9a || command == 0x9d)
      return false;
   if (command == 0xa1 || command == 0xa4 || command == 0xaa || command == 0xad)
      return false;
   if (command == 0xb1 || command == 0xb3 || command == 0xb4 || command == 0xba || command == 0xbd)
      return false;
   // -----
   group = (command & 0x7);         // --   111
   action = ((command >> 3) & 0x1); // --  a
   button = ((command >> 4) & 0x3); // --bb
   // -----
   if (group == 0x3)
   {
      action = button;
      button = 0;
   }
   else
   {
      if (button == 3)
      {
         if (group == 0x6)
         { // toggle command bit
            button = 0;
            action = (~action) & 1;
         }
         if (group == 0x1 || group == 0x5)
         { // no toggle
            button = 4;
         }
         if (group == 0x0)
         {
            action = (~action) & 1; // toggle command bit
            button = 8;
         }
         if (group == 0x2 || group == 0x4)
         { // no toggle
            button = 12;
         }
      }
      else if (button == 0)
      {
         if (group == 0x6 || group == 0x1)
         { // no toggle
            button = 1;
         }
         if (group == 0x5)
         { // toggle command bit
            button = 5;
            action = (~action) & 1;
         }
         if (group == 0x0 || group == 0x4)
         { // no toggle
            button = 9;
         }
         if (group == 0x2)
         { // toggle command bit
            button = 13;
            action = (~action) & 1;
         }
      }
      else if (button == 2)
      {
         if (group == 0x6)
         { // toggle command bit
            button = 2;
            action = (~action) & 1;
         }
         if (group == 0x1 || group == 0x5)
            button = 6; //

         if (group == 0x0)
         { // toggle command bit
            button = 10;
            action = (~action) & 1;
         }

         if (group == 0x2 || group == 0x4)
         { // no toggle
            button = 14;
            //action=(~action)&1;
         }
      }
      else if (button == 1)
      {
         if (group == 0x6)
         { // toggle command bit
            button = 3;
            action = (~action) & 1;
         }
         if (group == 0x1 || group == 0x5)
         { // no toggle
            button = 7;
            //action=(~action)&1;
         }
         if (group == 0x0)
         { // toggle command bit
            button = 11;
            action = (~action) & 1;
         }
         if (group == 0x2 || group == 0x4)
         { // no toggle
            button = 15;
            //action=(~action)&1;
         }
      }
   }
   //==================================================================================
   // Output
   // ----------------------------------
   display_Header();
   display_Name(PSTR("Conrad"));
   display_IDn((bitstream & 0xFFFFFF), 6);     //"%S%06lx"
   display_SWITCH(button);                     // %02x to %02d
   display_CMD((group == 0x3), (action == 1)); // #ALL , #ON
   display_Footer();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // PLUGIN_007

#ifdef PLUGIN_TX_007
void RSL2_Send(unsigned long address);

boolean PluginTX_007(byte function, char *string)
{
   boolean success = false;
   //10;CONRAD;000fa0;0;OFF;
   //10;CONRAD;009200;1;ON;
   //10;CONRAD;ff0607;1;OFF;
   //012345678901234567890123
   if (strncasecmp(InputBuffer_Serial + 3, "CONRAD;", 7) == 0)
   { // KAKU Command eg.
      unsigned long bitstream = 0L;
      unsigned long command = 0L;
      if (InputBuffer_Serial[16] != ';')
         return success;
      InputBuffer_Serial[8] = 0x30;
      InputBuffer_Serial[9] = 0x78;
      InputBuffer_Serial[16] = 0;
      bitstream = str2int(InputBuffer_Serial + 8); // Address

      byte temp = str2int(InputBuffer_Serial + 17); // het button/unit number (0x00..0x0f)
      if (temp < 16)
      {                                               // No button with a number higher than 15
         byte cmd = str2cmd(InputBuffer_Serial + 19); // ON/OFF
         if (cmd == VALUE_OFF)
         {
            if (temp == 0)
               command = 0xbe;
            if (temp == 1)
               command = 0x81;
            if (temp == 2)
               command = 0xae;
            if (temp == 3)
               command = 0x9e;
            if (temp == 4)
               command = 0xb5;
            if (temp == 5)
               command = 0x8d;
            if (temp == 6)
               command = 0xa5;
            if (temp == 7)
               command = 0x95;
            if (temp == 8)
               command = 0xb8;
            if (temp == 9)
               command = 0x84;
            if (temp == 10)
               command = 0xa8;
            if (temp == 11)
               command = 0x98;
            if (temp == 12)
               command = 0xb2;
            if (temp == 13)
               command = 0x8a;
            if (temp == 14)
               command = 0xa2;
            if (temp == 15)
               command = 0x92;
         }
         else // ON
             if (cmd == VALUE_ON)
         {
            if (temp == 0)
               command = 0xb6;
            if (temp == 1)
               command = 0x8e;
            if (temp == 2)
               command = 0xa6;
            if (temp == 3)
               command = 0x96;
            if (temp == 4)
               command = 0xb9;
            if (temp == 5)
               command = 0x85;
            if (temp == 6)
               command = 0xa9;
            if (temp == 7)
               command = 0x99;
            if (temp == 8)
               command = 0xb0;
            if (temp == 9)
               command = 0x88;
            if (temp == 10)
               command = 0xa0;
            if (temp == 11)
               command = 0x90;
            if (temp == 12)
               command = 0xbc;
            if (temp == 13)
               command = 0x82;
            if (temp == 14)
               command = 0xac;
            if (temp == 15)
               command = 0x9c;
         }
         else // AllON
             if (cmd == VALUE_ALLON)
         {
            command = 0x93;
         }
         else // AllOff
             if (cmd == VALUE_ALLOFF)
         {
            command = 0xa3;
         }
         command = command << 24;
         bitstream = bitstream + command;
      }
      RSL2_Send(bitstream); // the full bitstream to send
      success = true;
   }
   return success;
}

void RSL2_Send(unsigned long address)
{
   int fpulse = 650;  // Pulse witdh in microseconds 650?
   int fpulse2 = 450; // Pulse witdh in microseconds 650?
   int fretrans = 4;  // Number of code retransmissions
   uint32_t fdatabit;
   uint32_t fdatamask = 0x80000000;
   uint32_t fsendbuff;

   digitalWrite(PIN_RF_RX_VCC, LOW);            // Spanning naar de RF ontvanger uit om interferentie met de zender te voorkomen.
   digitalWrite(PIN_RF_TX_VCC, HIGH);           // zet de 433Mhz zender aan
   delayMicroseconds(TRANSMITTER_STABLE_DELAY); // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)

   for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++)
   {
      fsendbuff = address;

      // send SYNC 1P High, 10P low
      digitalWrite(PIN_RF_TX_DATA, HIGH);
      delayMicroseconds(fpulse * 1);
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
            delayMicroseconds(fpulse2 * 1);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse2 * 3);
         }
         else
         { // Write 1
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse2 * 3);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse2 * 1);
         }
      }
      digitalWrite(PIN_RF_TX_DATA, HIGH);
      delayMicroseconds(fpulse2 * 1);
      digitalWrite(PIN_RF_TX_DATA, LOW);
      delayMicroseconds(fpulse * 14);
   }
   delayMicroseconds(TRANSMITTER_STABLE_DELAY); // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)
   digitalWrite(PIN_RF_TX_VCC, LOW);            // zet de 433Mhz zender weer uit
   digitalWrite(PIN_RF_RX_VCC, HIGH);           // Spanning naar de RF ontvanger weer aan.
   RFLinkHW();
}
#endif // PLUGIN_TX_007
