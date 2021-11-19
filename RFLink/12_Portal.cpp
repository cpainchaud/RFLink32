#include "RFLink.h"

#ifndef RFLINK_PORTAL_DISABLED

#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <index.html.gz.h>

#include "2_Signal.h"
#include "6_MQTT.h"
#include "9_Serial2Net.h"
#include "11_Config.h"
#include "10_Wifi.h"
#include "13_OTA.h"

#if defined(ESP8266)
#include "ESP8266WiFi.h"
#include "ESPAsyncTCP.h"
#include "flash_hal.h"
#include "FS.h"
#elif defined(ESP32)
#include "AsyncTCP.h"
#include "Update.h"
#endif

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
          request->send(404, F("text/plain"), F("Not found"));
        }

        void serverApiConfigGet(AsyncWebServerRequest *request) {
          String dump;
          Config::dumpConfigToString(dump);
          request->send(200, F("application/json"), dump);
        }

        void serveApiStatusGet(AsyncWebServerRequest *request) {
          DynamicJsonDocument output(3000);

          auto && obj = output.to<JsonObject>();

          RFLink::getStatusJsonString(obj);

          RFLink::Wifi::getStatusJsonString(obj);
          #ifndef RFLINK_MQTT_DISABLED
          RFLink::Mqtt::getStatusJsonString(obj);
          #endif // RFLINK_MQTT_DISABLED
          RFLink::Signal::getStatusJsonString(obj);
          RFLink::Serial2Net::getStatusJsonString(obj);

          String buffer;
          buffer.reserve(512);
          serializeJson(output, buffer);

          request->send(200, "application/json", buffer);
        }

        void serveApiReboot(AsyncWebServerRequest *request) {
          request->send(200, F("text/plain"), F("Rebooting in 5 seconds"));
          RFLink::scheduleReboot(5);
        }

        void serveApiFirmwareHttpUpdateGetStatus(AsyncWebServerRequest *request){
          DynamicJsonDocument output(500);
          JsonObject && root = output.to<JsonObject>();
          OTA::getHttpUpdateStatus(root);
          String buffer;
          buffer.reserve(256);
          serializeJson(output, buffer);
          request->send(200, "application/json", buffer);
        }

        void serveApiFirmwareUpdateFromUrl(AsyncWebServerRequest *request, JsonVariant &json)
        {
          if (not json.is<JsonObject>()) {
            request->send(400, F("text/plain"), F("Not an object"));
            return;
          }

          JsonVariant url = json["url"];

          if(url.isUndefined()) {
            request->send(400, F("text/plain"), F("malformed request data"));
            return;
          }

          if(!url.is<const char *>()) {
            request->send(400, F("text/plain"), F("malformed request data"));
            return;
          }

          const char *url_str = url.as<const char *>();
          int url_length = strlen(url_str);

          if(url_length < 7)  {
            request->send(400, F("text/plain"), F("malformed url provided"));
            return;
          }

          String errmsg;
          errmsg.reserve(256);

          if(!RFLink::OTA::scheduleHttpUpdate(url, errmsg)) {
            request->send(400, F("text/plain"), errmsg.c_str());
            return;
          }

          request->send(200, F("text/plain"), F("HTTP OTA scheduled"));
        }

        void serverApiConfigPush(AsyncWebServerRequest *request, JsonVariant &json) {
          if (not json.is<JsonObject>()) {
            Serial.println(F("API Config push requested but invalid JSON was received!"));
            request->send(400, F("text/plain"), F("Not an object"));
            return;
          }

          JsonObject && data = json.as<JsonObject>();

          String message;
          message.reserve(256); // reserve 256 to avoid fragmentation

          String response;
          response.reserve(256);

          if( !Config::pushNewConfiguration(data, message, true) ) {
            response = F("{ \"success\": false, \"message\": ");
          }
          else {
            response = F("{ \"success\": true, \"message\": ");
          }

          if( message.length() > 0 ) {
            response += '"';
            response += message + "\"}";
          } else {
            response += F(" null }");
          }

          request->send(200, F("application/json"), response);
        }


