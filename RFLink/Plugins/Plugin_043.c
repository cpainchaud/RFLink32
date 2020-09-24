//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-043: LaCrosse                                       ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding LaCrosse  weatherstation outdoor sensors
 * It also works for all non LaCrosse sensors that follow this protocol.
 * Lacrosse TX3-TH  Thermo/Humidity, Lacrosse TX4, Lacrosse TX4U
 * WS7000-15: Anemometer, WS7000-16: Rain precipitation, WS2500-19: Brightness Luxmeter, WS7000-20: Thermo/Humidity/Barometer
 * TFA 30.3125 (temperature + humidity), TFA 30.3120.90 (temperature)
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
 * Decodes signals from a LaCrosse Weatherstation outdoor unit, (88 pulses, 44 bits, 433 MHz).
 * Partially based on http://www.f6fbb.org/domo/sensors/tx3_th.php
 *
 * Temperature sensor (TX3)
 * Each frame is 44 bits long. It is composed of:
 * • 2 blocks of four bits = 0A (start sequence)
 * • 8 blocks of four bits (data)
 * • 1 block of four bits (checksum)
 *
 * The active values of the frames are explained below:
 *
 * Example
 * 0000 1010 0000 0000 1110 0111 0011 0001 0111 0011 1101
 * aaaa aaaa bbbb cccc cccd eeee ffff gggg hhhh iiii jjjj
 * 0    A    0    0    7  0 7    3    1    7    3    D
 *
 * 000a0004070705030705
 * 20;d8;LaCrosse;ID=0401;TEMP=00fd;
 * 000a0e04070608000608
 * 20;d9;LaCrosse;ID=0403;HUM=68;
 *
 * • a = Start sequence (always 0000 1010)
 * • b = Packet type (0=Thermo E=hygro)
 * • c = Address of sensor (changes when inserting batteries)
 * • d = Parity bit  (c+d+e bits sum is even)
 * • e-i = Measured values:
 *     e = tens (x 10)
 *     f = ones (x 1)
 *     g = digits (x 0.1) (is zero in case of humidity)
 *     h = copy of e value
 *     i = copy of f value
 * • j = Checksum (Lower four bits of the sum of all words)
 *
 * Checksum: (0 + A + 0 + 0 + E + 7 + 3 + 1 + 7 + 3) and F = D   D
 * Sample:
 * 20;11;DEBUG;Pulses=88;Pulses(uSec)=1200,875,1125,875,1125,875,1125,900,400,900,1150,875,400,900,1150,875,1125,875,1125,875,1150,875,1150,875,400,900,400,875,375,900,1150,875,1125,875,400,900,1150,875,1125,875,1125,875,400,900,400,875,1125,900,400,875,1150,875,1150,900,1125,875,1150,875,400,900,400,875,400,900,1150,875,400,900,400,875,1125,875,400,900,1150,900,1125,875,1150,875,375,900,400,900,400,900,400;
 * 20;9E;DEBUG;Pulses=88;Pulses(uSec)=1300,925,1225,925,1225,925,1200,925,425,925,1225,925,425,925,1225,925,1225,925,1225,925,1225,925,1225,925,1225,925,425,925,1225,925,1225,925,1225,925,425,925,425,925,1225,925,1225,925,425,925,425,925,425,925,1225,925,425,925,425,925,1225,925,425,925,1225,925,1225,925,1225,925,1225,925,425,925,425,925,425,925,1200,925,425,925,425,925,1225,925,1225,925,425,925,425,925,1225;
 * 20;9F;LaCrosse;ID=0403;TEMP=010c;
 * 20;A1;DEBUG;Pulses=88;Pulses(uSec)=1325,925,1225,925,1225,925,1225,925,425,925,1225,925,425,925,1225,925,425,925,425,925,425,925,1225,925,1225,925,425,925,1225,925,1225,925,1225,925,425,925,425,925,1225,925,1225,925,425,925,1225,925,425,925,1225,925,425,950,425,925,1225,925,1225,925,1225,925,1225,925,1225,925,1225,925,425,925,1225,925,425,925,1200,925,425,925,425,925,1225,925,425,925,1225,925,1225,925,1225;
 * 20;A2;LaCrosse;ID=0403;HUM=56;

 * 1275,925,1225,925,1225,925,1200,925,425,925,1225,925,425,925,1225,900,1225,925,1200,925,1225,925,1225,925,1225,925,425,925,1225,925,1200,925,1225,925,425,925,425,900,1225,900,1225,925,425,925,425,925,425,925,1225,900,425,925,1225,925,1225,925,1225,925,425,950,1225,900,425,950,1225,925,425,925,425,900,425,950,1200,900,425,925,1225,925,1225,925,425,925,425,925,425,925,425

