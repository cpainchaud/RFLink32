#include "11_Config.h"
#include <assert.h>

#ifdef ESP8266
#include <LittleFS.h>
#else
#include <FS.h>
#include <LittleFS.h>
#endif

#include "1_Radio.h"
#include "2_Signal.h"
#include "6_MQTT.h"
#include "9_Serial2Net.h"
#include "10_Wifi.h"
#include "12_Portal.h"

#if defined(DEBUG) || defined(RFLINK_DEBUG)
#define DEBUG_RFLINK_CONFIG
#endif

namespace RFLink
{
  namespace Config
  {

    namespace commands
    {
      String set("set");
      String reset("reset");
      String dump("dump");
    }

    const char configFileName[] = "/config.json";

    const char *jsonSections[] = {
            "wifi",
            "ota",
            "core",
            "mqtt",
            "portal",
            "signal",
            "radio",
            "serial2net",
            "root" // this is always the last one and matches index SectionId::EOF_id
    };

#define jsonSections_count sizeof(jsonSections) / sizeof(char *)

    static_assert(sizeof(jsonSections) / sizeof(char *) == SectionId::EOF_id + 1, "jsonSections has missing/extra sections names, please compare with SectionId enum declations");

    ConfigItem *configItemLists[] = {
#if defined(RFLINK_WIFI_ENABLED)
            &RFLink::Wifi::configItems[0],
            #ifndef RFLINK_MQTT_DISABLED
            &RFLink::Mqtt::configItems[0],
            #endif // RFLINK_MQTT_DISABLED
            &RFLink::Serial2Net::configItems[0],
            #ifndef RFLINK_PORTAL_DISABLED
            &RFLink::Portal::configItems[0],
            #endif // RFLINK_PORTAL_DISABLED
#endif
            &RFLink::Signal::configItems[0],
            &RFLink::Radio::configItems[0],
    };
#define configItemListsSize (sizeof(configItemLists) / sizeof(ConfigItem *))

    #ifndef RFLINK_CONFIG_JSON_BUFFER_SIZE
      #ifdef ESP8266
        #define RFLINK_CONFIG_JSON_BUFFER_SIZE 2200
      #else
        #define RFLINK_CONFIG_JSON_BUFFER_SIZE 4096
      #endif
    #endif

    StaticJsonDocument<RFLINK_CONFIG_JSON_BUFFER_SIZE> doc; // Current config's JSON document

    void resetConfig()
    {
      if (LittleFS.exists(configFileName))
      {
        Serial.println(F("Removing config file..."));
        LittleFS.remove(configFileName);
        if(LittleFS.exists(configFileName))
        {
          Serial.println(F("Failed to remove config file!"));
        }
      }
      LittleFS.end();

      Serial.println(F("Config has been reset and requires a reboot to complete"));
    }

    void printFile()
    {
      // Open file for reading
      Serial.println(F("Now dumping JSON file content:"));
      File file = LittleFS.open(configFileName, "r");
      if (!file)
      {
        Serial.println(F("Failed to read file"));
        return;
      }

      // Extract each characters by one by one
      while (file.available())
      {
        Serial.print((char)file.read());
      }
      Serial.println();

      // Close the file
      file.close();
    }

    ConfigItem *findConfigItem(const char *name, SectionId section)
    {
      for (unsigned int i = 0; i < configItemListsSize; i++)
      {
        ConfigItem *item = configItemLists[i];

        while (!item->typeIsEOF())
        {
          if (item->section == section && strcmp(item->json_name, name) == 0)
          {
            return item;
          }

          item++;
        }
      }

      return nullptr;
    }

