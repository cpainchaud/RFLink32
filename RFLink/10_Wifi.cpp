#if defined(RFLINK_WIFI_ENABLED)

#include "RFLink.h"
#include <Arduino.h>

#include "6_Credentials.h"
#include "10_Wifi.h"
#ifdef RFLINK_OTA_ENABLED
#include <ArduinoOTA.h>
#endif
#ifdef RFLINK_ASYNC_RECEIVER_ENABLED
#include "2_Signal.h"
#endif
#ifdef MQTT_ENABLED
#include "6_WiFi_MQTT.h"
#endif

#ifdef RFLINK_AUTOOTA_ENABLED
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#endif




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

const char json_name_ap_enabled[] = "ap_enabled";
const char json_name_ap_ssid[] = "ap_ssid";
const char json_name_ap_password[] = "ap_password";
const char json_name_ap_ip[] = "ap_ip";
const char json_name_ap_network[] = "ap_network";
const char json_name_ap_mask[] = "ap_mask";
// end of json variable names


Config::ConfigItem configItems[] =  {
  Config::ConfigItem("host", Config::SectionId::MQTT_id, "enter a hostname here", clientParamsUpdatedCallback),
  Config::ConfigItem("port", Config::SectionId::MQTT_id, 1900, clientParamsUpdatedCallback),

  Config::ConfigItem(json_name_client_enabled,      Config::SectionId::Wifi_id, false, clientParamsUpdatedCallback),
  Config::ConfigItem(json_name_client_dhcp_enabled, Config::SectionId::Wifi_id, true, clientParamsUpdatedCallback),
  Config::ConfigItem(json_name_client_ssid,         Config::SectionId::Wifi_id, "ESPLink-AP", clientParamsUpdatedCallback),
  Config::ConfigItem(json_name_client_password,     Config::SectionId::Wifi_id, "inputyourown", clientParamsUpdatedCallback),
  Config::ConfigItem(json_name_client_ip,           Config::SectionId::Wifi_id, "192.168.0.200", clientParamsUpdatedCallback),
  Config::ConfigItem(json_name_client_mask,         Config::SectionId::Wifi_id, "255.255.255.0", clientParamsUpdatedCallback),
  Config::ConfigItem(json_name_client_gateway,      Config::SectionId::Wifi_id, "192.168.0.1", clientParamsUpdatedCallback),
  Config::ConfigItem(json_name_client_dns,          Config::SectionId::Wifi_id, "192.168.0.1", clientParamsUpdatedCallback),

  Config::ConfigItem(json_name_ap_enabled,  Config::SectionId::Wifi_id, true, accessPointParamsUpdatedCallback),
  Config::ConfigItem(json_name_ap_ssid,     Config::SectionId::Wifi_id, "ESPLink-AP", accessPointParamsUpdatedCallback),
  Config::ConfigItem(json_name_ap_password, Config::SectionId::Wifi_id, "", accessPointParamsUpdatedCallback),
  Config::ConfigItem(json_name_ap_ip,       Config::SectionId::Wifi_id, "192.168.4.1", accessPointParamsUpdatedCallback),
  Config::ConfigItem(json_name_ap_network,  Config::SectionId::Wifi_id, "192.168.4.0", accessPointParamsUpdatedCallback),
  Config::ConfigItem(json_name_ap_mask,     Config::SectionId::Wifi_id, "255.255.255.0", accessPointParamsUpdatedCallback),

  Config::ConfigItem()
};

void refreshClientParametersFromConfig(bool triggerChanges=true) {
    params::client_enabled = Config::findConfigItem(json_name_client_enabled, Config::SectionId::Wifi_id)->getBoolValue();
    params::client_dhcp_enabled = Config::findConfigItem(json_name_client_dhcp_enabled, Config::SectionId::Wifi_id)->getBoolValue();

    params::client_ssid = Config::findConfigItem(json_name_client_ssid, Config::SectionId::Wifi_id)->getCharValue();
    params::client_password = Config::findConfigItem(json_name_client_password, Config::SectionId::Wifi_id)->getCharValue();

    params::client_ip = Config::findConfigItem(json_name_client_ip, Config::SectionId::Wifi_id)->getCharValue();
    params::client_mask = Config::findConfigItem(json_name_client_mask, Config::SectionId::Wifi_id)->getCharValue();
    params::client_gateway = Config::findConfigItem(json_name_client_gateway, Config::SectionId::Wifi_id)->getCharValue();
    params::client_dns = Config::findConfigItem(json_name_client_dns, Config::SectionId::Wifi_id)->getCharValue();
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
    if(triggerChanges && changesDetected)
      accessPointParamsHaveChanged = true;

}


void clientParamsUpdatedCallback() {
  refreshClientParametersFromConfig();
}

void accessPointParamsUpdatedCallback() {
  refreshAccessPointParametersFromConfig();
}


static String WIFI_PWR = String(WIFI_PWR_0);

