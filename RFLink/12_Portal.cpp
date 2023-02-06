#include "RFLink.h"

#ifndef RFLINK_PORTAL_DISABLED

#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <index.html.gz.h>
#include <LittleFS.h>

#include "2_Signal.h"
#include "6_MQTT.h"
#include "9_Serial2Net.h"
#include "11_Config.h"
#include "12_Portal.h"
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
#include <LittleFS.h>

namespace RFLink { namespace Portal {

        const char json_name_enabled[] = "enabled";
        const char json_name_auth_enabled[] = "auth_enabled";
        const char json_name_auth_user[] = "auth_user";
        const char json_name_auth_password[] = "auth_password";

        Config::ConfigItem configItems[] =  {
                Config::ConfigItem(json_name_enabled,      Config::SectionId::Portal_id, true, paramsUpdatedCallback),
                Config::ConfigItem(json_name_auth_enabled, Config::SectionId::Portal_id, true, paramsUpdatedCallback),
                Config::ConfigItem(json_name_auth_user,    Config::SectionId::Portal_id, RFLINK_WEBUI_DEFAULT_USER, paramsUpdatedCallback),
                Config::ConfigItem(json_name_auth_password,Config::SectionId::Portal_id, RFLINK_WEBUI_DEFAULT_PASSWORD, paramsUpdatedCallback),
                Config::ConfigItem(), // dont remove it!
        };

        namespace params {
          bool enabled = true;
          bool auth_enabled = false;
          String auth_user;
          String auth_password;
        }

        AsyncWebServer server(80);

        bool checkHttpAuthentication(AsyncWebServerRequest *request) {
          if(!params::auth_enabled)
            return true;

          if(!request->authenticate(params::auth_user.c_str(), params::auth_password.c_str())) {
            request->requestAuthentication();
            return false;
          }
          return true;
        }

        void notFound(AsyncWebServerRequest *request) {
          request->send(404, F("text/plain"), F("Not found"));
        }

        void serverApiConfigGet(AsyncWebServerRequest *request) {
          if(!checkHttpAuthentication(request))
            return;

          String dump;
          Config::dumpConfigToString(dump);
          request->send(200, F("application/json"), dump);
        }

        void serveApiStatusGet(AsyncWebServerRequest *request) {
          if(!checkHttpAuthentication(request))
            return;

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
          if(!checkHttpAuthentication(request))
            return;

          request->send(200, F("text/plain"), F("Rebooting in 5 seconds"));
          RFLink::scheduleReboot(5);
        }

        void serveApiFirmwareHttpUpdateGetStatus(AsyncWebServerRequest *request){
          if(!checkHttpAuthentication(request))
            return;

          DynamicJsonDocument output(500);
          JsonObject && root = output.to<JsonObject>();
          OTA::getHttpUpdateStatus(root);
          String buffer;
          buffer.reserve(256);
          serializeJson(output, buffer);
          request->send(200, "application/json", buffer);
        }

