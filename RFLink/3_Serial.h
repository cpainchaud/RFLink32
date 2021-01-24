// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef Serial_h
#define Serial_h

#include <Arduino.h>

#if (defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__))
#define BAUD 57600  // Baudrate for Arduino Uno
#else
#define BAUD 921600            // 57600      // Baudrate for serial communication.
#endif
#define INPUT_COMMAND_SIZE 60 // 60         // Maximum number of characters that a command via serial can be.
#define FOCUS_TIME_MS 50      // 50         // Duration in mSec. that, after receiving serial data from USB only the serial port is checked.

extern char InputBuffer_Serial[INPUT_COMMAND_SIZE];

boolean CheckSerial();
boolean CheckMQTT(byte *);
#endif