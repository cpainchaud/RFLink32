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
#include "ArduinoNvs.h"
#endif

#ifdef RFLINK_WIFI_ENABLED
#include "WiFi.h"
#endif


#ifdef RFLINK_WIFIMANAGER_ENABLED
#include "WiFiManager.h"
WiFiManager wifiManager;
#endif

#if defined(ESP32) && defined(MQTT_ENABLED) && defined(RFLINK_WIFIMANAGER_ENABLED) // to store MQTT configuration in Flash
#include "ArduinoNvs.h"
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


#ifdef RFLINK_WIFIMANAGER_ENABLED

#if defined(MQTT_ENABLED) // to store MQTT configuration in Flash
const String mqtt_s_paramid("mqtt_s");
const String mqtt_p_paramid("mqtt_p");
const String mqtt_id_paramid("mqtt_id");
const String mqtt_u_paramid("mqtt_u");
const String mqtt_sec_paramid("mqtt_sec");
WiFiManagerParameter mqtt_s_param(mqtt_s_paramid.c_str(), "hostname or ip",  Mqtt::params::SERVER.c_str(), 40);
WiFiManagerParameter mqtt_p_param(mqtt_p_paramid.c_str(), "port",  Mqtt::params::PORT.c_str(), 6);
WiFiManagerParameter mqtt_id_param(mqtt_id_paramid.c_str(), "id", Mqtt::params::ID.c_str(), 20);
WiFiManagerParameter mqtt_u_param(mqtt_u_paramid.c_str(), "user", Mqtt::params::USER.c_str(), 20);
WiFiManagerParameter mqtt_sec_param(mqtt_sec_paramid.c_str(), "mqtt password", Mqtt::params::PSWD.c_str(), 20);

void copyParamsFromWM_to_MQTT(){
  auto params = wifiManager.getParameters();
  for(int i=0; i<wifiManager.getParametersCount(); i++) {
    auto currentId =  params[i]->getID();
    
    if(mqtt_s_paramid == currentId) {
      Mqtt::params::SERVER =  params[i]->getValue();
      continue;
    }

    if(mqtt_p_paramid == currentId) {
      Mqtt::params::PORT =  params[i]->getValue();
      continue;
    }

    if(mqtt_id_paramid == currentId) {
      Mqtt::params::ID =  params[i]->getValue();
      continue;
    }

    if(mqtt_u_paramid == currentId) {
      Mqtt::params::USER =  params[i]->getValue();
      continue;
    }

    if(mqtt_sec_paramid == currentId) {
      Mqtt::params::PSWD =  params[i]->getValue();
      continue;
    }
  }
}
#endif


#if defined(RFLINK_AUTOOTA_ENABLED)
WiFiManagerParameter autoota_url_param(autoota_url_paramid.c_str(), "url_here",  Mqtt::params::SERVER.c_str(), 60);
#endif

#if defined(MQTT_ENABLED) || defined(RFLINK_AUTOOTA_ENABLED)
#if defined(ESP32)
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
    #if defined(MQTT_ENABLED)
    copyParamsFromWM_to_MQTT();
    Serial.println("Some parameters have changed, restart Mqtt Client is requested");
    RFLink::Mqtt::reconnect(1, true);
    #endif
  }
}
#else
void paramsUpdatedCallback(){
  copyParamsFromWM_to_MQTT();
  Serial.println("Some parameters have changed, restart Mqtt Client is requested");
  RFLink::Mqtt::reconnect(1, true);
}
#endif

#endif // MQTT_ENABLED

namespace types {
  enum portalActions {
    None, webRequested, CaptiveRequested, shutdownRequested
  };
}
namespace vars {
  types::portalActions portalAction = types::portalActions::None;
}

void managePortalPinInterrupt() {
  static int previousState = 0;                 // track button state
  static unsigned long buttonPressedTime = 0;   // track when button was pressed

  unsigned long now = millis();

  auto state = digitalRead(RFLINK_SHOW_CONFIG_PORTAL_PIN_BUTTON);

  if(previousState == state)
    return;

  if( previousState == 0 ) {
    buttonPressedTime = now;
  }
  else {
    unsigned long pressDuration = now - buttonPressedTime;

    if(pressDuration > 5) {
      if( wifiManager.getWebPortalActive() ||  wifiManager.getConfigPortalActive() ) {
        vars::portalAction = types::shutdownRequested;
      }
      else if(pressDuration > RFLINK_WIFIMANAGER_PORTAL_LONG_PRESS ) {
        vars::portalAction = types::CaptiveRequested;
      }else{
        vars::portalAction = types::webRequested;
      }
      detachInterrupt(RFLINK_SHOW_CONFIG_PORTAL_PIN_BUTTON);
    }
  }

  previousState = state;
}

