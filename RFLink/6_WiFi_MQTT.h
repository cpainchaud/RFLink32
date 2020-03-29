#ifndef WiFi_MQTT_h
#define WiFi_MQTT_h

#include <Arduino.h>

extern char MQTTbuffer[PRINT_BUFFER_SIZE];                                             // Buffer for MQTT message

#if defined(MQTT_ACTIVATED) && (defined(ESP32) || defined(ESP8266))
void setup_WIFI();
void setup_MQTT();
void reconnect();
void publishMsg();
void checkMQTTloop();
#else
void setup_WIFI_OFF();
#endif

#endif