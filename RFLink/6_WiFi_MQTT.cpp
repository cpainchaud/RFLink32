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
#ifdef AUTOCONNECT_ENABLED
#include "9_AutoConnect.h"
#else
#include "6_Credentials.h"
#endif

#ifndef AUTOCONNECT_ENABLED
#ifdef ESP32
#include <WiFi.h>
#elif ESP8266
#include <ESP8266WiFi.h>
#endif
#endif // !AUTOCONNECT_ENABLED

#ifdef MQTT_ENABLED

// MQTT_KEEPALIVE : keepAlive interval in Seconds
#define MQTT_KEEPALIVE 60

// MQTT_SOCKET_TIMEOUT: socket timeout interval in Seconds
#define MQTT_SOCKET_TIMEOUT 60

#include <PubSubClient.h>

// Update these with values suitable for your network.

WiFiClient WIFIClient;
PubSubClient MQTTClient; // MQTTClient(WIFIClient);

void callback(char *, byte *, unsigned int);

#ifndef AUTOCONNECT_ENABLED
static String WIFI_PWR = String(WIFI_PWR_0);

void setup_WIFI()
{
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
#ifdef ESP8266
  WiFi.setSleepMode(WIFI_MODEM_SLEEP);
  WiFi.setOutputPower(WIFI_PWR.toInt());
#endif // ESP8266
  WiFi.mode(WIFI_STA);

  // Comment out for Dynamic IP
  WiFi.config(ipaddr_addr(WIFI_IP.c_str()), ipaddr_addr(WIFI_GATEWAY.c_str()), ipaddr_addr(WIFI_SUBNET.c_str()));

  // We start by connecting to a WiFi network
  Serial.print(F("WiFi SSID :\t\t"));
  Serial.println(WIFI_SSID.c_str());
  Serial.print(F("WiFi Connection :\t"));
  WiFi.begin(WIFI_SSID.c_str(), WIFI_PSWD.c_str());

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println(F("Established"));
  Serial.print(F("WiFi IP :\t\t"));
  Serial.println(WiFi.localIP());
  Serial.print(F("WiFi RSSI :\t\t"));
  Serial.println(WiFi.RSSI());
}
#endif

void setup_MQTT()
{
  if (MQTT_PORT == "")
    MQTT_PORT = "1883"; // just in case ....
  MQTTClient.setClient(WIFIClient);
  MQTTClient.setServer(MQTT_SERVER.c_str(), MQTT_PORT.toInt());
  MQTTClient.setCallback(callback);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  payload[length] = 0;
  CheckMQTT(payload);
}

#ifdef AUTOCONNECT_ENABLED
void reconnect()
{ // MQTT connection (documented way from AutoConnect : https://github.com/Hieromon/AutoConnect/tree/master/examples/mqttRSSI_NA)

  uint8_t retry = 3;
  while (!MQTTClient.connected())
  {
    Serial.print(F("MQTT Server :\t\t"));
    Serial.println(MQTT_SERVER.c_str());
    Serial.print(F("MQTT Connection :\t"));

    if (MQTTClient.connect(MQTT_ID.c_str(), MQTT_USER.c_str(), MQTT_PSWD.c_str()))
    {
      // Once connected, resubscribe
      MQTTClient.subscribe(MQTT_TOPIC_IN.c_str());
      Serial.println(F("Established"));
      Serial.print(F("MQTT ID :\t\t"));
      Serial.println(MQTT_ID.c_str());
      Serial.print(F("MQTT Username :\t\t"));
      Serial.println(MQTT_USER.c_str());
    }
    else
    {
      Serial.print(F("Failed - rc="));
      Serial.println(MQTTClient.state());
      if (!--retry)
        break;
      delay(500);
    }
  }
}

#else
void reconnect()
{
  // Loop until we're reconnected
  // delay(1);
  while (!MQTTClient.connected())
  {
    Serial.print(F("MQTT Server :\t\t"));
    Serial.println(MQTT_SERVER.c_str());
    Serial.print(F("MQTT Connection :\t"));

    // Attempt to connect
    if (MQTTClient.connect(MQTT_ID.c_str(), MQTT_USER.c_str(), MQTT_PSWD.c_str()))
    {
      // Once connected, resubscribe
      MQTTClient.subscribe(MQTT_TOPIC_IN.c_str());
      Serial.println(F("Established"));
      Serial.print(F("MQTT ID :\t\t"));
      Serial.println(MQTT_ID.c_str());
      Serial.print(F("MQTT Username :\t\t"));
      Serial.println(MQTT_USER.c_str());
    }
    else
    {
      Serial.print(F("Failed - rc="));
      Serial.print(MQTTClient.state());
      Serial.println(F("MQTT Retry :\tTry again in 5 seconds"));
      // Wait 5 seconds before retrying
      for (byte i = 0; i < 10; i++)
        delay(500); // delay(5000) may cause hang
    }
  }
}
#endif // AUTOCONNECT_ENABLED

void publishMsg()
{
#ifndef AUTOCONNECT_ENABLED
  static boolean MQTT_RETAINED = MQTT_RETAINED_0;
#endif // !AUTOCONNECT_ENABLED

  if (!MQTTClient.connected())
  {
    reconnect();
  }
  MQTTClient.publish(MQTT_TOPIC_OUT.c_str(), pbuffer, MQTT_RETAINED);
}

void checkMQTTloop()
{
  static unsigned long lastCheck = millis();

  if (millis() > lastCheck + MQTT_LOOP_MS)
  {
    if (!MQTTClient.connected())
    {
      reconnect();
    }
    // Serial.print(F("Calling MQTT loop()..."));
    MQTTClient.loop();
    // Serial.println(F("Done"));
    lastCheck = millis();
  }
}
#endif // MQTT_ENABLED

#if (!defined(AUTOCONNECT_ENABLED) && !defined(MQTT_ENABLED))
#if (defined(ESP32) || defined(ESP8266))
void setup_WIFI_OFF()
{
  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);
#ifdef ESP8266
  WiFi.setSleepMode(WIFI_MODEM_SLEEP);
#endif
  WiFi.mode(WIFI_OFF);
#ifdef ESP8266
  WiFi.forceSleepBegin();
#endif
}
#endif
#endif
