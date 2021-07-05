//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                      Plugin-037: AcuRite 986                                         ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding the protocol used for AcuRite 986 Refrigerator and Freezer sensors
 * This Plugin works at least with: 
 * 
 * Author  (present)  : NorthernMan54
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : NorthernMan54 ( Borrowed heavily from rtl_433 )
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Technical Information:
 * Message Format: (10 nibbles, 40 bits):
 * 
 *     TT II II SS CC
- T - Temperature in Fahrenheit, integer, MSB = sign.
      Encoding is "Sign and magnitude", LSB first
- I - 16 bit sensor ID
      changes at each power up
- S - status/sensor type
      0x01 = Sensor 2
      0x02 = low battery
- C = CRC (CRC-8 poly 0x07, little-endian)
 *
 * Format for Temperature 
 *   TTTT TTTT IIII IIII IIII IIII SSSS SSSS CCCC CCCC 
 *   1110 0100 0100 0111 1001 0100 0000 0001 0111 1100
 *   0110 0100 0100 0111 1001 0100 0000 0000 0100 1010
 *   0100 0000 0001 0100 1111 1001 0000 0001 0101 1110
 *
 *   T - Temperature in Fahrenheit, integer, MSB = sign. Encoding is "Sign and magnitude"
 *   I - 16 bit sensor ID, changes at each power up
 *   S - status/sensor type, 0x01 = Sensor 2, 0x02 = low battery
 *
 * 20;XX;DEBUG;Pulses=176;Pulses(uSec)=1696,1472,1664,1280,256,416,256,448,256,512,256,832,256,512,256,832,256,416,256,512,256,864,256,800,256,512,256,832,224,512,256,832,256,512,256,832,256,512,256,864,256,896,256,832,224,448,256,512,256,832,256,448,224,480,256,448,256,448,256,448,224,448,224,480,224,480,224,480,224,512,224,832,224,512,224,896,224,832,224,480,224,480,224,544,224,224,224,224,1632,1504,1632,1504,1632,1504,1632,1344,224,480,224,480,224,544,224,832,224,544,224,864,224,480,224,544,224,896,224,864,224,544,224,864,224,544,224,864,224,544,224,864,224,544,224,928,224,928,224,864,224,480,224,544,224,864,224,480,224,480,224,480,224,480,224,480,224,480,224,480,192,480,192,480,192,544,192,864,224,544,224,928,224,864,192,480,192,480,192,480;
 *  \*********************************************************************************************/
#define ACURITE_PLUGIN_ID 037
#define PLUGIN_DESC_037 "AcuRite 986"
#define ACURITE_PULSECOUNT 84

#define ACURITE_MIDHI_D 2000
#define ACURITE_PULSEMIN_D 150
#define ACURITE_PULSEMINMAX_D 2500
#define ACURITE_PULSEMAXMIN_D 650


#ifdef PLUGIN_037
#include "../4_Display.h"

boolean Plugin_037(byte function, const char *string)
{
   const long ACURITE_MIDHI = ACURITE_MIDHI_D / RawSignal.Multiply;
   const long ACURITE_PULSEMAXMIN = ACURITE_PULSEMAXMIN_D / RawSignal.Multiply;

   if (RawSignal.Number < ACURITE_PULSECOUNT || RawSignal.Number > (ACURITE_PULSECOUNT + 4))
      return false;

   unsigned long bitstream = 0L;
   byte bitstream2 = 0;
   byte data[4];
   int temperature = 0;
   unsigned long rc = 0;
   byte rc2 = 0;
   byte battery = 0;
   byte status = 0;
   byte crcc;
   //==================================================================================
   // Get all 36 bits
   //==================================================================================
   // Serial.print("Bitstream: ");
   for (byte x = 0; x < 64; x += 2)
   {
      if (RawSignal.Pulses[x + 1] > ACURITE_MIDHI)
         return false; // in between pulses should be short
      bitstream <<= 1;
      if (RawSignal.Pulses[x + 1] > ACURITE_PULSEMAXMIN)
      {
         bitstream |= 0x1;
         // Serial.print("1");
      }
      else
      {
         // Serial.print("0");
      }
   }
   // Serial.print(" ");
   for (byte x = 64; x < 80; x = x + 2)
   {
      bitstream2 <<= 1;

      if (RawSignal.Pulses[x + 1] > ACURITE_PULSEMAXMIN)
      {
         bitstream2 |= 0x1;
         // Serial.print("1");
      }
      else
      {
         // Serial.print("0");
      }
   }
   // char dataPrint[9];
   //sprintf(dataPrint, "%04lx %01x", bitstream, bitstream2);
   //Serial.println("");
   //Serial.print("Datastream: ");
   //Serial.println(dataPrint);
   //==================================================================================
   // Perform a quick sanity check
   //==================================================================================
   if (bitstream == 0)
      return false;
   //==================================================================================
   // Prepare nibbles from bit stream
   //==================================================================================
   data[3] = reverse8(bitstream & 0xFF);         // 0x78
   data[2] = reverse8((bitstream >> 8) & 0xFF);  // 0x56
   data[1] = reverse8((bitstream >> 16) & 0xFF); // 0x34
   data[0] = reverse8((bitstream >> 24) & 0xFF); // 0x12

   // Serial.print("bitstream: ");
   // Serial.println(bitstream);
   // Serial.print("bitstream2: ");
   // Serial.println(bitstream2);
   //==================================================================================
   // CRC Check
   //==================================================================================
   crcc = crc8le(data, 4, 0x07, 0);
   if (crcc != reverse8(bitstream2))
   {
      //Serial.println("ERROR: crc failed.");
      //Serial.print("crcc le: ");
      //Serial.println(crcc);
      //Serial.print("crc: ");
      //Serial.println(reverse8(bitstream2));
      return false;
   }
   //==================================================================================
   // Now process the various sensor types
   //==================================================================================
   if (data[0] & 0x80)
   {
      temperature = (data[0] & 0x7f) * -1;
   }
   else
   {
      temperature = data[0];
   }
   temperature = ((temperature - 32) * 5 / 9 * 10);   // ACURITE sensors are in Farenheit
   // Serial.print("temperature: ");
   // Serial.println(temperature);
   if ( temperature < 0 ) 
      {
         temperature = (temperature * -1 ) | 0x8000;
      }

   rc = (data[1] << 8) + data[2];
   status = data[3];
   rc2 = (status & 0x01) + 1;
   status = status >> 1;
   battery = ((status & 1) == 0);
   // Serial.print("id: ");
   // Serial.println(rc);
   // Serial.print("unit: ");
   // Serial.println(rc2);
   // Serial.print("battery: ");
   // Serial.println(battery);
   // Serial.print("data[3]: ");
   // Serial.println(data[3]);
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   unsigned long tmpval = (((bitstream << 8) & 0xFFF0)); // All but 8 1st ID bits ...

   if ((SignalHash != SignalHashPrevious) || ((RepeatingTimer + 500) < millis()) || (SignalCRC != tmpval))
      SignalCRC = tmpval; // not seen this RF packet recently
   else
      return true; // already seen the RF packet recently
   //==================================================================================
   // Output
   //==================================================================================
   display_Header();
   display_Name(PSTR("AcuRite 986"));
   char c_ID[5];
   sprintf(c_ID, "%02x%02x", (rc & 0xFF), rc2);
   display_IDc(c_ID);
   display_TEMP(temperature);
   display_BAT(battery);
   display_Footer();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // PLUGIN_037
