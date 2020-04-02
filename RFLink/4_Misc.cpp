// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#include <Arduino.h>
#include "2_Signal.h"
#include "3_Serial.h"
#include "6_WiFi_MQTT.h"

/*********************************************************************************************\
   Convert HEX or DEC tring to unsigned long HEX, DEC
  \*********************************************************************************************/
unsigned long str2int(char *string)
{
  return (strtoul(string, NULL, 0));
}
/*********************************************************************************************\
   Convert string to command code
  \*********************************************************************************************/
/*
  int str2cmd(char *command) {
  if (strcasecmp(command, "ON") == 0) return VALUE_ON;
  if (strcasecmp(command, "OFF") == 0) return VALUE_OFF;
  if (strcasecmp(command, "ALLON") == 0) return VALUE_ALLON;
  if (strcasecmp(command, "ALLOFF") == 0) return VALUE_ALLOFF;
  if (strcasecmp(command, "PAIR") == 0) return VALUE_PAIR;
  if (strcasecmp(command, "DIM") == 0) return VALUE_DIM;
  if (strcasecmp(command, "BRIGHT") == 0) return VALUE_BRIGHT;
  if (strcasecmp(command, "UP") == 0) return VALUE_UP;
  if (strcasecmp(command, "DOWN") == 0) return VALUE_DOWN;
  if (strcasecmp(command, "STOP") == 0) return VALUE_STOP;
  if (strcasecmp(command, "CONFIRM") == 0) return VALUE_CONFIRM;
  if (strcasecmp(command, "LIMIT") == 0) return VALUE_LIMIT;
  return false;
  }
*/
/********************************************************************************************\
   Convert unsigned long to float long through memory
  \*********************************************************************************************/
float ul2float(unsigned long ul)
{
  float f;
  memcpy(&f, &ul, 4);
  return f;
}
/*********************************************************************************************/
void PrintHex8(uint8_t *data, uint8_t length)
{ // prints 8-bit data in hex (lowercase)
  char tmp[length * 2 + 1];
  byte first;
  int j = 0;
  for (uint8_t i = 0; i < length; i++)
  {
    first = (data[i] >> 4) | 48;
    if (first > 57)
      tmp[j] = first + (byte)39;
    else
      tmp[j] = first;
    j++;

    first = (data[i] & 0x0F) | 48;
    if (first > 57)
      tmp[j] = first + (byte)39;
    else
      tmp[j] = first;
    j++;
  }
  tmp[length * 2] = 0;
  Serial.print(tmp);
}
/*********************************************************************************************/
// todo: make uppercase?  3a = 3 or 48 (0x30) = 0x33   >57 (0x39)   a>3a >39 >   +27
void PrintHexByte(uint8_t data)
{ // prints 8-bit value in hex (single byte)
  char tmp[3];
  byte first;
  first = (data >> 4) | 48; // or with 0x30
  if (first > 57)
    tmp[0] = first + (byte)7; // 39;  // if > 0x39 add 0x27
  else
    tmp[0] = first;

  first = (data & 0x0F) | 48;
  if (first > 57)
    tmp[1] = first + (byte)7; // 39;
  else
    tmp[1] = first;
  tmp[2] = 0;
  Serial.print(tmp);
}
/*********************************************************************************************/
// Reverse all bits in a byte
byte reverseBits(byte data)
{
  byte b = data;
  for (byte i = 0; i < 8; ++i)
  {
    data = (data << 1) | (b & 1);
    b >>= 1;
  }
  return data;
}
/*********************************************************************************************/

/*********************************************************************************************/
// Display shared func //
// ------------------- //

// Internal Print subpart
void display_Print()
{
#ifdef SERIAL_ENABLED
  Serial.print(pbuffer);
#if defined(MQTT_ACTIVATED) && (defined(ESP32) || defined(ESP8266))
  strcat(MQTTbuffer, pbuffer);
#endif
}

// Common Header
void display_Header(void)
{
  sprintf_P(pbuffer, PSTR("%S%02X"), F("20;"), PKSequenceNumber++);
  display_Print();
}

// Plugin Name
void display_Name(const char *input)
{
  sprintf_P(pbuffer, PSTR(";%S"), input);
  display_Print();
}

