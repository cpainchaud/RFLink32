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
#if (defined(ESP32) || defined(ESP8266))

extern char MQTTbuffer[PRINT_BUFFER_SIZE]; // Buffer for MQTT message

#ifdef MQTT_ENABLED
void setup_WIFI();
void setup_MQTT();
void reconnect();
void publishMsg();
void checkMQTTloop();
#else
void setup_WIFI_OFF();
#endif // MQTT_ENABLED

#endif // (defined(ESP32) || defined(ESP8266))

#endif // WiFi_MQTT_h