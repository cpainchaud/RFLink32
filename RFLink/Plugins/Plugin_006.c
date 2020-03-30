//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                          Plugin-006 Blyss & Avidsen                               ##
//#######################################################################################################
/*********************************************************************************************\
 * This Plugin takes care of receiving of the Blyss protocol
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
 * RF packets contain 106 pulses, 52 bits
 *
 * BLYSS Message Format: 
 * AAAAAAAA BBBBCCCC CCCCCCCC CCCCDDDD | EEEEFFFF FFFFGGGG GGGG
 *
 * A = Preamble, always 0xFE (0x32 in case of Avidsen)
 * B = Global Channel (A=0,B=1,C=2,D=3)
 * C = Address
 * D = sub channel (channel 1=8, 2=4, 3=2, 4=1, 5=3) all channels = 0
 * E = Light Status
 * F = Rolling Code (0x98 -> 0xDA -> 0x1E -> 0xE6 -> 0x67)
 * G = Time Stamp (random value?)
 *  
 * Details https://skyduino.wordpress.com/2012/07/19/hack-partie-2-reverse-engineering-des-interrupteurs-domotique-blyss/
 * https://barbudor.wiki.zoho.com/Système-domotique-Blyss-de-Castorama.html
 *
 * BlyssSend address,switch,cmd;  => (16 bits,8 bits,on/off/allon/alloff)
 * 20;0B;DEBUG;Pulses=106;Pulses(uSec)=2160,450,570,420,600,420,600,450,570,420,600,420,600,450,570,810,210,870,150,840,180,840,180,840,180,420,600,420,600,420,600,450,570,420,600,420,600,420,600,420,600,420,600,840,180,840,210,450,570,450,600,810,180,840,180,840,180,420,600,810,210,840,180,810,210,810,210,870,180,810,210,450,570,450,570,840,180,840,210,450,570,420,600,840,180,810,210,840,180,840,210,840,180,840,180,810,210,840,210,420,600,810,210,420,600,6990;
 * 20;0C;Blyss;ID=ff98;SWITCH=A1;CMD=OFF;
 \*********************************************************************************************/