void setup_WIFI()
{
    refreshClientParametersFromConfig(false);
    refreshAccessPointParametersFromConfig(false);

    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);
    #ifdef ESP32
    WiFi.setTxPower(WIFI_POWER_11dBm);
    #elif ESP8266
    WiFi.setSleepMode(WIFI_MODEM_SLEEP);
    WiFi.setOutputPower(WIFI_PWR.toInt());
    #endif // ESP

    if(!params::client_dhcp_enabled)
      WiFi.config(ipaddr_addr(params::client_ip.c_str()), ipaddr_addr(params::client_gateway.c_str()), ipaddr_addr(params::client_mask.c_str()));
    
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
    Serial.print("* WIFI AP starting ... ");
    if( !WiFi.softAP(params::AP_ssid.c_str(), params::AP_password.c_str()) )
      Serial.println("FAILED");
    else
      Serial.println("OK");
  }

  if(params::client_enabled) {
      // We start by connecting to a WiFi network
    Serial.print(F("WiFi SSID :\t\t"));
    Serial.println(params::client_ssid.c_str());
    Serial.print(F("WiFi Connection :\t"));
    WiFi.begin(params::client_ssid.c_str(), params::client_password.c_str());

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


void setup() {
    setup_WIFI();
    start_WIFI();

    #ifdef RFLINK_OTA_ENABLED
        #ifdef RFLINK_OTA_PASSWORD
        ArduinoOTA.setPassword(RFLINK_OTA_PASSWORD);
        #endif

        #ifdef RFLINK_ASYNC_RECEIVER_ENABLED
        // we must stop the Receiver from interrupting the OTA process
        ArduinoOTA.onStart( [](){
            Serial.println("20;XX;DEBUG;MSG=OTA requested, turning off Receiver");
            AsyncSignalScanner::stopScanning();
            }
        );
        ArduinoOTA.onError( [](ota_error_t error){
            Serial.print("20;XX;DEBUG;MSG=OTA failed with error code #");
            Serial.print(error);
            Serial.println(" ,turning on Receiver");
            AsyncSignalScanner::startScanning();
            }
        );
        #endif // RFLINK_ASYNC_RECEIVER_ENABLED
    ArduinoOTA.begin();
    #endif // RFLINK_OTA_ENABLED
}

void mainLoop() {

    if(accessPointParamsHaveChanged) {
      accessPointParamsHaveChanged = false;

      Serial.println("Appliying new Wifi AP settings");

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
          Serial.println("WIFI AP start FAILED");
        else
          Serial.println("WIFI AP started");
    } else {
      Serial.println("Shutting down AP");
      WiFi.enableAP(false);
    }
    #endif

  }


  #if defined(RFLINK_OTA_ENABLED)
  ArduinoOTA.handle();
  #endif

} // end of mainLoop()

void getStatusJsonString(JsonObject &output) {

  auto && network = output.createNestedObject("network");

  auto && wifi_ap = network.createNestedObject("wifi_ap");

  if(params::AP_enabled) {
    wifi_ap["status"] = "enabled";
  } else {
    wifi_ap["status"] = "disabled";
  }

  auto && wifi_client = network.createNestedObject("wifi_client");

  if(params::client_enabled) {
    if(WiFi.isConnected()) {
      wifi_client["status"] = "connected";
      wifi_client["ip"] = WiFi.localIP().toString();
      wifi_client["netmask"] = WiFi.subnetMask().toString();
      wifi_client["dns"] = "unknown";
    } else {
      wifi_ap["status"] = "disconnected";
    }
  } else {
    wifi_client["status"] = "disabled";
  }

}


} // end of Wifi namespace

#ifdef RFLINK_AUTOOTA_ENABLED
namespace AutoOTA {

  void checkForUpdateAndApply()
  {
    NVS.begin();
    String CurrentFirmware = NVS.getString("FirmWare");
    String url = NVS.getString(autoota_url_paramid);
    if(url.length() < 1)
     url = AutoOTA_URL;
    NVS.close();

    // Return if WiFi not connected
    if (WiFi.status() != WL_CONNECTED) return;
    Serial.println();
    Serial.println("FOTA : "+ url);
    // Read Reference of installed Firmware
    
    HTTPClient http;
    WiFiClient client;

    //Look for FirmWare Update
    String FirmWareDispo="";
    const char * headerKeys[] = {"Last-Modified"};
    const size_t numberOfHeaders = 1;
    http.begin(url);
    http.collectHeaders(headerKeys, numberOfHeaders);
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK)
      {
        Serial.println("FOTA : No file available (" + http.errorToString(httpCode) + ")");
        http.end();
        return;
      }
    FirmWareDispo=http.header((size_t)0);
    http.end();

    // Check Date of UpDate
    Serial.println("FOTA : Firware available = " + FirmWareDispo);
    if (CurrentFirmware=="" || CurrentFirmware==FirmWareDispo)
      {
      Serial.println("FOTA : no new UpDate !");
      return;
      }

    //Download process
    //httpUpdate.setLedPin(Led_Pin, LOW); // Value for LED ON
    t_httpUpdate_return ret;
    Serial.println();
    Serial.println("*********************");
    Serial.println("FOTA : DOWNLOADING...");
    httpUpdate.rebootOnUpdate(false);
    ret = httpUpdate.update(client, url);
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.println(String("FOTA : Uploading Error !") + httpUpdate.getLastError() + httpUpdate.getLastErrorString().c_str());
        break;
      case HTTP_UPDATE_NO_UPDATES:
       Serial.println("FOTA : UpDate not Available");
        break;
      case HTTP_UPDATE_OK:
        Serial.println("FOTA : Update OK !!!");
        Serial.println("*********************");
        Serial.println();
        NVS.begin();
        NVS.setString("FirmWare", FirmWareDispo);
        NVS.close();
        WiFi.persistent(true);
        delay(1000);
        ESP.restart();
        break;
      }
  }
} // end of AutoOTA namespace
#endif // RFLINK_AUTOOTA_ENABLED

} // end RFLink namespace

#endif // RFLINK_WIFI_ENABLED