20;76;DEBUG;Pulses=88;Pulses(uSec)=810,1440,1200,930,1200,960,1200,930,390,930,1200,930,420,960,1200,930,1200,930,1200,930,1200,930,1200,930,1200,930,420,930,1200,960,1200,930,1200,930,420,930,420,930,1200,930,1200,960,390,930,420,930,420,960,1200,930,390,930,1200,930,420,930,1200,960,1170,930,1200,930,420,930,1200,930,420,930,420,930,420,930,1200,930,420,930,1200,930,420,930,420,930,420,930,1200,930,420,6990;
0000 1010 0000 0100 0110 0111 0101 0001 0111 0101 1101
20;78;DEBUG;Pulses=88;Pulses(uSec)=240,1980,1200,960,1200,960,1200,960,390,930,1200,930,390,960,1200,960,390,930,420,930,420,930,1200,930,1200,960,390,930,1200,930,1200,930,1200,930,420,930,420,930,1200,930,1200,930,420,930,420,930,1200,930,1200,930,420,960,1200,960,390,960,1200,930,1200,930,1200,930,1200,930,1200,930,420,960,390,930,1200,930,1200,960,390,930,1200,930,420,930,420,930,1200,930,1200,930,1200,6990;

0;29;DEBUG;Pulses=86;Pulses(uSec)=1260,930,1200,930,1200,930,420,960,1200,930,420,930,1200,930,1200,930,1200,930,1200,930,1200,930,1200,930,390,930,1200,960,1200,960,1200,930,420,930,390,960,1200,960,1200,930,420,930,420,930,390,960,1200,960,390,930,1200,930,1200,930,1200,930,420,930,420,960,1200,960,1200,930,390,960,390,960,390,930,1200,960,420,930,1200,930,1200,930,1200,930,1200,930,1200,930,1200,6990;
20;2A;DEBUG;Pulses=50;Pulses(uSec)=1200,900,1140,870,390,900,360,900,390,900,1140,930,420,930,1200,930,1200,930,1200,930,420,930,390,960,1200,930,1200,930,390,930,420,960,390,930,1200,930,390,960,1200,960,1200,930,1200,960,1170,930,1200,930,1200,6990;
20;2B;DEBUG;Pulses=86;Pulses(uSec)=1230,960,1200,960,1200,930,390,960,1200,930,420,930,1200,930,420,930,390,960,420,930,1200,930,1200,930,420,930,1200,960,1170,960,1200,930,390,930,420,930,390,930,1200,930,420,930,390,930,1200,960,1200,930,420,930,420,960,390,930,1200,930,1200,930,1200,930,1200,930,1200,930,390,960,390,930,1200,960,1200,930,420,930,420,960,390,930,420,960,390,930,1200,930,420,6990;

 * --------------------------------------------------------------------------------------------
 * Rain Packet:
 * Each frame is 46 bits long. It is composed of:
 * 10bits of 0 (start sequence)
 * 7 blocks of four bits separated by a bit 1 to be checked and skipped
 *
 * The 1st bit of each word is LSB, so we have to reverse the 4 bits of each word.
 *  Example
 * 0000 0000 0010 1111 1011 0010 1011 1111 1101
 *           aaaa bbbb ccc1 ccc2 ccc3 dddd eeee
 *           2    F    B    2    B    F    D
 * a = sensor type (2=Rain meter)
 * b = sensor address
 * c = rain data (LSB thus the right order is c3 c2 c1)
 * d = Check Xor : (2 ^ F ^ B ^ 2 ^ B ^ F) = 0
 * e = Check Sum : (const5 + 2 + F + B + 2 + B + F) and F = D
  \*********************************************************************************************/
#define LACROSSE43_PLUGIN_ID 043
#define PLUGIN_DESC_043 "LaCrosse"

#define LACROSSE43_PULSECOUNT 88 // also handles 84 to 92 pulses!

#define LACROSSE43_MIDLO 640 / RAWSIGNAL_SAMPLE_RATE       //900 //630
#define LACROSSE43_MIDHI 1056 / RAWSIGNAL_SAMPLE_RATE      //1500 //1050
#define LACROSSE43_PULSEMINMAX 576 / RAWSIGNAL_SAMPLE_RATE //810 //570
#define LACROSSE43_PULSEMAXMIN 992 / RAWSIGNAL_SAMPLE_RATE //1410 //990
#define LACROSSE43_PULSEMAX 1440 / RAWSIGNAL_SAMPLE_RATE   //2100 //1500

#ifdef PLUGIN_043
#include "../4_Display.h"