        #ifndef FIRMWARE_UPGRADE_VIA_WEBSERVER_DISABLED
        void serveApiFirmwareUpdateFromUrl(AsyncWebServerRequest *request, JsonVariant &json)
        {
          if(!checkHttpAuthentication(request))
            return;

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
        #endif // FIRMWARE_UPGRADE_VIA_WEBSERVER_DISABLED

        void serverApiConfigPush(AsyncWebServerRequest *request, JsonVariant &json) {
          if(!checkHttpAuthentication(request))
            return;

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

        #ifndef FIRMWARE_UPGRADE_VIA_WEBSERVER_DISABLED
        // Taken from of https://github.com/ayushsharma82/AsyncElegantOTA
        void handleFirmwareUpdateFinalResponse(AsyncWebServerRequest *request) {
          if(!checkHttpAuthentication(request))
            return;

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
        void handleFirmwareUpdateChunksReception(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
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
        #endif // FIRMWARE_UPGRADE_VIA_WEBSERVER_DISABLED


        void handleFileUploadFinalResponse(AsyncWebServerRequest *request) {
          if(!checkHttpAuthentication(request))
            return;

          //Serial.println(F("File upload via API request... done"));

          AsyncWebServerResponse *response = request->beginResponse(
                  200, "text/plain",
                  "OK"
          );
          response->addHeader(F("Connection"), F("close"));
          response->addHeader(F("Access-Control-Allow-Origin"), "*");
          request->send(response);
        }

        void handleFileUploadChunksReception(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        //Upload handler chunks in data

          if(!checkHttpAuthentication(request))
            return;

          if (!index) {
            Serial.print(F("File upload via API requested... "));
            if(!request->hasParam(F("filename"), false)) {
              Serial.println(F("missing 'filename' parameter"));
              return request->send(400, F("text/plain"), F("missing 'filename' parameter"));
            }
            String desiredFilename = request->getParam(F("filename"), false)->value();
            Serial.println(desiredFilename);
            #ifdef ESP32
            auto file = request->_tempFile = LittleFS.open(String('/' + desiredFilename), "w", true);
            #else
            auto file = request->_tempFile = LittleFS.open(String('/' + desiredFilename), "w");
            #endif

            if(!file) {
              //Serial.println(F("failed to open file for writing"));
              return request->send(500, F("text/plain"), F("failed to open file for writing"));
            }
          }

          if (len) {
            // stream the incoming chunk to the opened file
            auto written_bytes_count = request->_tempFile.write(data, len);
            if(written_bytes_count != len) {
              request->_tempFile.close();
              Serial.println(F("failed to write chunk to file"));
              return request->send(500, F("text/plain"), F("failed to write chunk to file"));
            }
          }
          if (final) {

            //Serial.println(F("File Uploaded !"));
            // close the file handle as the upload is now done

            #ifndef RFLINK_MQTT_DISABLED
            String desiredFilename('/' + request->getParam(F("filename"), false)->value());
            if(desiredFilename == Mqtt::mqtt_ca_cert_filename) {
              RFLink::Mqtt::triggerParamsHaveChanged();
              /*if(LittleFS.exists(Mqtt::mqtt_ca_cert_filename)) {
                Serial.println(F("MQTT CA cert file exists, MQTT will be reconfigured on next loop"));
              } else {
                Serial.println(F("MQTT CA cert file does not exist, MQTT will not be reconfigured on next loop"));
              }
              Serial.print(F("written filename:"));
              Serial.println(request->_tempFile.fullName());*/
            }
            #endif

            request->_tempFile.close();
            request->send(200, F("text/plain"), F("File Uploaded !"));
          }
        }

        void serveIndexHtml(AsyncWebServerRequest *request) {
          if(!checkHttpAuthentication(request))
            return;
          AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/html"), index_html_gz_start, index_html_gz_size);
          response->addHeader(F("Content-Encoding"), F("gzip"));
          request->send(response);
        }


        void init() {

          refreshParametersFromConfig(false);

          server.onNotFound(notFound);
          server.on(PSTR("/"), HTTP_GET, serveIndexHtml);
          server.on(PSTR("/index.html"), HTTP_GET, serveIndexHtml);
          server.on(PSTR("/wifi"), HTTP_GET, serveIndexHtml);
          server.on(PSTR("/home"), HTTP_GET, serveIndexHtml);
          server.on(PSTR("/radio"), HTTP_GET, serveIndexHtml);
          server.on(PSTR("/signal"), HTTP_GET, serveIndexHtml);
          #ifndef FIRMWARE_UPGRADE_VIA_WEBSERVER_DISABLED
          server.on(PSTR("/firmware"), HTTP_GET, serveIndexHtml);
          #endif
          server.on(PSTR("/services"), HTTP_GET, serveIndexHtml);

          server.on(PSTR("/api/config"), HTTP_GET, serverApiConfigGet);
          server.on(PSTR("/api/status"), HTTP_GET, serveApiStatusGet);

          server.on(PSTR("/api/reboot"), HTTP_GET, serveApiReboot);
          #ifndef FIRMWARE_UPGRADE_VIA_WEBSERVER_DISABLED
          server.on(PSTR("/api/firmware/update"), HTTP_POST, handleFirmwareUpdateFinalResponse,
                    handleFirmwareUpdateChunksReception);
          #endif
          server.on(PSTR("/upload"), HTTP_POST, handleFileUploadFinalResponse,
                    handleFileUploadChunksReception);

          server.on(PSTR("/api/firmware/http_update_status"), HTTP_GET, serveApiFirmwareHttpUpdateGetStatus);

          AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler(PSTR("/api/config"), serverApiConfigPush, 4000);
          server.addHandler(handler);

          #ifndef FIRMWARE_UPGRADE_VIA_WEBSERVER_DISABLED
          handler = new AsyncCallbackJsonWebHandler(PSTR("/api/firmware/update_from_url"), serveApiFirmwareUpdateFromUrl, 1000);
          server.addHandler(handler);
          #endif

        }

        void start() {
          if(params::enabled) {
            Serial.print(F("Starting WebServer... "));
            server.begin();
            Serial.println(F("OK"));
          }
        }

        void stop() {
          Serial.print(F("Stopping WebServer... "));
          server.end();
          Serial.println(F("OK"));
        }

      void paramsUpdatedCallback() {
        refreshParametersFromConfig();
      }

      void refreshParametersFromConfig(bool triggerChanges) {
        Config::ConfigItem *item;
        bool changesDetected = false;

        item = Config::findConfigItem(json_name_enabled, Config::SectionId::Portal_id);
        if (item->getBoolValue() != params::enabled) {
          changesDetected = true;
          params::enabled = item->getBoolValue();
        }

        item = Config::findConfigItem(json_name_auth_enabled, Config::SectionId::Portal_id);
        if (item->getBoolValue() != params::auth_enabled) {
          //changesDetected = true; // no need to trigger an update cycle
          params::auth_enabled = item->getBoolValue();
        }

        item = Config::findConfigItem(json_name_auth_user, Config::SectionId::Portal_id);
        if( params::auth_user != item->getCharValue() ) {
          //changesDetected = true; // no need to trigger an update cycle
          params::auth_user = item->getCharValue();
        }

        item = Config::findConfigItem(json_name_auth_password, Config::SectionId::Portal_id);
        if( params::auth_password != item->getCharValue() ) {
          //changesDetected = true; // no need to trigger an update cycle
          params::auth_password = item->getCharValue();
        }

        if (triggerChanges && changesDetected) {
          if(params::enabled)
            server.begin();
          else
            server.end();
        }
      }

    } // end of Portal namespace
} // end of RFLink namespace

#endif // RFLINK_PORTAL_DISABLED