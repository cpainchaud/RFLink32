#ifndef _13_OTA_H_
#define _13_OTA_H_

#include <WString.h>

namespace RFLink
{
  namespace OTA
  {
    enum statusEnum {
      Idle,
      Scheduled,
      InProgress,
      PendingReboot,
      Failed,
    };

    extern statusEnum currentHttpUpdateStatus;

    /*
     * To schedule an HttpUpdate (which must happen in the main loop!)
     *
     **/
    bool scheduleHttpUpdate(const char *url, String &errmsg);

    void mainLoop();

  }
} // end of RFLink namespace

#endif // _13_OTA_H_