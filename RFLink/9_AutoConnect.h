// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef AutoConnect_h
#define AutoConnect_h

#include <Arduino.h>
#include "RFLink.h"
#ifdef AUTOCONNECT_ENABLED
#ifdef ESP8266
#include <ESP8266WiFi.h>

#include <AutoConnect.h>
#define GET_CHIPID() (ESP.getChipId())

// Adds MQTT tab to Autoconnect
#define PARAM_FILE "/settings.json"
#define AUX_SETTING_URI "/settings"
#define AUX_SAVE_URI "/settings_save"
//#define AUX_CLEAR_URI   "/settings_clear"

extern byte ac_WIFI_PWR;
extern IPAddress ac_WIFI_IP;
extern IPAddress ac_WIFI_GATEWAYS;
extern IPAddress ac_WIFI_SUBNET;
extern IPAddress ac_WIFI_DNS;
extern char ac_WIFI_SSID[];
extern char ac_WIFI_PSWD[];

extern String ac_MQTT_SERVER;
extern String ac_MQTT_PORT;
extern String ac_MQTT_ID;
extern String ac_MQTT_USER;
extern String ac_MQTT_PSWD;
extern String ac_MQTT_TOPIC_OUT;
extern String ac_MQTT_TOPIC_IN;
extern boolean ac_MQTT_RETAINED;
extern String ac_Adv_HostName;

void setup_AutoConnect();

// JSON definition of AutoConnectAux.
// Multiple AutoConnectAux can be defined in the JSON array.
// In this example, JSON is hard-coded to make it easier to understand
// the AutoConnectAux API. In practice, it will be an external content
// which separated from the sketch, as the mqtt_RSSI_FS example shows.
static const char AUX_settings[] PROGMEM = R"raw(
[
  {
    "title": "RFlink-ESP Settings",
    "uri": "/settings",
    "menu": true,
    "element": [
      {
        "name": "style",
        "type": "ACStyle",
        "value": "label+input,label+select{position:sticky;left:180px;width:230px!important;box-sizing:border-box;}"
      },
      {
        "name": "newline0",
        "type": "ACElement",
        "value": "<hr>"
      },
      {
        "name": "header",
        "type": "ACText",
        "value": "<h2>MQTT broker settings</h2>",
        "style": "text-align:center;color:#2f4f4f;padding:10px;"
      },
      {
        "name": "caption",
        "type": "ACText",
        "value": "MQTT Broker settings",
        "style": "font-family:serif;color:#4682b4;"
      },
      {
        "name": "newline1",
        "type": "ACElement",
        "value": "<hr>"
      },
      {
        "name": "MQTT_SERVER",
        "type": "ACInput",
        "value": "",
        "label": "MQTT Server",
        "pattern": "^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])$",
        "placeholder": "MQTT broker server"
      },
      {
        "name": "MQTT_PORT",
        "type": "ACInput",
        "value": "",
        "label": "MQTT Port",
        "placeholder": "MQTT port (default 1883)"
      },
      {
        "name": "MQTT_ID",
        "type": "ACInput",
        "label": "MQTT ID",
        "pattern": "^[0-9]{6}$"
      },
      {
        "name": "MQTT_USER",
        "type": "ACInput",
        "label": "MQTT User",
        "pattern": "^[0-9]{6}$"
      },
      {
        "name": "MQTT_PSWD",
        "type": "ACInput",
        "label": "MQTT Password"
      },
      {
        "name": "MQTT_TOPIC_OUT",
        "type": "ACInput",
        "label": "Out Topic",
        "pattern": "(.*)[^\/]$",
        "placeholder": "example : RFlink/msg"
      },
      {
        "name": "MQTT_TOPIC_IN",
        "type": "ACInput",
        "label": "In Topic",
        "pattern": "(.*)[^\/]$",
        "placeholder": "example : RFlink/cmd"
      },
      {
        "name": "MQTT_RETAINED",
        "type": "ACCheckbox",
        "value": "unique",
        "label": "MQTT Retained",
        "checked": false
      },
      {
        "name": "newline2",
        "type": "ACElement",
        "value": "<hr>"
      },
     {
        "name": "style2",
        "type": "ACStyle",
        "value": "label+input,label+select{position:sticky;left:180px;width:230px!important;box-sizing:border-box;}"
      },
      {
        "name": "header2",
        "type": "ACText",
        "value": "<h2>Advanced Settings</h2>",
        "style": "text-align:center;color:#2f4f4f;padding:10px;"
      },
      {
        "name": "caption2",
        "type": "ACText",
        "value": "Other settings",
        "style": "font-family:serif;color:#4682b4;"
      },
      {
        "name": "newline3",
        "type": "ACElement",
        "value": "<hr>"
      },
      {
        "name": "Adv_HostName",
        "type": "ACInput",
        "value": "RFlink-ESP",
        "label": "Hostname",
        "pattern": "^\d*[a-z][a-z0-9!^(){}\-_~]*$"
        
      },
      {
        "name": "save",
        "type": "ACSubmit",
        "value": "Save",
        "uri": "/settings_save"
      },
      {
        "name": "discard",
        "type": "ACSubmit",
        "value": "Discard",
        "uri": "/"
      }
    ]
  },
  {
    "title": "RFlink-ESP Settings",
    "uri": "/settings_save",
    "menu": false,
    "element": [
      {
        "name": "caption",
        "type": "ACText",
        "value": "<h4>Parameters saved as:</h4>",
        "style": "text-align:center;color:#2f4f4f;padding:10px;"
      },
      {
        "name": "parameters",
        "type": "ACText"
      },
      {
        "name": "clear",
        "type": "ACSubmit",
        "value": "OK",
        "uri": "/settings"
      }
    ]
  }
]
)raw";

#endif
#endif
#endif