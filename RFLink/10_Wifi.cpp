#if defined(RFLINK_WIFI_ENABLED)

#include "RFLink.h"
#include <Arduino.h>

#include "6_Credentials.h"
#include "10_Wifi.h"
#ifdef RFLINK_OTA_ENABLED
#include <ArduinoOTA.h>
#endif
#include "2_Signal.h"
#include "6_MQTT.h"
#include "9_Serial2Net.h"
#include <time.h> // for NTP




#ifdef RFLINK_AUTOOTA_ENABLED
const String autoota_url_paramid("autoota_url");
#endif

namespace RFLink { namespace Wifi {

        namespace params {

            bool client_enabled;
            String client_ssid;
            String client_password;

            bool client_dhcp_enabled;
            String client_ip;
            String client_mask;
            String client_gateway;
            String client_dns;
            String client_hostname;

            bool AP_enabled;
            String AP_ssid;
            String AP_password;
            String AP_ip;
            String AP_network;
            String AP_mask;
        }

        bool clientParamsHaveChanged = false; // this will be set to True when Client Wifi mode configuration has changed
        bool accessPointParamsHaveChanged = false; // this will be set to True when Client Wifi mode configuration has changed

// All json variable names
        const char json_name_client_enabled[] = "client_enabled";
        const char json_name_client_dhcp_enabled[] = "client_dhcp_enabled";
        const char json_name_client_ssid[] = "client_ssid";
        const char json_name_client_password[] = "client_password";
        const char json_name_client_ip[] = "client_ip";
        const char json_name_client_mask[] = "client_mask";
        const char json_name_client_gateway[] = "client_gateway";
        const char json_name_client_dns[] = "client_dns";
        const char json_name_client_hostname[] = "client_hostname";

        const char json_name_ap_enabled[] = "ap_enabled";
        const char json_name_ap_ssid[] = "ap_ssid";
        const char json_name_ap_password[] = "ap_password";
        const char json_name_ap_ip[] = "ap_ip";
        const char json_name_ap_network[] = "ap_network";
        const char json_name_ap_mask[] = "ap_mask";
// end of json variable names


        Config::ConfigItem configItems[] =  {
                Config::ConfigItem(json_name_client_enabled,      Config::SectionId::Wifi_id, false, clientParamsUpdatedCallback),
                Config::ConfigItem(json_name_client_dhcp_enabled, Config::SectionId::Wifi_id, true, clientParamsUpdatedCallback),
                Config::ConfigItem(json_name_client_ssid,         Config::SectionId::Wifi_id, "My Home Wifi", clientParamsUpdatedCallback),
                Config::ConfigItem(json_name_client_password,     Config::SectionId::Wifi_id, "inputyourown", clientParamsUpdatedCallback),
                Config::ConfigItem(json_name_client_ip,           Config::SectionId::Wifi_id, "192.168.0.200", clientParamsUpdatedCallback),
                Config::ConfigItem(json_name_client_mask,         Config::SectionId::Wifi_id, "255.255.255.0", clientParamsUpdatedCallback),
                Config::ConfigItem(json_name_client_gateway,      Config::SectionId::Wifi_id, "192.168.0.1", clientParamsUpdatedCallback),
                Config::ConfigItem(json_name_client_dns,          Config::SectionId::Wifi_id, "192.168.0.1", clientParamsUpdatedCallback),
                Config::ConfigItem(json_name_client_hostname,     Config::SectionId::Wifi_id, DEFAULT_WIFI_CLIENT_HOSTNAME, clientParamsUpdatedCallback),

                Config::ConfigItem(json_name_ap_enabled,  Config::SectionId::Wifi_id, true, accessPointParamsUpdatedCallback),
                Config::ConfigItem(json_name_ap_ssid,     Config::SectionId::Wifi_id, "RFLink-AP", accessPointParamsUpdatedCallback),
                Config::ConfigItem(json_name_ap_password, Config::SectionId::Wifi_id, "", accessPointParamsUpdatedCallback),
                Config::ConfigItem(json_name_ap_ip,       Config::SectionId::Wifi_id, "192.168.4.1", accessPointParamsUpdatedCallback),
                Config::ConfigItem(json_name_ap_network,  Config::SectionId::Wifi_id, "192.168.4.0", accessPointParamsUpdatedCallback),
                Config::ConfigItem(json_name_ap_mask,     Config::SectionId::Wifi_id, "255.255.255.0", accessPointParamsUpdatedCallback),

                Config::ConfigItem()
        };

