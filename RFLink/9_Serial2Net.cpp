#include "9_Serial2Net.h"
#include "RFLink.h"

#ifndef RFLINK_SERIAL2NET_DISABLED


#include <WiFiClient.h>
#include <WiFiServer.h>
#include <lwip/sockets.h>

namespace RFLink {
  namespace Serial2Net {

    class Serial2NetClient : public WiFiClient {

    private:
      #ifdef ESP32
      static const uint16_t __buffer_size = 1024;
      #else
      static const uint16_t __buffer_size = 128;
      #endif
      uint16_t buffer_end;

    public:
      bool ignore = true;
      char buffer[__buffer_size + 1];

      Serial2NetClient() : WiFiClient::WiFiClient() {
        buffer[__buffer_size] = 0;
        buffer_end = 0;
      }

      Serial2NetClient &operator=(const WiFiClient &other) {
        WiFiClient::operator=(other);
        ignore = false;
        buffer_end = 0;
        return *this;
      }

      void enabledTcpKeepalive() {
        int keepIdle = 30;
        int keepInterval = 3;
        int keepCount = 3;

        #ifdef ESP8266
        this->keepAlive(keepIdle, keepInterval, keepCount);
        #else
        int keepAlive = 1;
        setSocketOption(SO_KEEPALIVE, (char *) &keepAlive, sizeof(keepAlive));
        setOption(TCP_KEEPIDLE, &keepIdle);
        setOption(TCP_KEEPINTVL, &keepInterval);
        setOption(TCP_KEEPCNT, &keepCount);
        #endif

        println(F("This is RFLink32, welcome!"));
      }

      /**
       * @return -1 if not found or error, index of last character of the commeand if found
       * */
      bool hasCommandAvailable() {
        int newBytesCount = available();

        if (!connected()) { // some errors happened during read operations
          ignore = true;
          return false;
        }

        if (newBytesCount < 1)
          return false;

#if defined(RFLINK_SERIAL2NET_DEBUG) || defined(DEBUG)
        Serial.printf(PSTR("Serial2Net: client has %i more bytes available to read\r\n"), newBytesCount);
#endif

        int readByte = timedRead();

        while (readByte > -1) {
          if (readByte == 0x0A || readByte == 0x0D) { // \r or \n
            if (buffer_end > 0) {
              buffer[buffer_end] = 0;
              return true;
            }
            //Serial.println("it was was first in array so it's ignored!");
          } else {
            buffer[buffer_end] = readByte;
            buffer_end++;
          }
          if (buffer_end >= __buffer_size) {
            buffer_end = 0;
            printf(PSTR("Error: command is too long, max supported length is %u\r\n"), __buffer_size - 1);
            return false;
          }
          readByte = timedRead();
        }

        return false;
      }

      /**
       * Removes the command from buffer and moves next bytes backward in the buffer
       * */
      void consumeCommand() {
        buffer[0] = 0;
        buffer_end = 0;
      }

      void disconnectAndClear() {
        this->stop();
        ignore = true;
        buffer_end = 0;
      }
    };

    namespace params {
      bool enabled = false;
      unsigned int port;
    }

    // All json variable names
    const char json_name_enabled[] = "enabled";
    const char json_name_port[] = "port";

    Config::ConfigItem configItems[] = {
            Config::ConfigItem(json_name_enabled, Config::SectionId::Serial2Net_id, false, paramsUpdatedCallback),
            Config::ConfigItem(json_name_port, Config::SectionId::Serial2Net_id, SERIAL2NET_PORT,
                               paramsUpdatedCallback),
            Config::ConfigItem()};

    WiFiServer server(1900);

    boolean alreadyConnected = false;
    const unsigned short clientsMax = 2;
    Serial2NetClient clients[clientsMax];

    void paramsUpdatedCallback() {
      refreshParametersFromConfig();
    }

    void refreshParametersFromConfig(bool triggerChanges) {
      Config::ConfigItem *item;
      bool changesDetected = false;

      item = Config::findConfigItem(json_name_enabled, Config::SectionId::Serial2Net_id);
      if (item->getBoolValue() != params::enabled) {
        changesDetected = true;
        params::enabled = item->getBoolValue();
      }

      item = Config::findConfigItem(json_name_port, Config::SectionId::Serial2Net_id);
      if (item->getUnsignedLongIntValue() != params::port) {
        changesDetected = true;
        params::port = item->getLongIntValue();
      }

      if (triggerChanges && changesDetected) {
        Serial.println(F("Serial2Net parameters have changed."));
        if (params::enabled)
          restartServer();
        else
          stopServer();
      }
    }

