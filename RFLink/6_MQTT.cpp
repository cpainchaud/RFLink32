// ************************************* //
// * Arduino Project RFLink32        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#include <Arduino.h>
#include "RFLink.h"

#ifndef RFLINK_MQTT_DISABLED
#ifdef RFLINK_WIFI_ENABLED

#include "3_Serial.h"
#include "4_Display.h"
#include "6_MQTT.h"
#include "6_Credentials.h"


#ifdef ESP32
#include <WiFi.h>
#elif ESP8266
#include <ESP8266WiFi.h>
#endif

#include <ESPNtpClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
WiFiClient WIFIClient;

#ifndef RFLINK_MQTT_CLIENT_SSL_DISABLED
WiFiClientSecure *WIFIClientSecure =  nullptr;
  #ifndef ESP32
  BearSSL::X509List *sslTrustedAnchors  = nullptr;
  #endif
#endif

#include <LittleFS.h>


// MQTT_KEEPALIVE : keepAlive interval in Seconds
#define MQTT_KEEPALIVE 60

// MQTT_SOCKET_TIMEOUT: socket timeout interval in Seconds
#define MQTT_SOCKET_TIMEOUT 60

#include <PubSubClient.h>
boolean bResub; // uplink reSubscribe after setup only

// Update these with values suitable for your network.

namespace RFLink { namespace Mqtt {

  namespace params {
    bool enabled;
    String server;
    int port;
    String id;
    String user;
    String password;

    String topic_out;
    String topic_in;

    bool lwt_enabled;
    String topic_lwt;

    bool ssl_enabled;
    bool ssl_insecure;
    String ca_cert;
  }

