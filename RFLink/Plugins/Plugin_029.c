//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-029 DKW2012/ACH2010                                 ##
//#######################################################################################################

/*********************************************************************************************\
 * Dit protocol zorgt voor ontvangst van Alecto weerstation buitensensoren
 * 
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : Martinus van den Broek 2015..2016
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Technische informatie:
 * DKW2012 Message Format: (11 Bytes, 88 bits):
 * AAAAAAAA AAAABBBB BBBB__CC CCCCCCCC DDDDDDDD EEEEEEEE FFFFFFFF GGGGGGGG GGGGGGGG HHHHHHHH IIIIIIII
 *                         Temperature Humidity Windspd_ Windgust Rain____ ________ Winddir  Checksum
 * A = start/unknown, first 8 bits are always 11111111
 * B = Rolling code
 * C = Temperature (10 bit value with -400 base)
 * D = Humidity
 * E = windspeed (* 0.3 m/s, correction for webapp = 3600/1000 * 0.3 * 100 = 108))
 * F = windgust (* 0.3 m/s, correction for webapp = 3600/1000 * 0.3 * 100 = 108))
 * G = Rain ( * 0.3 mm)
 * H = winddirection (0 = north, 4 = east, 8 = south 12 = west)
 * I = Checksum, calculation is still under investigation
 *
 * WS3000 and ACH2010 systems have no winddirection, message format is 8 bit shorter
 * Message Format: (10 Bytes, 80 bits):
 * AAAAAAAA AAAABBBB BBBB__CC CCCCCCCC DDDDDDDD EEEEEEEE FFFFFFFF GGGGGGGG GGGGGGGG HHHHHHHH
 *                         Temperature Humidity Windspd_ Windgust Rain____ ________ Checksum
 * 
 * --------------------------------------------------------------------------------------------
 * DCF Time Message Format: (NOT DECODED!, we already have time sync through webapp)
 * AAAAAAAA BBBBCCCC DDDDDDDD EFFFFFFF GGGGGGGG HHHHHHHH IIIIIIII JJJJJJJJ KKKKKKKK LLLLLLLL MMMMMMMM
 * 	    11                 Hours   Minutes  Seconds  Year     Month    Day      ?        Checksum
 * B = 11 = DCF
 * C = ?
 * D = ?
 * E = ?
 * F = Hours BCD format (7 bits only for this byte, MSB could be '1')
 * G = Minutes BCD format
 * H = Seconds BCD format
 * I = Year BCD format (only two digits!)
 * J = Month BCD format
 * K = Day BCD format
 * L = ?
 * M = Checksum
 \*********************************************************************************************/
#define DKW2012_PLUGIN_ID 029

#define ACH2010_MIN_PULSECOUNT 160 // reduce this value (144?) in case of bad reception
#define ACH2010_MAX_PULSECOUNT 160
#define DKW2012_MIN_PULSECOUNT 172
#define DKW2012_MAX_PULSECOUNT 182

#define DKW2012_PULSEMINMAX 768 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_029
#include "../4_Display.h"

uint8_t Plugin_029_ProtocolAlectoCRC8(uint8_t *addr, uint8_t len);

boolean Plugin_029(byte function, char *string)
{
  if (!(
          ((RawSignal.Number >= ACH2010_MIN_PULSECOUNT) &&
           (RawSignal.Number <= ACH2010_MAX_PULSECOUNT)) ||
          ((RawSignal.Number >= DKW2012_MIN_PULSECOUNT) &&
           (RawSignal.Number <= DKW2012_MAX_PULSECOUNT))))
    return false;

  byte c = 0;
  byte data[10];
  byte msgtype = 0;
  byte rc = 0;
  byte checksum = 0;
  byte checksumcalc = 0;
  byte maxidx = 8;
  //==================================================================================
  if (RawSignal.Number > ACH2010_MAX_PULSECOUNT)
    maxidx = 9;
  byte idx = maxidx;
  //==================================================================================
  // Get all 8x11 bits
  //==================================================================================
  // Get message back to front as the header is almost never received complete for ACH2010
  for (byte x = RawSignal.Number; x > 0; x -= 2)
  {
    data[idx] >>= 1; // Always shift
    if (RawSignal.Pulses[x - 1] < DKW2012_PULSEMINMAX)
      data[idx] |= 0x80;
    // else
    //  data[idx] |= 0x00;

    if (++c == 8)
    {
      if (idx-- == 0)
        break;
      c = 0;
    }
  }
  //==================================================================================
  // Perform a quick sanity check
  //==================================================================================
  msgtype = (data[0] >> 4) & 0xF; // msg type must be 5 or 10
  if ((msgtype != 0xA) && (msgtype != 0x5))
    return false; // why true? -> Time format?
  //==================================================================================
  // Perform checksum calculations, Alecto checksums are Rollover Checksums by design!
  //==================================================================================
  checksum = data[maxidx];
  checksumcalc = Plugin_029_ProtocolAlectoCRC8(data, maxidx);
  if (checksum != checksumcalc)
    return false;
  //==================================================================================
  // Prevent repeating signals from showing up
  //==================================================================================
  unsigned long tmpval = data[0] << 8 | data[1];

  if ((SignalHash != SignalHashPrevious) || (RepeatingTimer + 1000 < millis()) || (SignalCRC != tmpval))
    SignalCRC = tmpval; // not seen the RF packet recently
  else
    return true; // already seen the RF packet recently
  //==================================================================================
  // Now process the various sensor types
  //==================================================================================
  rc = (data[0] << 4) | (data[1] >> 4); // rolling code
  byte bat = !((data[1] >> 3) & 0x1); // supposed bat bit
  int temp = 0;
  unsigned int rain = 0;
  byte hum = 0;
  unsigned int wdir = 0;
  unsigned int wspeed = 0;
  unsigned int wgust = 0;

  temp = (((data[1] & 0x3) << 8 | data[2]) - 400);
  hum = data[3];
  wspeed = data[4] * 245;
  wspeed /= 20;
  wgust = data[5] * 245;
  wgust /= 20;
  rain = (data[6] << 8) | data[7];
  rain *= 3;
  if (RawSignal.Number >= DKW2012_MIN_PULSECOUNT)
  {
    wdir = (data[8] & 0xF);
  }
  //==================================================================================
  // Output
  //==================================================================================
  display_Header();
  if (RawSignal.Number >= DKW2012_MIN_PULSECOUNT)
    display_Name(PSTR("DKW2012"));
  else
    display_Name(PSTR("Alecto V2"));
  display_IDn(rc, 4);
  display_TEMP(temp);
  display_HUM(hum, HUM_HEX);
  display_WINSP(wspeed);
  display_WINGS(wgust);
  display_RAIN(rain);
  if (RawSignal.Number >= DKW2012_MIN_PULSECOUNT)
    display_WINDIR(wdir);
  display_BAT(bat);
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
uint8_t Plugin_029_ProtocolAlectoCRC8(uint8_t *addr, uint8_t len)
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
      inbyte <<= 1;  // changed from right shift
    }
  }
  return crc;
}
#endif // PLUGIN_029
