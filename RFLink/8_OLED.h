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

#define PIN_OLED_GND NOT_A_PIN // Ground power on this pin
#define PIN_OLED_VCC NOT_A_PIN // +3 volt / Vcc power on this pin#
#ifdef ESP8266
#define PIN_OLED_SCL D1        // I2C SCL
#define PIN_OLED_SDA D2        // I2C SDA
#elif ESP32
#define PIN_OLED_SCL 22        // I2C SCL
#define PIN_OLED_SDA 21        // I2C SDA
#endif

void setup_OLED();
void splash_OLED();
void print_OLED();

#endif // OLED_ENABLED
#endif // OLED_h