// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#include <Arduino.h>
#include "RFLink.h"
#include "3_Serial.h"
#include "4_Display.h"

byte PKSequenceNumber = 0;       // 1 byte packet counter
char dbuffer[30];                // Buffer for message chunk data
char pbuffer[PRINT_BUFFER_SIZE]; // Buffer for complete message data

// ------------------- //
// Display shared func //
// ------------------- //

#if (defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__))
#error "For AVR plaforms, in all sprintf_P above, please replace %s with %S"
#endif

// Common Header
void display_Header(void)
{
  sprintf_P(dbuffer, PSTR("%s%02X"), PSTR("20;"), PKSequenceNumber++);
  strcat(pbuffer, dbuffer);
}

// Plugin Name
void display_Name(const char *input)
{
  sprintf_P(dbuffer, PSTR(";%s"), input);
  strcat(pbuffer, dbuffer);
}

// Common Footer
void display_Footer(void)
{
  sprintf_P(dbuffer, PSTR("%s"), PSTR(";\r\n"));
  strcat(pbuffer, dbuffer);
}

// Start message
void display_Splash(void)
{
  sprintf_P(dbuffer, PSTR("%s%d.%d"), PSTR(";RFLink_ESP;VER="), BUILDNR, REVNR);
  strcat(pbuffer, dbuffer);
}

// ID=9999 => device ID (often a rolling code and/or device channel number) (Hexadecimal)
void display_IDn(unsigned long input, byte n)
{
  switch (n)
  {
  case 2:
    sprintf_P(dbuffer, PSTR("%s%02lx"), PSTR(";ID="), input);
    break;
  case 4:
    sprintf_P(dbuffer, PSTR("%s%04lx"), PSTR(";ID="), input);
    break;
  case 6:
    sprintf_P(dbuffer, PSTR("%s%06lx"), PSTR(";ID="), input);
    break;
  case 8:
  default:
    sprintf_P(dbuffer, PSTR("%s%08lx"), PSTR(";ID="), input);
  }
  strcat(pbuffer, dbuffer);
}

void display_IDc(const char *input)
{
  sprintf_P(dbuffer, PSTR("%s"), PSTR(";ID="));
  strcat(pbuffer, dbuffer);
  strcat(pbuffer, input);
}

// SWITCH=A16 => House/Unit code like A1, P2, B16 or a button number etc.
void display_SWITCH(byte input)
{
  sprintf_P(dbuffer, PSTR("%s%02x"), PSTR(";SWITCH="), input);
  strcat(pbuffer, dbuffer);
}

// SWITCH=A16 => House/Unit code like A1, P2, B16 or a button number etc.
void display_SWITCHc(const char *input)
{
  sprintf_P(dbuffer, PSTR("%s"), PSTR(";SWITCH="));
  strcat(pbuffer, dbuffer);
  strcat(pbuffer, input);
}

// CMD=ON => Command (ON/OFF/ALLON/ALLOFF) Additional for Milight: DISCO+/DISCO-/MODE0 - MODE8
void display_CMD(boolean all, byte on)
{
  sprintf_P(dbuffer, PSTR("%s"), PSTR(";CMD="));
  strcat(pbuffer, dbuffer);

  if (all == CMD_All)
  {
    sprintf_P(dbuffer, PSTR("%s"), PSTR("ALL"));
    strcat(pbuffer, dbuffer);
  }

  switch (on)
  {
  case CMD_On:
    sprintf_P(dbuffer, PSTR("%s"), PSTR("ON"));
    break;
  case CMD_Off:
    sprintf_P(dbuffer, PSTR("%s"), PSTR("OFF"));
    break;
  case CMD_Bright:
    sprintf_P(dbuffer, PSTR("%s"), PSTR("BRIGHT"));
    break;
  case CMD_Dim:
    sprintf_P(dbuffer, PSTR("%s"), PSTR("DIM"));
    break;
  case CMD_Unknown:
  default:
    sprintf_P(dbuffer, PSTR("%s"), PSTR("UNKNOWN"));
  }
  strcat(pbuffer, dbuffer);
}

// SET_LEVEL=15 => Direct dimming level setting value (decimal value: 0-15)
void display_SET_LEVEL(byte input)
{
  sprintf_P(dbuffer, PSTR("%s%02d"), PSTR(";SET_LEVEL="), input);
  strcat(pbuffer, dbuffer);
}

// TEMP=9999 => Temperature celcius (hexadecimal), high bit contains negative sign, needs division by 10
void display_TEMP(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04x"), PSTR(";TEMP="), input);
  strcat(pbuffer, dbuffer);
}

