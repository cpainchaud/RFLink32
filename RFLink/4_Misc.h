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

#endif