    void setup()
    {
      Serial.print(F("Loading persistent filesystem... "));

      #ifdef ESP32
      if (!LittleFS.begin(true))
      #else
      if( !LittleFS.begin())
      #endif
      {
        Serial.println(F(" FAILED!!"));
        return;
      }
      Serial.print(F("OK. "));

#ifdef ESP32
      Serial.printf_P(PSTR("File system usage: %u/%uKB.\r\n"), LittleFS.usedBytes() / 1024, LittleFS.totalBytes() / 1024);
#else // this is ESP8266
      FSInfo info;
      LittleFS.info(info);
      Serial.printf_P(PSTR("File system usage: %u/%uKB.\r\n"), info.usedBytes / 1024, info.totalBytes / 1024);
#endif


      for (unsigned int i = 0; i < configItemListsSize; i++)
      {
        ConfigItem *item = configItemLists[i];

        while (!item->typeIsEOF())
        {
          if (item->typeIsChar())
          {
#ifdef DEBUG_RFLINK_CONFIG
            Serial.printf_P(PSTR("Registered configItem '%s' with default_value="), item->json_name);
            Serial.println(item->getCharDefaultValue());
#endif
          }
          else if (item->typeIsLongInt())
          {
#ifdef DEBUG_RFLINK_CONFIG
            Serial.printf_P(PSTR("Registered configItem '%s' with default_value=%ld (long)\r\n"), item->json_name, item->getLongIntDefaultValue());
#endif
          }
          else if (item->typeIsBool())
          {
#ifdef DEBUG_RFLINK_CONFIG
            Serial.printf_P(PSTR("Registered configItem '%s' with default_value=%i (bool)\r\n"), item->json_name, (int) item->getBoolDefaultValue());
#endif
          }
          else
          {
            Serial.print(F("unsupported configitem type="));
            Serial.println(item->type);
          }

          item++;
        }
      }
      //sprintf(tmp, "Counted %i config items in total", countConfigItems);
      //Serial.println(tmp);

      Serial.printf(PSTR("Now opening JSON config file '%s'\r\n"), configFileName);

      File file = LittleFS.open(configFileName, "r");
      DeserializationError error = deserializeJson(doc, file);
      if (error)
        Serial.println(F("Failed to read file, using default configuration"));
      file.close();

      Serial.printf_P(PSTR("JSON file mem usage: %u / %u\r\n"), doc.memoryUsage(), doc.memoryPool().capacity());

      bool fileHasChanged = false;

      // We're hunting extra configurations which dont exist in json
      auto root = doc.as<JsonObject>();
      for (JsonPair kv : root)
      {
        //Serial.print("Remote has root object named: ");
        //Serial.println(kv.key().c_str());

        JsonVariant &&section_variant = kv.value();
        if (!section_variant.is<JsonObject>())
        {
          Serial.printf_P(PSTR("root entry '%s'  is not an object, it will be discarded\r\n"), kv.key().c_str());
          root.remove(kv.key().c_str());
          continue;
        }
        JsonObject &&sectionObject = section_variant.as<JsonObject>();

        SectionId lookupSectionID = getSectionIdFromString(kv.key().c_str());
        if (lookupSectionID == SectionId::EOF_id)
        {
          Serial.printf_P(PSTR("root entry '%s' is not a valid section name, it will be discarded\r\n"), kv.key().c_str());
          root.remove(kv.key().c_str());
          continue;
        }

        // from here we have a valid section, now we go down a level in the remote object
        for (JsonPair section_kv : sectionObject)
        {
          //Serial.print("Remote section has item named: ");
          //Serial.println(section_kv.key().c_str());

          ConfigItem *item = findConfigItem(section_kv.key().c_str(), lookupSectionID);
          if (item == nullptr)
          {
            Serial.printf_P(PSTR("section '%s' has extra configuration item named '%s' it will be discarded\r\n"), kv.key().c_str(), section_kv.key().c_str());
            sectionObject.remove(section_kv.key().c_str());
            fileHasChanged = true;
            continue;
          }
        }
      }

      // Let's fill missing json config with their default values
      for (unsigned int i = 0; i < configItemListsSize; i++)
      {
        ConfigItem *item = configItemLists[i];

        while (!item->typeIsEOF())
        {
          JsonVariant sectionVariant = doc[jsonSections[item->section]];
          JsonObject sectionObject;

          if (sectionVariant.isUndefined())
          {
            #ifdef DEBUG_RFLINK_CONFIG
            Serial.printf_P(PSTR("Config: created missing section Object '%s'\r\n"), jsonSections[item->section]);
            #endif
            fileHasChanged = true;
            sectionObject = doc.createNestedObject(jsonSections[item->section]);
          }
          else if (!sectionVariant.is<JsonObject>())
          {
            #ifdef DEBUG_RFLINK_CONFIG
            Serial.printf_P(PSTR("Config: section '%s' is not an Object it will recreated\r\n"), jsonSections[item->section]);
            #endif
            fileHasChanged = true;
            doc.remove(sectionVariant);
            sectionObject = doc.createNestedObject(jsonSections[item->section]);
          }
          else
          {
            sectionObject = sectionVariant.as<JsonObject>();
          }

          if (item->checkOrCreateValueInJsonObject(sectionObject))
          {
            #ifdef DEBUG_RFLINK_CONFIG
            Serial.printf_P(PSTR("Config: created missing object '%s' in section '%s'\r\n"), item->json_name, jsonSections[item->section]);
            #endif
            fileHasChanged = true;
          }

          item++;
        }
      }

      if (fileHasChanged)
      {
        saveConfigToFlash();
      }

      printFile();
    }

