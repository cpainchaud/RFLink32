/** @file
    A bridge between RFLink32 and the plugins from rtl_433

    Copyright (C) 2021 Olivier Sannier <obones (a) free (point) fr>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/
#ifndef rtl_433Bridge_h
#define rtl_433Bridge_h

namespace RFLink 
{ 
    namespace rtl_433Bridge 
    {
        void register_all_protocols(unsigned disabled);
        int processReceivedData();
    }
}
#endif