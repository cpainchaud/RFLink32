#ifndef CREDENTIALS_h
#define CREDENTIALS_h

// local AP
const char* ssid     = "your_AP_ssid";
const char* password = "your_AP_passwd";

// static IP
const IPAddress ip(192, 168, 0, 199);
// const IPAddress dns(8, 8, 8, 8);
const IPAddress gateway(192, 168, 0, 1);
const IPAddress subnet(255, 255, 255, 0);

// MQTT Server
const char* mqtt_server    = "raspberrypi.local";
const char* MQTT_ID        = "ESP8266-RFLink";
const char* MQTT_USER      = "your_mqtt_id";
const char* MQTT_PSWD      = "your_mqtt_pswd";

// MQTT Topic
const char* MQTT_TOPIC_OUT = "/RFLink/msg";
const char* MQTT_TOPIC_IN  = "/RFLink/cmd";

#endif