        void refreshClientParametersFromConfig(bool triggerChanges=true) {

          Config::ConfigItem *item;
          bool changesDetected = false;

          item = Config::findConfigItem(json_name_client_enabled, Config::SectionId::Wifi_id);
          if( item->getBoolValue() != params::client_enabled) {
            changesDetected = true;
            params::client_enabled = item->getBoolValue();
          }

          item = Config::findConfigItem(json_name_client_dhcp_enabled, Config::SectionId::Wifi_id);
          if( item->getBoolValue() != params::client_dhcp_enabled) {
            changesDetected = true;
            params::client_dhcp_enabled = item->getBoolValue();
          }

          item = Config::findConfigItem(json_name_client_ssid, Config::SectionId::Wifi_id);
          if( params::client_ssid != item->getCharValue() ) {
            changesDetected = true;
            params::client_ssid = item->getCharValue();
          }

          item = Config::findConfigItem(json_name_client_password, Config::SectionId::Wifi_id);
          if( params::client_password != item->getCharValue() ) {
            changesDetected = true;
            params::client_password = item->getCharValue();
          }

          item = Config::findConfigItem(json_name_client_ip, Config::SectionId::Wifi_id);
          if( params::client_ip != item->getCharValue() ) {
            changesDetected = true;
            params::client_ip = item->getCharValue();
          }

          item = Config::findConfigItem(json_name_client_mask, Config::SectionId::Wifi_id);
          if( params::client_mask != item->getCharValue() ) {
            changesDetected = true;
            params::client_mask = item->getCharValue();
          }

          item = Config::findConfigItem(json_name_client_gateway, Config::SectionId::Wifi_id);
          if( params::client_gateway != item->getCharValue() ) {
            changesDetected = true;
            params::client_gateway = item->getCharValue();
          }

          item = Config::findConfigItem(json_name_client_dns, Config::SectionId::Wifi_id);
          if( params::client_dns != item->getCharValue() ) {
            changesDetected = true;
            params::client_dns = item->getCharValue();
          }

          item = Config::findConfigItem(json_name_client_hostname, Config::SectionId::Wifi_id);
          if( params::client_hostname != item->getCharValue() ) {
            changesDetected = true;
            params::client_hostname = item->getCharValue();
          }

          // Applying changes will happen in mainLoop()
          if(triggerChanges && changesDetected) {
            clientParamsHaveChanged = true;
            Serial.println(F("Client Wifi settings haver changed and will be applied at next loop"));
          }
        }

        void refreshAccessPointParametersFromConfig(bool triggerChanges=true) {

          Config::ConfigItem *item;
          bool changesDetected = false;

          item = Config::findConfigItem(json_name_ap_enabled, Config::SectionId::Wifi_id);
          if( item->getBoolValue() != params::AP_enabled) {
            changesDetected = true;
            params::AP_enabled = item->getBoolValue();
          }

          item = Config::findConfigItem(json_name_ap_ssid, Config::SectionId::Wifi_id);
          if( params::AP_ssid != item->getCharValue() ) {
            changesDetected = true;
            params::AP_ssid = item->getCharValue();
          }

          item = Config::findConfigItem(json_name_ap_password, Config::SectionId::Wifi_id);
          if( params::AP_password != item->getCharValue() ) {
            changesDetected = true;
            params::AP_password = item->getCharValue();
          }

          item = Config::findConfigItem(json_name_ap_ip, Config::SectionId::Wifi_id);
          if( params::AP_ip != item->getCharValue() ) {
            changesDetected = true;
            params::AP_ip = item->getCharValue();
          }

          item = Config::findConfigItem(json_name_ap_network, Config::SectionId::Wifi_id);
          if( params::AP_network != item->getCharValue() ) {
            changesDetected = true;
            params::AP_network = item->getCharValue();
          }

          item = Config::findConfigItem(json_name_ap_mask, Config::SectionId::Wifi_id);
          if( params::AP_mask != item->getCharValue() ) {
            changesDetected = true;
            params::AP_mask = item->getCharValue();
          }

          // Applying changes will happen in mainLoop()
          if(triggerChanges && changesDetected) {
            accessPointParamsHaveChanged = true;
            Serial.println(F("AP Wifi settings haver changed and will be applied at next loop"));
          }

        }


        void clientParamsUpdatedCallback() {
          refreshClientParametersFromConfig();
        }

        void accessPointParamsUpdatedCallback() {
          refreshAccessPointParametersFromConfig();
        }


        static String WIFI_PWR = String(WIFI_PWR_0);



