//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-041: LaCrosse                                       ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding LaCrosse  weatherstation outdoor sensors
 * It also works for all non LaCrosse sensors that follow this protocol.
 * WS7000-15: Anemometer, WS7000-16: Rain precipitation, WS2500-19: Brightness Luxmeter, WS7000-20: Thermo/Humidity/Barometer
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
 * Meteo Sensor: (162 pulses)
 * Each frame is 80 bits long. It is composed of: 
 * 10 bits of 0 (preamble) 
 * 14 blocks of four bits separated by a 1 bit to be checked and skipped 
 *
 * 0100 0111 1000 0101 0010 0100 0000 0011 0001 0001 1000 1101 1110 1011 
 * aaaa bbbb cccc cccc cccc dddd dddd dddd eeee eeee eeee ffff gggg hhhh
 *
 * a = sensor type (04=Meteo sensor)
 * b = sensor address
 * c = temperature BCD, reversed
 * d = humidity BCD, reversed 
 * e = air pressure, BCD reversed + 200 offset in hPa
 * f = unknown?
 * g = xor value
 * h = checksum value
 *
 * Sample:
 * 20;07;DEBUG;Pulses=162;Pulses(uSec)=825,275,750,275,750,300,750,300,750,300,750,275,750,275,750,300,750,300,750,300,250,800,725,300,750,300,250,800,725,300,225,800,225,800,250,800,250,800,725,300,250,800,725,300,750,300,725,300,250,800,250,800,225,800,750,300,250,800,725,300,250,800,725,300,250,800,725,300,725,300,250,800,725,300,725,300,250,800,725,300,250,800,725,300,725,300,725,300,725,300,250,800,225,800,225,800,725,300,725,300,225,800,225,800,725,300,725,300,725,300,250,800,250,800,725,300,725,300,725,300,250,800,725,300,725,300,725,300,225,800,225,800,225,800,725,300,225,800,225,800,250,800,725,300,225,800,225,800,225,800,250,800,250,800,225,800,725,300,225,800,225,600;
 * 1010101010101010101001101001100101010110011010100101011001100110011010011010011001101010100101011010010110101001011010100110101001010110010101100101010101011001 00                                                                
 * 0000000000 1 0010 1 1110 1 0001 1 1010 1 0100 1 0010 1 0000 1 1100 1 1000 1 1000 1 0001 1 1011 1 0111 1 1101   0
 *              0010   1110   0001   1010   0100   0010   0000   1100   1000   1000   0001   1011   0111   1101 
 *              0100   0111   1000   0101   0010   0100   0000   0011   0001   0001   1000   1101   1110   1011
 * 4 7 852 403 118 D E B
 *    25.8 30.4 811+200
 * --------------------------------------------------------------------------------------------
 * Rain Packet: (92 pulses)
 * Each frame is 46 bits long. It is composed of: 
 * 10 bits of 0 (preamble) 
 * 7 blocks of four bits separated by a 1 bit to be checked and skipped 
 *
 * The 1st bit of each word is LSB, so we have to reverse the 4 bits of each word. 
 *  Example 
 * 0000000000 0010 1111 1011 0010 1011 1111 1101   
 *            aaaa bbbb ccc1 ccc2 ccc3 dddd eeee 
 *            2    F    B    2    B    F    D   
 *
 * a = sensor type (2=Rain meter)
 * b = sensor address 
 * c = rain data (LSB thus the right order is c3 c2 c1)
 * d = Check Xor : (2 ^ F ^ B ^ 2 ^ B ^ F) = 0
 * e = Check Sum : (const5 + 2 + F + B + 2 + B + F) and F = D   
 * --------------------------------------------------------------------------------------------
 * Wind packet: (122 pulses)
 * Each frame is composed of: 
 * 10bits of 0 (preamble) 
 * 10 blocks of four bits separated by a bit 1 to be checked and skipped 
 *
 * The 1st bit of each word is LSB, so we have to reverse the 4 bits of each word. 
 *  Example 
 * 0000000000 0011 0111 0101 0000 0001 0101 0100 0111 0110 1011  
 *            aaaa bbbb cccc cccc cccc dddd dddd ddee ffff gggg
 *            3    7    5    0    1   5   4   1 3   6   B  
 *
 * a = sensor type (2=Rain meter)
 * b = sensor address 
 * c = speed
 * d = direction
 * f = Check Xor 
 * g = Check Sum 
 * --------------------------------------------------------------------------------------------
 * UV packet (132 pulses)
 \*********************************************************************************************/
#define LACROSSE41_PLUGIN_ID 041
#define PLUGIN_DESC_041 "LaCrosseV3"

#define LACROSSE41_PULSECOUNT1 92  // Rain sensor
#define LACROSSE41_PULSECOUNT2 162 // Meteo sensor
#define LACROSSE41_PULSECOUNT3 122 // Wind sensor
#define LACROSSE41_PULSECOUNT4 132 // Brightness sensor

#define LACROSSE41_PULSEMID_D 500

#ifdef PLUGIN_041
#include "../4_Display.h"

