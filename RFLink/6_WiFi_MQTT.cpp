// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#include <Arduino.h>
#include "RFLink.h"
#include "3_Serial.h"
#include "4_Display.h"
#include "6_WiFi_MQTT.h"
#include "6_Credentials.h"

#ifdef MQTT_ENABLED

#ifdef ESP32
#include <WiFi.h>
#elif ESP8266
#include <ESP8266WiFi.h>
#endif

#ifdef MQTT_SSL
#include <WiFiClientSecure.h>
WiFiClientSecure WIFIClient;
#else //SSL
#include <WiFiClient.h>
WiFiClient WIFIClient;
#endif //SSL


// MQTT_KEEPALIVE : keepAlive interval in Seconds
#define MQTT_KEEPALIVE 60

// MQTT_SOCKET_TIMEOUT: socket timeout interval in Seconds
#define MQTT_SOCKET_TIMEOUT 60

#include <PubSubClient.h>
boolean bResub; // uplink reSubscribe after setup only

// Update these with values suitable for your network.

namespace RFLink { namespace Mqtt {

  namespace params {
    String TOPIC_OUT = MQTT_TOPIC_OUT;
    String TOPIC_IN = MQTT_TOPIC_IN;
    String TOPIC_LWT = MQTT_TOPIC_LWT;

    String SERVER = MQTT_SERVER;
    String PORT = MQTT_PORT;
    String ID = MQTT_ID;
    String USER = MQTT_USER;
    String PSWD = MQTT_PSWD;
  }


PubSubClient MQTTClient; // MQTTClient(WIFIClient);

void callback(char *, byte *, unsigned int);

static String WIFI_PWR = String(WIFI_PWR_0);


void setup_MQTT()
{
  Serial.print(F("SSL :\t\t\t"));
#ifdef MQTT_SSL
  if (MQTT_PORT == "")
    MQTT_PORT = "8883"; // just in case ....
#ifdef CHECK_CACERT
  Serial.println(F("Using ca_cert"));
  WIFIClient.setCACert(ca_cert);
#else
  Serial.println(F("Insecure (No Key/Cert/Fp)"));
#endif // MQTT_CACERT
#else
  if (params::PORT == "")
    params::PORT = "1883"; // just in case ....
  Serial.println(F("Not Set"));
#endif //SSL

  MQTTClient.setClient(WIFIClient);
  MQTTClient.setServer(params::SERVER.c_str(), params::PORT.toInt());
  MQTTClient.setCallback(callback);
  bResub = true;
}

void callback(char *topic, byte *payload, unsigned int length)
{
  payload[length] = 0;
  CheckMQTT(payload);
}


void reconnect(int retryCount, bool force)
{
  int retryLeft = retryCount;
  bResub = true;

  if(force && MQTTClient.connected()) {
    MQTTClient.disconnect();
  }

  while (!MQTTClient.connected() && (retryCount<=0 || retryLeft>0))
  {
    retryLeft--;
    if (WiFi.status() != WL_CONNECTED)
    {
      #ifndef RFLINK_WIFIMANAGER_ENABLED
      RFLink::Wifi::stop_WIFI();
      RFLink::Wifi::start_WIFI();
      #endif // RFLINK_WIFIMANAGER_ENABLED
    }

    Serial.print(F("Trying to connect to MQTT Server '"));
    Serial.print(params::SERVER.c_str());
    Serial.print(F("' ... "));

#ifdef MQTT_LWT
#ifdef ESP32
    if (MQTTClient.connect(params::ID.c_str(), params::USER.c_str(), params::PSWD.c_str(), (params::TOPIC_LWT).c_str(), 2, true, PSTR("Offline")))
#elif ESP8266
    if (MQTTClient.connect(params::ID.c_str(), params::USER.c_str(), params::PSWD.c_str(), (params::TOPIC_LWT).c_str(), 2, true, "Offline"))
#endif // ESP
#else  // MQTT_LWT
    if (MQTTClient.connect(params::ID.c_str(), params::USER.c_str(), params::PSWD.c_str()))
#endif // MQTT_LWT
    {
      Serial.println(F("Established"));
      Serial.print(F("MQTT ID :\t\t"));
      Serial.println(params::ID.c_str());
      Serial.print(F("MQTT Username :\t\t"));
      Serial.println(params::USER.c_str());
#ifdef MQTT_LWT
#ifdef ESP32
      MQTTClient.publish((params::TOPIC_LWT).c_str(), PSTR("Online"), true);
#elif ESP8266
      MQTTClient.publish((params::TOPIC_LWT).c_str(), "Online", true);
#endif // ESP
#endif // MQTT_LWT
    }
    else
    {
      Serial.print(F("Failed - rc="));
      Serial.println(MQTTClient.state());
    }
  }
}

void publishMsg()
{
  static boolean MQTT_RETAINED = MQTT_RETAINED_0;

  if (!MQTTClient.connected())
    reconnect(1);
  MQTTClient.publish(params::TOPIC_OUT.c_str(), pbuffer, MQTT_RETAINED);
}

void checkMQTTloop()
{
  static unsigned long lastCheck = millis();

  if (millis() > lastCheck + MQTT_LOOP_MS)
  {
    if (!MQTTClient.connected()) {
      Serial.println("MQTT Client is disconnected");
      reconnect(1);
    }

    if (bResub)
    {
      // Once connected, resubscribe
      MQTTClient.subscribe(params::TOPIC_IN.c_str());
      bResub = false;
      delay(10);
    }
    MQTTClient.loop();
    lastCheck = millis();
  }

}

}} // end of Mqtt namespace

#endif // MQTT_ENABLED