// HUM=99 => Humidity (decimal value: 0-100 to indicate relative humidity in %)
void display_HUM(byte input, boolean bcd)
{
  if (bcd == HUM_BCD)
    sprintf_P(dbuffer, PSTR("%s%02x"), PSTR(";HUM="), input);
  else
    sprintf_P(dbuffer, PSTR("%s%02d"), PSTR(";HUM="), input);
  strcat(pbuffer, dbuffer);
}

// BARO=9999 => Barometric pressure (hexadecimal)
void display_BARO(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04x"), PSTR(";BARO="), input);
  strcat(pbuffer, dbuffer);
}

// HSTATUS=99 => 0=Normal, 1=Comfortable, 2=Dry, 3=Wet
void display_HSTATUS(byte input)
{
  sprintf_P(dbuffer, PSTR("%s%02x"), PSTR(";HSTATUS="), input);
  strcat(pbuffer, dbuffer);
}

// BFORECAST=99 => 0=No Info/Unknown, 1=Sunny, 2=Partly Cloudy, 3=Cloudy, 4=Rain
void display_BFORECAST(byte input)
{
  sprintf_P(dbuffer, PSTR("%s%02x"), PSTR(";BFORECAST="), input);
  strcat(pbuffer, dbuffer);
}

// UV=9999 => UV intensity (hexadecimal)
void display_UV(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04x"), PSTR(";UV="), input);
  strcat(pbuffer, dbuffer);
}

// LUX=9999 => Light intensity (hexadecimal)
void display_LUX(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04x"), PSTR(";LUX="), input);
  strcat(pbuffer, dbuffer);
}

// BAT=OK => Battery status indicator (OK/LOW)
void display_BAT(boolean input)
{
  if (input == true)
    sprintf_P(dbuffer, PSTR("%s"), PSTR(";BAT=OK"));
  else
    sprintf_P(dbuffer, PSTR("%s"), PSTR(";BAT=LOW"));
  strcat(pbuffer, dbuffer);
}

// RAIN=1234 => Total rain in mm. (hexadecimal) 0x8d = 141 decimal = 14.1 mm (needs division by 10)
void display_RAIN(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04x"), PSTR(";RAIN="), input);
  strcat(pbuffer, dbuffer);
}

// RAINRATE=1234 => Rain rate in mm. (hexadecimal) 0x8d = 141 decimal = 14.1 mm (needs division by 10)
void display_RAINRATE(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04x"), PSTR(";RAINRATE="), input);
  strcat(pbuffer, dbuffer);
}

// WINSP=9999 => Wind speed in km. p/h (hexadecimal) needs division by 10
void display_WINSP(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04x"), PSTR(";WINSP="), input);
  strcat(pbuffer, dbuffer);
}

// AWINSP=9999 => Average Wind speed in km. p/h (hexadecimal) needs division by 10
void display_AWINSP(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04x"), PSTR(";AWINSP="), input);
  strcat(pbuffer, dbuffer);
}

// WINGS=9999 => Wind Gust in km. p/h (hexadecimal)
void display_WINGS(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04x"), PSTR(";WINGS="), input);
  strcat(pbuffer, dbuffer);
}

// WINDIR=123 => Wind direction (integer value from 0-15) reflecting 0-360 degrees in 22.5 degree steps
void display_WINDIR(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%03d"), PSTR(";WINDIR="), input);
  strcat(pbuffer, dbuffer);
}

// WINCHL => wind chill (hexadecimal, see TEMP)
void display_WINCHL(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04x"), PSTR(";WINCHL="), input);
  strcat(pbuffer, dbuffer);
}

// WINTMP=1234 => Wind meter temperature reading (hexadecimal, see TEMP)
void display_WINTMP(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04x"), PSTR(";WINTMP="), input);
  strcat(pbuffer, dbuffer);
}

// CHIME=123 => Chime/Doorbell melody number
void display_CHIME(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%03d"), PSTR(";CHIME="), input);
  strcat(pbuffer, dbuffer);
}

// SMOKEALERT=ON => ON/OFF
void display_SMOKEALERT(boolean input)
{
  if (input == SMOKE_On)
    sprintf_P(dbuffer, PSTR("%s"), PSTR(";SMOKEALERT=ON"));
  else
    sprintf_P(dbuffer, PSTR("%s"), PSTR(";SMOKEALERT=OFF"));
  strcat(pbuffer, dbuffer);
}

// PIR=ON => ON/OFF
void display_PIR(boolean input)
{
  if (input == PIR_On)
    sprintf_P(dbuffer, PSTR("%s"), PSTR(";PIR=ON"));
  else
    sprintf_P(dbuffer, PSTR("%s"), PSTR(";PIR=OFF"));
  strcat(pbuffer, dbuffer);
}