    class CallbackManager
    {
    private:
      void (*callbacks[20])();

    public:
      CallbackManager()
      {
        for (unsigned int i = 0; i < sizeof(callbacks) / sizeof(void (*)()); i++)
        {
          callbacks[i] = nullptr;
        }
      }

      void add(void (*c)())
      {
        if (c == nullptr)
          return;
        for (unsigned int i = 0; i < sizeof(callbacks) / sizeof(void (*)()); i++)
        {
          if (callbacks[i] == c)
            return;
          if (callbacks[i] == nullptr)
          {
            callbacks[i] = c;
            return;
          }
        }
      }

      void execute()
      {
        for (unsigned int i = 0; i < sizeof(callbacks) / sizeof(void (*)()); i++)
        {
          if (callbacks[i] == nullptr)
            return;
          callbacks[i]();
        }
      }
    };

    bool pushNewConfiguration(const JsonObject &data, String &message, bool escapeNewLine, bool triggerUpdateCallbacks)
    {

      bool configHasChanged = false;
      CallbackManager callbackMgr;
      message.reserve(256);
      String new_line(F("\n"));

      if (escapeNewLine)
        new_line = F("\\n");

      // we iterate root level to find sections!
      for (JsonPair kv : data)
      {

#ifdef DEBUG_RFLINK_CONFIG
        Serial.print(F("Remote has root object named: "));
                Serial.println(kv.key().c_str());
#endif

        JsonVariant &&section_variant = kv.value();
        if (!section_variant.is<JsonObject>())
        {
          message += F("root entry '");
          message += kv.key().c_str();
          message += F("' is not an object, it will be ignored!");
          message += new_line;
          continue;
        }
        JsonObject &&sectionObject = section_variant.as<JsonObject>();

        SectionId lookupSectionID = getSectionIdFromString(kv.key().c_str());
        if (lookupSectionID == SectionId::EOF_id)
        {
          message += F("root entry '");
          message += kv.key().c_str();
          message += F("' is not a valid section name, it will be ignored");
          message += new_line;
          continue;
        }

        // from here we have a valid section, now we go down a level in the remote object
        //Serial.printf_P(PSTR("Section %s has %i members\r\n"), kv.key().c_str(), sectionObject.size());
        for (JsonPair section_kv : sectionObject)
        {
#ifdef RFLINK_CONFIG_DEBUG
          Serial.print(F("Remote section has item named: "));
                    Serial.println(section_kv.key().c_str());
#endif

          ConfigItem *item = findConfigItem(section_kv.key().c_str(), lookupSectionID);
          if (item == nullptr)
          {
            message += F("section '");
            message += kv.key().c_str();
            message += F("' has extra configuration item named '");
            message += section_kv.key().c_str();
            message += F("' it will be ignored");
            message += new_line;
            continue;
          }

          JsonVariant &&remoteVariant = section_kv.value();

          if(remoteVariant.isNull() && item->canBeNull) {
            if( !item->isUndefined() ) {
              item->deleteJsonRecord();
              configHasChanged = true;
              callbackMgr.add(item->update_callback);
            }
            continue;
          }

          // in case the object does not exist in config we need to create it
          if(!remoteVariant.isNull() && item->isUndefined()) {
            #ifdef DEBUG_RFLINK_CONFIG
            Serial.printf_P(PSTR("Creating entry '%s' in section '%s' for nonexistent \r\n"), section_kv.key().c_str(), kv.key().c_str());
            #endif
            JsonVariant tmp = createElementInSection(lookupSectionID, section_kv.key().c_str());
            //JsonVariant tmp = createElementInSection(lookupSectionID, "hello"); // use char* to force copy
            //tmp.set(remoteVariant);
            //tmp.set(2);
            item->assignJsonRef(tmp);
            configHasChanged = true;
            callbackMgr.add(item->update_callback);
          }

          if (item->typeIsChar())
          {
            if (!remoteVariant.is<const char *>())
            {
              message += F("section '");
              message += kv.key().c_str();
              message += F("' has '");
              message += section_kv.key().c_str();
              message += F("' with mismatched type (not string) so it will be ignored");
              message += new_line;
              continue;
            }
            const char *str = remoteVariant.as<const char *>();
            if (strcmp(str, item->getCharValue()) == 0) // no change!
              continue;

            configHasChanged = true;
            callbackMgr.add(item->update_callback);
            item->setCharValue(str);
          }
          else if (item->typeIsLongInt())
          {

            long int remote_value;
            if (remoteVariant.is<signed long>())
              remote_value = remoteVariant.as<signed long>();
            else
            {
              message += F("section '");
              message += kv.key().c_str();
              message += F("' has item '");
              message += section_kv.key().c_str();
              message += F("' with mismatched type (not long int) so it will be ignored");
              message += new_line;
              continue;
            }

            if (remote_value == item->getLongIntValue()) // no change!
              continue;

            configHasChanged = true;
            callbackMgr.add(item->update_callback);
            item->setLongIntValue(remote_value);
          }
          else if (item->typeIsBool())
          {
            if (!remoteVariant.is<bool>())
            {
              message += F("section '");
              message += kv.key().c_str();
              message += F("' has item '");
              message += section_kv.key().c_str();
              message += F("' with mismatched type (not bool) so it will be ignored");
              message += new_line;
              continue;
            }
            auto remote_value = remoteVariant.as<bool>();
            if (remote_value == item->getBoolValue()) // no change!
              continue;

            configHasChanged = true;
            callbackMgr.add(item->update_callback);
            item->setBoolValue(remote_value);
          }
        }
      }

      Serial.println(message.c_str());

      if (configHasChanged)
      {
        if (!saveConfigToFlash())
        {
          message += F("Error! Failed to write JSON config to FLASH!");
          //Serial.println(F("Error! Failed to write JSON config to FLASH!"));
          return false;
        }
        else
        {
          Serial.println(F("Config file saved to flash."));
        }
        if (triggerUpdateCallbacks)
          callbackMgr.execute();
      }
      else
      {
        Serial.println(F("Config file has not changed."));
      }

      return true;
    }

