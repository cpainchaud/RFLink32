//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                      Plugin-034: Cresta                                           ##
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
#define CRESTA_PLUGIN_ID 034
#define PLUGIN_DESC_034 "Cresta"

#define CRESTA_MIN_PULSECOUNT 124 // unknown until we have a collection of all packet types but this seems to be the minimum
#define CRESTA_MAX_PULSECOUNT 284 // unknown until we have a collection of all packet types

#define CRESTA_PULSEMID_D 700

#ifdef PLUGIN_034
#include "../4_Display.h"
#include "../7_Utils.h"

byte Plugin_034_WindDirSeg(byte data);

boolean Plugin_034(byte function, const char *string)
{
   if ((RawSignal.Number < CRESTA_MIN_PULSECOUNT) || (RawSignal.Number > CRESTA_MAX_PULSECOUNT))
      return false;

   #ifdef PLUGIN_034_DEBUG
   char tmpbuf[100];
   #endif

   const long CRESTA_PULSEMID = CRESTA_PULSEMID_D / RawSignal.Multiply;

   byte bytecounter = 0;  // used for counting the number of received bytes
   byte bitcounter = 0;   // counts number of received bits (converted from pulses)
   int pulseposition = 1; // first pulse is always empty
   byte halfbit = 0;      // high pulse = 1, 2 low pulses = 0, halfbit keeps track of low pulses
   byte parity = 0;       // to calculate byte parity

   byte checksum = 0;
   byte data[18];
   byte length = 0;
   byte channel = 0;
   byte battery = 0;

   int sensor_data = 0;
   int windtemp = 0;
   unsigned int windchill = 0;
   unsigned int windgust = 0;
   unsigned int windspeed = 0;
   unsigned int winddirection = 0;
   unsigned int uv = 0;
   //==================================================================================
   // Get all 28 bits
   // and determine if byte parity is set correctly
   // for the cresta protocol on the fly
   //==================================================================================
   do
   {
      if (RawSignal.Pulses[pulseposition] > CRESTA_PULSEMID)
      {                      // high value = 1 bit
         if (halfbit == 1) { // cant receive a 1 bit after a single low value
            #ifdef PLUGIN_034_DEBUG
            sprintf_P(tmpbuf, PSTR("Cresta: failed halfbit == 1 at pulse#=%i length=%i"), pulseposition, RawSignal.Pulses[pulseposition]*RawSignal.Multiply);
            RFLink::sendRawPrint(tmpbuf, true);
            #endif
            return false;    // pulse error, must not be a Cresta packet or reception error
         }

         if (bitcounter == 8)
         {
            if (parity != 1) {  // now receiving parity bit
              #ifdef PLUGIN_034_DEBUG
              sprintf_P(tmpbuf, PSTR("Cresta: failed parity != 1 at pulse#=%i length=%i"), pulseposition, RawSignal.Pulses[pulseposition]*RawSignal.Multiply);
              RFLink::sendRawPrint(tmpbuf, true);
              #endif
              return false;    // parity error, must not be a Cresta packet or reception error
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
            data[bytecounter] <<= 1;
            data[bytecounter] |= 0x1; // 1 bit
            parity ^= 1;              // update parity
            bitcounter++;             // received a bit
            halfbit = 0;              // waiting for first low or a new high pulse
         }
      }
      else
      {
         if (halfbit == 0) // 2 times a low value = 0 bit
            halfbit = 1;   // first half received
         else
         {
            if (bitcounter == 8)
            {
               if (parity != 0) { // now receiving parity bit
                  #ifdef PLUGIN_034_DEBUG
                  sprintf_P(tmpbuf, PSTR("Cresta: failed parity != 0 at pulse#=%i length=%i"), pulseposition, RawSignal.Pulses[pulseposition]*RawSignal.Multiply);
                  RFLink::sendRawPrint(tmpbuf, true);
                  #endif
                  return false;   // parity error, must not be a Cresta packet or reception error
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
               data[bytecounter] <<= 1; // 0 bit
               parity ^= 0;             // update parity
               bitcounter++;            // received a bit
               halfbit = 0;             // wait for next first low or high pulse
            }
         }
      }
      pulseposition++; // point to next pulse
      if (pulseposition > RawSignal.Number)
         break;                // reached the end? done processing
   } while (bytecounter < 16); // receive maximum number of bytes from pulses
   //==================================================================================
   // Perform checksum calculations
   //==================================================================================
   // for (byte i = 0; i < bytecounter; i++)
   //    data[i] = reverse8(data[i]);
   reflect_bytes(data, bytecounter);

   // get packet length
   length = data[2] & 0x3F; // drop bits 6 and 7
   length >>= 1;            // drop bit 0
   if (length > 20) {
     #ifdef PLUGIN_034_DEBUG
     sprintf_P(tmpbuf, PSTR("Cresta: failed length > 20 at length=%i"), (int) length);
     RFLink::sendRawPrint(tmpbuf, true);
     #endif
     return false; // Additional check for illegal packet lengths to protect against false positives.
   }
   if (length == 0) {
     #ifdef PLUGIN_034_DEBUG
     sprintf_P(tmpbuf, PSTR("Cresta: failed length ==0"));
     RFLink::sendRawPrint(tmpbuf, true);
     #endif
     return false; // Additional check for illegal packet lengths to protect against false positives.
   }
   // Checksum: XOR of all bytes from byte 1 till byte length+2, should result in 0
   checksum = 0;
   for (byte i = 1; i < length + 2; i++)
      checksum ^= data[i];

   if (checksum != 0) {
     #ifdef PLUGIN_034_DEBUG
     sprintf_P(tmpbuf, PSTR("Cresta: failed checksum != 0"));
     RFLink::sendRawPrint(tmpbuf, true);
     #endif
     return false;
   }
   // ==================================================================================
   // now process the various sensor types
   // ==================================================================================
   if (data[1] > 0x1F && data[1] < 0x40)
      channel = 1;
   if (data[1] > 0x3F && data[1] < 0x60)
      channel = 2;
   if (data[1] > 0x5F && data[1] < 0x80)
      channel = 3;
   if (data[1] > 0x7F && data[1] < 0xA0)
      channel = 1; // no channel settings on Anemometer/rainmeter and uvsensor
   if (data[1] > 0x9F && data[1] < 0xC0)
      channel = 4;
   if (data[1] > 0xbF && data[1] < 0xE0)
      channel = 5;
   data[3] &= 0x1F;
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   unsigned long tmpval = ((unsigned long)data[3] << 16) | ((data[1]) << 8) | channel;

   if ((SignalHash != SignalHashPrevious) || ((RepeatingTimer + 500) < millis()) || (SignalCRC != tmpval))
      SignalCRC = tmpval; // not seen the RF packet recently
   else
      return true; // already seen the RF packet recently
   // ----------------------------------
   battery = !((data[2]) >> 6);
   // ----------------------------------
   if (data[3] == 0x0C)
   { // Anemometer

      sensor_data = ((data[5] & 0x3F) * 100);
      sensor_data += ((data[4] >> 4) * 10);
      sensor_data += (data[4] & 0x0F);

      if ((data[5] & 0x80) != 0x80)
         sensor_data |= 0x8000; // set highest bit (minus bit)

      windtemp = sensor_data;

      sensor_data = ((data[7] & 0x3f) * 100);
      sensor_data += ((data[6] >> 4) * 10);
      sensor_data += (data[6] & 0x0F);

      if ((data[7] & 0x80) != 0x80)
         sensor_data |= 0x8000; // set highest bit (minus bit)

      windchill = sensor_data;

      windspeed = ((data[9] & 0x0F) * 100);
      windspeed += ((data[8] >> 4) * 10);
      windspeed += (data[8] & 0x0F);

      windgust = ((data[10] >> 4) * 100);
      windgust += ((data[10] & 0x0F) * 10);
      windgust += (data[9] >> 4);

      winddirection = Plugin_034_WindDirSeg(((data[11] & 0xf0) >> 4));
      winddirection &= 0x0F; // make sure we dont get overflows
      //==================================================================================
      // Output
      //==================================================================================
      display_Header();
      display_Name(PSTR("Cresta"));
      char c_ID[5];
      sprintf(c_ID, "%02X%02X", data[1], channel);
      display_IDc(c_ID);
      display_WINDIR(winddirection);
      display_WINSP(windspeed);
      display_WINGS(windgust);
      display_WINTMP(windtemp);
      display_WINCHL(windchill);
      display_BAT(battery);
      display_Footer();
      //==================================================================================
   }
   else
       // ----------------------------------
       if (data[3] == 0x0D)
   { // UV Sensor

      sensor_data = ((data[5] & 0x3F) * 100);
      sensor_data += ((data[4] >> 4) * 10);
      sensor_data += (data[4] & 0x0F);

      if ((data[5] & 0x80) != 0x80)
         sensor_data |= 0x8000; // set highest bit (minus bit)

      // UV sensor reports the temperature but does not report negative values!, skip temperature info?
      uv = ((data[8] & 0x0F) << 8) | data[7];
      //==================================================================================
      // Output
      //==================================================================================
      display_Header();
      display_Name(PSTR("Cresta"));
      char c_ID[5];
      sprintf(c_ID, "%02X%02X", data[1], channel);
      display_IDc(c_ID);
      display_TEMP(sensor_data);
      display_UV(uv);
      display_BAT(battery);
      display_Footer();
      //==================================================================================
   }
   else // 9F 80 CC 4E 00 00 66 64
       // ----------------------------------
       // 9f 80 cc 4e 01 00 66 65
       if (data[3] == 0x0E)
   {                                          // Rain meter  // 9F 80 CC 4E 76 00 66 12
      sensor_data = (data[5] << 8) + data[4]; // 80=rain 4e=>0E = rain
      sensor_data = sensor_data * 7;          // 66 = always 66    rain units * 0.7 = mm.
      //==================================================================================
      // Output
      //==================================================================================
      display_Header();
      display_Name(PSTR("Cresta"));
      char c_ID[5];
      sprintf(c_ID, "%02X%02X", data[1], channel);
      display_IDc(c_ID);
      display_RAIN(sensor_data);
      display_BAT(battery);
      display_Footer();
      //==================================================================================
   }
   else
       // ----------------------------------
       if (data[3] == 0x1E)
   { // Thermo/Hygro
      sensor_data = ((data[5] & 0x3f) * 100);
      sensor_data += ((data[4] >> 4) * 10);
      sensor_data += (data[4] & 0x0f);

      if ((data[5] & 0x80) != 0x80)
         sensor_data |= 0x8000; // set highest bit (minus bit)
      //==================================================================================
      // Output
      //==================================================================================
      display_Header();
      display_Name(PSTR("Cresta"));
      char c_ID[5];
      sprintf(c_ID, "%02X%02X", data[1], channel);
      display_IDc(c_ID);
      display_TEMP(sensor_data);
      display_HUM(data[6]);
      display_BAT(battery);
      display_Footer();
      //==================================================================================
   }
   else
   {
      //==================================================================================
      // Output
      //==================================================================================
      display_Header();
      display_Name(PSTR("Cresta;DEBUG"));
      char c_ID[5];
      sprintf(c_ID, "%02X%02X", data[1], channel);
      display_IDc(c_ID);
      display_Footer();
      // ----------------------------------
      char dbuffer[3];
      for (byte i = 0; i < length + 2; i++)
      {
         sprintf(dbuffer, "%02x", data[i]);
         Serial.print(dbuffer);
      }
      Serial.print(F(";"));
      Serial.println();
      //==================================================================================
   }
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}

byte Plugin_034_WindDirSeg(byte data)
{
   // Encrypted using: a=-a&0xF; b=a^(a>>1);
   data ^= (data & 8) >> 1; /* Solve bit 2 */
   data ^= (data & 4) >> 1; /* Solve bit 1 */
   data ^= (data & 2) >> 1; /* Solve bit 0 */
   return -data & 0xF;
}
#endif // PLUGIN_034
