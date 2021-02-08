// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef WiFi_MQTT_h
#define WiFi_MQTT_h

#include <Arduino.h>
#include "RFLink.h"

#ifdef MQTT_ENABLED

#ifdef ESP32
#include <WiFi.h>
#include "10_Wifi.h"
#elif ESP8266
#include <ESP8266WiFi.h>
#include "10_Wifi.h"
#endif
#include "4_Display.h"
extern char MQTTbuffer[PRINT_BUFFER_SIZE]; // Buffer for MQTT message

namespace RFLink { namespace Mqtt {

    namespace params {
        extern String SERVER;
        extern String PORT;
        extern String ID;
        extern String USER;
        extern String PSWD;
    }

void setup_MQTT();
void reconnect(int retryCount=-1, bool force=false);
void publishMsg();
void checkMQTTloop();

}} // end of MQTT namespace

#endif // MQTT_ENABLED
#endif // WiFi_MQTT_h