    JsonVariant ConfigItem::createInJsonObject(JsonObject &obj)
    {
      if (this->typeIsChar())
      {
        obj[this->json_name] = this->getCharDefaultValue();
        this->jsonRef = obj[this->json_name];
        return this->jsonRef;
      }
      else if (this->typeIsLongInt())
      {
        obj[this->json_name] = this->getLongIntDefaultValue();
        this->jsonRef = obj[this->json_name];
        return this->jsonRef;
      }
      else if (this->typeIsBool())
      {
        obj[this->json_name] = this->getBoolDefaultValue();
        this->jsonRef = obj[this->json_name];
        return this->jsonRef;
      }
      return JsonVariant();
    }

    bool ConfigItem::checkOrCreateValueInJsonObject(JsonObject &obj)
    {

      bool result = false;

      JsonVariant value = obj[this->json_name];

      if (value.isUndefined() || value.isNull())
      {
        if(this->canBeNull) // no need to create it it can be null/nonexistent
          return false;

        createInJsonObject(obj);
        return true;
      }

      this->jsonRef = value;

      if (this->typeIsChar())
      {
        if (!value.is<const char *>())
        {
          value.set((const char *)this->getCharDefaultValue());
          return true;
        }
      }

      if (this->typeIsLongInt())
      {
        if (!value.is<signed long>())
        {
          value.set(this->getLongIntDefaultValue());
          return true;
        }
      }

      if (this->typeIsBool())
      {
        if (!value.is<bool>())
        {
          value.set(this->getBoolDefaultValue());
          return true;
        }
      }

      return result;
    }

    ConfigItem::ConfigItem(const char *name,
                           SectionId section,
                           const char *default_value,
                           void (*update_callback)(),
                           bool can_be_null)
    {

      this->json_name = name;
      this->section = section;
      this->type = ConfigItemType::STRING_t;
      this->update_callback = update_callback;

      static_assert(sizeof(this->defaultValue) <= sizeof(char *), "variable size is too small");
      this->defaultValue = (void *)default_value;
    }

