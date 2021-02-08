#ifndef _10_WiFi_H
#define _10_WiFi_H

#include "RFLink.h"

#ifdef RFLINK_WIFIMANAGER_ENABLED
#include "WifiManager.h"
#elif RFLINK_WIFI_ENABLED
#include "Wifi.h"
#endif


namespace RFLink {
    namespace Wifi {
        void setup();
        void mainLoop();
        
        void stop_WIFI();
        void start_WIFI();
    }
}

#endif //_10_WiFi_H

