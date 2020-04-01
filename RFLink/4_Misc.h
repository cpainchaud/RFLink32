// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef Misc_h
#define Misc_h

#include <Arduino.h>

unsigned long str2int(char *string);
float ul2float(unsigned long ul);
void PrintHex8(uint8_t *data, uint8_t length);
void PrintHexByte(uint8_t data);
byte reverseBits(byte data);

void display_Header(void);
void display_Name(const char *);
void display_Footer(void);
void display_Start(void);
void display_ID(unsigned int);
void display_ID(byte, byte);
void display_SWITCH(const char *);
void display_CMD(boolean, boolean);
void display_SET_LEVEL(byte);
void display_TEMP(unsigned int);
void display_HUM(byte, boolean);
void display_HUM(byte);
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
void display_WINCHILL(unsigned int);
void display_WINTMP(unsigned int);
void display_CHIME(unsigned int);
void display_SMOKEALERT(boolean);
void display_PIR(boolean);
void display_CO2(unsigned int);
void display_SOUND(unsigned int);
void display_KWATT(unsigned int);
void display_WATT(unsigned int);
void display_CURRENT(unsigned int);
void display_CURRENT2(unsigned int);
void display_CURRENT3(unsigned int);
void display_DIST(unsigned int);
void display_METER(unsigned int);
void display_VOLT(unsigned int);
void display_RGBW(unsigned int);

#endif