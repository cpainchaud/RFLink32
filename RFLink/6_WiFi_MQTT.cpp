// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#include <Arduino.h>
// #include <ArduinoOTA.h>
#include "RFLink.h"
#include "3_Serial.h"
#include "4_Display.h"
#include "6_WiFi_MQTT.h"
#include "6_Credentials.h"

#ifdef ESP32
#include <WiFi.h>
#elif ESP8266
#include <ESP8266WiFi.h>
#endif

#ifdef MQTT_ENABLED

// MQTT_KEEPALIVE : keepAlive interval in Seconds
#define MQTT_KEEPALIVE 60

// MQTT_SOCKET_TIMEOUT: socket timeout interval in Seconds
#define MQTT_SOCKET_TIMEOUT 60

#include <PubSubClient.h>
boolean bResub; // uplink reSubscribe after setup only

// Update these with values suitable for your network.

#ifdef MQTT_SSL
#include <WiFiClientSecure.h>
WiFiClientSecure WIFIClient;
#else //SSL
#include <WiFiClient.h>
WiFiClient WIFIClient;
#endif //SSL

PubSubClient MQTTClient; // MQTTClient(WIFIClient);

void callback(char *, byte *, unsigned int);

static String WIFI_PWR = String(WIFI_PWR_0);

#ifndef USE_WIFIMANAGER
void setup_WIFI()
{
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
#ifdef ESP32
  WiFi.setTxPower(WIFI_POWER_11dBm);
#elif ESP8266
  WiFi.setSleepMode(WIFI_MODEM_SLEEP);
  WiFi.setOutputPower(WIFI_PWR.toInt());
#endif // ESP

  // For Static IP
#ifndef USE_DHCP
  WiFi.config(ipaddr_addr(WIFI_IP.c_str()), ipaddr_addr(WIFI_GATEWAY.c_str()), ipaddr_addr(WIFI_SUBNET.c_str()));
#endif // USE_DHCP
}

void start_WIFI()
{
  WiFi.mode(WIFI_STA);

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

  /*
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  */
}

void stop_WIFI()
{
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  delay(500);
}
#else //USE_WIFIMANAGER
WiFiManager wifiManager;

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void start_WifiManager(){

  WiFiManagerParameter mqtt_s_param("mqtt_s", "hostname or ip", "", 40);
  wifiManager.addParameter(&mqtt_s_param);

  WiFiManagerParameter mqtt_p_param("mqtt_p", "port", "", 6);
  wifiManager.addParameter(&mqtt_p_param);

  WiFiManagerParameter mqtt_u_param("mqtt_u", "user", "", 20);
  wifiManager.addParameter(&mqtt_u_param);
    
  WiFiManagerParameter mqtt_sec_param("mqtt_sec", "password", "", 20);
  wifiManager.addParameter(&mqtt_sec_param);

  wifiManager.setAPCallback(configModeCallback);
  wifiManager.autoConnect();

  MQTT_SERVER = mqtt_s_param.getValue();
  MQTT_PORT = mqtt_p_param.getValue();
  MQTT_USER = mqtt_u_param.getValue();
  MQTT_PSWD = mqtt_sec_param.getValue();

}
#endif //USE_WIFIMANAGER


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
  if (MQTT_PORT == "")
    MQTT_PORT = "1883"; // just in case ....
  Serial.println(F("Not Set"));
#endif //SSL

  MQTTClient.setClient(WIFIClient);
  MQTTClient.setServer(MQTT_SERVER.c_str(), MQTT_PORT.toInt());
  MQTTClient.setCallback(callback);
  bResub = true;
}

void callback(char *topic, byte *payload, unsigned int length)
{
  payload[length] = 0;
  CheckMQTT(payload);
}

void reconnect()
{
  bResub = true;

  while (!MQTTClient.connected())
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      #ifndef USE_WIFIMANAGER
      stop_WIFI();
      start_WIFI();
      #endif // USE_WIFIMANAGER
    }

    Serial.print(F("MQTT Server :\t\t"));
    Serial.println(MQTT_SERVER.c_str());
    Serial.print(F("MQTT Connection :\t"));

#ifdef MQTT_LWT
#ifdef ESP32
    if (MQTTClient.connect(MQTT_ID.c_str(), MQTT_USER.c_str(), MQTT_PSWD.c_str(), (MQTT_TOPIC_LWT).c_str(), 2, true, PSTR("Offline")))
#elif ESP8266
    if (MQTTClient.connect(MQTT_ID.c_str(), MQTT_USER.c_str(), MQTT_PSWD.c_str(), (MQTT_TOPIC_LWT).c_str(), 2, true, "Offline"))
#endif // ESP
#else  // MQTT_LWT
    if (MQTTClient.connect(MQTT_ID.c_str(), MQTT_USER.c_str(), MQTT_PSWD.c_str()))
#endif // MQTT_LWT
    {
      Serial.println(F("Established"));
      Serial.print(F("MQTT ID :\t\t"));
      Serial.println(MQTT_ID.c_str());
      Serial.print(F("MQTT Username :\t\t"));
      Serial.println(MQTT_USER.c_str());
#ifdef MQTT_LWT
#ifdef ESP32
      MQTTClient.publish((MQTT_TOPIC_LWT).c_str(), PSTR("Online"), true);
#elif ESP8266
      MQTTClient.publish((MQTT_TOPIC_LWT).c_str(), "Online", true);
#endif // ESP
#endif // MQTT_LWT
    }
    else
    {
      Serial.print(F("Failed - rc="));
      Serial.println(MQTTClient.state());
      Serial.println(F("MQTT Retry :\tTry again in 5 seconds"));
      // Wait 5 seconds before retrying
      for (byte i = 0; i < 10; i++)
        delay(500); // delay(5000) may cause hang
    }
  }
}

void publishMsg()
{
  static boolean MQTT_RETAINED = MQTT_RETAINED_0;

  if (!MQTTClient.connected())
    reconnect();
  MQTTClient.publish(MQTT_TOPIC_OUT.c_str(), pbuffer, MQTT_RETAINED);
}

void checkMQTTloop()
{
  static unsigned long lastCheck = millis();

  if (millis() > lastCheck + MQTT_LOOP_MS)
  {
    if (!MQTTClient.connected())
      reconnect();

    if (bResub)
    {
      // Once connected, resubscribe
      MQTTClient.subscribe(MQTT_TOPIC_IN.c_str());
      bResub = false;
      delay(10);
    }
    MQTTClient.loop();
    lastCheck = millis();
  }
  // ArduinoOTA.handle();
}

#else // MQTT_ENABLED

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
#endif // MQTT_ENABLED