#define BLYSS_PULSECOUNT 106
#define BLYSS_PULSEMID 500 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_006
boolean Plugin_006(byte function, char *string)
{
   if (RawSignal.Number != BLYSS_PULSECOUNT)
      return false;
   unsigned long bitstream = 0L;
   unsigned long bitstream1 = 0L;
   byte bitcounter = 0;
   byte checksum = 0;
   int type = 0;
   //==================================================================================
   // get bits
   for (byte x = 2; x < BLYSS_PULSECOUNT; x = x + 2)
   {
      if (RawSignal.Pulses[x] > BLYSS_PULSEMID)
      {
         if (bitcounter < 32)
         {
            bitstream = (bitstream << 1);
         }
         else
         {
            bitstream1 = (bitstream1 << 1);
         }
         bitcounter++;
      }
      else
      {
         if (bitcounter < 32)
         {
            bitstream = (bitstream << 1) | 0x1;
         }
         else
         {
            bitstream1 = (bitstream1 << 1) | 0x1;
         }
         bitcounter++;
      }
   }
   //==================================================================================
   // all bits received, make sure checksum is okay
   //==================================================================================
   checksum = ((bitstream) >> 24); // test preamble
   if ((checksum != 0xFE) && (checksum != 0x32))
      return false; // must be 0xFE/32
   if (checksum == 0x32)
      type = 1;
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   if (SignalHash != SignalHashPrevious || RepeatingTimer < millis())
   {
      // not seen the RF packet recently
   }
   else
   {
      // already seen the RF packet recently
      return true;
   }
   //==================================================================================
   byte status = ((bitstream1) >> 16) & 0x0f;
   if (status > 3)
      return false;
   byte channel = ((bitstream) >> 20) & 0x0f;
   unsigned int address = ((bitstream) >> 4) & 0xffff;
   byte subchan = (bitstream)&0xf;
   channel = channel + 0x41;
   if (subchan == 8)
   {
      subchan = 1;
   }
   else if (subchan == 4)
   {
      subchan = 2;
   }
   else if (subchan == 2)
   {
      subchan = 3;
   }
   else if (subchan == 1)
   {
      subchan = 4;
   }
   else if (subchan == 3)
   {
      subchan = 5;
   }
   //==================================================================================
   // Output
   // ----------------------------------
   sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number
   Serial.print(pbuffer);
   // ----------------------------------
   if (type == 0)
   {
      Serial.print(F("Blyss;")); // Label
   }
   else
   {
      Serial.print(F("Avidsen;")); // Label
   }
   sprintf(pbuffer, "ID=%04x;", address); // ID
   Serial.print(pbuffer);
   sprintf(pbuffer, "SWITCH=%c%d;", channel, subchan);
   Serial.print(pbuffer);
   Serial.print(F("CMD=")); // command
   if (status == 0)
      Serial.print(F("ON;"));
   if (status == 1)
      Serial.print(F("OFF;"));
   if (status == 2)
      Serial.print(F("ALLON;"));
   if (status == 3)
      Serial.print(F("ALLOFF;"));
   Serial.println();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // PLUGIN_006

#ifdef PLUGIN_TX_006
void Blyss_Send(unsigned long address, byte devtype);

boolean PluginTX_006(byte function, char *string)
{
   boolean success = false;
   //10;Avidsen;00ff98;A1;OFF;
   //012345678901234567890123456
   //10;Blyss;00ff98;A1;OFF;
   //012345678901234567890123456
   int offset = 0;
   if (strncasecmp(InputBuffer_Serial + 3, "AVIDSEN;", 8) == 0)
   { // Blyss Command eg.
      offset = 2;
   }
   if ((strncasecmp(InputBuffer_Serial + 3, "BLYSS;", 6) == 0) || (offset == 2))
   { // Blyss Command eg.
      unsigned long Bitstream = 0L;
      if (InputBuffer_Serial[15 + offset] != ';')
         return success; // check
      if (InputBuffer_Serial[18 + offset] != ';')
         return success; // check

      unsigned long Home = 0; // Blyss channel A..P
      byte Address = 0;       // Blyss subchannel 1..5
      byte c;
      byte subchan = 0; // subchannel

      InputBuffer_Serial[7 + offset] = 0x30;
      InputBuffer_Serial[8 + offset] = 0x78;
      InputBuffer_Serial[15 + offset] = 0;
      Bitstream = str2int(InputBuffer_Serial + 7 + offset); // get address

      c = tolower(InputBuffer_Serial[16 + offset]); // A..P
      if (c >= 'a' && c <= 'p')
      {
         Home = c - 'a';
      }
      c = tolower(InputBuffer_Serial[17 + offset]); // 1..5
      if (c >= '1' && c <= '5')
      {
         Address = Address + c - '0';
      }

      if (Address == 1)
         subchan = 0x80;
      if (Address == 2)
         subchan = 0x40;
      if (Address == 3)
         subchan = 0x20;
      if (Address == 4)
         subchan = 0x10;
      if (Address == 5)
         subchan = 0x30;

      Home = Home << 24;
      Bitstream = (Bitstream) << 8;
      Bitstream = Bitstream + subchan;
      Bitstream = Bitstream + Home;

      c = str2cmd(InputBuffer_Serial + 19 + offset); // ALL ON/OFF command
      if (c == VALUE_OFF)
      {
         Bitstream = Bitstream | 1;
      }
      else
      {
         if (c == VALUE_ALLOFF)
         {
            Bitstream = Bitstream | 3;
         }
         else if (c == VALUE_ALLON)
         {
            Bitstream = Bitstream | 2;
         }
      }
      Blyss_Send(Bitstream, offset); // Bitstream contains the middle part of the bitstream to send
      success = true;
   }
   return success;
}

void Blyss_Send(unsigned long address, byte devtype)
{
   int fpulse = 400; // Pulse witdh in microseconds
   int fretrans = 8; // Number of code retransmissions
   uint32_t fdatabit;
   uint32_t fdatamask = 0x800000;
   uint32_t fsendbuff;
   unsigned char RollingCode[] = {0x98, 0xDA, 0x1E, 0xE6, 0x67, 0x98};

   digitalWrite(PIN_RF_RX_VCC, LOW);            // Power off the RF receiver
   digitalWrite(PIN_RF_TX_VCC, HIGH);           // Turn on the RF transmitter
   delayMicroseconds(TRANSMITTER_STABLE_DELAY); // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)
   byte temp = (millis() & 0xff);               // used for the timestamp at the end of the RF packet
   for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++)
   {
      // send SYNC 1P low, 6P high
      digitalWrite(PIN_RF_TX_DATA, LOW);
      delayMicroseconds(fpulse);
      digitalWrite(PIN_RF_TX_DATA, HIGH);
      delayMicroseconds(fpulse * 6);
      // end send SYNC
      // --------------
      // Send preamble (0xfe) - 8 bits
      if (devtype == 0)
      {
         fsendbuff = 0x32;
      }
      else
      {
         fsendbuff = 0xfe;
      }
      fdatamask = 0x80;
      for (int i = 0; i < 8; i++)
      { // Preamble
         // read data bit
         fdatabit = fsendbuff & fdatamask; // Get most left bit
         fsendbuff = (fsendbuff << 1);     // Shift left
         if (fdatabit != fdatamask)
         { // Write 0
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 2);
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 1);
         }
         else
         { // Write 1
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 1);
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 2);
         }
      }
      // --------------
      fsendbuff = address;
      fdatamask = 0x8000000;
      // Send command (channel/address/status) - 28 bits
      for (int i = 0; i < 28; i++)
      {
         // read data bit
         fdatabit = fsendbuff & fdatamask; // Get most left bit
         fsendbuff = (fsendbuff << 1);     // Shift left
         if (fdatabit != fdatamask)
         { // Write 0
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 2);
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 1);
         }
         else
         { // Write 1
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 1);
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 2);
         }
      }
      // --------------
      // Send rolling code & timestamp - 16 bits
      fsendbuff = RollingCode[nRepeat];
      fsendbuff = (fsendbuff << 8) + temp;
      //fsendbuff=0x9800 + temp;
      fdatamask = 0x8000;
      for (int i = 0; i < 16; i++)
      {
         // read data bit
         fdatabit = fsendbuff & fdatamask; // Get most left bit
         fsendbuff = (fsendbuff << 1);     // Shift left
         if (fdatabit != fdatamask)
         { // Write 0
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 2);
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 1);
         }
         else
         { // Write 1
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 1);
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 2);
         }
      }
      // --------------
      digitalWrite(PIN_RF_TX_DATA, LOW);
      //delayMicroseconds(fpulse * 18);             // delay between RF retransmits
      delay(24); // delay 23.8 ms
   }
   delayMicroseconds(TRANSMITTER_STABLE_DELAY); // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)
   digitalWrite(PIN_RF_TX_VCC, LOW);            // turn off the RF transmitter
   digitalWrite(PIN_RF_RX_VCC, HIGH);           // power on the RF receiver
   RFLinkHW();
}
#endif // PLUGIN_TX_006
