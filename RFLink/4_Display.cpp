// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#include <Arduino.h>
#include "4_Display.h"

byte PKSequenceNumber = 0;       // 1 byte packet counter
char dbuffer[30];                // Buffer for message chunk data
char pbuffer[PRINT_BUFFER_SIZE]; // Buffer for complete message data

// ------------------- //
// Display shared func //
// ------------------- //

// Common Header
void display_Header(void)
{
  sprintf_P(dbuffer, PSTR("%S%02X"), F("20;"), PKSequenceNumber++);
  strcat(pbuffer, dbuffer);
}

// Plugin Name
void display_Name(const char *input)
{
  sprintf_P(dbuffer, PSTR(";%S"), input);
  strcat(pbuffer, dbuffer);
}

// Common Footer
void display_Footer(void)
{
  sprintf_P(dbuffer, PSTR("%S"), F(";\r\n"));
  strcat(pbuffer, dbuffer);
}

// Start message
void display_Start(void)
{
  sprintf_P(dbuffer, PSTR("%S"), F(";RFLink_ESP Gateway V3.0 - "));
  strcat(pbuffer, dbuffer);

  sprintf_P(dbuffer, PSTR("R%02x"), REVNR);
  strcat(pbuffer, dbuffer);
}

// ID=9999 => device ID (often a rolling code and/or device channel number) (Hexadecimal)
void display_IDn(unsigned int input, byte n)
{
  switch (n)
  {
  case 2:
    sprintf_P(dbuffer, PSTR("%S%02x"), F(";ID="), input);
    break;
  case 4:
    sprintf_P(dbuffer, PSTR("%S%04x"), F(";ID="), input);
    break;
  case 6:
    sprintf_P(dbuffer, PSTR("%S%06x"), F(";ID="), input);
    break;
  case 8:
  default:
    sprintf_P(dbuffer, PSTR("%S%08x"), F(";ID="), input);
  }
  strcat(pbuffer, dbuffer);
}

void display_IDc(const char *input)
{
  sprintf_P(dbuffer, PSTR("%S"), F(";ID="));
  strcat(pbuffer, dbuffer);
  strcat(pbuffer, input);
}

// SWITCH=A16 => House/Unit code like A1, P2, B16 or a button number etc.
void display_SWITCH(byte input)
{
  sprintf_P(dbuffer, PSTR("%S%02x"), F(";SWITCH="), input);
  strcat(pbuffer, dbuffer);
}

// SWITCH=A16 => House/Unit code like A1, P2, B16 or a button number etc.
void display_SWITCHc(const char *input)
{
  sprintf_P(dbuffer, PSTR("%S"), F(";SWITCH="));
  strcat(pbuffer, dbuffer);
  strcat(pbuffer, input);
}

// CMD=ON => Command (ON/OFF/ALLON/ALLOFF) Additional for Milight: DISCO+/DISCO-/MODE0 - MODE8
void display_CMD(boolean all, byte on)
{
  sprintf_P(dbuffer, PSTR("%S"), F(";CMD="));
  strcat(pbuffer, dbuffer);

  if (all == CMD_All)
  {
    sprintf_P(dbuffer, PSTR("%S"), F("ALL"));
    strcat(pbuffer, dbuffer);
  }

  switch (on)
  {
  case CMD_On:
    sprintf_P(dbuffer, PSTR("%S"), F("ON"));
    break;
  case CMD_Off:
    sprintf_P(dbuffer, PSTR("%S"), F("OFF"));
    break;
  case CMD_Bright:
    sprintf_P(dbuffer, PSTR("%S"), F("BRIGHT"));
    break;
  case CMD_Dim:
    sprintf_P(dbuffer, PSTR("%S"), F("DIM"));
    break;
  case CMD_Unknown:
  default:
    sprintf_P(dbuffer, PSTR("%S"), F("UNKNOWN"));
  }
  strcat(pbuffer, dbuffer);
}

// SET_LEVEL=15 => Direct dimming level setting value (decimal value: 0-15)
void display_SET_LEVEL(byte input)
{
  sprintf_P(dbuffer, PSTR("%S%02d"), F(";SET_LEVEL="), input);
  strcat(pbuffer, dbuffer);
}

// TEMP=9999 => Temperature celcius (hexadecimal), high bit contains negative sign, needs division by 10
void display_TEMP(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04x"), F(";TEMP="), input);
  strcat(pbuffer, dbuffer);
}

// HUM=99 => Humidity (decimal value: 0-100 to indicate relative humidity in %)
void display_HUM(byte input, boolean bcd)
{
  if (bcd == HUM_BCD)
    sprintf_P(dbuffer, PSTR("%S%02x"), F(";HUM="), input);
  else
    sprintf_P(dbuffer, PSTR("%S%02d"), F(";HUM="), input);
  strcat(pbuffer, dbuffer);
}

// BARO=9999 => Barometric pressure (hexadecimal)
void display_BARO(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04x"), F(";BARO="), input);
  strcat(pbuffer, dbuffer);
}

// HSTATUS=99 => 0=Normal, 1=Comfortable, 2=Dry, 3=Wet
void display_HSTATUS(byte input)
{
  sprintf_P(dbuffer, PSTR("%S%02x"), F(";HSTATUS="), input);
  strcat(pbuffer, dbuffer);
}