// CO2=1234 => CO2 air quality
void display_CO2(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04d"), PSTR(";CO2="), input);
  strcat(pbuffer, dbuffer);
}

// SOUND=1234 => Noise level
void display_SOUND(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04d"), PSTR(";SOUND="), input);
  strcat(pbuffer, dbuffer);
}

// KWATT=9999 => KWatt (hexadecimal)
void display_KWATT(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04x"), PSTR(";KWATT="), input);
  strcat(pbuffer, dbuffer);
}

// WATT=9999 => Watt (hexadecimal)
void display_WATT(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04x"), PSTR(";WATT="), input);
  strcat(pbuffer, dbuffer);
}

// CURRENT=1234 => Current phase 1
void display_CURRENT(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04d"), PSTR(";CURRENT="), input);
  strcat(pbuffer, dbuffer);
}

// DIST=1234 => Distance
void display_DIST(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04d"), PSTR(";DIST="), input);
  strcat(pbuffer, dbuffer);
}

// METER=1234 => Meter values (water/electricity etc.)
void display_METER(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04d"), PSTR(";METER="), input);
  strcat(pbuffer, dbuffer);
}

// VOLT=1234 => Voltage
void display_VOLT(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04d"), PSTR(";VOLT="), input);
  strcat(pbuffer, dbuffer);
}

// RGBW=9999 => Milight: provides 1 byte color and 1 byte brightness value
void display_RGBW(unsigned int input)
{
  sprintf_P(dbuffer, PSTR("%s%04x"), PSTR(";RGBW="), input);
  strcat(pbuffer, dbuffer);
}

// --------------------- //
// get label shared func //
// --------------------- //

char *ptr;
const char c_delim[2] = ";";
char c_label[12];

void retrieve_Init()
{
  ptr = strtok(InputBuffer_Serial, c_delim);
}

boolean retrieve_Name(const char *c_Name)
{
  if (ptr != NULL)
  {
    if (strncasecmp(ptr, c_Name, strlen(c_Name)) != 0)
      return false;
    ptr = strtok(NULL, c_delim);
    return true;
  }
  else
    return false;
}

boolean retrieve_ID(unsigned long &ul_ID)
{
  // ID
  char c_ID[10];

  if (ptr != NULL)
  {
    strcpy(c_label, "ID=");
    if (strncasecmp(ptr, c_label, strlen(c_label)) == 0)
      ptr += strlen(c_label);

    if (strlen(ptr) > 8)
      return false;

    for (byte i = 0; i < strlen(ptr); i++)
      if (!isxdigit(ptr[i]))
        return false;

    strcpy(c_ID, ptr);
    c_ID[8] = 0;

    ul_ID = strtoul(c_ID, NULL, HEX);
    ul_ID &= 0x03FFFFFF;

    ptr = strtok(NULL, c_delim);
    return true;
  }
  else
    return false;
}

boolean retrieve_Switch(byte &b_Switch)
{
  // Switch
  char c_Switch[10];

  if (ptr != NULL)
  {
    strcpy(c_label, "SWITCH=");
    if (strncasecmp(ptr, c_label, strlen(c_label)) == 0)
      ptr += strlen(c_label);

    if (strlen(ptr) > 1)
      return false;

    for (byte i = 0; i < strlen(ptr); i++)
      if (!isxdigit(ptr[i]))
        return false;

    strcpy(c_Switch, ptr);

    b_Switch = (byte)strtoul(c_Switch, NULL, HEX);
    b_Switch--; // 1 to 16 -> 0 to 15 (displayed value is one more)
    if (b_Switch > 0xF)
      return false; // invalid address

    ptr = strtok(NULL, c_delim);
    return true;
  }
  else
    return false;
}

boolean retrieve_Command(byte &b_Cmd, byte &b_Cmd2)
{
  // Command
  char c_Cmd[10];

  if (ptr != NULL)
  {
    strcpy(c_label, "SET_LEVEL=");
    if (strncasecmp(ptr, c_label, strlen(c_label)) == 0)
      ptr += strlen(c_label);

    strcpy(c_label, "CMD=");
    if (strncasecmp(ptr, c_label, strlen(c_label)) == 0)
      ptr += strlen(c_label);

    if (strlen(ptr) > 7)
      return false;

    for (byte i = 0; i < strlen(ptr); i++)
      if (!isalnum(ptr[i]))
        return false;

    strcpy(c_Cmd, ptr);

    b_Cmd2 = str2cmd(c_Cmd); // Get ON/OFF etc. command
    if (b_Cmd2 == false)     // Not a valid command received? ON/OFF/ALLON/ALLOFF
      b_Cmd2 = (byte)strtoul(c_Cmd, NULL, HEX);
    // ON
    switch (b_Cmd2)
    {
    case VALUE_ON:
    case VALUE_ALLON:
      b_Cmd |= B01;
      break;
    }
    // Group
    switch (b_Cmd2)
    {
    case VALUE_ALLON:
    case VALUE_ALLOFF:
      b_Cmd |= B10;
      break;
    }
    // Dimmer
    switch (b_Cmd2)
    {
    case VALUE_ON:
    case VALUE_OFF:
    case VALUE_ALLON:
    case VALUE_ALLOFF:
      b_Cmd2 = 0xFF;
      break;
    }

    ptr = strtok(NULL, c_delim);
    return true;
  }
  else
    return false;
}

