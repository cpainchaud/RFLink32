#include "13_OTA.h"

#include "RFLink.h"

#ifdef ESP32
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#elif defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#define httpUpdate ESPhttpUpdate
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
    String currentHttpUpdateUrl;
    String currentHttpUpdateErrorMsg;

    statusEnum currentHttpUpdateStatus = statusEnum::Idle;

    void sendRequest(){
      if(request.readyState() == 0 || request.readyState() == 4){
        request.open("GET", currentHttpUpdateUrl.c_str());
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
      Serial.println(F("Request has changed state : %i\r\n"));
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
      Serial.printf_P(PSTR("Total read so far %u\r\n"), bytesReceived);
    }

    bool scheduleHttpUpdate(const char *url, String &errmsg) {
      if(currentHttpUpdateStatus == statusEnum::InProgress ||currentHttpUpdateStatus == statusEnum::Scheduled){
        errmsg = F("Another OTA update is already in progress");
        return false;
      }
      if(currentHttpUpdateStatus == statusEnum::PendingReboot){
        errmsg = F("Another OTA update has been applied and requires a reboot");
        return false;
      }

      currentHttpUpdateStatus = statusEnum::Scheduled;
      currentHttpUpdateUrl = url;
      currentHttpUpdateErrorMsg = "";

      Serial.printf_P(PSTR("An HttpUpdate has been scheduled with URL: %s\r\n"), url);

      return true;
    }

    void mainLoop() {
      if(currentHttpUpdateStatus != statusEnum::Scheduled)
        return;

      WiFiClient client;
      WiFiClientSecure clientS;
      clientS.setInsecure();

      currentHttpUpdateStatus = statusEnum::InProgress;

      t_httpUpdate_return ret;
      Serial.print(F("HttpUpdate started ... "));
      httpUpdate.rebootOnUpdate(false);
      httpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
      if(currentHttpUpdateUrl.startsWith("https"))
        ret = httpUpdate.update(clientS, currentHttpUpdateUrl);
      else
        ret = httpUpdate.update(client, currentHttpUpdateUrl);

      switch (ret)
      {
        case HTTP_UPDATE_FAILED:
          currentHttpUpdateErrorMsg =  httpUpdate.getLastError() + httpUpdate.getLastErrorString();
          Serial.printf_P(PSTR("HttpUpdate error: %s\r\n"), currentHttpUpdateErrorMsg.c_str());
          currentHttpUpdateStatus = statusEnum::Failed;
          break;
        case HTTP_UPDATE_NO_UPDATES:
          currentHttpUpdateErrorMsg = F("No update found at provided URL");
          Serial.println(currentHttpUpdateUrl.c_str());
          currentHttpUpdateStatus = statusEnum::Failed;
          break;
        case HTTP_UPDATE_OK:
          currentHttpUpdateStatus = statusEnum::PendingReboot;
          Serial.println(F("HttpUpdate successful! Reboot is scheduled in 10 seconds"));
          RFLink::scheduleReboot(10);
          break;
      }

    }
  } // end of AutoOTA namespace
} // end of RFLink namespace