// BFORECAST=99 => 0=No Info/Unknown, 1=Sunny, 2=Partly Cloudy, 3=Cloudy, 4=Rain
void display_BFORECAST(byte input)
{
  sprintf_P(dbuffer, PSTR("%S%02x"), F(";BFORECAST="), input);
  strcat(pbuffer, dbuffer);
}

// UV=9999 => UV intensity (hexadecimal)
void display_UV(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04x"), F(";UV="), input);
  strcat(pbuffer, dbuffer);
}

// LUX=9999 => Light intensity (hexadecimal)
void display_LUX(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04x"), F(";LUX="), input);
  strcat(pbuffer, dbuffer);
}

// BAT=OK => Battery status indicator (OK/LOW)
void display_BAT(boolean input)
{
  if (input == true)
    sprintf_P(dbuffer, PSTR("%S"), F(";BAT=OK"));
  else
    sprintf_P(dbuffer, PSTR("%S"), F(";BAT=LOW"));
  strcat(pbuffer, dbuffer);
}

// RAIN=1234 => Total rain in mm. (hexadecimal) 0x8d = 141 decimal = 14.1 mm (needs division by 10)
void display_RAIN(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04x"), F(";RAIN="), input);
  strcat(pbuffer, dbuffer);
}

// RAINRATE=1234 => Rain rate in mm. (hexadecimal) 0x8d = 141 decimal = 14.1 mm (needs division by 10)
void display_RAINRATE(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04x"), F(";RAINRATE="), input);
  strcat(pbuffer, dbuffer);
}

// WINSP=9999 => Wind speed in km. p/h (hexadecimal) needs division by 10
void display_WINSP(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04x"), F(";WINSP="), input);
  strcat(pbuffer, dbuffer);
}

// AWINSP=9999 => Average Wind speed in km. p/h (hexadecimal) needs division by 10
void display_AWINSP(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04x"), F(";AWINSP="), input);
  strcat(pbuffer, dbuffer);
}

// WINGS=9999 => Wind Gust in km. p/h (hexadecimal)
void display_WINGS(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04x"), F(";WINGS="), input);
  strcat(pbuffer, dbuffer);
}

// WINDIR=123 => Wind direction (integer value from 0-15) reflecting 0-360 degrees in 22.5 degree steps
void display_WINDIR(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%03d"), F(";WINDIR="), input);
  strcat(pbuffer, dbuffer);
}

// WINCHL => wind chill (hexadecimal, see TEMP)
void display_WINCHL(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04x"), F(";WINCHL="), input);
  strcat(pbuffer, dbuffer);
}

// WINTMP=1234 => Wind meter temperature reading (hexadecimal, see TEMP)
void display_WINTMP(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04x"), F(";WINTMP="), input);
  strcat(pbuffer, dbuffer);
}

// CHIME=123 => Chime/Doorbell melody number
void display_CHIME(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%03d"), F(";CHIME="), input);
  strcat(pbuffer, dbuffer);
}

// SMOKEALERT=ON => ON/OFF
void display_SMOKEALERT(boolean input)
{
  if (input == SMOKE_On)
    sprintf_P(dbuffer, PSTR("%S"), F(";SMOKEALERT=ON"));
  else
    sprintf_P(dbuffer, PSTR("%S"), F(";SMOKEALERT=OFF"));
  strcat(pbuffer, dbuffer);
}

// PIR=ON => ON/OFF
void display_PIR(boolean input)
{
  if (input == PIR_On)
    sprintf_P(dbuffer, PSTR("%S"), F(";PIR=ON"));
  else
    sprintf_P(dbuffer, PSTR("%S"), F(";PIR=OFF"));
  strcat(pbuffer, dbuffer);
}

// CO2=1234 => CO2 air quality
void display_CO2(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04d"), F(";CO2="), input);
  strcat(pbuffer, dbuffer);
}

// SOUND=1234 => Noise level
void display_SOUND(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04d"), F(";SOUND="), input);
  strcat(pbuffer, dbuffer);
}

// KWATT=9999 => KWatt (hexadecimal)
void display_KWATT(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04x"), F(";KWATT="), input);
  strcat(pbuffer, dbuffer);
}

// WATT=9999 => Watt (hexadecimal)
void display_WATT(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04x"), F(";WATT="), input);
  strcat(pbuffer, dbuffer);
}

// CURRENT=1234 => Current phase 1
void display_CURRENT(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04d"), F(";CURRENT="), input);
  strcat(pbuffer, dbuffer);
}

// DIST=1234 => Distance
void display_DIST(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04d"), F(";DIST="), input);
  strcat(pbuffer, dbuffer);
}

// METER=1234 => Meter values (water/electricity etc.)
void display_METER(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04d"), F(";METER="), input);
  strcat(pbuffer, dbuffer);
}

// VOLT=1234 => Voltage
void display_VOLT(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04d"), F(";VOLT="), input);
  strcat(pbuffer, dbuffer);
}

// RGBW=9999 => Milight: provides 1 byte color and 1 byte brightness value
void display_RGBW(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%S%04x"), F(";RGBW="), input);
  strcat(pbuffer, dbuffer);
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