        void resetClientWifi() {
          if(params::client_enabled) {

            // We start by connecting to a WiFi network
            Serial.print(F("Trying to connect WIFI SSID "));
            Serial.print(params::client_ssid.c_str());
            Serial.println(F(". A status will be given whenever it occurs."));

#ifdef ESP32
            WiFi.setHostname(params::client_hostname.c_str());
#else
            WiFi.hostname(params::client_hostname);
#endif

            if( !params::client_dhcp_enabled) {
              IPAddress ip, gateway, mask, dns;

              ip.fromString(params::client_ip);
              gateway.fromString(params::client_gateway);
              mask.fromString(params::client_mask);
              dns.fromString(params::client_dns);

              WiFi.config(ip, gateway, mask, dns );

            } else {
              WiFi.config(IPAddress((uint32_t) 0), IPAddress((uint32_t) 0), IPAddress((uint32_t) 0));
            }

            delay(500);
            if(!WiFi.isConnected() || WiFi.SSID() != params::client_ssid) {
              WiFi.begin(params::client_ssid.c_str(), params::client_password.c_str());
            }

            WiFi.setAutoConnect(true);
            WiFi.setAutoReconnect(true);

          }
          else {
            Serial.println(F("WiFi Client mode will be disconnected"));
            WiFi.setAutoConnect(false);
            WiFi.setAutoReconnect(false);
            WiFi.disconnect();
          }

        }

        void start_WIFI()
        {
          if( params::AP_enabled && params::client_enabled) {
            WiFi.mode(WIFI_AP_STA);
          } else if(params::AP_enabled) {
            WiFi.mode(WIFI_AP);
          } else if (params::client_enabled) {
            WiFi.mode(WIFI_STA);
          }

          if( params::AP_enabled ) {
            Serial.printf_P(PSTR("* WIFI AP starting with SSID '%s'... "), params::AP_ssid.c_str());
            if( !WiFi.softAP(params::AP_ssid.c_str(), params::AP_password.c_str()) )
              Serial.println(F("FAILED"));
            else
              Serial.println(F("OK"));
          }
          else {
            // just in case it
            WiFi.softAPdisconnect();
          }

          if(params::client_enabled) {
            resetClientWifi();
          }

        }

        void stop_WIFI()
        {
          WiFi.disconnect();
          WiFi.mode(WIFI_OFF);
          delay(500);
        }

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

        #ifdef ESP8266
        bool getLocalTime(struct tm * info, uint32_t ms=5000)
        {
            uint32_t start = millis();
            time_t now;
            while((millis()-start) <= ms) {
                time(&now);
                localtime_r(&now, info);
                if(info->tm_year > (2016 - 1900)){
                    return true;
                }
                delay(10);
            }
            return false;
        }
        #endif

        void printLocalTime(struct timeval *newTime = nullptr)
        {
          struct tm tmp;
          struct tm *timeinfo;

          if(newTime == nullptr) {
            if(!getLocalTime(&tmp)){
              Serial.println(F("Failed to obtain time"));
              return;
            }
            timeinfo = &tmp;
          }
          else {
            timeinfo = localtime(&newTime->tv_sec);
          }

          Serial.printf_P(PSTR("Current time is %04i-%02i-%02i %02i:%02i:%02i\r\n"),
                          timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday,
                          timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        }


        void ntpUpdateCallback(struct timeval *newTime){

          if(timeAtBoot.tv_sec < 10000 ) {
            timeAtBoot.tv_sec = newTime->tv_sec - 10;
            printLocalTime(newTime);
          }

        }

        void reconnectServices() {
          #ifdef ESP32
          // in case NTP forces a time drift we need to recalculate timeAtBoot
          sntp_set_time_sync_notification_cb(ntpUpdateCallback);
          #endif

          configTzTime("UTC", RFLink::params::ntpServer.c_str());

          if(RFLink::Mqtt::params::enabled)
            RFLink::Mqtt::reconnect(1, true);
          if(RFLink::Serial2Net::params::enabled)
            RFLink::Serial2Net::restartServer();
        }

#ifdef ESP32
        void eventHandler_WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {

          Serial.print(F("Connected to AP SSID:"));
          for(int i=0; i<info.connected.ssid_len; i++){
            Serial.print((char) info.connected.ssid[i]);
          }

          Serial.print("  BSSID: ");
          for(int i=0; i<6; i++){
            Serial.printf("%02X", info.connected.bssid[i]);

            if(i<5){
              Serial.print(":");
            }
          }

          Serial.print(F("  Channel: "));
          Serial.print(info.connected.channel);

          Serial.print(F("  Auth mode: "));
          Serial.println(info.connected.authmode);

        }

        void eventHandler_WiFiStationGotIp(WiFiEvent_t event, WiFiEventInfo_t info) {
          Serial.printf_P(PSTR("WiFi Client has received a new IP: %s\r\n"), WiFi.localIP().toString().c_str());
          reconnectServices();
        }

        void eventHandler_WiFiStationLostIp(WiFiEvent_t event, WiFiEventInfo_t info) {
          Serial.println("WiFi Client has lost its IP");
        }
#endif //ESP32

#ifdef ESP8266
        WiFiEventHandler e1;
void eventHandler_WiFiStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
  Serial.println(F("Connected to AP!"));
 
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());
     
