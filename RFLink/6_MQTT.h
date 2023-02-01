// ************************************* //
// * Arduino Project RFLink32        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef WiFi_MQTT_h
#define WiFi_MQTT_h

#ifndef RFLINK_MQTT_DISABLED

#include <Arduino.h>
#include "RFLink.h"

#ifdef ESP32
#include <WiFi.h>
#include "10_Wifi.h"
#elif ESP8266
#include <ESP8266WiFi.h>
#include "10_Wifi.h"
#endif

#include "4_Display.h"
#include "11_Config.h"

#include <time.h>
#include <sys/time.h>


// #define MQTT_CLIENT_SSL_DISABLED // mainly used to save some memory on ESP8266 if wanted

extern char MQTTbuffer[PRINT_BUFFER_SIZE]; // Buffer for MQTT message

namespace RFLink { namespace Mqtt {

    extern Config::ConfigItem configItems[];
    extern struct timeval lastMqttConnectionAttemptTime;

    namespace params {
        extern bool enabled;
        extern String server;
        extern int port;
        extern String id;
        extern String user;
        extern String password;

        extern String topic_out;
        extern String topic_in;

        extern bool lwt_enabled;
        extern String topic_lwt;

        #ifndef RFLINK_MQTT_CLIENT_SSL_DISABLED
        extern bool ssl_enabled;
        extern bool ssl_insecure;
        extern String ca_cert;
        #endif
    }

void setup_MQTT();
void reconnect(int retryCount=-1, bool force=false);
void publishMsg();
void checkMQTTloop();

void paramsUpdatedCallback();
void refreshParametersFromConfig(bool triggerChanges=true);

void getStatusJsonString(JsonObject &output);

}} // end of MQTT namespace

#endif // RFLINK_MQTT_DISABLED

#endif // WiFi_MQTT_h