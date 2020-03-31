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
#define WS3500_PULSECOUNT 74

#ifdef PLUGIN_030
boolean Plugin_030(byte function, char *string)
{
   if (RawSignal.Number != WS3500_PULSECOUNT)
      return false;
   unsigned long bitstream = 0L;
   byte nibble0 = 0;
   byte nibble1 = 0;
   byte nibble2 = 0;
   byte nibble3 = 0;
   byte nibble4 = 0;
   byte nibble5 = 0;
   byte nibble6 = 0;
   byte nibble7 = 0;
   byte checksum = 0;
   int temperature = 0;
   byte humidity = 0;
   unsigned int rain = 0;
   unsigned int windspeed = 0;
   unsigned int windgust = 0;
   unsigned int winddirection = 0;
   byte checksumcalc = 0;
   byte rc = 0;
   byte battery = 0;
   //==================================================================================
   for (byte x = 2; x <= 64; x = x + 2)
   {
      if (RawSignal.Pulses[x + 1] * RawSignal.Multiply > 700)
         return false; // in between pulses should be short
      if (RawSignal.Pulses[x] * RawSignal.Multiply > 2560)
      {
         // Reverses order, as number are in LSB 1st
         // Imply all nibbles are reversed, especially N2 and N3 !
         bitstream = ((bitstream >> 1) | (0x1L << 31));
      }
      else
      {
         bitstream = (bitstream >> 1);
      }
   }
   for (byte x = 66; x <= 72; x = x + 2)
   {
      if (RawSignal.Pulses[x] * RawSignal.Multiply > 2560)
      {
         checksum = ((checksum >> 1) | (0x1L << 3));
      }
      else
      {
         checksum = (checksum >> 1);
      }
   }
   //==================================================================================
   if (bitstream == 0)
      return false; // Perform a sanity check
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   if ((SignalHash != SignalHashPrevious) || ((RepeatingTimer + 1000 < millis()) && (SignalCRC != bitstream)) || (SignalCRC != bitstream))
   {
      // not seen the RF packet recently
      SignalCRC = bitstream;
   }
   else
   {
      // already seen the RF packet recently
      return true;
   }
   //==================================================================================
   // Sort nibbles
   nibble0 = (bitstream >> 0) & 0xf;
   nibble1 = (bitstream >> 4) & 0xF;
   nibble2 = (bitstream >> 8) & 0xF;
   nibble3 = (bitstream >> 12) & 0xF;
   nibble4 = (bitstream >> 16) & 0xF;
   nibble5 = (bitstream >> 20) & 0xF;
   nibble6 = (bitstream >> 24) & 0xF;
   nibble7 = (bitstream >> 28) & 0xF;

   //==================================================================================
   // Perform checksum calculations, Alecto checksums are Rollover Checksums by design!
   if ((nibble2 & B0110) != B0110) // Keep in mind, reversed nibble order
   {                               // Temperature packet
      checksumcalc = (0xF - nibble0 - nibble1 - nibble2 - nibble3 - nibble4 - nibble5 - nibble6 - nibble7) & 0xF;
   }
   else
   {
      if ((nibble3 & B0111) == B0011) // Keep in mind, reversed nibble order
      {                               // Rain packet
         checksumcalc = (0x7 + nibble0 + nibble1 + nibble2 + nibble3 + nibble4 + nibble5 + nibble6 + nibble7) & 0xF;
      }
      else //
      {    // Wind packet
         checksumcalc = (0xF - nibble0 - nibble1 - nibble2 - nibble3 - nibble4 - nibble5 - nibble6 - nibble7) & 0xF;
      }
   }
   if (checksum != checksumcalc)
      return false;
   //==================================================================================
   battery = (nibble2)&B0001; // get battery indicator
   nibble2 = (nibble2)&B0110; // prepare nibble to contain only the needed bits
   nibble3 = (nibble3)&B0111; // prepare nibble to contain only the needed bits
   //==================================================================================
   rc = bitstream & 0xFF;

   if ((nibble2) != B0110)
   { // nibble 2 needs to be set to something other than 'x11x' to be a temperature packet
      // Temperature packet
      temperature = (bitstream >> 12) & 0xFFF;
      //fix 12 bit signed number conversion
      if ((temperature & 0x800) == 0x800)
      {
         // if ((nibble2 & B0110) != 0)      // Will never happen, see line #201
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
      humidity = (nibble7 << 4) | nibble6;
      if (humidity > 0x99)
         return false; // Humidity out of range, assume ALL data is bad?
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number
      Serial.print(pbuffer);
      // ----------------------------------
      Serial.print(F("Alecto V1;"));                              // Label
      sprintf(pbuffer, "ID=%02x%02x;", (rc & 0x03), (rc & 0xfc)); // ID is split into channel number and rolling code
      Serial.print(pbuffer);
      sprintf(pbuffer, "TEMP=%04x;", temperature);
      Serial.print(pbuffer);
      if (humidity < 0x99)
      {                                           // Some AlectoV1 devices actually lack the humidity sensor and always report 99%
         sprintf(pbuffer, "HUM=%02x;", humidity); // Only report humidity when it is below 99%
         Serial.print(pbuffer);
      }
      if (battery == 0)
      {
         Serial.print("BAT=OK;");
      }
      else
      {
         Serial.print("BAT=LOW;");
      }
      Serial.println();
      //==================================================================================
      RawSignal.Repeats = true; // suppress repeats of the same RF packet
      RawSignal.Number = 0;
      return true;
   }
   else
   {
      if ((nibble3) == B0011)
         rain = ((bitstream >> 16) & (0xFFFF)); // 0.25mm step
      {                                                                      // Rain packet
         rain = (rain * 10) / 4;                                             // to get 10th of mm
         //==================================================================================
         // Output
         // ----------------------------------
         sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number
         Serial.print(pbuffer);
         // ----------------------------------
         Serial.print(F("Alecto V1;"));      // Label
         sprintf(pbuffer, "ID=00%02x;", rc); // ID
         Serial.print(pbuffer);
         sprintf(pbuffer, "RAIN=%04x;", rain);
         Serial.print(pbuffer);
         if (battery == 0)
         {
            Serial.print("BAT=OK;");
         }
         else
         {
            Serial.print("BAT=LOW;");
         }
         Serial.println();
         //==================================================================================
         RawSignal.Repeats = true; // suppress repeats of the same RF packet
         RawSignal.Number = 0;
         return true;
      }
      if ((nibble3) == B0001)
         windspeed = ((bitstream >> 24) & 0xFF);
      {                                        // Windspeed packet
         windspeed = (windspeed * 72) / 10;    // to get 10th of kph
         //==================================================================================
         // Output
         // ----------------------------------
         sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number
         Serial.print(pbuffer);
         // ----------------------------------
         Serial.print(F("Alecto V1;"));      // Label
         sprintf(pbuffer, "ID=00%02x;", rc); // ID
         Serial.print(pbuffer);
         sprintf(pbuffer, "WINSP=%04x;", windspeed);
         Serial.print(pbuffer);
         if (battery == 0)
         {
            Serial.print("BAT=OK;");
         }
         else
         {
            Serial.print("BAT=LOW;");
         }
         Serial.println();
         //==================================================================================
         RawSignal.Repeats = true; // suppress repeats of the same RF packet
         RawSignal.Number = 0;
         return true;
      }
      if ((nibble3) == B0111)
      { // Winddir packet
         winddirection = ((bitstream >> 15) & 0x1ff) / 45;
         winddirection = winddirection & 0x0f;
         windgust = ((bitstream >> 24) & 0xff);
         windgust = (windgust * 72) / 10;     // to get 10th of kph
         //==================================================================================
         // Output
         // ----------------------------------
         sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number
         Serial.print(pbuffer);
         // ----------------------------------
         Serial.print(F("Alecto V1;"));      // Label
         sprintf(pbuffer, "ID=00%02x;", rc); // ID
         Serial.print(pbuffer);
         sprintf(pbuffer, "WINDIR=%04d;", winddirection);
         Serial.print(pbuffer);
         sprintf(pbuffer, "WINGS=%04x;", windgust);
         Serial.print(pbuffer);
         if (battery == 0)
         {
            Serial.print("BAT=OK;");
         }
         else
         {
            Serial.print("BAT=LOW;");
         }
         Serial.println();
         //==================================================================================
         RawSignal.Repeats = true; // suppress repeats of the same RF packet
         RawSignal.Number = 0;
         return true;
      }
   }
   return false;
}
#endif // PLUGIN_030