  Serial.print(F("\r\nChannel: "));
  Serial.println(WiFi.channel());
    
}
WiFiEventHandler e2;
void eventHandler_WiFiStationGotIp(const WiFiEventStationModeGotIP& evt) {
  Serial.printf_P(PSTR("WiFi Client has received a new IP: %s\r\n"), WiFi.localIP().toString().c_str());
  Serial.flush();
  reconnectServices();
}
WiFiEventHandler e3;
void eventHandler_WiFiStationDisconnected(const WiFiEventStationModeDisconnected& evt) {
  Serial.println(F("WiFi Client has been disconnected"));
}
#endif



        void setup()
        {

          refreshClientParametersFromConfig(false);
          refreshAccessPointParametersFromConfig(false);

#ifdef ESP32
          WiFi.onEvent(eventHandler_WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
          WiFi.onEvent(eventHandler_WiFiStationGotIp, SYSTEM_EVENT_STA_GOT_IP);
          WiFi.onEvent(eventHandler_WiFiStationLostIp, SYSTEM_EVENT_STA_LOST_IP);
#else
          e1 = WiFi.onSoftAPModeStationConnected(&eventHandler_WiFiStationConnected);
          e2 = WiFi.onStationModeGotIP(&eventHandler_WiFiStationGotIp);
          e3 = WiFi.onStationModeDisconnected(&eventHandler_WiFiStationDisconnected);
#endif

#ifdef ESP32
          WiFi.setTxPower(WIFI_POWER_11dBm);
#elif ESP8266
          WiFi.setSleepMode(WIFI_MODEM_SLEEP);
  WiFi.setOutputPower(WIFI_PWR.toInt());
#endif // ESP

          start_WIFI();

#ifdef RFLINK_OTA_ENABLED
          #ifdef RFLINK_OTA_PASSWORD
  ArduinoOTA.setPassword(RFLINK_OTA_PASSWORD);
#endif

  // we must stop the Receiver from interrupting the OTA process
  ArduinoOTA.onStart([]() {
    Serial.println("20;XX;DEBUG;MSG=OTA requested, turning off Receiver");
    AsyncSignalScanner::stopScanning();
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.print(F("20;XX;DEBUG;MSG=OTA failed with error code #"));
    Serial.print(error);
    Serial.println(" ,turning on Receiver");
    AsyncSignalScanner::startScanning();
  });
  ArduinoOTA.begin();
#endif // RFLINK_OTA_ENABLED
        }

        void mainLoop() {

          if(accessPointParamsHaveChanged) {
            accessPointParamsHaveChanged = false;

            Serial.println(F("Applying new Wifi AP settings"));

#ifdef ESP32
            bool isEnabled = ((WiFi.getMode() & WIFI_MODE_AP) != 0);

            if(params::AP_enabled) {
              if(!isEnabled)
                WiFi.enableAP(true);

              if( !WiFi.softAP(params::AP_ssid.c_str(), params::AP_password.c_str()) )
                Serial.println("WIFI AP start FAILED");
              else
                Serial.println("WIFI AP started");
            } else {
              Serial.println("Shutting down AP");
              WiFi.enableAP(false);
            }
#endif
#ifdef ESP8266
            if(params::AP_enabled) {
        WiFi.enableAP(true);
        if( !WiFi.softAP(params::AP_ssid.c_str(), params::AP_password.c_str()) )
          Serial.println(F("WIFI AP start FAILED"));
        else
          Serial.println(F("WIFI AP started"));
    } else {
      Serial.println(F("Shutting down AP"));
      WiFi.enableAP(false);
    }
#endif
          } // end of accessPointParamsHaveChanges

          if( clientParamsHaveChanged ) {
            clientParamsHaveChanged = 0;
            resetClientWifi();
          }


#if defined(RFLINK_OTA_ENABLED)
          ArduinoOTA.handle();
#endif

        } // end of mainLoop()

        void getStatusJsonString(JsonObject &output) {

          auto && network = output.createNestedObject(F("network"));

          auto && wifi_ap = network.createNestedObject(F("wifi_ap"));

          if(params::AP_enabled) {
            wifi_ap[F("status")] = F("enabled");
          } else {
            wifi_ap[F("status")] = F("disabled");
          }

          auto && wifi_client = network.createNestedObject(F("wifi_client"));

          if(params::client_enabled) {
            if(WiFi.isConnected()) {
              wifi_client[F("status")] = F("connected");
              wifi_client[F("ip")] = WiFi.localIP().toString();
              wifi_client[F("netmask")] = WiFi.subnetMask().toString();
              wifi_client[F("dns")] = WiFi.dnsIP().toString();
            } else {
              wifi_ap[F("status")] = F("disconnected");
            }
          } else {
            wifi_client[F("status")] = F("disabled");
          }

        }

    } // end of Wifi namespace


} // end RFLink namespace

#endif // RFLINK_WIFI_ENABLED

