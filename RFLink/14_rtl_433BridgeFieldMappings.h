#ifndef rtl_433BridgeFieldMappings_h
#define rtl_433BridgeFieldMappings_h

#include <stdint.h>
#include "4_Display.h"

extern "C" {
#include "data.h"
}
namespace RFLink 
{ 
    namespace rtl_433Bridge 
    {
        typedef struct 
        {
            void (*uint_display_fn)(unsigned int value);
            void (*str_display_fn)(const char *str);
            void (*byte_display_fn)(uint8_t value);
            void (*ulong_display_fn)(unsigned long value);
            void (*double_display_fn)(double value);
        } field_mapping_functions;

        // the field order might seem a bit strange but it is optimized for declaration usage by placing the least
        // used fields at the end. 
        // The prefix length is close to the bit fields so as not to waste any alignment bytes should we have placed
        // prefix and prefix_len before the display functions as one would have expected
        typedef struct 
        {
            field_mapping_functions display_functions;
            const char* prefix;
            uint8_t prefix_len: 7;
            uint8_t multiply_value_by_ten: 1;
        } field_mapping;

        const field_mapping* get_field_mapping(const char* key);

        unsigned int multiply_by_ten(double);

        void display_unknown_data(data_t *);

    }
}

#endif