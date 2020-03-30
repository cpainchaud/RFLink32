//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-34 Cresta                                           ##
//#######################################################################################################
/*********************************************************************************************\
 * Dit protocol zorgt voor ontvangst van Cresta temperatuur weerstation buitensensoren 
 * Tevens alle sensoren die het Cresta (Hideki) protocol volgen waaronder:
 * Hideki, TFA Nexus, Mebus, Irox, Irox-Pro X, Honeywell, Cresta TE923, TE923W, TE821W,  
 * WXR810, DV928, Ventus W906
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
 * Technical Information:
 * Decodes signals from a Cresta Weatherstation outdoor unit, (29 pulses, 28 bits, 433 MHz).
 * Message Format can be found at: http://members.upc.nl/m.beukelaar/Crestaprotocol.pdf
 * Thermo/Hygro: 10 bytes, seen as 128, 132, 134, 136, 138 pulses
 * Anemometer:   14 bytes
 * UV Index:     11 bytes
 * Rain:          9 bytes, seen as 132, 136 pulses
 \*********************************************************************************************/
#define CRESTA_MIN_PULSECOUNT 124 // unknown until we have a collection of all packet types but this seems to be the minimum
#define CRESTA_MAX_PULSECOUNT 284 // unknown until we have a collection of all packet types
#define CRESTA_PULSEMID 700 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_034
byte Plugin_034_reverseBits(byte data);
byte Plugin_034_WindDirSeg(byte data);