boolean retrieve_End()
{
  // End
  if (ptr != NULL)
    return false;
  return true;
}

/*********************************************************************************************\
   Convert string to command code
\*********************************************************************************************/
int str2cmd(char *command)
{
  if (strcasecmp(command, "ON") == 0)
    return VALUE_ON;
  if (strcasecmp(command, "OFF") == 0)
    return VALUE_OFF;
  if (strcasecmp(command, "ALLON") == 0)
    return VALUE_ALLON;
  if (strcasecmp(command, "ALLOFF") == 0)
    return VALUE_ALLOFF;
  if (strcasecmp(command, "PAIR") == 0)
    return VALUE_PAIR;
  if (strcasecmp(command, "DIM") == 0)
    return VALUE_DIM;
  if (strcasecmp(command, "BRIGHT") == 0)
    return VALUE_BRIGHT;
  if (strcasecmp(command, "UP") == 0)
    return VALUE_UP;
  if (strcasecmp(command, "DOWN") == 0)
    return VALUE_DOWN;
  if (strcasecmp(command, "STOP") == 0)
    return VALUE_STOP;
  if (strcasecmp(command, "CONFIRM") == 0)
    return VALUE_CONFIRM;
  if (strcasecmp(command, "LIMIT") == 0)
    return VALUE_LIMIT;
  return false;
}

void replacechar(char *str, char orig, char rep)
{
  char *ix = str;
  int n = 0;
  while ((ix = strchr(ix, orig)) != NULL)
  {
    *ix++ = rep;
    n++;
  }
}

#ifdef ESP8266
uint8_t String2GPIO(String sGPIO)
{
  byte num_part;
  char cGPIO[4];

  sGPIO.toCharArray(cGPIO, 4);

  if (strlen(cGPIO) != 2)
    return NOT_A_PIN;
  if (cGPIO[0] != 'D')
    return NOT_A_PIN;
  if (isdigit(cGPIO[1]))
    num_part = (cGPIO[1] - '0');
  else
    return NOT_A_PIN;

  switch (num_part)
  {
  case 0:
    return D0;
    break;
  case 1:
    return D1;
    break;
  case 2:
    return D2;
    break;
  case 3:
    return D3;
    break;
  case 4:
    return D4;
    break;
  case 5:
    return D5;
    break;
  case 6:
    return D6;
    break;
  case 7:
    return D7;
    break;
  case 8:
    return D8;
    break;
  default:
    return NOT_A_PIN;
  }
}

String GPIO2String(uint8_t uGPIO)
{
  switch (uGPIO)
  {
  case D0:
    return "D0";
    break;
  case D1:
    return "D1";
    break;
  case D2:
    return "D2";
    break;
  case D3:
    return "D3";
    break;
  case D4:
    return "D4";
    break;
  case D5:
    return "D5";
    break;
  case D6:
    return "D6";
    break;
  case D7:
    return "D7";
    break;
  case D8:
    return "D8";
    break;
  default:
    return "NOT_A_PIN";
  }
}
#endif // ESP8266

#ifdef ESP32
uint8_t String2GPIO(String sGPIO)
{
  char cGPIO[4];

  sGPIO.trim();
  sGPIO.toCharArray(cGPIO, 4);

  switch (strlen(cGPIO))
  {
  case 1:
    if (isdigit(cGPIO[0]))
      return (cGPIO[0] - '0');
  case 2:
    if ((isdigit(cGPIO[0])) && (isdigit(cGPIO[1])))
      return ((cGPIO[0] - '0') * 10 + (cGPIO[1] - '0'));
  default:
    return NOT_A_PIN;
  }
}

String GPIO2String(uint8_t uGPIO)
{
  if (uGPIO < 40)
    return String(uGPIO);
  else
    return "NOT_A_PIN";
}
#endif // ESP32
