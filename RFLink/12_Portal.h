#ifndef _12_PORTAL_H_
#define _12_PORTAL_H_

#include "RFLink.h"

#ifndef RFLINK_PORTAL_DISABLED

#include "11_Config.h"

namespace RFLink {
    namespace Portal {

        extern Config::ConfigItem configItems[];
        /**
         * puts things together but doesn't start server *yet*
         * */
        void init();
        void start();
        void stop();

        void paramsUpdatedCallback();
        void refreshParametersFromConfig(bool triggerChanges=true);
    }
}

#endif // RFLINK_PORTAL_DISABLED

#endif