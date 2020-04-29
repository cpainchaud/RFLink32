// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#include <Arduino.h>
#include "RFLink.h"
#ifdef AUTOCONNECT_ENABLED

#include "1_Radio.h"
#include "4_Display.h" // To allow displaying the last message
#include "5_Plugin.h"
#include "6_WiFi_MQTT.h"
#include "9_AutoConnect.h"
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
typedef ESP8266WebServer WebServer;
#include <FS.h> // To save plugins parameters
#include <ArduinoJson.h>
#elif ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
typedef WebServer WebServer;
#endif

//#include <SPI.h>              // To acces Flash Memory
#include <FS.h> // To save MQTT parameters
#include <AutoConnect.h>

AutoConnect portal;
AutoConnectConfig config;

String MQTT_SERVER;
String MQTT_PORT;
String MQTT_ID;
String MQTT_USER;
String MQTT_PSWD;
String MQTT_TOPIC_OUT;
String MQTT_TOPIC_IN;
boolean MQTT_RETAINED;
// Adds advanced tab to Autoconnect
String Adv_HostName;
String Adv_Power;
String LastMsg;
// Radio pins settings
uint8_t PIN_RF_RX_PMOS;
uint8_t PIN_RF_RX_NMOS;
uint8_t PIN_RF_RX_VCC;
uint8_t PIN_RF_RX_GND;
uint8_t PIN_RF_RX_NA;
uint8_t PIN_RF_RX_DATA;
boolean PULLUP_RF_RX_DATA;
uint8_t PIN_RF_TX_PMOS;
uint8_t PIN_RF_TX_NMOS;
uint8_t PIN_RF_TX_VCC;
uint8_t PIN_RF_TX_GND;
uint8_t PIN_RF_TX_DATA;

// Prototypes
String loadParams(AutoConnectAux &aux, PageArgument &args);
String saveParams(AutoConnectAux &aux, PageArgument &args);

