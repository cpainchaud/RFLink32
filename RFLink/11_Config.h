#ifndef _11_CONFIG_H_
#define _11_CONFIG_H_

#include "RFLink.h"
#include <ArduinoJson.h>

namespace RFLink
{
    namespace Config
    {

        void setup();

        enum ConfigItemType
        {
            STRING_t,
            BOOLEAN_t,
            LONG_INT_t,
            EOF_t // must always be the last!
        };

        enum SectionId
        { // WARNING!!! any change here must be reflected in jsonSections variable in Config.cpp
            Wifi_id,
            OTA_id,
            Core_id,
            MQTT_id,
            Portal_id,
            Signal_id,
            Radio_id,
            Serial2Net_id,
            EOF_id // must always be the last!
        };

        class ConfigItem
        {

        private:
            bool boolDefaultValue;

        public:
            ConfigItemType type;
            const char *json_name;
            SectionId section;
            void (*update_callback)();
            void *defaultValue;
            JsonVariant jsonRef;
            bool canBeNull;

            ConfigItem(const char *name, SectionId section, const char *default_value, void (*update_callback)(), bool can_be_null = false);
            ConfigItem(const char *name, SectionId section, long int default_value, void (*update_callback)(), bool can_be_null = false);
            ConfigItem(const char *name, SectionId section, int default_value, void (*update_callback)(), bool can_be_null = false) : ConfigItem(name, section, (long int)default_value, update_callback, can_be_null){};
            ConfigItem(const char *name, SectionId section, bool default_value, void (*update_callback)(), bool can_be_null = false);
            ConfigItem();

            JsonVariant createInJsonObject(JsonObject &object);
            bool checkOrCreateValueInJsonObject(JsonObject &object);

            inline bool typeIsChar() { return this->type == ConfigItemType::STRING_t; }
            inline bool typeIsLongInt() { return this->type == ConfigItemType::LONG_INT_t; }
            inline bool typeIsBool() { return this->type == ConfigItemType::BOOLEAN_t; }
            inline bool typeIsEOF() { return this->type == ConfigItemType::EOF_t; }

            inline void deleteJsonRecord(){
                jsonRef.clear();
            }

            inline bool isUndefined(){
                return jsonRef.isUndefined() ||jsonRef.isNull();
            }

          inline void assignJsonRef(JsonVariant &json){
              this->jsonRef = json;
          }

            inline const char *getCharDefaultValue()
            {
                return (const char *)this->defaultValue;
            }

            inline long int getLongIntDefaultValue()
            {
                return (long int)this->defaultValue;
            }

            inline unsigned long int getUnsignedLongIntDefaultValue()
            {
                return (unsigned long int)this->defaultValue;
            }

            inline bool getBoolDefaultValue()
            {
                return boolDefaultValue;
            }

            inline const char *getCharValue()
            {
              if(isUndefined())
                return getCharDefaultValue();

              return (const char *)this->jsonRef.as<const char *>();
            }

            /**
                 * does not make a copy of the char!!
                 */
            inline void setCharValue(const char *newValue)
            {
              this->jsonRef.set((char *)newValue);
            }

            inline long int getLongIntValue()
            {
              if(isUndefined())
                return getLongIntDefaultValue();

              return (long int)this->jsonRef.as<signed long>();
            }

            inline void setLongIntValue(long int newValue)
            {
                this->jsonRef.set(newValue);
            }

            inline unsigned long int getUnsignedLongIntValue()
            {
              if(isUndefined())
                return getUnsignedLongIntDefaultValue();

              return (unsigned long int)this->jsonRef.as<unsigned long>();
            }

            inline void setUnsignedLongIntValue(unsigned long int newValue)
            {
                this->jsonRef.set(newValue);
            }

            inline bool getBoolValue()
            {
              if(isUndefined())
                return getBoolDefaultValue();

              return this->jsonRef.as<bool>();
            }

            inline void setBoolValue(bool newValue)
            {
                this->jsonRef.set(newValue);
            }
        };

        ConfigItem *findConfigItem(const char *name, SectionId section);
        void dumpConfigToString(String &destination);
        void dumpConfigToSerial();
        bool pushNewConfiguration(const JsonObject &data, String &message, bool escapeNewLine, bool triggerUpdateCallbacks = true);

        /**
         * @return SectionId::EOF_id is not found
         *  */
        SectionId getSectionIdFromString(const char *);
        JsonVariant createElementInSection(SectionId, const char *name);

        bool saveConfigToFlash();

        void executeCliCommand(const char *cmd);
        void resetConfig();

    }
}

#endif // _11_CONFIG_H_