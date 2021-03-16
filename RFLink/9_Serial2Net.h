#ifndef _9_Serial2Net_H_
#define  _9_Serial2Net_H_

#ifndef RFLINK_SERIAL2NET_DISABLED

#ifndef SERIAL2NET_PORT
#define SERIAL2NET_PORT 1900
#endif

#include "11_Config.h"

//#define RFLINK_SERIAL2NET_DEBUG

namespace RFLink {
    namespace Serial2Net {

        namespace params
        {
            extern bool enabled;
            extern unsigned int port;
        }

        extern Config::ConfigItem configItems[];

        /**
         * Include in your setup loop after Wifi has been enabled
         * */
        void setup();
        /**
         * Include in your main loop so connections can be handled properly!
         * */
        void serverLoop();

        /**
         * Send a message to all connected clients
         * */
        void broadcastMessage(const char *msg);

        void paramsUpdatedCallback();
        void refreshParametersFromConfig(bool triggerChanges=true);

        void restartServer();
        void startServer();
        void stopServer(bool show_message=true);

        void getStatusJsonString(JsonObject &output);
    }
}

#endif // !RFLINK_SERIAL2NET_DISABLED
#endif // _9_Serial2Net_H_