void rootPage()
{

    WebServer &webServer = portal.host();
    if (webServer.hasArg("BtnSave"))
    { // On n'enregistre les values que si ce n'est pas le bouton "test" qui a été appuyé

        // === Debug Part ===
        // String message = "Number of args received: ";
        // message += webServer.args(); //Get number of parameters
        // message += "\n";             //Add a new line
        // for (int i = 0; i < webServer.args(); i++)
        // {
        //     message += "Arg nº" + (String)i + " – > "; //Include the current iteration value
        //     message += webServer.argName(i) + ": ";    //Get the name of the parameter
        //     message += webServer.arg(i) + "\n";        //Get the value of the parameter
        // }
        // Serial.println(message);
        // ==================

        //const int capacity = JSON_ARRAY_SIZE(254) + 2 * JSON_OBJECT_SIZE(2);
        StaticJsonDocument<6400> doc;
        //JsonObject obj = doc.createNestedObject();

        for (byte x = 0; x < PLUGIN_MAX; x++)
        {
            if (Plugin_id[x] != 0)
            {
                // pour chaque plugin activé lors de la compilation du firmware, on créé un enregistrement dans le fichier protocols.json
                // si le serveur a un argument c'est que la checkbox est cochée

                // doc[x]["protocol"] = Plugin_id[x];
                // webServer.hasArg(Plugin_id[x] + "_ProtocolState") ? doc[x]["state"] = 1 : doc[x]["state"] = 0;
                if (webServer.hasArg(String(Plugin_id[x]) + "_ProtocolState"))
                {
                    doc[x][String(Plugin_id[x])] = 1;
                    Plugin_State[x] = 2;
                }
                else
                {
                    doc[x][String(Plugin_id[x])] = 0;
                    Plugin_State[x] = 1;
                }
            }
        }

        SPIFFS.begin();
        File configFile = SPIFFS.open(PROTOCOL_FILE, "w");
        Serial.print(PROTOCOL_FILE);

        String configString;
        serializeJson(doc, configString);
        configFile.print(configString);
        // === Debug Part ===
        // Serial.println(configString);
        // ==================

        Serial.println(F(" saved"));
        configFile.close();
        SPIFFS.end();
    }

    // This is the Home Page - Choose theme here : https://www.bootstrapcdn.com/bootswatch/?theme

    String content =
        "<html>"
        "<title>RFLink-ESP</title>"
        "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
        "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js'></script>"
        "<link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootswatch/4.4.1/flatly/bootstrap.min.css'><script src='https://stackpath.bootstrapcdn.com/bootstrap/4.4.1/js/bootstrap.min.js'></script>"
        "</head>"
        "<body>"

        // !!!!!!!!!!!!!!! Ajax auto refresh, disable it to avoid a lot of request on the ESP !!!!!!!!!!!!!!!
        // to do : add a checkbox to enable/disable it dynamically
        "<script>"
        "setInterval(function() {" // Call a function repetatively with 2 Second interval"
        "  getData();"
        "}, 1000);" //1 Second update rate
        " "
        "function getData() {"
        "  var xhttp = new XMLHttpRequest();"
        "  xhttp.onreadystatechange = function() {"
        "	if (this.readyState == 4 && this.status == 200) {"
        "	  document.getElementById('LastMsg').innerHTML ="
        "	  this.responseText;"
        "	}"
        "  };"
        "  xhttp.open('GET', 'LastMsg', true);"
        "  xhttp.send();"
        "}"
        "</script>"
        // !!!!!!!!!!!!!!! Ajax auto refresh, disable it to avoid a lot of request on the ESP !!!!!!!!!!!!!!!

        // Navigation bar

        "<nav class='navbar navbar-expand-lg navbar-dark bg-primary'>"
        "  <a class='navbar-brand' href='#'>RFlink-ESP</a>"
        "  <button class='navbar-toggler' type='button' data-toggle='collapse' data-target='#navbarColor01' aria-controls='navbarColor01' aria-expanded='false' aria-label='Toggle navigation'>"
        "	<span class='navbar-toggler-icon'></span>"
        "  </button>"
        "  <div class='collapse navbar-collapse' id='navbarColor01'>"
        "	<ul class='navbar-nav mr-auto'>"
        "	  <li class='nav-item active'>"
        "		<a class='nav-link' href='#'>Home <span class='sr-only'>(current)</span></a>"
        "	  </li>"
        "	  <li class='nav-item'>"
        "		<a class='nav-link' href='/_ac'>Network Config</a>"
        "	  </li>"
        "	  <li class='nav-item'>"
        "		<a class='nav-link' href='https://github.com/couin3/RFLink' target='_blank'>About</a>"
        "	  </li>"
        "	</ul>"
        "  </div>"
        "</nav>";

    //// Graphical icon to access network config
    // "<li class='nav-item'>" AUTOCONNECT_LINK(COG_32) "</li>";
    // "<h1>RFLink-ESP  " AUTOCONNECT_LINK(COG_32) "</h1><Br>";

    //// iframe test :
    // "<iframe width=\"450\" height=\"260\" style=\"transform:scale(0.79);-o-transform:scale(0.79);-webkit-transform:scale(0.79);-moz-transform:scale(0.79);-ms-transform:scale(0.79);transform-origin:0 0;-o-transform-origin:0 0;-webkit-transform-origin:0 0;-moz-transform-origin:0 0;-ms-transform-origin:0 0;border: 1px solid #cccccc;\" src=\"https://thingspeak.com/channels/454951/charts/1?bgcolor=%23ffffff&color=%23d62020&dynamic=true&type=line\"></iframe>"

    // content += "Last refresh : "; // Require NTP time, We'll see that later....
    // content +=          ctime(&now);
    content += "<Br>";
    content += "<div class='card bg-light mb-3' style='max-width: 50rem;'>";
    content += "  <div class='card-header'>Last Message</div>";
    content += "  <div class='card-body'>";
    //  ======== Ajax = autrefresh mode ========
    content += "<p class='card-text'><span id='LastMsg'></p>";
    //==========================================
    //==========================================
    //  ===== No Ajax = no autrefresh mode =====
    // content += "<p class='card-text'>" + LastMsg + "</p>";
    //==========================================
    content += "  </div>";
    content += "</div>";

    content += "<Br>";

    content += "<form action='/' method='POST'><button type='button submit' name='BtnTimeBeforeSWoff' value='0' class='btn btn-secondary'>Refresh</button></form><Br>";

    content += "<table class='table table-hover'  style='max-width: 50rem;'>";
    content += "<thead><tr><th>N&deg;</th><th>Name</th><th>Enabled</th></tr></thead>"; // Table Header    // é = &eacute;
    content += "<tbody>";                                                              // Table content
    content += "<form action='/' method='POST'>";
    for (byte x = 0; x < PLUGIN_MAX; x++)
    {
        if ((Plugin_id[x] != 0)) //  && (Plugin_State[x] >= P_Enabled)
        {
            ////////////////// One table line ///////////////////
            x % 2 ? content += "<tr class='table-light'><td>" : content += "<tr><td>";

            content += Plugin_id[x];
            content += "</td><td>";
            content += Plugin_Description[x];
            content += "</td><td>";
            content += "<input type='checkbox' class='form-check-input' name='";
            content += Plugin_id[x];
            content += "_ProtocolState' value='State'";

            if (Plugin_State[x] == 2)
            {
                content += " checked";
            }
            content += ">";
            content += "</td>";
            ////////////////// One table line ///////////////////
        }
    }

    content += "</tr><tr><td></td><td></td><td></td></tr>"; // we add a last line to bottom of the table
    content += "</tbody></table>";

    content += "<button type='button submit' name='BtnSave' value='0' class='btn btn-success btn-lg'>save</button></form></div>";

    content += "</body>";
    content += "</html>";

    webServer.send(200, "text/html", content);
}

