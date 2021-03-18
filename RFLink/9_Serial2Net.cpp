#include "9_Serial2Net.h"
#include "RFLink.h"

#ifndef RFLINK_SERIAL2NET_DISABLED


#include <WiFiClient.h>
#include <WiFiServer.h>
#include <lwip/sockets.h>

namespace RFLink { namespace Serial2Net {

        class Serial2NetClient : public WiFiClient {

        private:
            static const uint16_t __buffer_size = 64;
            char buffer[__buffer_size+1];
            uint16_t buffer_end = 0;

        public:
            bool ignore = true;

            Serial2NetClient(): WiFiClient::WiFiClient(){
                buffer[__buffer_size] = 0;
            }

            Serial2NetClient & operator=(const WiFiClient &other)  {
                WiFiClient::operator=(other);
                ignore = false;
                buffer_end = 0;
                return *this;
            }

            void enabledTcpKeepalive() {
                int keepAlive = 1; // used only with ESP32
                int keepIdle = 30;
                int keepInterval = 3;
                int keepCount = 3;

#ifdef ESP8266
                this->keepAlive(keepIdle, keepInterval, keepCount);
#else
                setSocketOption(SO_KEEPALIVE, (char*) &keepAlive, sizeof (keepAlive));
                setOption(TCP_KEEPIDLE, &keepIdle);
                setOption(TCP_KEEPINTVL, &keepInterval);
                setOption(TCP_KEEPCNT, &keepCount);
                println(F("This is RFLink-ESP, welcome!"));
#endif
            }

            /**
             * @return -1 if not found or error, index of last character of the commeand if found
             * */
            int hasCommandAvailable() {
#if defined(RFLINK_SERIAL2NET_DEBUG) || defined(DEBUG)
                String debugmsg;
#endif

                int newBytesCount = available();

                if( !connected() ) { // some errors happened during read operations
                    ignore = true;
                    return -1;
                }

                if(newBytesCount < 1)
                    return -1;

#if defined(RFLINK_SERIAL2NET_DEBUG) || defined(DEBUG)
                debugmsg += "Serial2Net: client has " + String(newBytesCount) + " more bytes to available to read";
                    Serial.println(debugmsg);
#endif

                uint16_t spaceLeft = __buffer_size - buffer_end;

                if(newBytesCount > spaceLeft)
                    newBytesCount = spaceLeft;

                readBytes(buffer + buffer_end, newBytesCount);

                int result = -1;

                for(int i=buffer_end; i < buffer_end + newBytesCount; i++) {
                    if(buffer[i] == 0x0d ||buffer[i] == 0x0a) {  // '\r' or "\n"
                        result = i-1;
#if defined(RFLINK_SERIAL2NET_DEBUG) || defined(DEBUG)
                        debugmsg += "Serial2Net: end of command found at position " + String(result) ;
                        Serial.println(debugmsg);
#endif
                        break;
                    }
                }
                buffer_end += newBytesCount;

                if( result < 0 && buffer_end >= __buffer_size - 1 ){
                    println(F("Command is too long, we're closing this connection!"));
                    ignore = true;
                    stop();
                    return -1;
                }

                return result;
            }

            /**
             * Removes the command of which end at given position from the buffer (does not include new line character)
             * */
            String consumeCommand(uint16_t position){
                buffer[position+1] = 0;
                String result(buffer);

                if (buffer[position+2] == 0x0A) { // "\n"
                    memcpy(buffer, buffer+position+3, buffer_end - position - 2);
                    buffer_end = buffer_end - position - 3;
                }
                else{
                    memcpy(buffer, buffer+position+2, buffer_end - position - 1);
                    buffer_end = buffer_end - position - 2;
                }
                return result;
            }

            void disconnectAndClear(){
                //close(fd());
                this->stop();
                ignore = true;
                buffer_end = 0;
            }
        };

        namespace params
        {
            bool enabled = false;
            unsigned int port;
        }

        // All json variable names
        const char json_name_enabled[] = "enabled";
        const char json_name_port[] = "port";

        Config::ConfigItem configItems[] = {
                Config::ConfigItem(json_name_enabled, Config::SectionId::Serial2Net_id, false, paramsUpdatedCallback),
                Config::ConfigItem(json_name_port, Config::SectionId::Serial2Net_id,SERIAL2NET_PORT, paramsUpdatedCallback),
                Config::ConfigItem()};

        WiFiServer server(1900);

        boolean alreadyConnected = false;
        const unsigned short clientsMax=5;
        Serial2NetClient clients[clientsMax];

        void paramsUpdatedCallback()
        {
            refreshParametersFromConfig();
        }