void setup_WIFI(){

  const char* menu[] = {"wifi","param","info","close","sep","erase","restart","exit"}; 

  #if defined(RFLINK_WIFIMANAGER_ENABLED) && defined(MQTT_ENABLED)  
  wifiManager.addParameter(&mqtt_s_param);
  wifiManager.addParameter(&mqtt_p_param);
  wifiManager.addParameter(&mqtt_id_param);
  wifiManager.addParameter(&mqtt_u_param);
  wifiManager.addParameter(&mqtt_sec_param);

  #if defined(ESP32) // ESP8266 doesn't support NVS
  NVS.begin();
  auto params = wifiManager.getParameters();
  for(int i=0; i<wifiManager.getParametersCount(); i++) {
    auto currentParameter = params[i];
    auto currentId = currentParameter->getID();
    auto value = NVS.getString(currentId);
    if(value.length() > 0)
      currentParameter->setValue(value.c_str(), currentParameter->getValueLength());
  }
  #endif

  wifiManager.setSaveParamsCallback(paramsUpdatedCallback); // if Config portal is used to change paramaters, we must know about it
  #endif

  wifiManager.setMenu(menu, sizeof(menu));
}

void start_WIFI(){
  wifiManager.autoConnect();

  #if (defined(ESP32) || defined(ESP8266)) && defined(MQTT_ENABLED)
  Mqtt::params::SERVER = mqtt_s_param.getValue();
  Mqtt::params::PORT = mqtt_p_param.getValue();
  Mqtt::params::USER = mqtt_u_param.getValue();
  Mqtt::params::PSWD = mqtt_sec_param.getValue();
  Mqtt::params::ID = mqtt_id_param.getValue();
  #endif

  #if defined(RFLINK_SHOW_CONFIG_PORTAL_PIN_BUTTON) && RFLINK_SHOW_CONFIG_PORTAL_PIN_BUTTON != NOT_A_PIN
    #ifdef ESP8266
    pinMode(RFLINK_SHOW_CONFIG_PORTAL_PIN_BUTTON, INPUT);           // ESP8266 doesnt support PULLDOWN/UP
    #else
    pinMode(RFLINK_SHOW_CONFIG_PORTAL_PIN_BUTTON, INPUT_PULLDOWN);
    #endif
    Serial.print("Config portal can be started on demand via PIN #");
    Serial.println(RFLINK_SHOW_CONFIG_PORTAL_PIN_BUTTON);
    attachInterrupt(RFLINK_SHOW_CONFIG_PORTAL_PIN_BUTTON, managePortalPinInterrupt, CHANGE);
  #endif
}
#else // Regular wifi is used
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
#endif // WIFIMANAGER_ENABLED

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
    #ifdef RFLINK_WIFIMANAGER_ENABLED
    wifiManager.process(); // required for non blocking portal
        #if defined(RFLINK_SHOW_CONFIG_PORTAL_PIN_BUTTON) && RFLINK_SHOW_CONFIG_PORTAL_PIN_BUTTON != NOT_A_PIN
        if( vars::portalAction != types::portalActions::None ) { 
          if(vars::portalAction == types::portalActions::shutdownRequested) {
            Serial.println("Button pressed, WifiManager portal will be shutdown");
            wifiManager.stopConfigPortal();
            wifiManager.stopWebPortal();
          }
          else if(vars::portalAction == types::portalActions::webRequested) {
            if( wifiManager.getWebPortalActive() ||  wifiManager.getConfigPortalActive() ) {
              Serial.println("Button pressed (short) but portal is already running so no actions required");
            }
            else {
              Serial.println("Button pressed (short), WebPortal will be started");
              wifiManager.setConfigPortalBlocking(false);
              wifiManager.startWebPortal();
            }
          }
          else if(vars::portalAction == types::portalActions::CaptiveRequested) {
            if( wifiManager.getWebPortalActive() ||  wifiManager.getConfigPortalActive() ) {
              Serial.println("Button pressed (long) but portal is already running so no actions required");
            }
            else {
              Serial.println("Button pressed (long), CaptivePortal will be started");
              wifiManager.setConfigPortalBlocking(false);
              wifiManager.startConfigPortal();
            }
          }
          vars::portalAction = types::portalActions::None;
          attachInterrupt(RFLINK_SHOW_CONFIG_PORTAL_PIN_BUTTON, managePortalPinInterrupt, CHANGE);
        }
        #endif // RFLINK_SHOW_CONFIG_PORTAL_PIN_BUTTON && RFLINK_SHOW_CONFIG_PORTAL_PIN_BUTTON != NOT_A_PIN
    #endif // RFLINK_WIFIMANAGER_ENABLED

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