// Common Footer
void display_Footer(void)
{
  sprintf_P(pbuffer, PSTR("%S"), F(";\r\n"));
  display_Print();
}

// Start message
void display_Start(void)
{
  sprintf_P(pbuffer, PSTR("%S"), F("20;00;Nodo RadioFrequencyLink - RFLink Gateway V3.0 - "));
  display_Print();

  sprintf_P(pbuffer, PSTR("R%02x"), REVNR);
  display_Print();

  PKSequenceNumber++;
}

// ID=9999 => device ID (often a rolling code and/or device channel number) (Hexadecimal)
void display_ID(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04X"), F(";ID="), input);
  display_Print();
}

void display_ID(byte b1, byte b0)
{
  display_ID(((b1 << 8) | b0));
}

void display_IDn(unsigned int input, byte n)
{
  switch (n)
  {
  case 2:
    sprintf_P(pbuffer, PSTR("%S%02x"), F(";ID="), input);
    break;
  case 4:
    sprintf_P(pbuffer, PSTR("%S%04x"), F(";ID="), input);
    break;
  case 6:
    sprintf_P(pbuffer, PSTR("%S%06x"), F(";ID="), input);
    break;
  case 8:
  default:
    sprintf_P(pbuffer, PSTR("%S%08x"), F(";ID="), input);
  }
  display_Print();
}

// SWITCH=A16 => House/Unit code like A1, P2, B16 or a button number etc.
void display_SWITCH(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%d"), F(";SWITCH="), input);
  display_Print();
}

// SWITCH=A16 => House/Unit code like A1, P2, B16 or a button number etc.
void display_SWITCH(const char *input)
{
  sprintf_P(pbuffer, PSTR("%S%S"), F(";SWITCH="), input);
  display_Print();
}

// CMD=ON => Command (ON/OFF/ALLON/ALLOFF) Additional for Milight: DISCO+/DISCO-/MODE0 - MODE8
void display_CMD(boolean all, boolean on)
{
  sprintf_P(pbuffer, PSTR("%S"), F(";CMD="));
  display_Print();

  if (all == true)
  {
    sprintf_P(pbuffer, PSTR("%S"), F("ALL"));
    display_Print();
  }

  if (on == true)
    sprintf_P(pbuffer, PSTR("%S"), F("ON"));
  else
    sprintf_P(pbuffer, PSTR("%S"), F("OFF"));
  display_Print();
}

// SET_LEVEL=15 => Direct dimming level setting value (decimal value: 0-15)
void display_SET_LEVEL(byte input)
{
  sprintf_P(pbuffer, PSTR("%S%02d"), F(";SET_LEVEL="), input);
  display_Print();
}

// TEMP=9999 => Temperature celcius (hexadecimal), high bit contains negative sign, needs division by 10
void display_TEMP(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04x"), F(";TEMP="), input);
  display_Print();
}

// HUM=99 => Humidity (decimal value: 0-100 to indicate relative humidity in %)
void display_HUM(byte input, boolean bcd)
{
  if (bcd == false)
    sprintf_P(pbuffer, PSTR("%S%02d"), F(";HUM="), input);
  else
    sprintf_P(pbuffer, PSTR("%S%02x"), F(";HUM="), input);
  display_Print();
}

void display_HUM(byte input)
{
  display_HUM(input, false);
}

// BARO=9999 => Barometric pressure (hexadecimal)
void display_BARO(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04x"), F(";BARO="), input);
  display_Print();
}

// HSTATUS=99 => 0=Normal, 1=Comfortable, 2=Dry, 3=Wet
void display_HSTATUS(byte input)
{
  sprintf_P(pbuffer, PSTR("%S%02x"), F(";HSTATUS="), input);
  display_Print();
}

// BFORECAST=99 => 0=No Info/Unknown, 1=Sunny, 2=Partly Cloudy, 3=Cloudy, 4=Rain
void display_BFORECAST(byte input)
{
  sprintf_P(pbuffer, PSTR("%S%02x"), F(";BFORECAST="), input);
  display_Print();
}

// UV=9999 => UV intensity (hexadecimal)
void display_UV(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04x"), F(";UV="), input);
  display_Print();
}

// LUX=9999 => Light intensity (hexadecimal)
void display_LUX(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04x"), F(";LUX="), input);
  display_Print();
}

