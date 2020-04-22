// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef OLED_h
#define OLED_h

#include <Arduino.h>
#include "RFLink.h"

#ifdef OLED_ENABLED

void setup_OLED();
void splash_OLED();
void print_OLED();

#endif // OLED_ENABLED
#endif // OLED_h