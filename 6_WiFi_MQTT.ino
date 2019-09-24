#if defined(MQTT_ACTIVATED) && (defined(ESP32) || defined(ESP8266))

#ifdef ESP32
#include <WiFi.h>
#elif ESP8266
#include <ESP8266WiFi.h>
#endif

#include <PubSubClient.h>
#include "6_Credentials.h"

// Update these with values suitable for your network.

WiFiClient espClient;
PubSubClient client(espClient);


void setup_wifi() {

  delay(10);

  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_STA);

  // Comment out for Dynamic IP
  WiFi.config(ip, gateway, subnet);

  // We start by connecting to a WiFi network
  Serial.print(F("\nConnecting to "));
  Serial.print(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print(F("\nWiFi connected\t"));
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());
}

void setup_MQTT() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

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

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    // Attempt to connect
    if (client.connect(MQTT_ID, MQTT_USER, MQTT_PSWD)) {
      Serial.println(F("Connected"));
      // Once connected, resubscribe
      client.subscribe(MQTT_TOPIC_IN);
    } else {
      Serial.print(F("Failed, rc="));
      Serial.print(client.state());
      Serial.println(F("Try again in 5 seconds"));
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void publishMsg() {
  if (MQTTbuffer[0] != 0)
  {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    client.publish(MQTT_TOPIC_OUT, MQTTbuffer);
    MQTTbuffer[0] = 0;
  }
}

#endif
