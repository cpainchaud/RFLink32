#include "9_Serial2Net.h"



namespace RFLink { namespace Serial2Net {
    
    WiFiServer server;

    boolean alreadyConnected = false;
    const unsigned short clientsMax=10;
    WiFiClient clients[clientsMax];

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
        #if defined(SERIAL2NET_DEBUG) || defined(DEBUG)
        String debugmsg("Serial2Net: ");
        #endif

        WiFiClient client = server.available();

        if( isNewClient(client) ) {
            #if defined(SERIAL2NET_DEBUG) || defined(DEBUG)
            debugmsg += "Serial2Net: new client detected: " + client.remoteIP().toString() + " port:" + client.remotePort();
            Serial.println(debugmsg);
            #endif
            registerClient(client);
            return;
        }

        //from here we have a client sending data
        char buffer[256];
        int readMax = client.available();
        if (readMax > 0) {
            if (readMax > 256)
                readMax = 256; 
            client.readBytesUntil(0x0d, buffer, readMax);

            #if defined(SERIAL2NET_DEBUG) || defined(DEBUG)
                debugmsg += "Serial2Net: client sent data: ";
                debugmsg += buffer;
                Serial.println(debugmsg);
            #endif
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