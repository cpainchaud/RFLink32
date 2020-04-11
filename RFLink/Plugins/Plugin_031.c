//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-31 AlectoV3                                         ##
//#######################################################################################################
/*********************************************************************************************\
 * Dit protocol zorgt voor ontvangst van Alecto weerstation buitensensoren
 * WS1100, WS1200, WSD-19
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
 * Technische informatie:
 * Decodes signals from Alecto Weatherstation outdoor unit, type 3 (94/126 pulses, 47/63 bits, 433 MHz).
 * WS1100 Message Format: (7 bits preamble, 5 Bytes, 40 bits):
 * AAAAAAA AAAABBBB BBBB__CC CCCCCCCC DDDDDDDD EEEEEEEE
 *                        Temperature Humidity Checksum
 * A = start/unknown, first 8 bits are always 11111111
 * B = Rolling code
 * C = Temperature (10 bit value with -400 base)
 * D = Checksum
 * E = Humidity
 *
 * WS1200 Message Format: (7 bits preamble, 7 Bytes, 56 bits):
 * AAAAAAA AAAABBBB BBBB__CC CCCCCCCC DDDDDDDD DDDDDDDD EEEEEEEE FFFFFFFF 
 *                        Temperature Rain LSB Rain MSB ???????? Checksum
 * A = start/unknown, first 8 bits are always 11111111
 * B = Rolling code
 * C = Temperature (10 bit value with -400 base)
 * D = Rain ( * 0.3 mm)
 * E = ?
 * F = Checksum
 
 AAAAAAA AAAABBBB BBBBCCCC CCCCCCCC DDDDDDDD DDDDDDDD EEEEEEEE FFFFFFFF 
 1111111 00111010 01010010 10000010 00000000 00000000 11111111 10111010 1 24,2 gr 0 mm
 1111111 00111010 01010010 10000010 00000001 00000000 11111111 11111100 0 24,2 gr 0,3 mm
 1111111 00111010 01010010 10000010 00000010 00000000 11111111 00110110 0 24,2 gr 0,6 mm
 1111111 00111010 01010010 10000010 00001000 00000000 11111111 11101000 1 24,2 gr 2,4 mm
 1111111 00111010 01010010 10000010 00001101 00000000 11111111 10000111 0 24,2 gr 3.9 mm
 1111111 00111010 01010010 01110001 00001101 00000000 11111111 01000010 0 22,5 gr 3,9 mm
 1111111 00111010 01010010 01010111 00001101 00000000 11111111 00111011 1 19,9 gr 3,9 mm
 1111111 00111010 01010010 00111110 00010010 00000000 11111111 00111001 1 17,4 gr 5,4 mm
 1111111 00111010 01010010 00101000 00010010 00000000 11111111 00001000 0 15,2 gr 5,4 mm
 
 1111111 00111010 01010001 00101011 10011010 00000001 11111111 10100011 -10,1 gr/123,0 mm
 WS1200 temp:-101
 WS1200 rain LSB:154
 WS1200 rain MSB:1
 WS1200 rain:1230

 1111111 00111010 01010001 10101101 10011111 00000001 11111111 00110100 2,9 gr/124,5mm
 WS1200 temp:29
 WS1200 rain LSB:159
 WS1200 rain MSB:1
 WS1200 rain:1245


 * 20;AE;DEBUG;Pulses=126;Pulses(uSec)=900,950,825,450,325,450,325,950,325,450,325,450,825,950,825,450,325,950,825,450,350,950,325,450,825,950,825,450,325,450,325,950,825,925,350,450,825,950,825,925,350,450,825,450,350,925,825,450,350,450,325,950,350,450,825,950,325,450,350,450,325,450,825,450,325,450,325,450,325,450,325,950,825,950,325,450,825,950,325,450,825,450,325,950,325,450,325,450,825,925,350,450,350,450,825,950,825,925,350,425,350,450,350,450,350,450,350,450,825,950,825,950,325,450,350,450,825,950,825,950,825,950,325,450,325;
 * 20;AF;Alecto V3;ID=009a;TEMP=ffe7;RAIN=7a;
 \*********************************************************************************************/
#define ALECTOV3_PLUGIN_ID 31
#define WS1100_PULSECOUNT 94
#define WS1200_PULSECOUNT 126

#define ALECTOV3_PULSEMID 300 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_031
#include "../4_Display.h"

uint8_t Plugin_031_ProtocolAlectoCRC8(uint8_t *addr, uint8_t len);
// unsigned int Plugin_031_ProtocolAlectoRainBase = 0;