// Taken from of https://github.com/ayushsharma82/AsyncElegantOTA
        void handleFirmwareUpdateFinalResponse(AsyncWebServerRequest *request) {
          // the request handler is triggered after the upload has finished...
          // create the response, add header, and send response
          AsyncWebServerResponse *response = request->beginResponse(
                  (Update.hasError()) ? 500 : 200, "text/plain",
                  (Update.hasError()) ? "FAIL" : "OK"
          );
          response->addHeader(F("Connection"), F("close"));
          response->addHeader(F("Access-Control-Allow-Origin"), "*");
          request->send(response);
          //restartRequired = true;
        }

// Taken from of https://github.com/ayushsharma82/AsyncElegantOTA
        void handleChunksReception(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
          //Upload handler chunks in data

          if (!index) {
            Serial.println(F("OTA via Portal Requested"));

            //if(!request->hasParam("MD5", true)) {
            //    return request->send(400, "text/plain", "MD5 parameter missing");
            //}

            //if(!Update.setMD5(request->getParam("MD5", true)->value().c_str())) {
            //    return request->send(400, "text/plain", "MD5 parameter invalid");
            //}

#if defined(ESP8266)
            int cmd = (filename == "filesystem") ? U_FS : U_FLASH;
            Update.runAsync(true);
            size_t fsSize = ((size_t) &_FS_end - (size_t) &_FS_start);
            uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
            if (!Update.begin((cmd == U_FS)?fsSize:maxSketchSpace, cmd)){ // Start with max available size
#elif defined(ESP32)
              int cmd = (filename == "filesystem") ? U_SPIFFS : U_FLASH;
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) { // Start with max available size
#endif
              Update.printError(Serial);
              return request->send(400, F("text/plain"), F("OTA could not begin"));
            }
          }

          // Write chunked data to the free sketch space
          if(len){
            if (Update.write(data, len) != len) {
              return request->send(400, F("text/plain"), F("OTA could not begin"));
            }
          }

          if (final) { // if the final flag is set then this is the last frame of data
            if (!Update.end(true)) { //true to set the size to the current progress
              Update.printError(Serial);
              return request->send(400, F("text/plain"), F("Could not end OTA"));
            }
            Serial.println(F("OTA via Portal is a success. You are one reboot away your new shiny release! See you in 5 seconds..."));
            RFLink::scheduleReboot(5);
          }else{

          }
          return;
        }

        void serveIndexHtml(AsyncWebServerRequest *request) {

          AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/html"), index_html_gz_start, index_html_gz_size);
          response->addHeader(F("Content-Encoding"), F("gzip"));
          request->send(response);
        }


        void init() {
          server.onNotFound(notFound);

          server.on(PSTR("/"), HTTP_GET, serveIndexHtml);
          server.on(PSTR("/index.html"), HTTP_GET, serveIndexHtml);
          server.on(PSTR("/wifi"), HTTP_GET, serveIndexHtml);
          server.on(PSTR("/home"), HTTP_GET, serveIndexHtml);
          server.on(PSTR("/radio"), HTTP_GET, serveIndexHtml);
          server.on(PSTR("/signal"), HTTP_GET, serveIndexHtml);
          server.on(PSTR("/firmware"), HTTP_GET, serveIndexHtml);
          server.on(PSTR("/services"), HTTP_GET, serveIndexHtml);

          server.on(PSTR("/api/config"), HTTP_GET, serverApiConfigGet);
          server.on(PSTR("/api/status"), HTTP_GET, serveApiStatusGet);

          server.on(PSTR("/api/reboot"), HTTP_GET, serveApiReboot);

          server.on(PSTR("/api/firmware/update"), HTTP_POST, handleFirmwareUpdateFinalResponse, handleChunksReception);
          server.on(PSTR("/api/firmware/http_update_status"), HTTP_GET, serveApiFirmwareHttpUpdateGetStatus);

          AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler(PSTR("/api/config"), serverApiConfigPush, 4000);
          server.addHandler(handler);

          handler = new AsyncCallbackJsonWebHandler(PSTR("/api/firmware/update_from_url"), serveApiFirmwareUpdateFromUrl, 1000);
          server.addHandler(handler);

        }

        void start() {
          Serial.print(F("Starting WebServer... "));
          server.begin();
          Serial.println(F("OK"));
        }


    } // end of Portal namespace
} // end of RFLink namespace

#endif // RFLINK_PORTAL_DISABLED