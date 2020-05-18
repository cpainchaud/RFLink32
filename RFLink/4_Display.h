// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef Misc_h
#define Misc_h

#include <Arduino.h>

#define PRINT_BUFFER_SIZE 90 // 90         // Maximum number of characters that a command should print in one go via the print buffer.

// extern byte PKSequenceNumber;     // 1 byte packet counter
extern char pbuffer[PRINT_BUFFER_SIZE]; // Buffer for printing data

void display_Header(void);
void display_Name(const char *);
void display_Footer(void);
void display_Splash(void);
void display_IDn(unsigned long, byte);
void display_IDc(const char *);
void display_SWITCH(byte);
void display_SWITCHc(const char *);
enum CMD_Group
{
    CMD_Single,
    CMD_All
};
enum CMD_OnOff
{
    CMD_Off,
    CMD_On,
    CMD_Bright,
    CMD_Dim,
    CMD_Unknown
};
void display_CMD(boolean, byte);
void display_SET_LEVEL(byte);
void display_TEMP(unsigned int);
enum HUM_Type
{
    HUM_HEX,
    HUM_BCD
};
void display_HUM(byte, boolean);
void display_BARO(unsigned int);
void display_HSTATUS(byte);
void display_BFORECAST(byte);
void display_UV(unsigned int);
void display_LUX(unsigned int);
void display_BAT(boolean);
void display_RAIN(unsigned int);
void display_RAINRATE(unsigned int);
void display_WINSP(unsigned int);
void display_AWINSP(unsigned int);
void display_WINGS(unsigned int);
void display_WINDIR(unsigned int);
void display_WINCHL(unsigned int);
void display_WINTMP(unsigned int);
void display_CHIME(unsigned int);
enum SMOKE_OnOff
{
    SMOKE_Off,
    SMOKE_On
};
void display_SMOKEALERT(boolean);
enum PIR_OnOff
{
    PIR_Off,
    PIR_On
};
void display_PIR(boolean);
void display_CO2(unsigned int);
void display_SOUND(unsigned int);
void display_KWATT(unsigned int);
void display_WATT(unsigned int);
void display_CURRENT(unsigned int);
void display_DIST(unsigned int);
void display_METER(unsigned int);
void display_VOLT(unsigned int);
void display_RGBW(unsigned int);

void retrieve_Init();
boolean retrieve_Name(const char *);
boolean retrieve_ID(unsigned long &);
boolean retrieve_Switch(byte &);
boolean retrieve_Command(byte &, byte &);
boolean retrieve_End();

#define VALUE_PAIR 44
#define VALUE_ALLOFF 55
#define VALUE_OFF 74
#define VALUE_ON 75
#define VALUE_DIM 76
#define VALUE_BRIGHT 77
#define VALUE_UP 78
#define VALUE_DOWN 79
#define VALUE_STOP 80
#define VALUE_CONFIRM 81
#define VALUE_LIMIT 82
#define VALUE_ALLON 141

int str2cmd(char *);
void replacechar(char *, char, char);

#if (defined(ESP8266) || defined(ESP32))
uint8_t String2GPIO(String);
String GPIO2String(uint8_t uGPIO);
#endif // ESP8266 || ESP32

#endif