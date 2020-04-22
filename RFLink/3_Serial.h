// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef Serial_h
#define Serial_h

#include <Arduino.h>
#include "RFLink.h"

boolean CheckSerial();
boolean CheckMQTT(byte *);

#endif