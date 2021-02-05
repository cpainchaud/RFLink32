#ifndef _9_Serial2Net_H_
#define  _9_Serial2Net_H_

#ifdef SERIAL2NET_ENABLED

#ifndef SERIAL2NET_PORT
#define SERIAL2NET_PORT 1900
#endif

//#define SERIAL2NET_DEBUG


#include <WifiServer.h>


namespace RFLink {
    namespace Serial2Net {
        /**
         * Include in your setup loop after Wifi has been enabled
         * */
        void startServer();
        /**
         * Include in your main loop so connections can be handled properly!
         * */
        void serverLoop();

        /**
         * Send a message to all connected clients
         * */
        void broadcastMessage(const char *msg);
    }
}

#endif // SERIAL2NET_ENABLED
#endif // _9_Serial2Net_H_