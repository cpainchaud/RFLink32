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
            uint16_t end = 0;

        public:
            bool ignore = true;

            Serial2NetClient(): WiFiClient::WiFiClient(){
                buffer[__buffer_size] = 0;
            }

            Serial2NetClient & operator=(const WiFiClient &other)  {
                WiFiClient::operator=(other);
                ignore = false;
                end = 0;
                return *this;
            }

            void enabledTcpKeepalive() {
                int keepAlive = 1;
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
                println("This is RFLink-ESP, welcome!");
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

                uint16_t spaceLeft = __buffer_size - end;

                if(newBytesCount > spaceLeft)
                    newBytesCount = spaceLeft;

                readBytes(buffer + end, newBytesCount);

                int result = -1;

                for(int i=end; i< end+newBytesCount; i++) {
                    if(buffer[i] == 0x0d) {
                        result = i-1;
                        #if defined(RFLINK_SERIAL2NET_DEBUG) || defined(DEBUG)
                            debugmsg += "Serial2Net: end of command found at position " + String(result) ;
                            Serial.println(debugmsg);
                        #endif
                    }
                }
                end += newBytesCount;

                if( result < 0 && end >= __buffer_size-1 ){
                    println("Command is too long, we're closing this connection!");
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

                if (buffer[position+2] == 0x0A) {
                    memcpy(buffer, buffer+position+3, end-position-2);
                    end = end - position -3;
                }
                else{
                    memcpy(buffer, buffer+position+2, end-position-1);
                    end = end - position -2;
                }
                return result;
            }
    };
    
    WiFiServer server(SERIAL2NET_PORT);

    boolean alreadyConnected = false;
    const unsigned short clientsMax=10;
    Serial2NetClient clients[clientsMax];

    inline bool isNewClient(WiFiClient &client) {
        for(int i=0; i<clientsMax; i++) {
            if(clients[i] == client)
                return false;
        }
        return true;
    }

    /**
     * @return true if we could find and empty spot for this client. Client connections will be closed in this case
     *
     * */
    bool registerClient(WiFiClient &client) {
        for(int i=0; i<clientsMax; i++) {
            if(!clients[i].connected()) {
                clients[i] = client;
                clients[i].enabledTcpKeepalive();
                return true;
            }
        }
        client.println("Too many clients connected, goodbye!");
        return false;
    }

    void startServer(){
        server.setNoDelay(true);
        server.begin(SERIAL2NET_PORT);
    }

    void serverLoop(){
        #if defined(RFLINK_SERIAL2NET_DEBUG) || defined(DEBUG)
        String debugmsg;
        #endif

        WiFiClient client = server.available();

        if( isNewClient(client) ) {
            #if defined(RFLINK_SERIAL2NET_DEBUG) || defined(DEBUG)
            debugmsg += "Serial2Net: new client detected IP=" + client.remoteIP().toString() + " port=" + client.remotePort();
            Serial.println(debugmsg);
            #endif
            registerClient(client);
            return;
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
                        RFLink::executeCliCommand(command.c_str());
                    }
                }
            }
        }
    }

    void broadcastMessage(const char *msg) {
        for(int i=0; i<clientsMax; i++) {
            if(clients[i].connected()) {
                clients[i].print(msg);
            }
        }
    }

}}

#endif // !RFLINK_SERIAL2NET_DISABLED