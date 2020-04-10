//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-32 AlectoV4                                         ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding the protocol used for outdoor sensors of the Alecto weather stations 
 * following protocol version 4
 * This Plugin works at least with: Banggood SKU174397, Sako CH113, Homemart/Onemall FD030 and Blokker (Dake) 1730796 outdoor sensors
 * But probably with many others as the OEM sensor is sold under many brand names
 * 
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : StuntTeam 2015..2016
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Technische informatie:
 * Message Format: (9 nibbles, 36 bits):
 *
 * Format for Temperature Humidity
 *   AAAAAAAA AAAA BCDD EEEE EEEE EEEE FFFFFFFF 
 *   01011100 0001 1000 1111 0111 1011 00001110
     01110000 0000 1111 1011 0000 0000 00000101
     10110101 0000 1x00                01001001

     01000101 1000 0110 1111 0000 1100 00100110
     01011111 1101 1000 0000 1111 0001 00001110
     01000101 1000 0010
 *
 *   A = Rolling Code
 *   B = 1 (fixed value)
 *   C = 0=scheduled transmission, 1=requested transmission (button press)
 *   D = Channel number (00=ch1 01=ch2 10=ch3)
 *   E = Temperature (two's complement)
 *   F = Humidity BCD format
 *
 * 20;3F;DEBUG;Pulses=74;Pulses(uSec)=525,1725,425,3600,425,1725,425,3600,425,3625,425,1725,425,3600,425,1725,425,1725,425,1700,425,3600,425,3600,425,3600,425,1725,425,1725,425,1725,425,1725,425,1725,400,1725,425,3600,425,1725,425,1725,425,1725,425,3600,400,1725,425,1725,425,3625,400,1725,425,1725,425,1750,400,3600,425,1725,400,1750,400,3625,425,1725,400,1725,425;
 * 20;C2;DEBUG;Pulses=76;Pulses(uSec)=325,500,250,1800,375,3650,375,1775,375,3650,375,3650,375,1775,375,3650,375,1800,350,1800,375,3650,375,3650,375,3650,375,3650,375,1775,375,1775,375,1775,375,1775,375,1775,375,1775,375,1775,375,3650,375,3650,375,3650,375,1775,375,3650,375,3650,375,1775,375,1775,375,1775,375,1775,375,1775,375,1775,375,3650,375,3650,375,3650,375,3650,375;
 * 20;3E;DEBUG;Pulses=78;Pulses(uSec)=525,250,500,375,600,1650,450,3550,475,1675,450,3550,475,3550,450,1675,450,3575,450,1675,450,1700,450,1700,450,3575,425,3600,450,3575,475,1700,425,1725,425,1725,425,1725,400,1725,425,1725,425,3625,425,1725,425,1725,425,1725,425,3600,425,1725,400,1725,425,3600,425,1725,425,1725,400,1725,425,3600,400,1725,425,1725,400,3600,425,1725,425,1725,400;
 \*********************************************************************************************/
#define ALECTOV4_PLUGIN_ID 32
#define ALECTOV4_PULSECOUNT 74

#define ALECTOV4_MIDHI 550 / RAWSIGNAL_SAMPLE_RATE
#define ALECTOV4_PULSEMIN 1500 / RAWSIGNAL_SAMPLE_RATE
#define ALECTOV4_PULSEMINMAX 2500 / RAWSIGNAL_SAMPLE_RATE
#define ALECTOV4_PULSEMAXMIN 3000 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_032
boolean Plugin_032(byte function, char *string)
{
   if (RawSignal.Number < ALECTOV4_PULSECOUNT || RawSignal.Number > (ALECTOV4_PULSECOUNT + 4))
      return false;

   unsigned long bitstream = 0L;
   int temperature = 0;
   byte humidity = 0; //bitstream2 !
   byte rc = 0;
   byte rc2 = 0;

   //==================================================================================
   // Get all 36 bits
   //==================================================================================
   byte start = 0;
   if (RawSignal.Number == (ALECTOV4_PULSECOUNT + 4))
      start = 4;
   if (RawSignal.Number == (ALECTOV4_PULSECOUNT + 2))
      start = 2;

   for (byte x = 2 + start; x <= 56 + start; x = x + 2)
   { // Get first 28 bits
      if (RawSignal.Pulses[x + 1] > ALECTOV4_MIDHI)
         return false;

      if (RawSignal.Pulses[x] > ALECTOV4_PULSEMAXMIN)
         bitstream = (bitstream << 1) | 0x01;
      else
      {
         if (RawSignal.Pulses[x] < ALECTOV4_PULSEMIN)
            return false;

         if (RawSignal.Pulses[x] > ALECTOV4_PULSEMINMAX)
            return false;

         bitstream = (bitstream << 1);
      }
   }

   for (byte x = 58 + start; x <= 72 + start; x = x + 2)
   { // Get remaining 8 bits
      if (RawSignal.Pulses[x + 1] > ALECTOV4_MIDHI)
         return false;

      if (RawSignal.Pulses[x] > ALECTOV4_PULSEMAXMIN)
         humidity = (humidity << 1) | 0x01;
      else
      {
         if (RawSignal.Pulses[x] < ALECTOV4_PULSEMIN)
            return false;

         if (RawSignal.Pulses[x] > ALECTOV4_PULSEMINMAX)
            return false;

         humidity = (humidity << 1);
      }
   }
   //==================================================================================
   // Perform a quick sanity check
   //==================================================================================
   if (bitstream == 0)
      return false;

   if (humidity == 0)
      return false; // Sanity check
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   unsigned long tmpval = (((bitstream << 8) & 0xFFF0) | humidity); // All but 8 1st ID bits ...

   if ((SignalHash != SignalHashPrevious) || ((RepeatingTimer + 500) < millis()) || (SignalCRC != tmpval))
      SignalCRC = tmpval; // not seen this RF packet recently
   else
      return true; // already seen the RF packet recently
   //==================================================================================
   // Now process the various sensor types
   //==================================================================================
   rc = (bitstream >> 20) & 0xFF;
   rc2 = (bitstream >> 12) & 0xFF;
   if (((rc2)&0x08) != 0x08)
      return false; // needs to be 1
   temperature = (bitstream & 0xFFF);
   //fix 12 bit signed number conversion
   if ((temperature & 0x800) == 0x800)
   {
      temperature = 4096 - temperature; // fix for minus temperatures
      if (temperature > 0x258)
         return false;                    // temperature out of range ( > 60.0 degrees)
      temperature = temperature | 0x8000; // turn highest bit on for minus values
   }
   else
   {
      if (temperature > 0x258)
         return false; // temperature out of range ( > 60.0 degrees)
   }
   if (humidity > 99)
      return false; // Humidity out of range
   //==================================================================================
   // Output
   //==================================================================================
   display_Header();
   display_Name(PSTR("Alecto V4"));
   char c_ID[4];
   sprintf(c_ID, "%02x%02x", rc, rc2);
   display_IDc(c_ID);
   display_TEMP(temperature);
   if (humidity < 99) // Only report valid humidty values
      display_HUM(humidity, false);
   display_Footer();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // PLUGIN_032