    ConfigItem::ConfigItem(const char *name,
                           SectionId section,
                           long int default_value,
                           void (*update_callback)(),
                           bool can_be_null)
    {
      this->json_name = name;
      this->section = section;
      this->type = ConfigItemType::LONG_INT_t;
      this->update_callback = update_callback;
      this->canBeNull = can_be_null;

      static_assert(sizeof(this->defaultValue) <= sizeof(long int), "variable size is too small");
      this->defaultValue = (void *)default_value;
    }

    ConfigItem::ConfigItem(const char *name,
                           SectionId section,
                           bool default_value,
                           void (*update_callback)(),
                           bool can_be_null)
    {
      this->json_name = name;
      this->section = section;
      this->type = ConfigItemType::BOOLEAN_t;
      this->update_callback = update_callback;
      this->canBeNull = can_be_null;

      this->boolDefaultValue = default_value;
    }

    ConfigItem::ConfigItem()
    {
      this->json_name = nullptr;
      this->section = SectionId::EOF_id;
      this->type = ConfigItemType::EOF_t;
      this->update_callback = nullptr;
      this->canBeNull = false;
    }

    void dumpConfigToString(String &destination)
    {
      //serializeJson(doc, destination);
      serializeJsonPretty(doc, destination);
    }

    void dumpConfigToSerial()
    {
      serializeJson(doc, Serial);
      Serial.println();
    }

    SectionId getSectionIdFromString(const char *name)
    {

      for (unsigned int i = 0; i < jsonSections_count; i++)
      {
        if (strcmp(name, jsonSections[i]) == 0)
          return (SectionId)i;
      }

      return SectionId::EOF_id;
    }

    JsonVariant createElementInSection(SectionId section, const char *name)
    {
      JsonVariant ret = doc.getMember(jsonSections[section]).getOrAddMember((char*)name); // casting to char* forces a copy! avoids crash!
      return ret;
    }

    bool saveConfigToFlash()
    {
      if(Signal::AsyncSignalScanner::isEnabled())
        Signal::AsyncSignalScanner::stopScanning();

      Serial.print(F("Saving JSON config to FLASH.... "));

      if (LittleFS.exists(F("/tmp.json")))
        LittleFS.remove(F("/tmp.json"));
      #ifdef ESP32
      File file = LittleFS.open(F("/tmp.json"), "w", true);
      #else
      File file = LittleFS.open(F("/tmp.json"), "w");
      #endif

      auto bytes_written = serializeJson(doc, file);
      file.close();

      if (bytes_written == 0)
      {
        Serial.println(F("failed!"));
        if(Signal::AsyncSignalScanner::isEnabled())
          Signal::AsyncSignalScanner::startScanning();
        return false;
      }
      else
      {
        if (LittleFS.exists(configFileName))
          LittleFS.remove(configFileName);
        LittleFS.rename(F("/tmp.json"), configFileName);
        Serial.println(F("OK"));
      }

      if(Signal::AsyncSignalScanner::isEnabled())
        Signal::AsyncSignalScanner::startScanning();

      return true;
    }

    void executeCliCommand(const char *cmd)
    {
      String strCmd(cmd);

      int commaIndex = strCmd.indexOf(';');

      if (commaIndex < 0)
      {
        Serial.println(F("Error : failed to find ending ';' for the command"));
        return;
      }

      String command = strCmd.substring(0, commaIndex);

      if (command.equalsIgnoreCase(commands::reset))
      {
        resetConfig();
      }
      else if (command.equalsIgnoreCase(commands::dump))
      {
        dumpConfigToSerial();
      }
      else if (command.equalsIgnoreCase(commands::set))
      {
        //String argsStr = strCmd.substring(commaIndex+1);
        DynamicJsonDocument json(2500);

        if (deserializeJson(json, cmd + commaIndex + 1) != DeserializationError::Ok)
        {
          Serial.println(F("An error occurred while reading json"));
          return;
        }

        auto root = json.as<JsonObject>();
        String msg;
        pushNewConfiguration(root, msg, false);
        if (msg.length() > 0)
        {
          Serial.println(F("Some warning/errors occured while trying to SET config from CLI:"));
          Serial.println(msg.c_str());
        }
      }
      else
      {
        Serial.printf_P(PSTR("Error : unknown command '%s'\r\n"), command.c_str());
      }
    }

  } // end of Config namespace
} // end of RFLink namespace