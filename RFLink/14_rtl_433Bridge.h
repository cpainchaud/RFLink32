#ifndef rtl_433Bridge_h
#define rtl_433Bridge_h

namespace RFLink 
{ 
    namespace rtl_433Bridge 
    {
        void register_all_protocols(unsigned disabled);
        void processReceivedData();
    }
}
#endif