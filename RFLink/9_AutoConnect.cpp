// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#include <Arduino.h>
#include "RFLink.h"
#include "9_AutoConnect.h"

#ifdef AUTOCONNECT_ENABLED
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h> // Replace with WebServer.h for ESP32
#include <SPI.h>              // To acces Flash Memory
#include <FS.h>               // To save MQTT parameters
#include <AutoConnect.h>
#endif

ESP8266WebServer Server; // Replace with WebServer for ESP32
AutoConnect portal(Server);
AutoConnectConfig config;

void rootPage()
{
    char content[] = "RFLink ESP";
    Server.send(200, "text/plain", content);
}

String ac_MQTT_SERVER;
String ac_MQTT_PORT;
String ac_MQTT_ID;
String ac_MQTT_USER;
String ac_MQTT_PSWD;
String ac_MQTT_TOPIC_OUT;
String ac_MQTT_TOPIC_IN;
boolean ac_MQTT_RETAINED;
// Adds advanced tab to Autoconnect
String ac_Adv_HostName;
String ac_Adv_Power;

// Prototypes
String loadParams(AutoConnectAux &aux, PageArgument &args);
String saveParams(AutoConnectAux &aux, PageArgument &args);

void setup_AutoConnect()
{
    SPIFFS.begin();
    if (portal.load(FPSTR(AUX_settings)))
    { // we load all the settings from "/settings" uri
        AutoConnectAux &my_settings = *portal.aux(AUX_SETTING_URI);
        PageArgument args;
        loadParams(my_settings, args);

        config.apid = String("RFLink-ESP-") + String(GET_CHIPID(), HEX);
        Serial.println("AP name set to " + config.apid);

        // if not defined, default Wifi AP is 12345678, you can change it here
        // config.psk = "RFlink-ESP";

        if (ac_Adv_HostName.length())
        {
            config.hostName = ac_Adv_HostName;
            Serial.println("hostname set to " + config.hostName);
        }
        // config.bootUri = AC_ONBOOTURI_HOME;
        // config.homeUri = "/";
        config.title = "RFlink ESP";
        config.autoReconnect = true;
        portal.config(config);

        portal.on(AUX_SETTING_URI, loadParams);
        portal.on(AUX_SAVE_URI, saveParams);
    }
    else
        Serial.println("load error");

    //-------------------------------------

    if (portal.begin())
    {
        if (MDNS.begin("RFLink_ESP"))
        {
            MDNS.addService("http", "tcp", 80);
        }
        Serial.println("connected: " + WiFi.SSID());
        Serial.println("IP: " + WiFi.localIP().toString());
    }
    else
    {
        Serial.println("connection failed:" + String(WiFi.status()));
        while (1)
        {
            delay(100);
            yield();
        }
    }
}

void getParams(AutoConnectAux &aux)
{
    //////  MQTT  settings //////
    ac_MQTT_SERVER = aux["MQTT_SERVER"].value;
    ac_MQTT_SERVER.trim();
    ac_MQTT_PORT = aux["MQTT_PORT"].value;
    ac_MQTT_PORT.trim();
    ac_MQTT_ID = aux["MQTT_ID"].value;
    ac_MQTT_ID.trim();
    ac_MQTT_USER = aux["MQTT_USER"].value;
    ac_MQTT_USER.trim();
    ac_MQTT_PSWD = aux["MQTT_PSWD"].value;
    ac_MQTT_PSWD.trim();
    ac_MQTT_TOPIC_OUT = aux["MQTT_TOPIC_OUT"].value;
    ac_MQTT_TOPIC_OUT.trim();
    ac_MQTT_TOPIC_IN = aux["MQTT_TOPIC_IN"].value;
    ac_MQTT_TOPIC_IN.trim();
    ac_MQTT_RETAINED = aux["MQTT_RETAINED"].as<AutoConnectCheckbox>().checked;

    ////// advanced settings //////
    ac_Adv_HostName = aux["Adv_HostName"].value;
    ac_Adv_HostName.trim();
    ac_Adv_Power = aux["Adv_Power"].value;
    ac_Adv_Power.trim();
}

// Load parameters saved with  saveParams from SPIFFS into the
// elements defined in /settings JSON.
String loadParams(AutoConnectAux &aux, PageArgument &args)
{
    (void)(args);
    File param = SPIFFS.open(PARAM_FILE, "r");
    if (param)
    {
        if (aux.loadElement(param))
        {
            getParams(aux);
            Serial.println(PARAM_FILE " loaded");
        }
        else
            Serial.println(PARAM_FILE " failed to load");
        param.close();
    }
    else
    {
        Serial.println(PARAM_FILE " open+r failed");
#ifdef ESP32
        Serial.println("If you get error as 'SPIFFS: mount failed, -10025', Please modify with 'SPIFFS.begin(true)'.");
#endif
    }
    return String("");
}

// Save the value of each element entered by '/settings' to the
// parameter file. The saveParams as below is a callback function of
// /settings_save. When invoking this handler, the input value of each
// element is already stored in '/settings'.
// In Sketch, you can output to stream its elements specified by name.
String saveParams(AutoConnectAux &aux, PageArgument &args)
{
    // The 'where()' function returns the AutoConnectAux that caused
    // the transition to this page.
    AutoConnectAux &my_settings = *portal.aux(portal.where());
    getParams(my_settings);
    // AutoConnectInput& mqttserver = my_settings["mqttserver"].as<AutoConnectInput>();  //-> BUG
    // The entered value is owned by AutoConnectAux of /settings.
    // To retrieve the elements of /settings, it is necessary to get
    // the AutoConnectAux object of /settings.
    File param = SPIFFS.open(PARAM_FILE, "w");
    if (param)
    {
        my_settings.saveElement(param, {"MQTT_SERVER", "MQTT_PORT",
                                        "MQTT_ID", "MQTT_USER", "MQTT_PSWD",
                                        "MQTT_TOPIC_OUT", "MQTT_TOPIC_IN", "MQTT_RETAINED",
                                        "Adv_HostName", "Adv_Power"});
        param.close();
    }
    else
    {
        Serial.println(PARAM_FILE " open+w failed");
    }
    // Echo back saved parameters to AutoConnectAux page.
    AutoConnectText &echo = aux["parameters"].as<AutoConnectText>();
    echo.value = "<u><b>MQTT settings</b></u><br>";
    echo.value += "<b>Connexion</b><br>";
    echo.value += "Server: " + ac_MQTT_SERVER + "<br>";
    echo.value += "Port: " + ac_MQTT_PORT + "<br>";
    echo.value += "ID: " + ac_MQTT_ID + "<br>";
    echo.value += "Username: " + ac_MQTT_USER + "<br>";
    echo.value += "Password: " + ac_MQTT_PSWD + "<br>";
    echo.value += "<br><b>Messages</b><br>";
    echo.value += "Out Topic: " + ac_MQTT_TOPIC_OUT + "<br>";
    echo.value += "In Topic: " + ac_MQTT_TOPIC_IN + "<br>";
    echo.value += "Retained: " + String(ac_MQTT_RETAINED == true ? "true" : "false") + "<br>";
    echo.value += "<u><br><b>Advanced settings</b></u><br>";
    echo.value += "<b>WiFi</b><br>";
    echo.value += "Hostname: " + ac_Adv_HostName + "<br>";
    echo.value += "TX Power: " + ac_Adv_Power + "<br>";
    return String("");
}

#endif
