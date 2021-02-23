#ifndef _11_CONFIG_H_
#define _11_CONFIG_H_

#include "RFLink.h"
#include <ArduinoJson.h>


namespace RFLink {
    namespace Config {


        void init();

        enum ConfigItemType {
            STRING_t,
            BOOLEAN_t,
            LONG_INT_t,
            EOF_t // must always be the last!
        };

        enum SectionId {  // WARNING!!! any change here must be reflected in jsonSections variable in Config.cpp
            WifiOTA_id,
            Core_id,
            MQTT_id,
            EOF_id // must always be the last!
        };

        class ConfigItem {
            

            public:
                ConfigItemType type;
                const char *json_name;
                SectionId section;
                void* (*update_callback)(void *);
                void* defaultValue;
                void* currentValue;

                ConfigItem(const char *name, SectionId section, const char* default_value, void *(*update_callback)(void *));
                ConfigItem(const char *name, SectionId section, long int default_value, void *(*update_callback)(void *));
                ConfigItem(const char *name, SectionId section, int default_value, void *(*update_callback)(void *)): 
                    ConfigItem(name, section, (long int) default_value,  update_callback){};
                ConfigItem(const char *name, SectionId section, bool default_value, void *(*update_callback)(void *));
                ConfigItem();

                JsonVariant createInJsonObject(JsonObject &object);
                bool updateValueInObject(JsonObject &object);

                inline bool typeIsChar() { return this->type == ConfigItemType::STRING_t;}
                inline bool typeIsLongInt() { return this->type == ConfigItemType::LONG_INT_t;}
                inline bool typeIsBool() { return this->type == ConfigItemType::BOOLEAN_t;}
                inline bool typeIsEOF() { return this->type == ConfigItemType::EOF_t;}

                inline const char * getCharDefaultValue() {
                    return (const char *) this->defaultValue;
                }

                inline long int getLongIntDefaultValue() {
                    return (long int) this->defaultValue;
                }

                inline bool getBoolDefaultValue() {
                    return (long int) this->defaultValue == 0;
                }

                inline const char * getCharValue() {
                    return (const char *) this->currentValue;
                }

                inline long int getLongIntValue() {
                    return (long int) this->currentValue;
                }

                inline bool getBoolValue() {
                    return (long int) this->currentValue == 0;
                }
        };

    }
}


#endif // _11_CONFIG_H_