        void refreshParametersFromConfig(bool triggerChanges)
        {
            Config::ConfigItem *item;
            bool changesDetected = false;

            item = Config::findConfigItem(json_name_enabled, Config::SectionId::Serial2Net_id);
            if (item->getBoolValue() != params::enabled)
            {
                changesDetected = true;
                params::enabled = item->getBoolValue();
            }

            item = Config::findConfigItem(json_name_port, Config::SectionId::Serial2Net_id);
            if (item->getLongIntValue() != params::port)
            {
                changesDetected = true;
                params::port = item->getLongIntValue();
            }

            if (triggerChanges && changesDetected)
            {
                Serial.println(F("Serial2Net parameters have changed."));
                if(params::enabled)
                    restartServer();
                else
                    stopServer();
            }
        }

        inline bool isNewClient(WiFiClient &testClient) {
            for(auto & client : clients) {
                if(!client.ignore && client == testClient)
                    return false;
            }
            return true;
        }

        /**
         * @return true if we could find and empty spot for this newClient. Client connections will be closed in this case
         *
         * */
        bool registerClient(WiFiClient &newClient) {
            for(auto & client : clients) {
                if(client.ignore && !client.connected()) {
                    client = newClient;
                    client.enabledTcpKeepalive();
                    return true;
                }
            }
            newClient.println(F("Too many clients connected, goodbye!"));
            return false;
        }

        void setup(){
            server.setNoDelay(true);
            refreshParametersFromConfig(false);
        }

        void serverLoop(){
#if defined(RFLINK_SERIAL2NET_DEBUG) || defined(DEBUG)
            String debugmsg;
#endif

            WiFiClient client = server.available();

            if(client.connected()) {
                if( isNewClient(client) ) {
#if defined(RFLINK_SERIAL2NET_DEBUG) || defined(DEBUG)
                    debugmsg += "Serial2Net: new client detected IP=" + client.remoteIP().toString() + " port=" + client.remotePort();
                    Serial.println(debugmsg);
#endif
                    Serial.println("registering new client");
                    registerClient(client);
                    return;
                }
            }

            // Let's see if any client has sent some data
            for(int i=0; i<clientsMax; i++) {
                if(!clients[i].ignore) {
                    int commandLastCharPos = clients[i].hasCommandAvailable();
                    if (commandLastCharPos >=0) {
                        String command = clients[i].consumeCommand(commandLastCharPos);
#if defined(RFLINK_SERIAL2NET_DEBUG) || defined(DEBUG)
                        debugmsg += "Serial2Net: client has sent a command >>>> " + command + " <<<<<<";
                    Serial.println(debugmsg);
#endif

                        if(command.length() > 0) { // Let's request RFLink to parse this command
                            RFLink::sendRawPrint(F("\33[2K\r"));
                            Serial.flush();
                            RFLink::sendRawPrint(PSTR("Message arrived [Ser2Net]:"));
                            RFLink::sendRawPrint(command.c_str());
                            RFLink::sendRawPrint(PSTR("\r\n"));
                            Serial.flush();
                            RFLink::executeCliCommand((char*)command.c_str());
                        }
                    }
                }
            }
        }

        void broadcastMessage(const char *msg) {
            for(int i=0; i<clientsMax; i++) {
                if(!clients[i].ignore && clients[i].connected()) {
                    clients[i].print(msg);
                }
            }
        }

        void broadcastMessage(char c) {
          for(int i=0; i<clientsMax; i++) {
            if(!clients[i].ignore && clients[i].connected()) {
              clients[i].write(c);
            }
          }
        }

        void restartServer() {
            for(int i=0; i<clientsMax; i++) {
                if(!clients[i].ignore && clients[i].connected()) {
                    clients[i].printf(PSTR("\nSerial2Net will restart on port %ui\n"), params::port);
                    clients[i].flush();
                }
            }
            stopServer(false);
            startServer();
        }
        void startServer(){
            server.begin(params::port);
            Serial.println(F("Serial2Net Server started!"));
        }

        void stopServer(bool show_message){
            for(int i=0; i<clientsMax; i++) {
                if(!clients[i].ignore && clients[i].connected()) {
                    if(show_message) {
                        clients[i].printf(PSTR("\nSerial2Net will now stop!\n"));
                        clients[i].flush();
                    }
                    clients[i].disconnectAndClear();
                }
            }
            server.stop();
            if(show_message)
                Serial.println(F("Serial2Net Server stopped!"));
        }

        void getStatusJsonString(JsonObject &output)
        {
            auto &&signal = output.createNestedObject("serial2net");

            unsigned int countClient =0;

            for(int i=0; i<clientsMax; i++) {
                if(!clients[i].ignore && clients[i].connected())
                    countClient++;
            }
            if(params::enabled)
                signal[F("status")] = F("running");
            else
                signal[F("status")] = F("disabled");

            signal[F("clients_count")] = countClient;
        }

    } // end Serial2Net namespace
} // end of RFLink namespace

#endif // !RFLINK_SERIAL2NET_DISABLED