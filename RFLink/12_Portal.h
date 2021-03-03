#ifndef _12_PORTAL_H_
#define _12_PORTAL_H_

#include "RFLink.h"
#include "11_Config.h"

namespace RFLink {
    namespace Portal {

        extern Config::ConfigItem configItems[];
        /**
         * puts things together but doesn't start server *yet*
         * */
        void init();
        void start();

        void paramsUpdatedCallback();
    }
}


#endif