  namespace vars {
    String ca_cert_content;
    String lastError;
    bool disabledBecauseOfError = false;
  }

// All json variable names
const char json_name_enabled[] = "enabled";
const char json_name_server[] = "server";
const char json_name_port[] = "port";
const char json_name_id[] = "id";
const char json_name_user[] = "user";
const char json_name_password[] = "password";

const char json_name_topic_in[] = "topic_in";
const char json_name_topic_out[] = "topic_out";

const char json_name_lwt_enabled[] = "lwt_enabled";
const char json_name_topic_lwt[] = "topic_lwt";

#ifndef RFLINK_MQTT_CLIENT_SSL_DISABLED
const char json_name_ssl_enabled[] = "ssl_enabled";
const char json_name_ssl_insecure[] = "ssl_insecure";
const char json_name_ca_cert[] = "ca_cert";
const char *mqtt_ca_cert_filename = "/mqtt_ca_cert.pem";
#endif
// end of json variable names

struct timeval lastMqttConnectionAttemptTime;
bool paramsHaveChanged = true; 

Config::ConfigItem configItems[] =  {
  Config::ConfigItem(json_name_enabled, Config::SectionId::MQTT_id, RFLink_default_MQTT_ENABLED, paramsUpdatedCallback),
  Config::ConfigItem(json_name_server,  Config::SectionId::MQTT_id, MQTT_SERVER, paramsUpdatedCallback),
  Config::ConfigItem(json_name_port,    Config::SectionId::MQTT_id, MQTT_PORT, paramsUpdatedCallback),
  Config::ConfigItem(json_name_id,      Config::SectionId::MQTT_id, MQTT_ID, paramsUpdatedCallback),
  Config::ConfigItem(json_name_user,    Config::SectionId::MQTT_id, MQTT_USER, paramsUpdatedCallback),
  Config::ConfigItem(json_name_password,Config::SectionId::MQTT_id, MQTT_PSWD, paramsUpdatedCallback),

  Config::ConfigItem(json_name_topic_in,   Config::SectionId::MQTT_id, MQTT_TOPIC_IN, paramsUpdatedCallback),
  Config::ConfigItem(json_name_topic_out,  Config::SectionId::MQTT_id, MQTT_TOPIC_OUT, paramsUpdatedCallback),

  Config::ConfigItem(json_name_lwt_enabled, Config::SectionId::MQTT_id, RFLink_default_MQTT_LWT, paramsUpdatedCallback),
  Config::ConfigItem(json_name_topic_lwt,   Config::SectionId::MQTT_id, MQTT_TOPIC_LWT, paramsUpdatedCallback),

  #ifndef RFLINK_MQTT_CLIENT_SSL_DISABLED
  Config::ConfigItem(json_name_ssl_enabled, Config::SectionId::MQTT_id, RFLink_default_MQTT_SSL_ENABLED, paramsUpdatedCallback),
  Config::ConfigItem(json_name_ssl_insecure,Config::SectionId::MQTT_id, true, paramsUpdatedCallback),
  Config::ConfigItem(json_name_ca_cert,     Config::SectionId::MQTT_id, "", paramsUpdatedCallback),
  #endif

  Config::ConfigItem()
};

PubSubClient MQTTClient; // MQTTClient(WIFIClient);

void callback(char *, byte *, unsigned int);

void paramsUpdatedCallback() {
  refreshParametersFromConfig();
}

void refreshParametersFromConfig(bool triggerChanges) {

    Config::ConfigItem *item;
    bool changesDetected = false;

    item = Config::findConfigItem(json_name_enabled, Config::SectionId::MQTT_id);
    if( item->getBoolValue() != params::enabled) {
      changesDetected = true;
      params::enabled = item->getBoolValue();
    }

    item = Config::findConfigItem(json_name_server, Config::SectionId::MQTT_id);
    if( params::server != item->getCharValue() ) {
      changesDetected = true;
      params::server = item->getCharValue();
    }

    item = Config::findConfigItem(json_name_port, Config::SectionId::MQTT_id);
    if( item->getLongIntValue() != params::port) {
      changesDetected = true;
      params::port = item->getLongIntValue();
    }

    item = Config::findConfigItem(json_name_id, Config::SectionId::MQTT_id);
    if( params::id != item->getCharValue() ) {
      changesDetected = true;
      params::id = item->getCharValue();
    }

    item = Config::findConfigItem(json_name_user, Config::SectionId::MQTT_id);
    if( params::user != item->getCharValue() ) {
      changesDetected = true;
      params::user = item->getCharValue();
    }

    item = Config::findConfigItem(json_name_password, Config::SectionId::MQTT_id);
    if( params::password != item->getCharValue() ) {
      changesDetected = true;
      params::password = item->getCharValue();
    }

    item = Config::findConfigItem(json_name_topic_in, Config::SectionId::MQTT_id);
    if( params::topic_in != item->getCharValue() ) {
      changesDetected = true;
      params::topic_in = item->getCharValue();
    }

    item = Config::findConfigItem(json_name_topic_out, Config::SectionId::MQTT_id);
    if( params::topic_out != item->getCharValue() ) {
      changesDetected = true;
      params::topic_out = item->getCharValue();
    }

    item = Config::findConfigItem(json_name_lwt_enabled, Config::SectionId::MQTT_id);
    if( item->getBoolValue() != params::lwt_enabled) {
      changesDetected = true;
      params::lwt_enabled = item->getBoolValue();
    }

    item = Config::findConfigItem(json_name_topic_lwt, Config::SectionId::MQTT_id);
    if( params::topic_lwt != item->getCharValue() ) {
      changesDetected = true;
      params::topic_lwt = item->getCharValue();
    }

    #ifndef RFLINK_MQTT_CLIENT_SSL_DISABLED

    item = Config::findConfigItem(json_name_ssl_enabled, Config::SectionId::MQTT_id);
    if( item->getBoolValue() != params::ssl_enabled) {
      changesDetected = true;
      params::ssl_enabled = item->getBoolValue();
    }

    item = Config::findConfigItem(json_name_ssl_insecure, Config::SectionId::MQTT_id);
    if( item->getBoolValue() != params::ssl_insecure) {
      changesDetected = true;
      params::ssl_insecure = item->getBoolValue();
    }
    #endif

    // Applying changes will happen in mainLoop()
    if(triggerChanges && changesDetected) {
      Serial.println(F("Mqtt parameters have changed, they will be applied at next 'loop'."));
      paramsHaveChanged = true; 
    }

}


void setup_MQTT()
{
  lastMqttConnectionAttemptTime.tv_sec = 0;

  refreshParametersFromConfig(false);

  MQTTClient.setKeepAlive(MQTT_KEEPALIVE);

  if (params::port == 0)
    params::port = 1883;

  MQTTClient.setCallback(callback);

  bResub = true;
  paramsHaveChanged = true; // force parameters to be applied at next loop
}

void callback(char *topic, byte *payload, unsigned int length)
{
  payload[length] = 0;
  CheckMQTT(payload);
}


void reconnect(int retryCount, bool force)
{
  if(!params::enabled)
    return;

  if(paramsHaveChanged)
    return;

  if(params::ssl_enabled && !params::ssl_insecure && !Wifi::ntpIsSynced()) // Certificates cannot be validated if time is not synced
    return;


  struct timeval currentTime;
  gettimeofday(&currentTime, nullptr);

  if(!force) {
    if(currentTime.tv_sec- lastMqttConnectionAttemptTime.tv_sec < 30) {
      return;
    }
  }

  int retryLeft = retryCount;
  bResub = true;

  if(force && MQTTClient.connected()) {
    MQTTClient.disconnect();
  }


  while (!MQTTClient.connected() && (retryCount<=0 || retryLeft>0))
  {
    retryLeft--;
    lastMqttConnectionAttemptTime.tv_sec = currentTime.tv_sec;

    Serial.print(F("Trying to connect to MQTT Server '"));
    Serial.print(params::server.c_str());
    Serial.print(F("' ... "));

    bool connectOK;

    if(params::lwt_enabled) {
      #ifdef ESP32
      connectOK = MQTTClient.connect(params::id.c_str(), params::user.c_str(), params::password.c_str(), (params::topic_lwt).c_str(), 2, true, PSTR("Offline"));
      #elif defined(ESP8266)
      connectOK = MQTTClient.connect(params::id.c_str(), params::user.c_str(), params::password.c_str(), (params::topic_lwt).c_str(), 2, true, "Offline");
      #endif // ESP

    } else {
      connectOK = MQTTClient.connect(params::id.c_str(), params::user.c_str(), params::password.c_str());
    }

    if(connectOK)
    {
      Serial.println(F("Established"));
      Serial.print(F("MQTT ID :\t\t"));
      Serial.println(params::id.c_str());
      Serial.print(F("MQTT Username :\t\t"));
      Serial.println(params::user.c_str());
      if(params::lwt_enabled) {
        #ifdef ESP32
              MQTTClient.publish((params::topic_lwt).c_str(), PSTR("Online"), true);
        #elif ESP8266
              MQTTClient.publish((params::topic_lwt).c_str(), "Online", true);
        #endif // ESP
      }
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
  if(!params::enabled)
    return;

  if(!MQTTClient.connected())
    return;

  static boolean MQTT_RETAINED = MQTT_RETAINED_0;

  if (!MQTTClient.connected())
    reconnect(1);
  MQTTClient.publish(params::topic_out.c_str(), pbuffer, MQTT_RETAINED);
}

void checkMQTTloop()
{
  if(!RFLink::Wifi::clientNetworkIsUp()) return;

  if(paramsHaveChanged) {
    paramsHaveChanged = false;
    vars::disabledBecauseOfError = false;

    Serial.println(F("MQTT parameters have changed, first disconnecting..."));

    if(MQTTClient.connected()) {
      MQTTClient.disconnect();
    }

    if(WIFIClient.connected()) {
      WIFIClient.stop();
    }

    #ifndef RFLINK_MQTT_CLIENT_SSL_DISABLED
    if(WIFIClientSecure != nullptr) {
      if(WIFIClientSecure->connected())
        WIFIClientSecure->stop();
      delete WIFIClientSecure;
      WIFIClientSecure = nullptr;
    }
    #endif

    Serial.println(F("MQTT parameters have changed, now applying..."));


    #ifndef RFLINK_MQTT_CLIENT_SSL_DISABLED
    if(params::ssl_enabled) {
      if(WIFIClientSecure == nullptr) {
        WIFIClientSecure = new WiFiClientSecure();
      }
      #ifndef ESP32
      if(sslTrustedAnchors != nullptr) {
        delete sslTrustedAnchors;
        sslTrustedAnchors = nullptr;
      }
      #endif
      MQTTClient.setClient(*WIFIClientSecure);

      if(!params::ssl_insecure) {
        Serial.print(F("MQTT SSL trusted CA is required... "));
        if(LittleFS.exists(mqtt_ca_cert_filename)) {
          File caCertFile = LittleFS.open(mqtt_ca_cert_filename, "r");
          if(caCertFile) {
            vars::ca_cert_content = caCertFile.readString();
            #ifdef ESP32
            WIFIClientSecure->setCACert(vars::ca_cert_content.c_str());
            #else
            sslTrustedAnchors = new X509List(vars::ca_cert_content.c_str());
            WIFIClientSecure->setTrustAnchors(sslTrustedAnchors);
            #endif
            Serial.println(F("OK!"));
            Serial.println(F("NOTE: until NTP time is synchronized, MQTT will try to connect"));
          } else {
            vars::disabledBecauseOfError = true;
            vars::lastError = F("Cannot open ca_crt file: ");
            vars::lastError += mqtt_ca_cert_filename;
          }
        }
        else {
          vars::disabledBecauseOfError = true;
          vars::lastError = F("MQTT CA cert file not found in LittleFS volume!");
          Serial.println(vars::lastError);
        }
      } else {
        WIFIClientSecure->setInsecure();
      }
    }
    else
    #endif
      MQTTClient.setClient(WIFIClient);

    if(vars::disabledBecauseOfError) {
      Serial.println(F("MQTT disabled because of previous error"));
      return;
    }

    MQTTClient.setServer(params::server.c_str(), params::port);
    reconnect(1, true);
    gettimeofday(&lastMqttConnectionAttemptTime, nullptr);
    return;
  }

  if(!params::enabled || vars::disabledBecauseOfError) {
    return;
  }

  static unsigned long lastCheck = millis();

  if (millis() > lastCheck + MQTT_LOOP_MS)
  {
    if (!MQTTClient.connected()) {
      reconnect(1);
    }

    if (bResub)
    {
      // Once connected, resubscribe
      MQTTClient.subscribe(params::topic_in.c_str());
      bResub = false;
      delay(10);
    }
    MQTTClient.loop();
    lastCheck = millis();
  }

}

void getStatusJsonString(JsonObject &output) {
  auto && mqtt = output.createNestedObject("mqtt");

  if(params::enabled) {
    if( MQTTClient.connected() ) {
      mqtt["status"] = "connected";
    } else {
      mqtt["status"] = "error";
    }
  } else {
    mqtt["status"] = "disabled";
  }


}

void triggerParamsHaveChanged() {
  paramsHaveChanged = true;
}


}} // end of Mqtt namespace

#endif // RFLINK_WIFI_ENABLED
#endif // not RFLINK_MQTT_DISABLED



