#include "4_Display.h"
#include "14_rtl_433BridgeFieldMappings.h"

namespace RFLink 
{ 
    namespace rtl_433Bridge 
    {
        const field_mapping_functions displaySwitchStrByte = 
        {
            uint_display_fn : NULL,
            str_display_fn : &display_SWITCHc,
            byte_display_fn : &display_SWITCH
        };
        const field_mapping_functions displaySwitchByte = 
        {
            uint_display_fn : NULL,
            str_display_fn : NULL,
            byte_display_fn : &display_SWITCH
        };
        const field_mapping_functions displaySwitchStr = 
        {
            uint_display_fn : NULL,
            str_display_fn : &display_SWITCHc
        };
        const field_mapping_functions displayIdUlong = 
        {
            uint_display_fn : NULL,
            str_display_fn : NULL,
            byte_display_fn : NULL,
            ulong_display_fn : &display_ID
        };
        const field_mapping_functions displayIdStrUlong = 
        {
            uint_display_fn : NULL,
            str_display_fn : &display_IDc,
            byte_display_fn : NULL,
            ulong_display_fn : &display_ID
        };
        const field_mapping_functions displayLux = { uint_display_fn : &display_LUX };

        void display_battery(const char* str)
        {
            ::display_BAT(strcasecmp(str, "OK") == 0);
        }

        void display_battery(unsigned int value)
        {
            ::display_BAT(value != 0);
        }

        void display_state(const char* str)
        {
            enum CMD_OnOff command = CMD_Unknown;
            bool all = false;
            if (str2cmdenum(str, all, command))
                display_CMD(all, command);
            else
                display_SWITCHc(str);
        }

        void display_wind_direction(unsigned int value)
        {
            display_wind_direction((double)value);
        }

        void display_wind_direction(double value)
        {
            display_WINDIR(round(value / 22.5));
        }

        void display_temperature(double value) 
        {
            unsigned int temperature = multiply_by_ten(value);
            if (value < 0)
                temperature |= 0x8000;

            display_TEMP(temperature);
        }

        void display_nothing(const char* str) {}