void setup_AutoConnect()
{
    if (portal.load(FPSTR(AUX_settings)))
    { // we load all the settings from "/settings" uri
        AutoConnectAux &aux1 = *portal.aux(AUX_SETTING_URI);
        PageArgument args;
        loadParams(aux1, args);
        // if not defined, default Wifi AP is 12345678, you can change it here
        // config.psk = "RFlink-ESP";
        config.apid = String("RFLink_ESP-") + String(GET_CHIPID(), HEX);

        if (Adv_HostName.length())
            config.hostName = Adv_HostName;
        else
            config.hostName = String("RFLink_ESP-") + String(GET_CHIPID(), HEX);

        config.title = "RFlink-ESP Network Configuration";
        config.autoReconnect = true;
        config.homeUri = "/";
        // config.menuItems = AC_MENUITEM_OPENSSIDS | AC_MENUITEM_HOME;   // choose exposed menu items

        // ----  little trick to launch soft AP directly after 1st boot : ----
        AutoConnectCredential credential;
        uint8_t SSIDqty = credential.entries();
        if (SSIDqty == 0)
        {
            Serial.println(F("No SSID recorded, starting soft AP mode"));
            config.immediateStart = true;
            config.autoRise = true;
            /////////////////
            Serial.print(F("AP name set to "));
            Serial.println(config.apid);
        }
        //---------------------------------------------------------------------
        portal.config(config);
        /////////////////
        Serial.print(F("hostname set to "));
        Serial.println(config.hostName);
        /////////////////
        portal.on(AUX_SETTING_URI, loadParams);
        portal.on(AUX_SAVE_URI, saveParams);
    }
    else
    {
        Serial.println(F("Impossible to load settings web page"));
    }
    //-------------------------------------

    if (portal.begin())
    {
        config.bootUri = AC_ONBOOTURI_HOME;
        if (MDNS.begin(config.hostName))
            MDNS.addService("http", "tcp", 80);
        Serial.print(F("connected: "));
        Serial.println(WiFi.SSID());
        Serial.print(F("IP: "));
        Serial.println(WiFi.localIP().toString());
    }
    else
    {
        Serial.print(F("connection failed:"));
        Serial.println(String(WiFi.status()));
        while (1)
        {
            delay(100);
            yield();
        }
    }

    WebServer &webServer = portal.host();
    webServer.on("/", rootPage);
    // for ajax refresh of LastMsg
    webServer.on("/LastMsg", HandleLastMsg);
}

void loop_AutoConnect()
{
    MDNS.update();
    portal.handleClient();
}

void HandleLastMsg() // Required only for ajax auto-refresh of the last message
{
    WebServer &webServer = portal.host();
    webServer.send(200, "text/plane", LastMsg); //Send Last Message  only to client ajax request
}