boolean Plugin_034(byte function, char *string)
{
   if ((RawSignal.Number < CRESTA_MIN_PULSECOUNT) || (RawSignal.Number > CRESTA_MAX_PULSECOUNT))
      return false;

   int sensor_data = 0;
   int windtemp = 0;
   int windchill = 0;
   int windgust = 0;
   int windspeed = 0;
   int winddirection = 0;
   int uv = 0;

   byte checksum = 0;
   byte data[18];
   byte length = 0;
   byte channel = 0;
   int units = 0;
   byte battery = 0;

   byte bytecounter = 0;  // used for counting the number of received bytes
   byte bitcounter = 0;   // counts number of received bits (converted from pulses)
   int pulseposition = 1; // first pulse is always empty
   byte halfbit = 0;      // high pulse = 1, 2 low pulses = 0, halfbit keeps track of low pulses
   byte parity = 0;       // to calculate byte parity
   // ==================================================================================
   // get bytes and determine if byte parity is set correctly for the cresta protocol on the fly
   do
   {
      //if(RawSignal.Pulses[pulseposition]*RawSignal.Multiply > 700) { // high value = 1 bit
      if (RawSignal.Pulses[pulseposition] > CRESTA_PULSEMID)
      { // high value = 1 bit
         if (halfbit == 1)
         {                // cant receive a 1 bit after a single low value
            return false; // pulse error, must not be a Cresta packet or reception error
         }
         if (bitcounter == 8)
         {
            if (parity != 1)
            {                // now receiving parity bit
               return false; // parity error, must not be a Cresta packet or reception error
            }
            else
            {
               bitcounter = 0; // reset for next byte
               parity = 0;     // reset for next byte
               halfbit = 0;    // wait for next first low or high pulse
               bytecounter++;  // 1 byte received
            }
         }
         else
         {
            data[bytecounter] = (data[bytecounter] << 1) | 0x1; // 1 bit
            parity = parity ^ 1;                                // update parity
            bitcounter++;                                       // received a bit
            halfbit = 0;                                        // waiting for first low or a new high pulse
         }
      }
      else
      {
         if (halfbit == 0)
         {               // 2 times a low value = 0 bit
            halfbit = 1; // first half received
         }
         else
         {
            if (bitcounter == 8)
            {
               if (parity != 0)
               {                // now receiving parity bit
                  return false; // parity error, must not be a Cresta packet or reception error
               }
               else
               {
                  bitcounter = 0; // reset for next byte
                  parity = 0;     // reset for next byte
                  halfbit = 0;    // wait for next first low or high pulse
                  bytecounter++;  // 1 byte received
               }
            }
            else
            {
               data[bytecounter] = (data[bytecounter] << 1); // 0 bit
               parity = parity ^ 0;                          // update parity
               bitcounter++;                                 // received a bit
               halfbit = 0;                                  // wait for next first low or high pulse
            }
         }
      }
      pulseposition++; // point to next pulse
      if (pulseposition > RawSignal.Number)
         break;                // reached the end? done processing
   } while (bytecounter < 16); // receive maximum number of bytes from pulses
   // ==================================================================================
   // all bytes received, make sure checksum is okay
   // ==================================================================================
   for (byte i = 0; i < bytecounter; i++)
   {
      data[i] = Plugin_034_reverseBits(data[i]);
   }
   // get packet length
   length = data[2] & 0x3f; // drop bits 6 and 7
   length >>= 1;            // drop bit 0
   if (length > 20)
      return false; // Additional check for illegal packet lengths to protect against false positives.
   if (length == 0)
      return false; // Additional check for illegal packet lengths to protect against false positives.
   // Checksum: XOR of all bytes from byte 1 till byte length+2, should result in 0
   checksum = 0;
   for (byte i = 1; i < length + 2; i++)
   {
      checksum = checksum ^ data[i];
   }
   if (checksum != 0)
      return false;
   // ==================================================================================
   // now process the various sensor types
   // ==================================================================================
   if (data[1] > 0x1f && data[1] < 0x40)
      channel = 1;
   if (data[1] > 0x3f && data[1] < 0x60)
      channel = 2;
   if (data[1] > 0x5f && data[1] < 0x80)
      channel = 3;
   if (data[1] > 0x7f && data[1] < 0xa0)
      channel = 1; // no channel settings on Anemometer/rainmeter and uvsensor
   if (data[1] > 0x9f && data[1] < 0xc0)
      channel = 4;
   if (data[1] > 0xbf && data[1] < 0xE0)
      channel = 5;
   data[3] = (data[3]) & 0x1f;
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   unsigned long tempval = data[3];
   tempval = ((tempval) << 16) + ((data[1]) << 8) + channel;
   if ((SignalHash != SignalHashPrevious) || (RepeatingTimer < millis() && SignalCRC != tempval) || (SignalCRC != tempval))
   {
      SignalCRC = tempval; // not seen the RF packet recently
   }
   else
   {
      return true; // already seen the RF packet recently
   }
   // ----------------------------------
   battery = (data[2]) >> 6;
   // ----------------------------------
   if (data[3] == 0x0c)
   { // Anemometer
      units = ((data[4] >> 4) * 10) + (data[4] & 0x0f);
      sensor_data = ((data[5] & 0x3f) * 100) + units;
      if ((data[5] & 0x80) != 0x80)
      {
         sensor_data = sensor_data | 0x8000; // set highest bit (minus bit)
      }
      windtemp = sensor_data;

      units = ((data[6] >> 4) * 10) + (data[6] & 0x0f);
      sensor_data = ((data[7] & 0x3f) * 100) + units;
      if ((data[7] & 0x80) != 0x80)
      {
         sensor_data = sensor_data | 0x8000; // set highest bit (minus bit)
      }
      windchill = sensor_data;

      windspeed = ((data[9] & 0x0F) * 100) + ((data[8] >> 4) * 10) + (data[8] & 0x0F);
      windgust = ((data[10] >> 4) * 100) + ((data[10] & 0x0F) * 10) + (data[9] >> 4);

      //windspeed = (data[9] << 8) + data[8];
      //windgust = (data[10] << 4) + ( (data[9] &0xf0) >> 4 );

      winddirection = Plugin_034_WindDirSeg(((data[11] & 0xf0) >> 4));
      winddirection = winddirection & 0x0f; // make sure we dont get overflows
      //==================================================================================
      // Output
      // ----------------------------------
      Serial.print("20;");
      PrintHexByte(PKSequenceNumber++);
      Serial.print(F(";Cresta;ID=")); // Label
      PrintHexByte(data[1]);
      PrintHexByte(channel);
      // ----------------------------------
      sprintf(pbuffer, ";WINDIR=%04d;", winddirection);
      Serial.print(pbuffer);
      sprintf(pbuffer, "WINSP=%04x;", windspeed);
      Serial.print(pbuffer);
      sprintf(pbuffer, "WINGS=%04x;", windgust);
      Serial.print(pbuffer);
      sprintf(pbuffer, "WINTMP=%04x;", windtemp);
      Serial.print(pbuffer);
      sprintf(pbuffer, "WINCHL=%04x;", windchill);
      Serial.print(pbuffer);
      if (battery != 0)
      {
         Serial.print(F("BAT=OK;")); // Label
      }
      else
      {
         Serial.print(F("BAT=LOW;")); // Label
      }
      Serial.println();
      //==================================================================================
   }
   else
       // ----------------------------------
       if (data[3] == 0x0d)
   { // UV Sensor
      units = ((data[4] >> 4) * 10) + (data[4] & 0x0f);
      sensor_data = ((data[5] & 0x3f) * 100) + units;
      if ((data[5] & 0x80) != 0x80)
      {
         sensor_data = sensor_data | 0x8000; // set highest bit (minus bit)
      }
      // UV sensor reports the temperature but does not report negative values!, skip temperature info?
      uv = ((data[8] & 0x0f) << 8) + data[7];
      //==================================================================================
      // Output
      // ----------------------------------
      Serial.print("20;");
      PrintHexByte(PKSequenceNumber++);
      Serial.print(F(";Cresta;ID=")); // Label
      PrintHexByte(data[1]);
      PrintHexByte(channel);
      // ----------------------------------
      sprintf(pbuffer, ";TEMP=%04x;", sensor_data);
      Serial.print(pbuffer);
      sprintf(pbuffer, "UV=%04x;", uv);
      Serial.print(pbuffer);
      if (battery != 0)
      {
         Serial.print(F("BAT=OK;")); // Label
      }
      else
      {
         Serial.print(F("BAT=LOW;")); // Label
      }
      Serial.println();
      //==================================================================================
   }
   else // 9F 80 CC 4E 00 00 66 64
       // ----------------------------------         // 9f 80 cc 4e 01 00 66 65
       if (data[3] == 0x0e)
   {                                          // Rain meter  // 9F 80 CC 4E 76 00 66 12
      sensor_data = (data[5] << 8) + data[4]; // 80=rain 4e=>0E = rain
      sensor_data = sensor_data * 7;          // 66 = always 66    rain units * 0.7 = mm.
      //==================================================================================
      // Output
      // ----------------------------------
      Serial.print("20;");
      PrintHexByte(PKSequenceNumber++);
      Serial.print(F(";Cresta;ID=")); // Label
      PrintHexByte(data[1]);
      PrintHexByte(channel);
      // ----------------------------------
      sprintf(pbuffer, ";RAIN=%04x;", sensor_data);
      Serial.print(pbuffer);
      if (battery != 0)
      {
         Serial.print(F("BAT=OK;"));
      }
      else
      {
         Serial.print(F("BAT=LOW;"));
      }
      Serial.println();
      //==================================================================================
   }
   else
       // ----------------------------------
       if (data[3] == 0x1e)
   { // Thermo/Hygro
      units = ((data[4] >> 4) * 10) + (data[4] & 0x0f);
      sensor_data = ((data[5] & 0x3f) * 100) + units;
      if ((data[5] & 0x80) != 0x80)
      {
         sensor_data = sensor_data | 0x8000; // set highest bit (minus bit)
      }
      //==================================================================================
      // Output
      // ----------------------------------
      Serial.print("20;");
      PrintHexByte(PKSequenceNumber++);
      Serial.print(F(";Cresta;ID=")); // Label
      PrintHexByte(data[1]);
      PrintHexByte(channel);
      // ----------------------------------
      sprintf(pbuffer, ";TEMP=%04x;", sensor_data);
      Serial.print(pbuffer);
      sprintf(pbuffer, "HUM=%02x;", data[6]);
      Serial.print(pbuffer);
      if (battery != 0)
      {
         Serial.print(F("BAT=OK;"));
      }
      else
      {
         Serial.print(F("BAT=LOW;"));
      }
      Serial.println();
      //==================================================================================
   }
   else
   {
      //==================================================================================
      // Output
      // ----------------------------------
      Serial.print("20;");
      PrintHexByte(PKSequenceNumber++);
      Serial.print(F(";Cresta;DEBUG;ID=")); // Label
      PrintHexByte(data[1]);
      PrintHexByte(channel);
      // ----------------------------------
      PrintHex8(data, length + 2);
      Serial.print(F(";"));
      Serial.println();
      //==================================================================================
   }
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}

// *********************************************************************************************
// * Reverse all bits in a byte
// *********************************************************************************************
byte Plugin_034_reverseBits(byte data)
{
   byte b = data;
   for (byte i = 0; i < 8; ++i)
   {
      data = (data << 1) | (b & 1);
      b >>= 1;
   }
   return data;
}

byte Plugin_034_WindDirSeg(byte data)
{
   // Encrypted using: a=-a&0xf; b=a^(a>>1);
   data ^= (data & 8) >> 1; /* Solve bit 2 */
   data ^= (data & 4) >> 1; /* Solve bit 1 */
   data ^= (data & 2) >> 1; /* Solve bit 0 */
   return -data & 0xf;
}
#endif // PLUGIN_034