        const field_mapping field_mappings[] =
        {
            {
                display_functions : displayIdUlong,
                prefix : "address", 
                prefix_len : 7,
            },
            {
                display_functions : displaySwitchStrByte,
                prefix : "button", 
                prefix_len : 6,
            },
            {
                display_functions : { uint_display_fn : &display_battery, str_display_fn : &display_battery },
                prefix : "battery", 
                prefix_len : 7,
            },
            {
                display_functions : { uint_display_fn : NULL, str_display_fn : NULL, byte_display_fn: &display_CHAN },
                prefix : "channel", 
                prefix_len : 7,
            },
            {
                display_functions : displaySwitchStrByte,
                prefix : "code", 
                prefix_len : 4,
            },
            {
                display_functions : displaySwitchStrByte,
                prefix : "command", 
                prefix_len : 7,
            },
            {
                display_functions : { uint_display_fn : &display_CURRENT },
                prefix : "current", 
                prefix_len : 7
            },
            {
                display_functions : displaySwitchStr,
                prefix : "data", 
                prefix_len : 4,
            },
            {
                display_functions : { uint_display_fn : NULL, str_display_fn : NULL, byte_display_fn: &display_SET_LEVEL },
                prefix : "dim", 
                prefix_len : 3
            },
            {
                display_functions : displayIdUlong,
                prefix : "housecode", 
                prefix_len : 9,
            },
            {
                display_functions : { uint_display_fn : NULL, str_display_fn : NULL, byte_display_fn: &display_HUM },
                prefix : "humidity", 
                prefix_len : 8
            },
            {
                display_functions : displayIdStrUlong,
                prefix : "id", 
                prefix_len : 2,
            },
            {
                display_functions : { uint_display_fn : &display_METER },
                prefix : "impulses", 
                prefix_len : 8
            },
            {
                display_functions : displaySwitchStr,
                prefix : "key", 
                prefix_len : 3,
            },
            {
                display_functions : displayLux,
                prefix : "light", 
                prefix_len : 5
            },
            {
                display_functions : displayLux,
                prefix : "lux", 
                prefix_len : 3
            },
            {
                display_functions : { uint_display_fn : NULL, str_display_fn : &display_Name },
                prefix : "model", 
                prefix_len : 5,
            },
            {
                display_functions : { uint_display_fn : NULL, str_display_fn : &display_nothing }, // mic is ignored
                prefix : "mic", 
                prefix_len : 3,
            },
            {
                display_functions : { uint_display_fn : &display_WATT },
                prefix : "power", 
                prefix_len : 5
            },
            {
                display_functions : { uint_display_fn : &display_BARO },
                prefix : "pressure_kPa", 
                prefix_len : 8,
                multiply_value_by_ten : 1
            },
            {
                display_functions : { uint_display_fn : &display_BARO },  // keep after pressure_kPa to avoid incorrect match
                prefix : "pressure", 
                prefix_len : 8
            },
            {
                display_functions : { uint_display_fn : &display_RAINRATE },
                prefix : "rain_rate", 
                prefix_len : 9,
                multiply_value_by_ten : 1
            },
            {
                display_functions : { uint_display_fn : &display_RAIN },  // keep after rain_rate to avoid incorrect match
                prefix : "rain", 
                prefix_len : 4,
                multiply_value_by_ten : 1
            },
            {
                display_functions : { uint_display_fn : NULL, str_display_fn : &display_state },
                prefix : "state", 
                prefix_len : 5,
            },
            {
                display_functions : displaySwitchStr,
                prefix : "switch", 
                prefix_len : 6,
            },
            {
                display_functions : { uint_display_fn : NULL, str_display_fn : NULL, byte_display_fn : NULL, ulong_display_fn : NULL, double_display_fn : &display_temperature },
                prefix : "temperature", 
                prefix_len : 11,
            },
            {
                display_functions : { uint_display_fn : &display_UV },
                prefix : "uv", 
                prefix_len : 2
            },
            {
                display_functions : { uint_display_fn : &display_VOLT },
                prefix : "voltage", 
                prefix_len : 7
            },
            {
                display_functions : { uint_display_fn : &display_WINGS },
                prefix : "wind_max", 
                prefix_len : 8
            },
            {
                display_functions : { uint_display_fn : &display_AWINSP },
                prefix : "wind_avg", 
                prefix_len : 8,
                multiply_value_by_ten : 1
            },
            {
                display_functions : { uint_display_fn : &display_wind_direction, str_display_fn : NULL, byte_display_fn : NULL, ulong_display_fn : NULL, double_display_fn : &display_wind_direction },
                prefix : "wind_dir_deg", 
                prefix_len : 12,
            },            
        };

        const int num_field_mappings = sizeof(field_mappings) / sizeof(field_mapping);

        const field_mapping* get_field_mapping(const char* key)
        {
            for(int mapping_index = 0; mapping_index < num_field_mappings; mapping_index++)
            {
                const field_mapping* mapping = &field_mappings[mapping_index]; 
                if (strncmp(key, mapping->prefix, mapping->prefix_len) == 0)
                    return mapping;
            }
            return NULL;
        }

        unsigned int multiply_by_ten(double value)
        {
            return ((unsigned int)round(abs(value * 10.0))) & 0xFFFF;
        }

        void display_unknown_data(data_t *d)
        {
            const int buffer_size = 25;
            char buffer[buffer_size];

            display_FieldSeparator();
            display_RawString(d->key);
            display_RawString("=");
            switch (d->type)
            {
                case DATA_STRING:
                    display_RawString((const char*)(d->value.v_ptr));
                case DATA_DOUBLE:
                    snprintf(buffer, buffer_size, (d->format) ? d->format : "%.3f", d->value.v_dbl);
                    display_RawString(buffer);
                    break;
                case DATA_INT:
                    snprintf(buffer, buffer_size, (d->format) ? d->format : "%d", d->value.v_int);
                    display_RawString(buffer);
                    break;
                case DATA_DATA:
                case DATA_ARRAY:
                case DATA_COND:
                case DATA_COUNT:
                case DATA_FORMAT:
                    break;
            }
        }
    }
}