boolean Plugin_043(byte function, char *string)
{
   if ((RawSignal.Number < LACROSSE43_PULSECOUNT - 4) || (RawSignal.Number > LACROSSE43_PULSECOUNT + 4))
      return false;

   unsigned long bitstream1 = 0L; // holds first 5x4=20 bits
   unsigned long bitstream2 = 0L; // holds last  6x4=24 bits
   byte bitcounter = 0;           // counts number of received bits (converted from pulses)
   byte data[11];
   byte checksumcalc = 0;
   int temperature = 0;
   byte humidity = 0;
   //unsigned int rain = 0; // Not implemented
   //==================================================================================
   // Get all 44 bits
   //==================================================================================
   for (byte x = 1; x < RawSignal.Number; x += 2)
   {
      if ((RawSignal.Pulses[x + 1] < LACROSSE43_MIDLO) || (RawSignal.Pulses[x + 1] > LACROSSE43_MIDHI))
      {
         if (x == 1) // Make sure the first bit is correct..
            RawSignal.Pulses[1] = LACROSSE43_PULSEMAX - 1;
         else
         {
            if ((x + 1) < RawSignal.Number) // in between pulse check
               return false;
         }
      }
      if (RawSignal.Pulses[x] > LACROSSE43_PULSEMAXMIN)
      {
         if ((RawSignal.Pulses[x] > LACROSSE43_PULSEMAX) && (x > 1))
            return false;

         if (bitcounter < 20)
         {
            bitstream1 <<= 1;
            bitcounter++; // only need to count the first 16 bits
         }
         else
            bitstream2 <<= 1;
      }
      else
      {
         if (RawSignal.Pulses[x] > LACROSSE43_PULSEMINMAX)
            return false;

         if (bitcounter < 20)
         {
            bitstream1 <<= 1;
            bitstream1 |= 0x1;
            bitcounter++; // only need to count the first 20 bits
         }
         else
         {
            bitstream2 <<= 1;
            bitstream2 |= 0x1;
         }
      }
   }
   if (RawSignal.Number == (LACROSSE43_PULSECOUNT - 4))
      bitstream2 = (bitstream2 << 2); // add missing zero bit
   if (RawSignal.Number == (LACROSSE43_PULSECOUNT - 2))
      bitstream2 = (bitstream2 << 1); // add missing zero bit
   //==================================================================================
   // Perform a quick sanity check
   //==================================================================================
   if (bitstream1 == 0) // && (bitstream2 == 0)
      return false;

   data[0] = (bitstream1 >> 16) & 0xF;
   if (data[0] != 0x0)
      return false;

   data[1] = (bitstream1 >> 12) & 0xF;
   if (data[1] != 0xA)
      return false;
   //==================================================================================
   // Prepare nibbles from bit stream
   //==================================================================================
   data[2] = (bitstream1 >> 8) & 0xF;
   data[3] = (bitstream1 >> 4) & 0xF;
   data[4] = (bitstream1 >> 0) & 0xF;
   data[5] = (bitstream2 >> 20) & 0xF;
   data[6] = (bitstream2 >> 16) & 0xF;
   data[7] = (bitstream2 >> 12) & 0xF;
   data[8] = (bitstream2 >> 8) & 0xF;
   data[9] = (bitstream2 >> 4) & 0xF;
   data[10] = (bitstream2 >> 0) & 0xF; // CRC
   //==================================================================================
   // Perform checksum calculations
   //==================================================================================
   for (byte i = 0; i < 10; i++) // max. value = 10*0xF 0x96
      checksumcalc += data[i];   // less with real values

   checksumcalc = checksumcalc & 0xF;
   if (checksumcalc != data[10])
      return false;
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   unsigned long tmpval = (bitstream1 << 4) | (data[10]); // sensor type + ID + checksum

   if ((SignalHash != SignalHashPrevious) || (RepeatingTimer < millis()) || (SignalCRC != tmpval))
      SignalCRC = tmpval; // not seen this RF packet recently
   else
      return true; // already seen the RF packet recently, but still want the humidity
   //==================================================================================
   // now process the various sensor types
   //==================================================================================
   data[4] = (data[4]) >> 1; // ID
   if (data[2] == 0x0)
   {
      temperature = (data[5] * 100);
      temperature += (data[6] * 10);
      temperature += (data[7]);
      temperature -= 500;
      //==================================================================================
      // Output
      //==================================================================================
      display_Header();
      display_Name(PSTR("LaCrosse"));
      char c_ID[5];
      sprintf(c_ID, "%02X%02X", data[3], data[4]);
      display_IDc(c_ID);
      display_TEMP(temperature);
      display_Footer();
      //==================================================================================
      RawSignal.Repeats = false;
      RawSignal.Number = 0;
      return true;
   }
   else if (data[2] == 0xE)
   {
      humidity = (data[5] * 10) + data[6];
      if (humidity == 0) // humidity should not be 0
         return false;
      //==================================================================================
      // Output
      //==================================================================================
      display_Header();
      display_Name(PSTR("LaCrosse"));
      char c_ID[5];
      sprintf(c_ID, "%02X%02X", data[3], data[4]);
      display_IDc(c_ID);
      display_HUM(humidity, HUM_HEX);
      display_Footer();
      //==================================================================================
      RawSignal.Repeats = true;
      RawSignal.Number = 0;
      return true;
   }
   else
      return false;
}
#endif // PLUGIN_043
