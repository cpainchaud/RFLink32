#include "11_Config.h"
#include <assert.h>

#ifdef ESP8266 
#include <LittleFS.h>
#else
#include <FS.h>
#include <LITTLEFS.h>
#endif

#include "10_Wifi.h"

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

namespace RFLink { namespace Config {


const char configFileName[] = "/config.json";



const char * jsonSections[] = {
    "wifi_ota",
    "core",
    "mqtt",
    "root" // this is always the last one and matches index SectionId::EOF_id
};

static_assert(sizeof(jsonSections)/sizeof(char *) == SectionId::EOF_id+1, "jsonSections has missing/extra sections names, please compare with SectionId enum declations");


ConfigItem* configItemLists[] = {
    #if defined(RFLINK_WIFIMANAGER_ENABLED) || defined(RFLINK_WIFI_ENABLED)
    &RFLink::Wifi::configItems[0],
    #endif
 };
 #define configItemListsSize (sizeof(configItemLists)/sizeof(ConfigItem*))

StaticJsonDocument<2048> doc;

void printFile() {
  // Open file for reading
  Serial.println("Now dumping JSON file content:");
  #ifdef ESP32
  File file = LITTLEFS.open(configFileName, "r");
  #else
   File file = LittleFS.open(configFileName, "r");
  #endif
  
  if (!file) {
    Serial.println(F("Failed to read file"));
    return;
  }

  // Extract each characters by one by one
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();

  // Close the file
  file.close();
}

void init() {

    char tmp[100];

    Serial.print("Loading persistent filesystem... ");

    #ifdef ESP32
        if( !LITTLEFS.begin(true) ) {
            Serial.println(" FAILED!!");
            return;
        }
        Serial.print("OK. ");

        sprintf(tmp, "File system usage: %lu/%luKB.", LITTLEFS.usedBytes()/1024, LITTLEFS.totalBytes()/1024);
        Serial.println(tmp);
    #else // this is ESP8266
        if( !LittleFS.begin() ) {
            Serial.println(" FAILED!!");
            return;
        }
        Serial.print("OK. ");

        FSInfo info;
        LittleFS.info(info);
        sprintf(tmp, "File system usage: %lu/%luKB.", info.usedBytes/1024, info.totalBytes/1024);
        Serial.println(tmp);
    #endif


    // Let's iterate over all registered configItems and count them/sanitize
    int countConfigItems = 0;
    for(int i=0; i<configItemListsSize; i++) {
        ConfigItem *item = configItemLists[i];

        while(!item->typeIsEOF()) {
            if (item->typeIsChar()) {
                sprintf(tmp, "added configitem '%s' with default_value=", item->json_name);
                Serial.print(tmp);
                Serial.println(item->getCharDefaultValue());

            } else if (item->typeIsLongInt()) {
                sprintf(tmp, "added configitem '%s' with default_value=", item->json_name);
                Serial.print(tmp);
                Serial.println(item->getLongIntDefaultValue());

            } else if (item->typeIsBool()) {
                sprintf(tmp, "added configitem '%s' with default_value=", item->json_name);
                Serial.print(tmp);
                Serial.println(item->getBoolDefaultValue());
            }
            else {
                Serial.print("unsupported configitem type=");
                Serial.println(item->type);
            }

            item++;
        }
    }
    sprintf(tmp, "Counted %i config items in total", countConfigItems);
    Serial.println(tmp);

    sprintf(tmp, "Now opening JSON config file '%s'", configFileName);
    Serial.println(tmp);

    #ifdef ESP32
    File file = LITTLEFS.open(configFileName, "r");
    #else
    File file = LittleFS.open(configFileName, "r");
    #endif

    DeserializationError error = deserializeJson(doc, file);
    if (error)
        Serial.println(F("Failed to read file, using default configuration"));
    file.close();

    sprintf(tmp, "JSON file mem usage: %lu", doc.memoryUsage());
    Serial.println(tmp);

    bool fileHasChanged = false;

    for(int i=0; i<configItemListsSize; i++) {
        ConfigItem *item = configItemLists[i];

        while(!item->typeIsEOF()) {
            JsonVariant sectionVariant = doc[jsonSections[item->section]];
            JsonObject sectionObject;
            if(sectionVariant.isUndefined()) {
                fileHasChanged = true;
                sectionObject = doc.createNestedObject(jsonSections[item->section]);

            } else if(!sectionVariant.is<JsonObject>()) {
                fileHasChanged = true;
                doc.remove(sectionVariant);
                sectionObject = doc.createNestedObject(jsonSections[item->section]);
            } else {
                sectionObject = sectionVariant.as<JsonObject>();
            }

            if(item->updateValueInObject(sectionObject)) {
                fileHasChanged = true;
            }

            item++;
        }
    }

    if(fileHasChanged) {
        Serial.print("Json configuration file has changed, let's save it... ");

        #ifdef ESP32
        LITTLEFS.remove("/tmp.json");
        File file = LITTLEFS.open("/tmp.json", "w");
        #else
        LittleFS.remove("/tmp.json");
        File file = LittleFS.open("/tmp.json", "w");
        #endif

        auto bytes_written = serializeJson(doc, file);
        file.close();

        if (bytes_written == 0) {
            Serial.println(F("Failed to write to file"));
        } else {
            #ifdef ESP32
            LITTLEFS.remove(configFileName);
            LITTLEFS.rename("/tmp.json", configFileName);
            #else
            LittleFS.remove(configFileName);
            LittleFS.rename("/tmp.json", configFileName);
            #endif
            Serial.println("OK");
        }
    }

    printFile();

}

JsonVariant ConfigItem::createInJsonObject(JsonObject &obj) {
    if(this->typeIsChar()) {
        return obj[this->json_name] = this->getCharValue();
    } else if(this->typeIsLongInt()) {
        return obj[this->json_name] = this->getLongIntValue();
    } else if(this->typeIsBool()) {
        return obj[this->json_name] = this->getBoolValue();
    }
    return JsonVariant();
}

bool ConfigItem::updateValueInObject(JsonObject &obj) {

    bool result = false;

    JsonVariant value = obj[this->json_name];

    if( value.isUndefined() ) {
        createInJsonObject(obj);
        return true;
    }

    if(this->typeIsChar()) {
        if(!value.is<char *>()) {
            value.set((char*)this->getCharValue());
            return true;
        }
        const char *str = value.as<const char *>();
        if(strcmp(str, this->getCharValue()) != 0) {
            value.set((char*)this->getCharValue());
            return true;
        }
    }

    if(this->typeIsLongInt()) {
        if(!value.is<signed long>()) {
            value.set(this->getLongIntValue());
            return true;
        }
        signed long val = value.as<signed long>();
        if(  val != this->getLongIntValue() ) {
            value.set(this->getLongIntValue());
            return true;
        }
    }

    if(this->typeIsBool()) {
        if(!value.is<bool>()) {
            value.set(this->getBoolValue());
            return true;
        }
        bool val = value.as<signed long>();
        if(  val != this->getBoolValue() ) {
            value.set(this->getBoolValue());
            return true;
        }
    }

    return result;       
}


ConfigItem::ConfigItem(const char *name,
                        SectionId section,
                        const  char* default_value,
                        void *(*update_callback)(void *) )
{

    this->json_name = name;
    this->section = section;
    this->type = ConfigItemType::STRING_t;
    this->update_callback = update_callback;
    

    static_assert( sizeof(this->defaultValue) <= sizeof(char *), "variable size is too small");
    this->defaultValue = (void *) default_value;
    this->currentValue = this->defaultValue;

}

ConfigItem::ConfigItem(const char *name,
                        SectionId section,
                        long int default_value,
                        void *(*update_callback)(void *))
{
    this->json_name = name;
    this->section = section;
    this->type = ConfigItemType::LONG_INT_t;
    this->update_callback = update_callback;

    static_assert( sizeof(this->defaultValue) <= sizeof(long int), "variable size is too small");
    this->defaultValue = (void *) default_value;
    this->currentValue = this->defaultValue;
}


ConfigItem::ConfigItem(const char *name,
                        SectionId section,
                        bool default_value,
                        void *(*update_callback)(void *))
{
    this->json_name = name;
    this->section = section;
    this->type = ConfigItemType::BOOLEAN_t;
    this->update_callback = update_callback;

    this->defaultValue = (void *) default_value;
    this->currentValue = this->defaultValue;
}

ConfigItem::ConfigItem()
{
    this->json_name = nullptr;
    this->section = SectionId::EOF_id;
    this->type = ConfigItemType::EOF_t;
    this->update_callback = nullptr;
    this->currentValue = this->defaultValue;
}


} // end of Config namespace
} // end of RFLink namespace