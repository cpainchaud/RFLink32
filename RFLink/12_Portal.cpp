#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include "index.html.gz.h"

#include "RFLink.h"
#include "11_Config.h"
#include "10_Wifi.h"

namespace RFLink { namespace Portal {

const char json_name_enabled[] = "enabled";
const char json_name_auth_enabled[] = "auth_enabled";
const char json_name_auth_user[] = "auth_user";
const char json_name_auth_password[] = "auth_password";
    
Config::ConfigItem configItems[] =  {
    Config::ConfigItem(json_name_enabled,      Config::SectionId::Portal_id, true, nullptr),
    Config::ConfigItem(json_name_auth_enabled, Config::SectionId::Portal_id, false, nullptr),
    Config::ConfigItem(json_name_auth_user,    Config::SectionId::Portal_id, "", nullptr),
    Config::ConfigItem(json_name_auth_password,Config::SectionId::Portal_id, "", nullptr),
    Config::ConfigItem(), // dont remove it!
};

AsyncWebServer server(80);

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void serverApiConfigGet(AsyncWebServerRequest *request) {
    String dump;
    Config::dumpConfigToString(dump);
    request->send(200, "application/json", dump);
}

void serverApiStatusGet(AsyncWebServerRequest *request) {
    DynamicJsonDocument output(10000);

    auto && obj = output.to<JsonObject>();

    RFLink::getStatusJsonString(obj);
    

    RFLink::Wifi::getStatusJsonString(obj);

    String buffer;
    serializeJson(output, buffer);

    request->send(200, "application/json", buffer);
}

void serverApiConfigPush(AsyncWebServerRequest *request, JsonVariant &json) {
    if (not json.is<JsonObject>()) {
        request->send(400, "text/plain", "Not an object");
        return;
    }

    JsonObject && data = json.as<JsonObject>();

    String message;
    message.reserve(256); // reserve 256 to avoid fragmentation

    String response;
    response.reserve(256);

    if( !Config::pushNewConfiguration(data, message) ) {
        response = "{ \"success\": False, \"message\": ";
    }
    else {
        response = "{ \"success\": True, \"message\": ";
    }

    if( message.length() > 0 ) {
        response += message;
    } else {
        response += " null";
    }

    response += " \"}";
    request->send(200, "application/json", response);
}

void serveIndexHtml(AsyncWebServerRequest *request) {

    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gz_start, index_html_gz_size);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
}


void init() {
    server.onNotFound(notFound);

    server.on("/", HTTP_GET, serveIndexHtml);
    server.on("/index.html", HTTP_GET, serveIndexHtml);
    server.on("/wifi", HTTP_GET, serveIndexHtml);
    server.on("/home", HTTP_GET, serveIndexHtml);
    server.on("/radio", HTTP_GET, serveIndexHtml);
    server.on("/signal", HTTP_GET, serveIndexHtml);
    server.on("/firmware", HTTP_GET, serveIndexHtml);

    server.on("/api/config", HTTP_GET, serverApiConfigGet);

    server.on("/api/status", HTTP_GET, serverApiStatusGet);

    AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/api/config", serverApiConfigPush, 10000);
    server.addHandler(handler);

}

void start() {
    Serial.print("Starting WebServer... ");
    server.begin();
    Serial.println("OK");
}


}
}