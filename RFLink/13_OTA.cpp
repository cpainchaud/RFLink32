#include "13_OTA.h"


#ifdef ESP32
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#elif defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#endif

#include <asyncHTTPrequest.h>
#include <Ticker.h>

asyncHTTPrequest request;
Ticker ticker;

namespace RFLink
{
    namespace OTA
    {
        unsigned bytesReceived;
        String currentUrl;

        void sendRequest(){
            if(request.readyState() == 0 || request.readyState() == 4){
                request.open("GET", currentUrl.c_str());
                request.send();
            }
        }

        void requestStateChangedCallback(void* optParm, asyncHTTPrequest* request, int readyState){
            /*
             *     enum    readyStates {
                readyStateUnsent = 0,           // Client created, open not yet called
                readyStateOpened =  1,          // open() has been called, connected
                readyStateHdrsRecvd = 2,        // send() called, response headers available
                readyStateLoading = 3,          // receiving, partial data available
                readyStateDone = 4} _readyState; // Request complete, all data available
             */
            Serial.println("Request has changed state : %i\r\n");
            if(readyState == 4){
                Serial.println(request->responseText());
                Serial.println();
                request->setDebug(false);
            }
        }

        void dataReceived(void* optParm, asyncHTTPrequest *req, size_t byteCount) {
            uint8_t buff[256];

            auto remaining = request.available();
            size_t amountToRead = 0;

            while( remaining > 0) {
                amountToRead = sizeof(buff);

                if( remaining < sizeof(buff) )
                    amountToRead = remaining;

                request.responseRead(buff, sizeof(buff));
                bytesReceived += amountToRead;
                remaining = request.available();
            }
            Serial.printf("Total read so far %u\r\n", bytesReceived);
        }

        void downloadFromUrl(const char *url)
        {
            HTTPClient http;
            HTTPClient httpRelocated;
            HTTPClient *currentHttpClient = &http;// fix for HTTPClient issue in Espressif framework https://github.com/espressif/arduino-esp32/issues/4931

            WiFiClient client;

            const char * headerKeys[] = {"Location"};
            const size_t numberOfHeaders = 1;

            http.setFollowRedirects(followRedirects_t::HTTPC_STRICT_FOLLOW_REDIRECTS);
            http.begin(url);
            //http.begin("https://github-releases.githubusercontent.com/330986901/fc934000-81e3-11eb-9549-66fb8ee40ece?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAIWNJYAX4CSVEH53A/20210311/us-east-1/s3/aws4_request&X-Amz-Date=20210311T084152Z&X-Amz-Expires=300&X-Amz-Signature=463e4c91369342bc1055435ad505ac464c272424d83db1f61807ad07461cc341&X-Amz-SignedHeaders=host&actor_id=6696638&key_id=0&repo_id=330986901&response-content-disposition=attachment; filename=esp32-firmware.bin&response-content-type=application/octet-stream");
            http.collectHeaders(headerKeys, numberOfHeaders);
            int httpCode = http.GET();

            currentUrl = url;

            if (httpCode == 302)
            {
                currentUrl = http.header("Location");
                http.end();
                delay(1);
                Serial.printf("Web server has replied with a code 302, we will follow link: %s\r\n", currentUrl.c_str());
                httpRelocated.begin(currentUrl);
                httpCode = httpRelocated.GET();
                currentHttpClient = &httpRelocated;
                httpRelocated.end();
                delay(1);
            }

            if (httpCode != HTTP_CODE_OK)
            {
                Serial.printf("HTTP request returned status code %i\r\n", httpCode);
                return;
            }

            Serial.printf("HTTP request returned status code %i\r\n", httpCode);

            /*
            bytesReceived = 0;
            request.setDebug(true);
            request.onReadyStateChange(requestStateChangedCallback);
            request.onData(dataReceived);
            ticker.attach(0.5, sendRequest);*/


            /*
            FirmWareDispo = http.header((size_t)0);
            http.end();

            // Check Date of UpDate
            Serial.println("FOTA : Firware available = " + FirmWareDispo);
            if (CurrentFirmware == "" || CurrentFirmware == FirmWareDispo)
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
            switch (ret)
            {
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
                */
        }
    } // end of AutoOTA namespace
} // end of RFLink namespace