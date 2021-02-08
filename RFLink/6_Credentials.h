// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef CREDENTIALS_h
#define CREDENTIALS_h

#include "RFLink.h"

// local AP
#define WIFI_SSID "xxx";
#define WIFI_PSWD "xxx";

// DHCP or Static IP
// #define USE_DHCP
#ifndef USE_DHCP
#define WIFI_IP "192.168.1.xxx";
#define WIFI_DNS "192.168.1.xxx";
#define WIFI_GATEWAY "192.168.1.xxx";
#define WIFI_SUBNET "255.255.255.0";
#endif

// MQTT Server
#define  MQTT_SERVER "192.168.1.xxx"
#define  MQTT_PORT "1883";
#define  MQTT_ID "ESP8266-RFLink_xxx";
#define  MQTT_USER "xxx";
#define  MQTT_PSWD "xxx";

// MQTT Topic
#define MQTT_TOPIC_OUT "/ESP00/msg";
#define MQTT_TOPIC_IN "/ESP00/cmd";
#define MQTT_TOPIC_LWT "/ESP00/lwt";

#ifdef CHECK_CACERT
static const char ca_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIID (...)
-----END CERTIFICATE-----
)EOF";
#endif //CHECK_CACERT

#endif