// BAT=OK => Battery status indicator (OK/LOW)
void display_BAT(boolean input)
{
  if (input == true)
    sprintf_P(pbuffer, PSTR("%S"), F(";BAT=OK"));
  else
    sprintf_P(pbuffer, PSTR("%S"), F(";BAT=LOW"));
  display_Print();
}

// RAIN=1234 => Total rain in mm. (hexadecimal) 0x8d = 141 decimal = 14.1 mm (needs division by 10)
void display_RAIN(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04x"), F(";RAIN="), input);
  display_Print();
}

// RAINRATE=1234 => Rain rate in mm. (hexadecimal) 0x8d = 141 decimal = 14.1 mm (needs division by 10)
void display_RAINRATE(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04x"), F(";RAINRATE="), input);
  display_Print();
}

// WINSP=9999 => Wind speed in km. p/h (hexadecimal) needs division by 10
void display_WINSP(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04x"), F(";WINSP="), input);
  display_Print();
}

// AWINSP=9999 => Average Wind speed in km. p/h (hexadecimal) needs division by 10
void display_AWINSP(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04x"), F(";AWINSP="), input);
  display_Print();
}

// WINGS=9999 => Wind Gust in km. p/h (hexadecimal)
void display_WINGS(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04x"), F(";WINGS="), input);
  display_Print();
}

// WINDIR=123 => Wind direction (integer value from 0-15) reflecting 0-360 degrees in 22.5 degree steps
void display_WINDIR(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%03d"), F(";WINDIR="), input);
  display_Print();
}

// WINCHL => wind chill (hexadecimal, see TEMP)
void display_WINCHILL(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04x"), F(";WINCHL="), input);
  display_Print();
}

// WINTMP=1234 => Wind meter temperature reading (hexadecimal, see TEMP)
void display_WINTMP(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04x"), F(";WINTMP="), input);
  display_Print();
}

// CHIME=123 => Chime/Doorbell melody number
void display_CHIME(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%03d"), F(";CHIME="), input);
  display_Print();
}

// SMOKEALERT=ON => ON/OFF
void display_SMOKEALERT(boolean input)
{
  if (input == true)
    sprintf_P(pbuffer, PSTR("%S"), F(";SMOKEALERT=ON"));
  else
    sprintf_P(pbuffer, PSTR("%S"), F(";SMOKEALERT=OFF"));
  display_Print();
}

// PIR=ON => ON/OFF
void display_PIR(boolean input)
{
  if (input == true)
    sprintf_P(pbuffer, PSTR("%S"), F(";PIR=ON"));
  else
    sprintf_P(pbuffer, PSTR("%S"), F(";PIR=OFF"));
  display_Print();
}

// CO2=1234 => CO2 air quality
void display_CO2(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04d"), F(";CO2="), input);
  display_Print();
}

// SOUND=1234 => Noise level
void display_SOUND(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04d"), F(";SOUND="), input);
  display_Print();
}

// KWATT=9999 => KWatt (hexadecimal)
void display_KWATT(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04x"), F(";KWATT="), input);
  display_Print();
}

// WATT=9999 => Watt (hexadecimal)
void display_WATT(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04x"), F(";WATT="), input);
  display_Print();
}

// CURRENT=1234 => Current phase 1
void display_CURRENT(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04d"), F(";CURRENT="), input);
  display_Print();
}

// CURRENT2=1234 => Current phase 2 (CM113)
void display_CURRENT2(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04d"), F(";CURRENT2="), input);
  display_Print();
}

// CURRENT3=1234 => Current phase 3 (CM113)
void display_CURRENT3(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04d"), F(";CURRENT3="), input);
  display_Print();
}

// DIST=1234 => Distance
void display_DIST(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04d"), F(";DIST="), input);
  display_Print();
}

// METER=1234 => Meter values (water/electricity etc.)
void display_METER(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04d"), F(";METER="), input);
  display_Print();
}

// VOLT=1234 => Voltage
void display_VOLT(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04d"), F(";VOLT="), input);
  display_Print();
}

// RGBW=9999 => Milight: provides 1 byte color and 1 byte brightness value
void display_RGBW(unsigned int input)
{
  sprintf_P(pbuffer, PSTR("%S%04x"), F(";RGBW="), input);
  display_Print();
}