boolean Plugin_031(byte function, char *string)
{
   if ((RawSignal.Number != WS1100_PULSECOUNT) && (RawSignal.Number != WS1200_PULSECOUNT))
      return false;

   unsigned long bitstream1 = 0L;
   unsigned long bitstream2 = 0L;
   byte rc = 0;
   int temperature = 0;
   byte humidity = 0;
   unsigned int rain = 0;
   byte checksum = 0;
   byte checksumcalc = 0;
   byte data[6];
   //==================================================================================
   // Get all 36 bits
   //==================================================================================
   for (byte x = 15; x <= 77; x += 2)
   {                    // get first 32 relevant bits
      bitstream1 <<= 1; // Always shift
      if (RawSignal.Pulses[x] < ALECTOV3_PULSEMID)
         bitstream1 |= 0x1;
      // else
      //    bitstream1 |= 0x0;
   }
   for (byte x = 79; x <= 141; x = x + 2)
   {                    // get second 32 relevant bits
      bitstream2 <<= 1; // Always shift
      if (RawSignal.Pulses[x] < ALECTOV3_PULSEMID)
         bitstream2 |= 0x1;
      // else
      //    bitstream2 |= 0x0;
   }
   //==================================================================================
   // Perform a quick sanity check
   //==================================================================================
   if (bitstream1 == 0)
      return false;
   //==================================================================================
   // Prepare nibbles from bit stream
   //==================================================================================
   data[0] = (bitstream1 >> 24) & 0xFF;
   data[1] = (bitstream1 >> 16) & 0xFF;
   data[2] = (bitstream1 >> 8) & 0xFF;
   data[3] = (bitstream1 >> 0) & 0xFF;
   data[4] = (bitstream2 >> 24) & 0xFF;
   data[5] = (bitstream2 >> 16) & 0xFF;
   //==================================================================================
   // Perform checksum calculations
   //==================================================================================
   if (RawSignal.Number == WS1200_PULSECOUNT)
   { // verify checksum
      checksum = (bitstream2 >> 8) & 0xFF;
      checksumcalc = Plugin_031_ProtocolAlectoCRC8(data, 6);
   }
   else
   {
      checksum = (bitstream2 >> 24) & 0xFF;
      checksumcalc = Plugin_031_ProtocolAlectoCRC8(data, 4);
   }
   if (checksum != checksumcalc)
      return false;
   //==================================================================================
   // Now process the various sensor types
   //==================================================================================
   rc = (bitstream1 >> 20) & 0xFF;
   temperature = ((bitstream1 >> 8) & 0x3FF); // 299=12b  -400 (0x190)  = FF9b
   if (temperature < 400)
   { // negative temperature value
      temperature = 400 - temperature;
      temperature = temperature | 0x8000; // turn highest bit on for minus values
   }
   else
   {
      if (temperature > 0x258)
         return false; // temperature out of range ( > 60.0 degrees)
   }
   //==================================================================================
   // Output
   //==================================================================================
   display_Header();
   display_Name(PSTR("Alecto V3"));
   display_IDn(rc, 2);
   display_TEMP(temperature);

   if (RawSignal.Number == WS1100_PULSECOUNT)
   {
      humidity = bitstream1 & 0xFF; // alleen op WS1100?
      display_HUM(humidity, false);
   }
   else
   {
      rain = ((((bitstream2 >> 24) & 0xFF) << 8) | ((bitstream1 >> 0) & 0xFF));
      display_RAIN(rain);
      // check if rain unit has been reset!
      // if (rain < Plugin_031_ProtocolAlectoRainBase)
      //    Plugin_031_ProtocolAlectoRainBase = rain;
      // if (Plugin_031_ProtocolAlectoRainBase > 0)
      // {
      //    display_RAINRATE(rain - Plugin_031_ProtocolAlectoRainBase);
      // }
      // Plugin_031_ProtocolAlectoRainBase = rain;
   }
   display_Footer();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;     // do not process the packet any further
   return true;
}

/*********************************************************************************************\
 * Calculates CRC-8 checksum
 * reference http://lucsmall.com/2012/04/29/weather-station-hacking-part-2/
 *           http://lucsmall.com/2012/04/30/weather-station-hacking-part-3/
 *           https://github.com/lucsmall/WH2-Weather-Sensor-Library-for-Arduino/blob/master/WeatherSensorWH2.cpp
 \*********************************************************************************************/
uint8_t Plugin_031_ProtocolAlectoCRC8(uint8_t *addr, uint8_t len)
{
   uint8_t crc = 0;
   // Indicated changes are from reference CRC-8 function in OneWire library
   while (len--)
   {
      uint8_t inbyte = *addr++;
      for (uint8_t i = 8; i; i--)
      {
         uint8_t mix = (crc ^ inbyte) & 0x80; // changed from & 0x01
         crc <<= 1;                           // changed from right shift
         if (mix)
            crc ^= 0x31; // changed from 0x8C;
         inbyte <<= 1;   // changed from right shift
      }
   }
   return crc;
}
#endif // PLUGIN_031
