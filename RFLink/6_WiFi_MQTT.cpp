// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#include <Arduino.h>
#include "RFLink.h"

#include "4_Display.h"
#include "6_WiFi_MQTT.h"
#include "9_AutoConnect.h"

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

void setup_WIFI()
{
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
#ifdef ESP8266
  WiFi.setSleepMode(WIFI_MODEM_SLEEP);
  WiFi.setOutputPower(ac_WIFI_PWR);
#endif // ESP8266
  WiFi.mode(WIFI_STA);

  // Comment out for Dynamic IP
  WiFi.config(ipaddr_addr(WIFI_IP.c_str()), ipaddr_addr(WIFI_GATEWAY.c_str()), ipaddr_addr(WIFI_SUBNET.c_str()));

  // We start by connecting to a WiFi network
  Serial.print(F("\nConnecting to "));
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PSWD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.print(F("\nWiFi connected\t"));
  Serial.print(F("IP address: "));
  Serial.print(WiFi.localIP());
  Serial.print(F("\tRSSI "));
  Serial.println(WiFi.RSSI());
}

void setup_MQTT()
{
  MQTTClient.setClient(WIFIClient);
  MQTTClient.setServer(MQTT_SERVER.c_str(), MQTT_PORT.toInt());
  // MQTTClient.setCallback(callback);
}

/*
  void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print(F("Message arrived ["));
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.write(payload[i]);
  }
  Serial.write('\n');
  Serial.println();
  }
*/

void reconnect()
{
  // Loop until we're reconnected
  // delay(1);
  while (!MQTTClient.connected())
  {
    Serial.print(F("Attempting MQTT connection..."));
    // Attempt to connect
    if (MQTTClient.connect(MQTT_ID.c_str(), MQTT_USER.c_str(), MQTT_PSWD.c_str()))
    {
      Serial.println(F("Connected"));
      // Once connected, resubscribe
      // MQTTClient.subscribe(MQTT_TOPIC_IN.c_str());
    }
    else
    {
      Serial.print(F("\nFailed, rc="));
      Serial.print(MQTTClient.state());
      Serial.println(F("\tTry again in 5 seconds"));
      // Wait 5 seconds before retrying
      for (byte i = 0; i < 10; i++)
        delay(500); // delay(5000) may cause hang
    }
  }
}

void publishMsg()
{
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
