// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#include <Arduino.h>
#include "RFLink.h"
#include "6_WiFi_MQTT.h"

#ifdef ESP32
#include <WiFi.h>
#elif ESP8266
#include <ESP8266WiFi.h>
#endif

char MQTTbuffer[PRINT_BUFFER_SIZE]; // Buffer for MQTT message

#if defined(MQTT_ACTIVATED) && (defined(ESP32) || defined(ESP8266))

// MQTT_KEEPALIVE : keepAlive interval in Seconds
#define MQTT_KEEPALIVE 60

// MQTT_SOCKET_TIMEOUT: socket timeout interval in Seconds
#define MQTT_SOCKET_TIMEOUT 60

#include <PubSubClient.h>
#include "6_Credentials.h"

// Update these with values suitable for your network.

WiFiClient WIFIClient;
PubSubClient MQTTClient; // MQTTClient(WIFIClient);

void setup_WIFI()
{
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.setSleepMode(WIFI_MODEM_SLEEP);
  WiFi.mode(WIFI_STA);

  // Comment out for Dynamic IP
  WiFi.config(ip, gateway, subnet);

  // We start by connecting to a WiFi network
  Serial.print(F("\nConnecting to "));
  Serial.print(ssid);
  WiFi.begin(ssid, password);

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
  MQTTClient.setServer(MQTT_SERVER, MQTT_PORT);
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
  delay(1);
  while (!MQTTClient.connected())
  {
    Serial.print(F("Attempting MQTT connection..."));
    // Attempt to connect
    if (MQTTClient.connect(MQTT_ID, MQTT_USER, MQTT_PSWD))
    {
      Serial.println(F("Connected"));
      // Once connected, resubscribe
      // MQTTClient.subscribe(MQTT_TOPIC_IN);
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
  if (MQTTbuffer[0] != 0)
  {
    if (!MQTTClient.connected())
    {
      reconnect();
    }
    MQTTClient.publish(MQTT_TOPIC_OUT, MQTTbuffer);
    MQTTbuffer[0] = 0;
  }
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

#else

void setup_WIFI_OFF()
{
  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);
  WiFi.setSleepMode(WIFI_MODEM_SLEEP);
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
}

#endif