boolean Plugin_041(byte function, const char *string)
{
   const long LACROSSE41_PULSEMID = LACROSSE41_PULSEMID_D / RawSignal.Multiply;

   if ((RawSignal.Number != LACROSSE41_PULSECOUNT1) && (RawSignal.Number != LACROSSE41_PULSECOUNT2) &&
       (RawSignal.Number != LACROSSE41_PULSECOUNT3) && (RawSignal.Number != LACROSSE41_PULSECOUNT4))
      return false;

   byte data[18];
   byte bitcounter = 0;  // counts number of received bits (converted from pulses)
   byte bytecounter = 0; // used for counting the number of received bytes
   byte checksum = 0;
   int sensor_data = 0;
   //==================================================================================
   // Check preamble
   //==================================================================================
   for (byte x = 1; x < 20; x += 2)
   {
      if ((RawSignal.Pulses[x] < LACROSSE41_PULSEMID) || (RawSignal.Pulses[x + 1] > LACROSSE41_PULSEMID))
         return false; // bad preamble bit detected, abort
   }
   if ((RawSignal.Pulses[21] > LACROSSE41_PULSEMID) || (RawSignal.Pulses[22] < LACROSSE41_PULSEMID))
      return false; // There should be a 1 bit after the preamble
   //==================================================================================
   // Get bits/nibbles
   //==================================================================================
   for (byte x = 23; x < RawSignal.Number - 2; x += 2)
   {
      if (RawSignal.Pulses[x] < LACROSSE41_PULSEMID)
         data[bytecounter] = ((data[bytecounter] >> 1) | 0x08); // 1 bit, store in reversed bit order
      else
         data[bytecounter] = ((data[bytecounter] >> 1) & 0x07); // 0 bit, store in reversed bit order
      bitcounter++;

      if (bitcounter == 4)
      {
         x = x + 2;
         if (x > RawSignal.Number - 2)
            break; // dont check the last marker

         if ((RawSignal.Pulses[x] > LACROSSE41_PULSEMID) || (RawSignal.Pulses[x + 1] < LACROSSE41_PULSEMID))
            return false; // There should be a 1 bit after each nibble

         bitcounter = 0;
         bytecounter++;

         if (bytecounter > 17)
            return false; // received too many nibbles/bytes, abort
      }
   }
   //==================================================================================
   // Perform checksum calculations
   //==================================================================================
   // check xor value
   checksum = 0;
   for (byte i = 0; i < bytecounter; i++)
      checksum ^= data[i];

   if (checksum != 0)
      return false; // all (excluding last) nibbles xored must result in 0

   // check checksum value
   checksum = 5;
   for (byte i = 0; i < bytecounter; i++)
      checksum += data[i];

   checksum = checksum & 0x0f;
   if (checksum != (data[bytecounter]))
      return false; // all (excluding last) nibble added must result in last nibble value
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   unsigned long tempval = (((unsigned long)(data[4]) >> 1) << 16) | ((data[3]) << 8) | data[2];

   if ((SignalHash != SignalHashPrevious) || ((RepeatingTimer + 150) < millis()) || (SignalCRC != tempval))
      SignalCRC = tempval; // not seen this RF packet recently
   else
      return true; // already seen the RF packet recently, but still want the humidity
   //==================================================================================
   // now process the various sensor types
   //==================================================================================
   // Output
   //==================================================================================
   display_Header();
   display_Name(PSTR("LaCrosseV3"));
   char c_ID[5];
   sprintf(c_ID, "%02X%02X", data[0], data[1]);
   display_IDc(c_ID);

   if (data[0] == 0x4) // Meteo sensor
   {
      sensor_data = (data[4] * 100);
      sensor_data += (data[3] * 10);
      sensor_data += (data[2]);
      display_TEMP(sensor_data);

      sensor_data = (data[7] * 10);
      sensor_data += (data[6]);
      display_HUM(((byte)sensor_data), HUM_HEX);

      sensor_data = (data[10] * 100);
      sensor_data += (data[9] * 10);
      sensor_data += (data[8]);
      sensor_data += 200;
      display_BARO(sensor_data);
   }
   else if (data[0] == 0x2) // Rain sensor
   {
      sensor_data = (data[4] * 100);
      sensor_data += (data[3] * 10);
      sensor_data += (data[2]);
      display_RAIN(sensor_data);
   }
   else if (data[0] == 0x3)
   { // wind sensor
      sensor_data = (data[4] * 100);
      sensor_data += (data[3] * 10);
      sensor_data += (data[2]);
      display_WINSP(sensor_data);

      sensor_data = (((data[7]) >> 2) * 100);
      sensor_data += (data[6] * 10);
      sensor_data += (data[5]);
      sensor_data *= 2;  // In two step
      sensor_data /= 45; // / 22.5
      display_WINDIR(sensor_data);
   }
   else if (data[0] == 0x5) // UV sensor
   {
      sensor_data = (data[4] * 100);
      sensor_data += (data[3] * 10);
      sensor_data += (data[2]);
      display_UV(sensor_data);
   }
   display_Footer();
   //==================================================================================
   RawSignal.Repeats = true;
   RawSignal.Number = 0;
   return true;
}
#endif // PLUGIN_041