void getParams(AutoConnectAux &aux)
{
    //////  MQTT  settings //////
    MQTT_SERVER = (aux["MQTT_SERVER"].value);
    MQTT_SERVER.trim();
    MQTT_PORT = aux["MQTT_PORT"].value;
    MQTT_PORT.trim();
    MQTT_ID = aux["MQTT_ID"].value;
    MQTT_ID.trim();
    MQTT_USER = aux["MQTT_USER"].value;
    MQTT_USER.trim();
    MQTT_PSWD = aux["MQTT_PSWD"].value;
    MQTT_PSWD.trim();
    MQTT_TOPIC_OUT = aux["MQTT_TOPIC_OUT"].value;
    MQTT_TOPIC_OUT.trim();
    MQTT_TOPIC_IN = aux["MQTT_TOPIC_IN"].value;
    MQTT_TOPIC_IN.trim();
    MQTT_RETAINED = aux["MQTT_RETAINED"].as<AutoConnectCheckbox>().checked;

    ////// advanced settings //////
    Adv_HostName = aux["Adv_HostName"].value;
    Adv_HostName.trim();
    Adv_Power = aux["Adv_Power"].value;
    Adv_Power.trim();

    // Radio pins settings
    PIN_RF_RX_PMOS = String2GPIO(aux["PIN_RF_RX_PMOS"].value);
    PIN_RF_RX_NMOS = String2GPIO(aux["PIN_RF_RX_NMOS"].value);
    PIN_RF_RX_VCC = String2GPIO(aux["PIN_RF_RX_VCC"].value);
    PIN_RF_RX_GND = String2GPIO(aux["PIN_RF_RX_GND"].value);
    PIN_RF_RX_NA = String2GPIO(aux["PIN_RF_RX_NA"].value);
    PIN_RF_RX_DATA = String2GPIO(aux["PIN_RF_RX_DATA"].value);
    PULLUP_RF_RX_DATA = aux["PULLUP_RF_RX_DATA"].as<AutoConnectCheckbox>().checked;
    PIN_RF_TX_PMOS = String2GPIO(aux["PIN_RF_TX_PMOS"].value);
    PIN_RF_TX_NMOS = String2GPIO(aux["PIN_RF_TX_NMOS"].value);
    PIN_RF_TX_VCC = String2GPIO(aux["PIN_RF_TX_VCC"].value);
    PIN_RF_TX_GND = String2GPIO(aux["PIN_RF_TX_GND"].value);
    PIN_RF_TX_DATA = String2GPIO(aux["PIN_RF_TX_DATA"].value);
}