    inline bool isNewClient(WiFiClient &testClient) {
      for (auto &client : clients) {
        if (!client.ignore && client == testClient)
          return false;
      }
      return true;
    }

    /**
     * @return true if we could find and empty spot for this newClient. Client connections will be closed in this case
     *
     * */
    bool registerClient(WiFiClient &newClient) {
      for (auto &client : clients) {
        if (client.ignore && !client.connected()) {
          client = newClient;
          client.enabledTcpKeepalive();
          #if defined(RFLINK_SERIAL2NET_DEBUG) || defined(DEBUG)
          Serial.println(F("Client accepted"));
          #endif
          return true;
        }
      }
      newClient.println(F("Too many clients connected, goodbye!"));
      newClient.stop();
      #if defined(RFLINK_SERIAL2NET_DEBUG) || defined(DEBUG)
      Serial.println("Client rejected due to lack of room");
      #endif
      return false;
    }

    void setup() {
      server.setNoDelay(true);
      refreshParametersFromConfig(false);
    }

    void serverLoop() {
#if defined(RFLINK_SERIAL2NET_DEBUG) || defined(DEBUG)
      String debugmsg;
#endif

      WiFiClient newClient = server.available();

      if (newClient.connected()) {
        if (isNewClient(newClient)) {
#if defined(RFLINK_SERIAL2NET_DEBUG) || defined(DEBUG)
          Serial.printf(PSTR("Serial2Net: new client detected IP=%s port=%i\r\n"), newClient.remoteIP().toString().c_str(), newClient.remotePort());
#endif
          registerClient(newClient);
          return;
        }
      }

      // Let's see if any client has sent some data
      for (auto & client : clients) {
        if (!client.ignore) {
          if (client.hasCommandAvailable()) {
            RFLink::sendRawPrint(F("\33[2K\r"));
            //Serial.flush();
            RFLink::sendRawPrint(F("Message arrived [Ser2Net]:"));
            RFLink::sendRawPrint(client.buffer);
            RFLink::sendRawPrint(F("\r\n"));
            //Serial.flush();
            RFLink::executeCliCommand(client.buffer);
            client.consumeCommand();
          }
        }
      }
    }

    void broadcastMessage(const char *msg) {
      for (auto & client : clients) {
        if (!client.ignore && client.connected()) {
          client.print(msg);
        }
      }
    }

    void broadcastMessage(const __FlashStringHelper *buf) {
      for (auto & client : clients) {
        if (!client.ignore && client.connected()) {
          client.print(buf);
        }
      }
    }

    void broadcastMessage(char c) {
      for (auto & client : clients) {
        if (!client.ignore && client.connected()) {
          client.write(c);
        }
      }
    }


    void restartServer() {
      for (auto & client : clients) {
        if (!client.ignore && client.connected()) {
          client.printf(PSTR("\nSerial2Net will restart on port %u\r\n"), params::port);
        }
      }
      stopServer(false);
      startServer();
    }

    void startServer() {
      server.begin(params::port);
      Serial.println(F("Serial2Net Server started!"));
    }

    void stopServer(bool show_message) {
      for (auto & client : clients) {
        if (!client.ignore && client.connected()) {
          if (show_message) {
            client.printf(PSTR("\nSerial2Net will now stop!\n"));
          }
          client.disconnectAndClear();
        }
      }
      server.stop();
      if (show_message)
        Serial.println(F("Serial2Net Server stopped!"));
    }

    void getStatusJsonString(JsonObject &output) {
      auto &&signal = output.createNestedObject("serial2net");

      unsigned int countClient = 0;

      for (auto & client : clients) {
        if (!client.ignore && client.connected())
          countClient++;
      }
      if (params::enabled)
        signal[F("status")] = F("running");
      else
        signal[F("status")] = F("disabled");

      signal[F("clients_count")] = countClient;
    }

  } // end Serial2Net namespace
} // end of RFLink namespace

#endif // !RFLINK_SERIAL2NET_DISABLED