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


#ifdef ESP32
#include <WiFi.h>
#elif ESP8266
#include <ESP8266WiFi.h>
#endif

#if defined(USE_OTA)
  #include "ArduinoOTA.h"
#endif // USE_OTA


#ifdef USE_WIFIMANAGER

namespace RFLink { namespace Wifi {
WiFiManager wifiManager;

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  Serial.println(myWiFiManager->getConfigPortalSSID());
}

const String mqtt_s_paramid("mqtt_s");
const String mqtt_p_paramid("mqtt_p");
const String mqtt_id_paramid("mqtt_id");
const String mqtt_u_paramid("mqtt_u");
const String mqtt_sec_paramid("mqtt_sec");

#if (defined(ESP32) || defined(ESP8266)) && defined(MQTT_ENABLED) // to store configuration in Flash
#include "ArduinoNvs.h"

WiFiManagerParameter mqtt_s_param(mqtt_s_paramid.c_str(), "hostname or ip",  MQTT_SERVER.c_str(), 40);
WiFiManagerParameter mqtt_p_param(mqtt_p_paramid.c_str(), "port",  MQTT_PORT.c_str(), 6);
WiFiManagerParameter mqtt_id_param(mqtt_id_paramid.c_str(), "id", MQTT_ID.c_str(), 20);
WiFiManagerParameter mqtt_u_param(mqtt_u_paramid.c_str(), "user", MQTT_USER.c_str(), 20);
WiFiManagerParameter mqtt_sec_param(mqtt_sec_paramid.c_str(), "mqtt password", MQTT_PSWD.c_str(), 20);

void copyParamsFromWM_to_MQTT(){
  auto params = wifiManager.getParameters();
  for(int i=0; i<wifiManager.getParametersCount(); i++) {
    auto currentId =  params[i]->getID();
    
    if(mqtt_s_paramid == currentId) {
      MQTT_SERVER =  params[i]->getValue();
      continue;
    }

    if(mqtt_p_paramid == currentId) {
      MQTT_PORT =  params[i]->getValue();
      continue;
    }

    if(mqtt_id_paramid == currentId) {
      MQTT_ID =  params[i]->getValue();
      continue;
    }

    if(mqtt_u_paramid == currentId) {
      MQTT_USER =  params[i]->getValue();
      continue;
    }

    if(mqtt_sec_paramid == currentId) {
      MQTT_PSWD =  params[i]->getValue();
      continue;
    }
  }
}

void paramsUpdatedCallback(){
  bool someParamsChanged = false;
  NVS.begin();
  auto params = wifiManager.getParameters();
  for(int i=0; i<wifiManager.getParametersCount(); i++) {
    WiFiManagerParameter* currentParameter = params[i];
    if(NVS.getString(currentParameter->getID())!=currentParameter->getValue())
      someParamsChanged = true;
    NVS.setString(currentParameter->getID(), currentParameter->getValue());
  }
  NVS.commit();

  if(someParamsChanged) {
    copyParamsFromWM_to_MQTT();
    Serial.println("Some parameters have changed, restart MQTT Client is requested");
    reconnect(1, true);
  }
}
#endif // MQTT_ENABLED

void setup_WifiManager(){

  const char* menu[] = {"wifi","param","info","close","sep","erase","restart","exit"}; 

  #if (defined(ESP32) || defined(ESP8266)) && defined(MQTT_ENABLED) // Get MQTT from Flash storage if they exist 
  wifiManager.addParameter(&mqtt_s_param);
  wifiManager.addParameter(&mqtt_p_param);
  wifiManager.addParameter(&mqtt_id_param);
  wifiManager.addParameter(&mqtt_u_param);
  wifiManager.addParameter(&mqtt_sec_param);

  NVS.begin();
  auto params = wifiManager.getParameters();
  for(int i=0; i<wifiManager.getParametersCount(); i++) {
    auto currentParameter = params[i];
    auto currentId = currentParameter->getID();
    auto value = NVS.getString(currentId);
    if(value.length() > 0)
      currentParameter->setValue(value.c_str(), currentParameter->getValueLength());
  }

  wifiManager.setSaveParamsCallback(paramsUpdatedCallback); // if Config portal is used to change paramaters, we must know about it
  #endif

  wifiManager.setMenu(menu, sizeof(menu));

  wifiManager.setAPCallback(configModeCallback);
}

void start_WifiManager(){
  wifiManager.autoConnect();

  #if (defined(ESP32) || defined(ESP8266)) && defined(MQTT_ENABLED)
  MQTT_SERVER = mqtt_s_param.getValue();
  MQTT_PORT = mqtt_p_param.getValue();
  MQTT_USER = mqtt_u_param.getValue();
  MQTT_PSWD = mqtt_sec_param.getValue();
  MQTT_ID = mqtt_id_param.getValue();
  #endif

  #if defined(SHOW_CONFIG_PORTAL_PIN_BUTTON) && SHOW_CONFIG_PORTAL_PIN_BUTTON != NOT_A_PIN
    pinMode(SHOW_CONFIG_PORTAL_PIN_BUTTON, INPUT);
  #endif
}

void setup() {
  setup_WifiManager();
  start_WifiManager();
  #ifdef USE_OTA
    #ifdef OTA_PASSWORD
    ArduinoOTA.setPassword(OTA_PASSWORD);
    #endif
    ArduinoOTA.begin();
  #endif // USE_OTA
}

void mainLoop() {
  wifiManager.process();
  #if defined(SHOW_CONFIG_PORTAL_PIN_BUTTON) && SHOW_CONFIG_PORTAL_PIN_BUTTON != NOT_A_PIN
    if (!RFLink::Wifi::wifiManager.getConfigPortalActive()) {
      if(digitalRead(SHOW_CONFIG_PORTAL_PIN_BUTTON) == HIGH) {
        Serial.println("Config portal requested");
        RFLink::Wifi::wifiManager.setConfigPortalBlocking(false);
        RFLink::Wifi::wifiManager.startWebPortal();
        Serial.println("Config portal started");
        sleep(4);
      }
    } else if(digitalRead(SHOW_CONFIG_PORTAL_PIN_BUTTON) == HIGH) {
      Serial.println("shutting down portal");
      RFLink::Wifi::wifiManager.stopConfigPortal();
      Serial.println("done");
      sleep(4);
    }
  #endif // SHOW_CONFIG_PORTAL_PIN_BUTTON && SHOW_CONFIG_PORTAL_PIN_BUTTON != NOT_A_PIN
}

}} // end of Wifi namespace

#endif // USE_WIFIMANAGER


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
      #ifndef USE_WIFIMANAGER
      stop_WIFI();
      start_WIFI();
      #endif // USE_WIFIMANAGER
    }

    Serial.print(F("Trying to connect to MQTT Server '"));
    Serial.print(MQTT_SERVER.c_str());
    Serial.print(F("' ... "));

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
    }
  }
}

void publishMsg()
{
  static boolean MQTT_RETAINED = MQTT_RETAINED_0;

  if (!MQTTClient.connected())
    reconnect(1);
  MQTTClient.publish(MQTT_TOPIC_OUT.c_str(), pbuffer, MQTT_RETAINED);
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