// Load parameters saved with  saveParams from SPIFFS into the
// elements defined in /settings JSON.
String loadParams(AutoConnectAux &aux, PageArgument &args)
{
    (void)(args);
    //static boolean initConfig = true;

    SPIFFS.begin();
    Serial.print(PARAM_FILE);
    File paramFile = SPIFFS.open(PARAM_FILE, "r");
    if (paramFile)
    {
        if (aux.loadElement(paramFile))
        {
            getParams(aux);
            Serial.println(F(" loaded"));
        }
        else
        {
            Serial.println(F(" failed to load"));
        }
        paramFile.close();
    }
    else
    {
        Serial.println(F(" open+r failed"));
#ifdef ESP32
        Serial.println(F("If you get error as 'SPIFFS: mount failed, -10025', Please modify with 'SPIFFS.begin(true)'."));
#endif // ESP32
    }
    SPIFFS.end();
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

    if (MQTT_PORT == "")
        MQTT_PORT = "1883"; // just in case ....
    if (MQTT_ID == "")
        MQTT_ID = "RFlink-ESP"; // just in case ....
    if (MQTT_TOPIC_IN == "")
        MQTT_TOPIC_IN = "/RFlink/cmd"; // just in case ....
    if (MQTT_TOPIC_OUT == "")
        MQTT_TOPIC_OUT = "/RFLink/msg"; // just in case ....
    if (Adv_HostName == "")
        Adv_HostName = "RFlink-ESP"; // just in case ....

    AutoConnectAux &src_aux = *portal.aux(portal.where());
    getParams(src_aux);
    // AutoConnectInput& mqttserver = my_settings["mqttserver"].as<AutoConnectInput>();  //-> BUG
    // The entered value is owned by AutoConnectAux of /settings.
    // To retrieve the elements of /settings, it is necessary to get
    // the AutoConnectAux object of /settings.
    SPIFFS.begin();
    File my_file = SPIFFS.open(PARAM_FILE, "w");
    Serial.print(PARAM_FILE);
    if (my_file)
    {
        src_aux.saveElement(my_file, {"MQTT_SERVER", "MQTT_PORT",
                                      "MQTT_ID", "MQTT_USER", "MQTT_PSWD",
                                      "MQTT_TOPIC_OUT", "MQTT_TOPIC_IN", "MQTT_RETAINED",
                                      "Adv_HostName", "Adv_Power",
                                      "PIN_RF_RX_PMOS", "PIN_RF_RX_NMOS",
                                      "PIN_RF_RX_VCC", "PIN_RF_RX_GND",
                                      "PIN_RF_RX_NA", "PIN_RF_RX_DATA", "PULLUP_RF_RX_DATA",
                                      "PIN_RF_TX_PMOS", "PIN_RF_TX_NMOS",
                                      "PIN_RF_TX_VCC", "PIN_RF_TX_GND",
                                      "PIN_RF_TX_DATA"});
        Serial.println(F(" saved"));
        my_file.close();
    }
    else
        Serial.print(F(" open+w failed"));
    SPIFFS.end();

    // Echo back saved parameters to AutoConnectAux page.
    AutoConnectText &echo = aux["parameters"].as<AutoConnectText>();
    echo.value = F("<u><b>MQTT settings</b></u>");
    echo.value += F("<br><b>Connexion</b>");
    echo.value += F("<br>Server: ");
    echo.value += MQTT_SERVER;
    echo.value += F("<br>Port: ");
    echo.value += MQTT_PORT;
    echo.value += F("<br>ID: ");
    echo.value += MQTT_ID;
    echo.value += F("<br>Username: ");
    echo.value += MQTT_USER;
    echo.value += F("<br>Password: ");
    echo.value += MQTT_PSWD;
    echo.value += F("<br><br><b>Messages</b>");
    echo.value += F("<br>Out Topic: ");
    echo.value += MQTT_TOPIC_OUT;
    echo.value += F("<br>In Topic: ");
    echo.value += MQTT_TOPIC_IN;
    echo.value += F("<br>Retained: ");
    echo.value += String(MQTT_RETAINED == true ? "true" : "false");
    echo.value += F("<br><u><br><b>Advanced settings</b></u>");
    echo.value += F("<br><b>WiFi</b>");
    echo.value += F("<br>Hostname: ");
    echo.value += Adv_HostName;
    echo.value += F("<br>TX Power: ");
    echo.value += Adv_Power;
    echo.value += F("<br><u><br><b>GPIO settings</b></u>");
    echo.value += F("<br><b>Radio Receiver</b>");
    echo.value += F("<br>RX_PMOS: ");
    echo.value += GPIO2String(PIN_RF_RX_PMOS);
    echo.value += F("<br>RX_NMOS: ");
    echo.value += GPIO2String(PIN_RF_RX_NMOS);
    echo.value += F("<br>RX_VCC: ");
    echo.value += GPIO2String(PIN_RF_RX_VCC);
    echo.value += F("<br>RX_GND: ");
    echo.value += GPIO2String(PIN_RF_RX_GND);
    echo.value += F("<br>RX_NA: ");
    echo.value += GPIO2String(PIN_RF_RX_NA);
    echo.value += F("<br>RX_DATA: ");
    echo.value += GPIO2String(PIN_RF_RX_DATA);
    echo.value += F("<br>Pullup on RX_DATA: ");
    echo.value += String(PULLUP_RF_RX_DATA == true ? "true" : "false");
    echo.value += F("<br><br><b>Radio Emitter</b>");
    echo.value += F("<br>TX_PMOS: ");
    echo.value += GPIO2String(PIN_RF_TX_PMOS);
    echo.value += F("<br>TX_NMOS: ");
    echo.value += GPIO2String(PIN_RF_TX_NMOS);
    echo.value += F("<br>TX_VCC: ");
    echo.value += GPIO2String(PIN_RF_TX_VCC);
    echo.value += F("<br>TX_GND: ");
    echo.value += GPIO2String(PIN_RF_TX_GND);
    echo.value += F("<br>TX_DATA: ");
    echo.value += GPIO2String(PIN_RF_TX_DATA);
    echo.value += F("<br>");

#ifdef MQTT_ENABLED
    setup_MQTT(); // Reload settings
#endif

    return String("");
}

#endif // AUTOCONNECT_ENABLE
