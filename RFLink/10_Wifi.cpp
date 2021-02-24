#if defined(RFLINK_WIFIMANAGER_ENABLED) || defined(RFLINK_WIFI_ENABLED)

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
    String SSID = WIFI_SSID;
    String PSWD = WIFI_PSWD;
    #ifndef USE_DHCP
    String IP = WIFI_IP;
    String DNS = WIFI_DNS;
    String GATEWAY = WIFI_GATEWAY;
    String SUBNET = WIFI_SUBNET;
    #endif
}


Config::ConfigItem configItems[] =  {
  Config::ConfigItem("host", Config::SectionId::MQTT_id, "enter a hostname here", nullptr),
  Config::ConfigItem("port", Config::SectionId::MQTT_id, 1900, nullptr),
  Config::ConfigItem()

};



static String WIFI_PWR = String(WIFI_PWR_0);

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
    WiFi.config(ipaddr_addr(params::IP.c_str()), ipaddr_addr(params::GATEWAY.c_str()), ipaddr_addr(params::SUBNET.c_str()));
    #endif // USE_DHCP
}

void start_WIFI()
{
  WiFi.mode(WIFI_STA);

  // We start by connecting to a WiFi network
  Serial.print(F("WiFi SSID :\t\t"));
  Serial.println(params::SSID.c_str());
  Serial.print(F("WiFi Connection :\t"));
  WiFi.begin(params::SSID.c_str(), params::PSWD.c_str());

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
    
    #if defined(RFLINK_OTA_ENABLED)
    ArduinoOTA.handle();
    #endif
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


#endif // RFLINK_WIFIMANAGER_ENABLED || RFLINK_WIFI_ENABLED

