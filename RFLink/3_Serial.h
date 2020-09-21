// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef Serial_h
#define Serial_h

#include <Arduino.h>

#define BAUD 57600            // 57600      // Baudrate for serial communication.
#define INPUT_COMMAND_SIZE 60 // 60         // Maximum number of characters that a command via serial can be.
#define FOCUS_TIME_MS 50      // 50         // Duration in mSec. that, after receiving serial data from USB only the serial port is checked.

extern char InputBuffer_Serial[INPUT_COMMAND_SIZE];

boolean CheckSerial();
boolean CheckMQTT(byte *);
#ifdef AUTOCONNECT_ENABLED
boolean CheckWeb(String &);
#endif // AUTOCONNECT_ENABLED
#endif