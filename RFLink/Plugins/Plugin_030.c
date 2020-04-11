//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-30 AlectoV1                                         ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding the protocol used for outdoor sensors of the Alecto weather stations
 * following protocol version 1
 * This Plugin works at least with: Alecto WS3500, Silvercrest, Otio sht-10, Otio sht-20
 *                                  Auriol H13726, Ventus WS155, Hama EWS 1500, Meteoscan W155/W160
 *                                  Alecto WS4500, Ventus W044, Balance RF-WS105
 *
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : StuntTeam 2015..2016
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited! *********************************************************************************************
 * Technical information :
 * Message Format: (9 nibbles, 36 bits):
 *
 * Format for Temperature Humidity
 * -------------------------------
 *  | N0 | N1 | N2 | N3 | N4 | N5 | N6 | N7 | CS |
 *   IIII IIII BMMP TTTT TTTT TTTT HHHH HHHH CCCC
 *   RC        Type Temperature___ Humidity  Checksum
 *   0011 0110 1000 1011 0111 0000 0001 1100 0110
 *
 *   I =  8 bits Rolling Code (includes channel number on low 2 bits of nibble1 (10=ch1 01=ch2 11=ch3) )
 *   B =  1 bit  Battery status (0 normal, 1 voltage is below ~2.6 V)
 *   M =  2 bit  Message type (Anything but '11')
 *   P =  1 bit  Transmission (0 regular, 1 requested by pushbutton)
 *   T = 12 bits Temperature (two's complement)
 *   H =  8 bits Humidity (BCD format)
 *   C =  4 bits Checksum
 *
 * Sample:
 * 20;F4;DEBUG;Pulses=74;Pulses(uSec)=450,1900,350,1900,350,3975,350,3975,350,1900,350,3975,350,3975,350,1900,350,3975,350,1900,350,1900,350,1900,350,3975,350,1900,350,3975,350,3975,350,1900,350,3975,350,3975,350,3975,350,1900,350,1900,350,1900,350,1900,350,1900,350,1900,350,1900,350,3975,350,3975,350,3975,350,1900,350,1900,350,1900,350,3975,350,3975,350,2025,350;
 * 20;F5;Alecto V1;ID=006c;TEMP=00ed;HUM=38;
 *
 * Format for Rain
 * ---------------
 *  | N0 | N1 | N2 | N3 | N4 | N5 | N6 | N7 | CS |
 *   IIII IIII BMMP SSSS RRRR RRRR RRRR RRRR CCCC
 *   RC        Type STyp      Rain           Checksum
 *   0110 0001 0110 1100 1010 1011 0010 0000 0010
 *
 *   I =  8 bits Rolling Code (includes channel number on low 2 bits of nibble1 (10=ch1 01=ch2 11=ch3) )
 *   B =  1 bit  Battery status (0 normal, 1 voltage is below ~2.6 V)
 *   M =  2 bit  Message type ('11')
 *   P =  1 bit  Transmission (0 regular, 1 requested by pushbutton)
 *   S =  4 bits Message Subtype ('1100' for Rain)
 *   R = 16 bits Rain (bitvalue * 0.25 mm)
 *   C =  4 bits Checksum
 *
 * Sample:
 * 20;A8;DEBUG;Pulses=74;Pulses(uSec)=550,1925,425,4100,425,4100,425,1975,425,1975,425,1975,425,1975,425,4100,400,2000,425,4100,425,4100,425,1975,425,4100,425,4100,425,1975,425,1975,425,4100,425,1975,425,4100,400,1975,425,4100,425,1975,425,4100,425,4100,425,1975,450,1975,425,4100,450,1950,450,1950,450,1950,425,1975,450,1950,450,1950,475,1925,500,4025,475,1950,475;
 * 20;A9;Alecto V1;ID=0086;RAIN=04d5;
 *
 * Format for Windspeed
 * --------------------
 *  | N0 | N1 | N2 | N3 | N4 | N5 | N6 | N7 | CS |
 *   IIII IIII BMMP SSSS 0000 0000 WWWW WWWW CCCC
 *   RC        Type STyp           Windspeed Checksum
 *   0110 0001 0110 1000 0000 0000 0010 0000 0010
 *
 *   I =  8 bits Rolling Code (includes channel number on low 2 bits of nibble1 (10=ch1 01=ch2 11=ch3) )
 *   B =  1 bit  Battery status (0 normal, 1 voltage is below ~2.6 V)
 *   M =  2 bit  Message type ('11')
 *   P =  1 bit  Transmission (0 regular, 1 requested by pushbutton)
 *   S =  4 bits Message Subtype ('1000' for Windspeed)
 *   W =  8 bits Windspeed  (bitvalue * 0.2 m/s, correction for webapp = 3600/1000 * 0.2 * 100 = 72)
 *   C =  4 bits Checksum
 *
 *
 * Format for Winddirection & Windgust
 * -----------------------------------
 *  | N0 | N1 | N2 | N3 | N4 | N5 | N6 | N7 | CS |
 *   IIII IIII BMMP 111D DDDD DDDD GGGG GGGG CCCC
 *   RC        Type STyp Winddir   Windgust  Checksum
 *   0111 0000 0110 1111 1011 0000 0000 0000 0101
 *
 *   I =  8 bits Rolling Code (includes channel number on low 2 bits of nibble1 (10=ch1 01=ch2 11=ch3) )
 *   B =  1 bit  Battery status (0 normal, 1 voltage is below ~2.6 V)
 *   M =  2 bit  Message type ('11')
 *   P =  1 bit  Transmission (0 regular, 1 requested by pushbutton)
 *   S =  3 bits Message Subtype ('111.' for Winddir/gust)
 *   D =  9 bits Wind direction (Only 0(N) 45(NE) 90(E) 135(SE) 180(S) 225(SW) 270(W) 315(NW))
 *   G =  8 bits Windgust (bitvalue * 0.2 m/s, correction for webapp = 3600/1000 * 0.2 * 100 = 72)
 *   C =  4 bits Checksum
 *
 * Sample:
 * 20;53;DEBUG;Pulses=74;Pulses(uSec)=425,3800,350,1825,350,1825,325,1825,350,1825,325,3800,350,3800,350,1825,325,3800,350,1825,325,1800,350,1825,350,1825,325,1825,325,3800,325,1825,350,1800,350,1825,325,3825,325,3800,325,1825,325,1825,325,1800,325,1825,350,3800,325,1825,325,3800,350,1800,350,1800,350,3800,350,1825,325,1825,325,1825,325,1825,350,1825,325,1925,325;
 *
 \*********************************************************************************************/
#define ALECTOV1_PLUGIN_ID 030
#define ALECTOV1_PULSECOUNT 74

#define ALECTOV1_MIDHI 700 / RAWSIGNAL_SAMPLE_RATE
#define ALECTOV1_PULSEMAXMIN 2560 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_030
#include "../4_Display.h"

boolean Plugin_030(byte function, char *string)
{
   if (RawSignal.Number != ALECTOV1_PULSECOUNT)
      return false;

   unsigned long bitstream = 0L;
   byte data[8];
   byte checksum = 0;
   byte checksumcalc = 0;
   int temperature = 0;
   byte humidity = 0;
   unsigned int rain = 0;
   unsigned int windspeed = 0;
   unsigned int windgust = 0;
   unsigned int winddirection = 0;
   byte rc = 0;
   byte battery = 0;
   //==================================================================================
   // Get all 36 bits
   //==================================================================================
   for (byte x = 2; x <= 64; x += 2)
   {
      if (RawSignal.Pulses[x + 1] > ALECTOV1_MIDHI)
         return false; // in between pulses should be short

      bitstream >>= 1;

      if (RawSignal.Pulses[x] > ALECTOV1_PULSEMAXMIN)
         bitstream |= (0x1L << 31); // Reverses order, as number are in LSB 1st, beware N2 and N3 !
   }
   for (byte x = 66; x <= 72; x = x + 2)
   {
      checksum >>= 1;

      if (RawSignal.Pulses[x] > ALECTOV1_PULSEMAXMIN)
         checksum |= (0x1L << 3);
   }
   //==================================================================================
   // Perform a quick sanity check
   //==================================================================================
   if (bitstream == 0)
      return false;
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   if ((SignalHash != SignalHashPrevious) || (RepeatingTimer + 1000 < millis()) || ((SignalCRC != bitstream) && (SignalCRC_1 != bitstream)))
   {
      SignalCRC_1 = SignalCRC; // for mixed message burst prevention
      SignalCRC = bitstream;   // not seen the RF packet recently
   }
   else
      return true; // already seen the RF packet recently
   //==================================================================================
   // Prepare nibbles from bit stream
   //==================================================================================
   for (byte i = 0; i < 8; i++)
   {
      data[i] = ((bitstream >> (4 * i)) & 0xF);
      checksumcalc += data[i];
   }
   //==================================================================================
   // Perform checksum calculations, Alecto checksums are Rollover Checksums by design!
   //==================================================================================
   if ((data[2] & B0110) != B0110)               // Keep in mind, reversed nibble order
      checksumcalc = (0xF - checksumcalc) & 0xF; // Temperature packet
   else
   {
      if ((data[3] & B0111) == B0011)               // Keep in mind, reversed nibble order
         checksumcalc = (0x7 + checksumcalc) & 0xF; // Rain packet
      else
         checksumcalc = (0xF - checksumcalc) & 0xF; // Wind packet
   }
   if (checksum != checksumcalc)
      return false;
   //==================================================================================
   // Now process the various sensor types
   //==================================================================================
   battery = !((data[2]) & B0001); // get battery indicator
   data[2] = (data[2]) & B0110;    // prepare nibble to contain only the needed bits
   data[3] = (data[3]) & B0111;    // prepare nibble to contain only the needed bits
   //==================================================================================
   rc = (data[1] << 4) | data[0];
   char c_ID[4];
   sprintf(c_ID, "%04X", (rc & 0x03) << 2 | (rc & 0xFC));

   if ((data[2]) != B0110)
   { // nibble 2 needs to be set to something other than 'x11x' to be a temperature packet
      // Temperature packet
      temperature = (data[5] << 8) | (data[4] << 4) | data[3];
      //fix 12 bit signed number conversion
      if ((temperature & 0x800) == 0x800)
      {
         // if ((data[2 & B0110) != 0)    // Will never happen, see line #201
         //   return false;                // reject alecto v4 on alecto v1... (causing high negative temperatures with valid checksums)
         temperature = 4096 - temperature; // fix for minus temperatures
         if (temperature > 0x258)
            return false;                    // temperature out of range ( > -60.0 degrees)
         temperature = temperature | 0x8000; // turn highest bit on for minus values
      }
      else
      {
         if (temperature > 0x258)
            return false; // temperature out of range ( > 60.0 degrees)
      }
      humidity = (data[7] << 4) | data[6];
      if (humidity > 0x99)
         return false; // Humidity out of range, assume ALL data is bad?
      //==================================================================================
      // Output
      //==================================================================================
      display_Header();
      display_Name(PSTR("Alecto V1"));
      display_IDc(c_ID);
      display_TEMP(temperature);
      if (humidity < 0x99)            // Some AlectoV1 devices actually lack the humidity sensor and always report 99%
         display_HUM(humidity, true); // Only report humidity when it is below 99%
      display_BAT(battery);
      display_Footer();
      //==================================================================================
      RawSignal.Repeats = true; // suppress repeats of the same RF packet
      RawSignal.Number = 0;
      return true;
   }
   else
   {
      display_Header();
      display_Name(PSTR("Alecto V1"));
      display_IDc(c_ID);
      if ((data[3]) == B0011)
      {                                                                      // Rain packet
         rain = (data[7] << 12) | (data[6] << 8) | (data[5] << 4) | data[4]; // 0.25mm step
         rain = (rain * 10) / 4;                                             // to get 10th of mm
         //==================================================================================
         // Output
         //==================================================================================
         display_RAIN(rain);
      }
      if ((data[3]) == B0001)
      {                                        // Windspeed packet
         windspeed = (data[7] << 4) | data[6]; // 0.2m/s step
         windspeed = (windspeed * 72) / 10;    // to get 10th of kph
         //==================================================================================
         // Output
         //==================================================================================
         display_WINSP(windspeed);
      }
      if ((data[3]) == B0111)
      {                                                                                      // Winddir packet
         winddirection = (data[5] << (4 + 1)) | (data[4] << (0 + 1)) | (data[3] >> (4 - 1)); // In degree, only 8 cardinal points
         winddirection = ((winddirection * 2) / 45) & 0x0f;                                  // Divided by 22.5
         windgust = (data[7] << 4) | data[6];                                                // 0.2m/s step
         windgust = (windgust * 72) / 100;                                                   // to get kph
         //==================================================================================
         // Output
         //==================================================================================
         display_WINDIR(winddirection);
         display_WINGS(windgust);
      }
      display_BAT(battery);
      display_Footer();
      //==================================================================================
      RawSignal.Repeats = true; // suppress repeats of the same RF packet
      RawSignal.Number = 0;
      return true;
   }
   return false;
}

#endif // PLUGIN_030
