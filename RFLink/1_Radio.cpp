// ************************************* //
// * Arduino Project RFLink32        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#include <Arduino.h>
#include "RFLink.h"
#include "1_Radio.h"
#include "4_Display.h"
#include "2_Signal.h"


#include <SPI.h>

#ifndef RFLINK_NO_RADIOLIB_SUPPORT
#include <RadioLib.h>

Module radioLibModule(5, -1, 4, -1);
SX1278 *radio_SX1278 = nullptr;
SX1276 *radio_SX1276 = nullptr;
RF69 *radio_RFM69 = nullptr;
#endif


enum RssiThresholdTypesEnum {
  Undefined = -1,  // Keep this one first in the list
  Fixed,      // Keep this one second in the list
  Average,
  Peak,
  RssiThresholdTypes_EOF,
};

#define RssiThresholdType_default_RFM69 RssiThresholdTypesEnum::Peak
#define RssiThresholdType_default_SX127X RssiThresholdTypesEnum::Peak
#define RssiFixedThresholdValue_undefined -9999
#define RssiFixedThresholdValue_default_RFM69 6
#define RssiFixedThresholdValue_default_SX127X 6

namespace RFLink { namespace Radio  {

    HardwareType hardware = RFLink_default_Radio_HardwareType;
    bool hardwareProperlyInitialized = false;


    namespace pins {
      int8_t RX_PMOS = PIN_RF_RX_PMOS_0;
      int8_t RX_NMOS = PIN_RF_RX_NMOS_0;
      int8_t RX_VCC = PIN_RF_RX_VCC_0;
      int8_t RX_GND = PIN_RF_RX_GND_0;
      int8_t RX_NA = PIN_RF_RX_NA_0;
      int8_t RX_DATA = PIN_RF_RX_DATA_0;
      int8_t RX_RESET = PIN_RF_RX_RESET;
      int8_t RX_CS = PIN_RF_RX_CS;

      boolean PULLUP_RX_DATA = PULLUP_RF_RX_DATA_0;


      int8_t TX_PMOS = PIN_RF_TX_PMOS_0;
      int8_t TX_NMOS = PIN_RF_TX_NMOS_0;
      int8_t TX_VCC = PIN_RF_TX_VCC_0;
      int8_t TX_GND = PIN_RF_TX_GND_0;
      int8_t TX_NA = PIN_RF_TX_NA_0;
      int8_t TX_DATA = PIN_RF_TX_DATA_0;
    }

    namespace params {

      #ifndef RFLINK_NO_RADIOLIB_SUPPORT

      const int32_t default_frequency = 433920000;
      const int32_t default_BitRate = 9600;
      const int32_t default_rxBandwidth = 250000;

      int32_t frequency;
      int32_t rxBandwidth;
      int32_t bitrate;

      RssiThresholdTypesEnum rssiThresholdType = RssiThresholdTypesEnum::Undefined;
      int16_t fixedRssiThreshold = RssiFixedThresholdValue_undefined;
      #endif // RFLINK_NO_RADIOLIB_SUPPORT

    }


    States current_State = Radio_NA;

    const char * hardwareNames[] = {
            "generic",
            #ifndef RFLINK_NO_RADIOLIB_SUPPORT
            "RFM69CW",
            "RFM69HCW",
            "SX1278",
            "SX1276",
            #endif
            "EOF" // this is always the last one and matches index HardareType::HW_EOF_t
    };
#define hardwareNames_count (int)(sizeof(hardwareNames)/sizeof(char *))

    static_assert(sizeof(hardwareNames)/sizeof(char *) == HardwareType::HW_EOF_t+1, "hardwareNames has missing/extra names, please compare with HardwareType enum declarations");

// All json variable names
    const char json_name_hardware[] = "hardware";

    const char json_name_rx_data[] =  "rx_data";
    const char json_name_rx_vcc[] =   "rx_vcc";
    const char json_name_rx_nmos[] =  "rx_nmos";
    const char json_name_rx_pmos[] =  "rx_pmos";
    const char json_name_rx_gnd[] =   "rx_gnd";
    const char json_name_rx_na[] =    "rx_na";
    const char json_name_rx_reset[] = "rx_reset";
    const char json_name_rx_cs[] =    "rx_cs";

    const char json_name_tx_data[] =  "tx_data";
    const char json_name_tx_vcc[] =   "tx_vcc";
    const char json_name_tx_nmos[] =  "tx_nmos";
    const char json_name_tx_pmos[] =  "tx_pmos";
    const char json_name_tx_gnd[] =   "tx_gnd";

    const char json_name_rssi_thresh_type[] = "rssi_thres_type";
    const char json_name_rssi_thresh_value[] = "rssi_thres_value";

    const char json_name_frequency[] = "rx_freq";
    const char json_name_rx_bandwidth[] = "rx_bandwidth";
    const char json_name_bitrate[] = "rx_bitrate";


    Config::ConfigItem configItems[] =  {
            Config::ConfigItem(json_name_hardware,  Config::SectionId::Radio_id, hardwareNames[RFLink_default_Radio_HardwareType], paramsUpdatedCallback),

            #ifndef RFLINK_USE_HARDCODED_RADIO_PINS_ONLY
            Config::ConfigItem(json_name_rx_data,   Config::SectionId::Radio_id, PIN_RF_RX_DATA_0, paramsUpdatedCallback),
            Config::ConfigItem(json_name_rx_vcc,    Config::SectionId::Radio_id, PIN_RF_RX_VCC_0, paramsUpdatedCallback),
            Config::ConfigItem(json_name_rx_nmos,   Config::SectionId::Radio_id, PIN_RF_RX_NMOS_0, paramsUpdatedCallback),
            Config::ConfigItem(json_name_rx_pmos,   Config::SectionId::Radio_id, PIN_RF_RX_PMOS_0, paramsUpdatedCallback),
            Config::ConfigItem(json_name_rx_gnd,    Config::SectionId::Radio_id, PIN_RF_RX_GND_0, paramsUpdatedCallback),
            Config::ConfigItem(json_name_rx_na,     Config::SectionId::Radio_id, PIN_RF_RX_NA_0, paramsUpdatedCallback),
            Config::ConfigItem(json_name_rx_reset,  Config::SectionId::Radio_id, PIN_RF_RX_RESET, paramsUpdatedCallback),
            Config::ConfigItem(json_name_rx_cs,  Config::SectionId::Radio_id, PIN_RF_RX_CS, paramsUpdatedCallback),

            Config::ConfigItem(json_name_tx_data,   Config::SectionId::Radio_id, PIN_RF_TX_DATA_0, paramsUpdatedCallback),
            Config::ConfigItem(json_name_tx_vcc,    Config::SectionId::Radio_id, PIN_RF_TX_VCC_0, paramsUpdatedCallback),
            Config::ConfigItem(json_name_tx_nmos,   Config::SectionId::Radio_id, PIN_RF_TX_NMOS_0, paramsUpdatedCallback),
            Config::ConfigItem(json_name_tx_pmos,   Config::SectionId::Radio_id, PIN_RF_TX_PMOS_0, paramsUpdatedCallback),
            Config::ConfigItem(json_name_tx_gnd,    Config::SectionId::Radio_id, PIN_RF_TX_GND_0, paramsUpdatedCallback),
            #endif // RFLINK_USE_HARDCODED_RADIO_PINS_ONLY

            #ifndef RFLINK_NO_RADIOLIB_SUPPORT
            Config::ConfigItem(json_name_rssi_thresh_type,  Config::SectionId::Radio_id, RssiThresholdTypesEnum::Undefined, paramsUpdatedCallback, true),
            Config::ConfigItem(json_name_rssi_thresh_value, Config::SectionId::Radio_id, RssiFixedThresholdValue_undefined, paramsUpdatedCallback, true),

            Config::ConfigItem(json_name_frequency,   Config::SectionId::Radio_id, params::default_frequency, paramsUpdatedCallback, true),
            Config::ConfigItem(json_name_rx_bandwidth,Config::SectionId::Radio_id, params::default_rxBandwidth, paramsUpdatedCallback, true),
            Config::ConfigItem(json_name_bitrate,     Config::SectionId::Radio_id, params::default_BitRate, paramsUpdatedCallback, true),
            #endif

            Config::ConfigItem()
    };

    void refreshParametersFromConfig() {
      States savedState = current_State;
      HardwareType newHardwareId = hardware;

      //yield();

      // let's save radio state before we shut it down
      if( savedState != States::Radio_OFF && savedState != States::Radio_NA) {
        set_Radio_mode(States::Radio_OFF);
        //Serial.println("Radio was active before config change was requested");
      }

      Config::ConfigItem *item;
      bool changesDetected = false;

      item = Config::findConfigItem(json_name_hardware, Config::SectionId::Radio_id);
      if( strcmp(hardwareNames[hardware], item->getCharValue()) != 0) {
        newHardwareId =  hardwareIDFromString(item->getCharValue());
        if(newHardwareId == HardwareType::HW_EOF_t ) {
          Serial.printf_P(PSTR("Unsupported radio hardware name '%s' was provided, falling back to default generic receiver!\r\n"), item->getCharValue());
          changesDetected = true;
          newHardwareId = HardwareType::HW_basic_t;
          item->setCharValue(hardwareNames[newHardwareId]);
        }
        else if(newHardwareId != hardware) {
          changesDetected = true;
        }
        // changes will be applied at the end of this function
      }

      #ifndef RFLINK_USE_HARDCODED_RADIO_PINS_ONLY

      item = Config::findConfigItem(json_name_rx_data, Config::SectionId::Radio_id);
      if( pins::RX_DATA != item->getLongIntValue() ) {
        changesDetected = true;
        pins::RX_DATA = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_rx_vcc, Config::SectionId::Radio_id);
      if( pins::RX_VCC != item->getLongIntValue() ) {
        changesDetected = true;
        pins::RX_VCC = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_rx_nmos, Config::SectionId::Radio_id);
      if( pins::RX_NMOS != item->getLongIntValue() ) {
        changesDetected = true;
        pins::RX_NMOS = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_rx_pmos, Config::SectionId::Radio_id);
      if( pins::RX_PMOS != item->getLongIntValue() ) {
        changesDetected = true;
        pins::RX_PMOS = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_rx_gnd, Config::SectionId::Radio_id);
      if( pins::RX_GND != item->getLongIntValue() ) {
        changesDetected = true;
        pins::RX_GND = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_rx_na, Config::SectionId::Radio_id);
      if( pins::RX_NA != item->getLongIntValue() ) {
        changesDetected = true;
        pins::RX_NA = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_rx_reset, Config::SectionId::Radio_id);
      if( pins::RX_RESET != item->getLongIntValue() ) {
        changesDetected = true;
        pins::RX_RESET = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_rx_cs, Config::SectionId::Radio_id);
      if( pins::RX_CS != item->getLongIntValue() ) {
        changesDetected = true;
        pins::RX_CS = item->getLongIntValue();
      }

      // TX

      item = Config::findConfigItem(json_name_tx_data, Config::SectionId::Radio_id);
      if( pins::TX_DATA != item->getLongIntValue() ) {
        changesDetected = true;
        pins::TX_DATA = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_tx_vcc, Config::SectionId::Radio_id);
      if( pins::TX_VCC != item->getLongIntValue() ) {
        changesDetected = true;
        pins::TX_VCC = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_tx_nmos, Config::SectionId::Radio_id);
      if( pins::TX_NMOS != item->getLongIntValue() ) {
        changesDetected = true;
        pins::TX_NMOS = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_tx_pmos, Config::SectionId::Radio_id);
      if( pins::TX_PMOS != item->getLongIntValue() ) {
        changesDetected = true;
        pins::TX_PMOS = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_tx_gnd, Config::SectionId::Radio_id);
      if( pins::TX_GND != item->getLongIntValue() ) {
        changesDetected = true;
        pins::TX_GND = item->getLongIntValue();
      }
      #endif // RFLINK_USE_HARDCODED_RADIO_PINS_ONLY

      #ifndef RFLINK_NO_RADIOLIB_SUPPORT
      long int value;

      item = Config::findConfigItem(json_name_rssi_thresh_type, Config::SectionId::Radio_id);
      if(item->isUndefined()){
        if(params::rssiThresholdType != RssiThresholdTypesEnum::Undefined)
          changesDetected = true;
        params::rssiThresholdType = RssiThresholdTypesEnum::Undefined;
      }
      else {
        value = item->getLongIntValue();
        if (value < RssiThresholdTypesEnum::Undefined || value >= RssiThresholdTypesEnum::RssiThresholdTypes_EOF ) {
          Serial.println(F("Invalid RssiThresholdType provided, resetting to default value"));
          if(item->canBeNull) {
            item->deleteJsonRecord();
          }
          else {
            item->setLongIntValue(item->getLongIntDefaultValue());
          }
          params::rssiThresholdType = RssiThresholdTypesEnum::Undefined;
          changesDetected = true;
        } else if (params::rssiThresholdType != value) {
          changesDetected = true;
          params::rssiThresholdType = (RssiThresholdTypesEnum) value;
        }
      }

      item = Config::findConfigItem(json_name_rssi_thresh_value, Config::SectionId::Radio_id);
      if(item->isUndefined()){
        if( params::fixedRssiThreshold != RssiFixedThresholdValue_undefined )
          changesDetected = true;
        params::fixedRssiThreshold = RssiFixedThresholdValue_undefined;
      }
      else {
        value = item->getLongIntValue();
        if (value <= 0 || value > 128 ) {
          Serial.println(F("Invalid RssiFixedThresholdValue provided, resetting to default value"));
          if(item->canBeNull) {
            item->deleteJsonRecord();
          }
          else
            item->setLongIntValue(item->getLongIntDefaultValue());
          changesDetected = true;
          params::fixedRssiThreshold = RssiFixedThresholdValue_undefined;
        } else if (params::fixedRssiThreshold != value) {
          changesDetected = true;
          params::fixedRssiThreshold = value;
        }
      }


      item = Config::findConfigItem(json_name_frequency, Config::SectionId::Radio_id);
      if(item->isUndefined()){
        if( params::frequency != params::default_frequency )
          changesDetected = true;
        params::frequency = params::default_frequency;
      }
      else {
        value = item->getLongIntValue();
        if (value < 0 || value > 1000000000 ) {
          Serial.println(F("Invalid frequency provided, resetting to default value"));
          if(item->canBeNull) {
            item->deleteJsonRecord();
          }
          else
            item->setLongIntValue(params::default_frequency);
          params::frequency = params::default_frequency;
          changesDetected = true;
        } else if (params::frequency != value) {
          changesDetected = true;
          params::frequency = value;
        }
      }


      item = Config::findConfigItem(json_name_rx_bandwidth, Config::SectionId::Radio_id);
      if(item->isUndefined()){
        if( params::rxBandwidth != params::default_rxBandwidth )
          changesDetected = true;
        params::rxBandwidth = params::default_rxBandwidth;
      }
      else {
        value = item->getLongIntValue();
        if (value < 0 || value > 500000 ) {
          Serial.println(F("Invalid rxBandwidth provided, resetting to default value"));
          if(item->canBeNull) {
            item->deleteJsonRecord();
          }
          else
            item->setLongIntValue(params::default_rxBandwidth);
          params::rxBandwidth = params::default_rxBandwidth;
          changesDetected = true;
        } else if (params::rxBandwidth != value) {
          changesDetected = true;
          params::rxBandwidth = value;
        }
      }


      item = Config::findConfigItem(json_name_bitrate, Config::SectionId::Radio_id);
      if(item->isUndefined()){
        if( params::bitrate != params::default_BitRate )
          changesDetected = true;
        params::bitrate = params::default_BitRate;
      }
      else {
        value = item->getLongIntValue();
        if (value < 0 || value > 500000 ) {
          Serial.println(F("Invalid BitRate provided, resetting to default value"));
          if(item->canBeNull) {
            item->deleteJsonRecord();
          }
          else
            item->setLongIntValue(params::default_BitRate);
          params::bitrate = params::default_BitRate;
          changesDetected = true;
        } else if (params::bitrate != value) {
          changesDetected = true;
          params::bitrate = value;
        }
      }
      #endif // #ifndef RFLINK_NO_RADIOLIB_SUPPORT


      if(changesDetected)
        initializeHardware(newHardwareId, true);

      // restore to RX state
      if( savedState != States::Radio_OFF && savedState != States::Radio_NA) {
        set_Radio_mode(States::Radio_RX);
        //Serial.println("Changes applied, bringing radio back to RX mode");
      }

    }

    void paramsUpdatedCallback() {
      refreshParametersFromConfig();
    }

    #ifndef RFLINK_NO_RADIOLIB_SUPPORT
    int32_t getFrequency() 
    {
      return params::frequency;
    }

    /// Sets the frequency of the transceiver, and returns the previously set frequency
    int32_t setFrequency(int32_t newFrequency)
    {
      int32_t result = getFrequency();
      switch(hardware)  
      {
        #ifndef RFLINK_NO_RADIOLIB_SUPPORT
        case HardwareType::HW_RFM69CW_t:
        case HardwareType::HW_RFM69HCW_t:
          radio_RFM69->setFrequency(newFrequency / 1000000.0);
          break;
        case HardwareType::HW_SX1278_t:
          radio_SX1278->setFrequency(newFrequency / 1000000.0);
          break;
        case HardwareType::HW_SX1276_t:
          radio_SX1276->setFrequency(newFrequency / 1000000.0);
          break;
        #endif

        default:
          return 0;  // other hardware cannot change its frequency
      }
      params::frequency = newFrequency;
      return result;
    }
    #endif // #ifndef RFLINK_NO_RADIOLIB_SUPPORT

    HardwareType hardwareIDFromString(const char *name) {
      for(int i=0; i<hardwareNames_count; i++) {
        if(strcmp(hardwareNames[i], name) == 0)
          return (HardwareType) i;
      }

      return HardwareType::HW_EOF_t;
    }

    void set_Radio_mode_generic(States new_State, bool force)
    {
      if (current_State != new_State || force)
      {
        switch (new_State)
        {
          case Radio_OFF:
            disableTX_generic();
            disableRX_generic();
            break;

          case Radio_RX:
            disableTX_generic();
            enableRX_generic();
            break;

          case Radio_TX:
            disableRX_generic();
            enableTX_generic();
            break;

          case Radio_NA:
            break;
        }
        current_State = new_State;
      }
    }

#if (defined(ESP8266) || defined(ESP32))
    void show_Radio_Pin()
    {
      if (pins::RX_PMOS != NOT_A_PIN)
      {
        Serial.print(F("Radio pin RF_RX_PMOS :\t"));
        Serial.println(GPIO2String(pins::RX_PMOS));
      }
      if (pins::RX_NMOS != NOT_A_PIN)
      {
        Serial.print(F("Radio pin RF_RX_NMOS :\t"));
        Serial.println(GPIO2String(pins::RX_NMOS));
      }
      if (pins::RX_VCC != NOT_A_PIN)
      {
        Serial.print(F("Radio pin RF_RX_VCC :\t"));
        Serial.println(GPIO2String(pins::RX_VCC));
      }
      if (pins::RX_GND != NOT_A_PIN)
      {
        Serial.print(F("Radio pin RF_RX_GND :\t"));
        Serial.println(GPIO2String(pins::RX_GND));
      }
      if (pins::RX_NA != NOT_A_PIN)
      {
        Serial.print(F("Radio pin RF_RX_NA :\t"));
        Serial.println(GPIO2String(pins::RX_NA));
      }
      if (pins::RX_DATA != NOT_A_PIN)
      {
        Serial.print(F("Radio pin RF_RX_DATA :\t"));
        Serial.print(GPIO2String(pins::RX_DATA));
        if (pins::PULLUP_RX_DATA)
          Serial.println(F(" (Pullup enabled)"));
        else
          Serial.println();
      }
      if (pins::RX_CS != NOT_A_PIN)
      {
        Serial.print(F("Radio pin RF_RX_CS :\t"));
        Serial.println(GPIO2String(pins::RX_CS));
      }
      if (pins::RX_RESET != NOT_A_PIN)
      {
        Serial.print(F("Radio pin RF_RX_RESET :\t"));
        Serial.println(GPIO2String(pins::RX_RESET));
      }
      //
      if (pins::TX_PMOS != NOT_A_PIN)
      {
        Serial.print(F("Radio pin RF_TX_PMOS :\t"));
        Serial.println(GPIO2String(pins::TX_PMOS));
      }
      if (pins::TX_NMOS != NOT_A_PIN)
      {
        Serial.print(F("Radio pin RF_TX_NMOS :\t"));
        Serial.println(GPIO2String(pins::TX_NMOS));
      }
      if (pins::TX_VCC != NOT_A_PIN)
      {
        Serial.print(F("Radio pin RF_TX_VCC :\t"));
        Serial.println(GPIO2String(pins::TX_VCC));
      }
      if (pins::TX_GND != NOT_A_PIN)
      {
        Serial.print(F("Radio pin RF_TX_GND :\t"));
        Serial.println(GPIO2String(pins::TX_GND));
      }
      if (pins::TX_DATA != NOT_A_PIN)
      {
        Serial.print(F("Radio pin RF_TX_DATA :\t"));
        Serial.println(GPIO2String(pins::TX_DATA));
      }
      if (pins::TX_NA != NOT_A_PIN)
      {
        Serial.print(F("Radio pin RF_TX_NA :\t"));
        Serial.println(GPIO2String(pins::TX_NA));
      }
    }
#endif // ESP8266 || ESP32

    void set_Radio_mode(States new_State, bool force)
    {
      if(hardware == HardwareType::HW_basic_t)
        set_Radio_mode_generic(new_State, force);
      #ifndef RFLINK_NO_RADIOLIB_SUPPORT
      else if( hardware == HardwareType::HW_RFM69CW_t || hardware == HardwareType::HW_RFM69HCW_t )
        set_Radio_mode_RFM69(new_State, force);
      else if( hardware == HardwareType::HW_SX1278_t )
        set_Radio_mode_SX1278(new_State, force);
      else if( hardware == HardwareType::HW_SX1276_t )
        set_Radio_mode_SX1276(new_State, force);
      #endif // RFLINK_NO_RADIOLIB_SUPPORT
      else
        Serial.printf_P(PSTR("Error while trying to switch Radio state: unknown hardware id '%i'\r\n"), new_State);
    }

    void setup() {
      refreshParametersFromConfig();
    }

    void enableRX_generic()
    {
      // RX pins
      pinMode(pins::RX_NA, INPUT);       // Initialise in/output ports
      pinMode(pins::RX_DATA, INPUT);     // Initialise in/output ports
      pinMode(pins::RX_NMOS, OUTPUT);    // MOSFET, always output
      pinMode(pins::RX_PMOS, OUTPUT);    // MOSFET, always output
      digitalWrite(pins::RX_NMOS, HIGH); // turn GND to RF receiver ON
      digitalWrite(pins::RX_PMOS, LOW);  // turn VCC to RF receiver ON
      pinMode(pins::RX_GND, OUTPUT);     // Initialise in/output ports
      pinMode(pins::RX_VCC, OUTPUT);     // Initialise in/output ports
      digitalWrite(pins::RX_GND, LOW);   // turn GND to RF receiver ON
      digitalWrite(pins::RX_VCC, HIGH);  // turn VCC to RF receiver ON
      if (pins::PULLUP_RX_DATA)
        pinMode(pins::RX_DATA, INPUT_PULLUP); // Initialise in/output ports
      delayMicroseconds(TRANSMITTER_STABLE_DELAY_US);
      if(RFLink::Signal::params::async_mode_enabled)
        RFLink::Signal::AsyncSignalScanner::startScanning();

    }

    void disableRX_generic()
    {
      if( RFLink::Signal::params::async_mode_enabled )
        RFLink::Signal::AsyncSignalScanner::stopScanning();

      // RX pins
      pinMode(pins::RX_DATA, INPUT);
      pinMode(pins::RX_NA, INPUT);
      pinMode(pins::RX_PMOS, OUTPUT);    // MOSFET, always output
      pinMode(pins::RX_NMOS, OUTPUT);    // MOSFET, always output
      digitalWrite(pins::RX_PMOS, HIGH); // turn VCC to RF receiver OFF
      digitalWrite(pins::RX_NMOS, LOW);  // turn GND to RF receiver OFF
      pinMode(pins::RX_VCC, INPUT);
      pinMode(pins::RX_GND, INPUT);
    }

    void enableTX_generic()
    {
      // TX Pins
      pinMode(pins::TX_NA, INPUT);       // Initialise in/output ports
      pinMode(pins::TX_DATA, OUTPUT);    // Initialise in/output ports
      digitalWrite(pins::TX_DATA, LOW);  // No signal yet
      pinMode(pins::TX_NMOS, OUTPUT);    // MOSFET, always output
      pinMode(pins::TX_PMOS, OUTPUT);    // MOSFET, always output
      digitalWrite(pins::TX_NMOS, HIGH); // turn GND to TX receiver ON
      digitalWrite(pins::TX_PMOS, LOW);  // turn VCC to TX receiver ON
      pinMode(pins::TX_GND, OUTPUT);     // Initialise in/output ports
      pinMode(pins::TX_VCC, OUTPUT);     // Initialise in/output ports
      digitalWrite(pins::TX_GND, LOW);   // turn GND to TX receiver ON
      digitalWrite(pins::TX_VCC, HIGH);  // turn VCC to TX receiver ON
      delayMicroseconds(TRANSMITTER_STABLE_DELAY_US);
    }

    void disableTX_generic()
    {
      // TX Pins
      delayMicroseconds(TRANSMITTER_STABLE_DELAY_US);
      digitalWrite(pins::TX_DATA, LOW);  // No more signal
      pinMode(pins::TX_DATA, INPUT);     //
      pinMode(pins::TX_NA, INPUT);       //
      pinMode(pins::TX_NMOS, OUTPUT);    // MOSFET, always output
      pinMode(pins::TX_PMOS, OUTPUT);    // MOSFET, always output
      digitalWrite(pins::TX_PMOS, HIGH); // turn VCC to TX receiver OFF
      digitalWrite(pins::TX_NMOS, LOW);  // turn GND to TX receiver OFF
      pinMode(pins::TX_VCC, INPUT);
      pinMode(pins::TX_GND, INPUT);
    }

    #ifndef RFLINK_NO_RADIOLIB_SUPPORT
    void set_Radio_mode_SX1278(States new_State, bool force)
    {
      // @TODO : review compatibility with ASYNC mode
      if (current_State != new_State || force)
      {
        switch (new_State)
        {
          case Radio_OFF: {
            if( RFLink::Signal::params::async_mode_enabled )
              RFLink::Signal::AsyncSignalScanner::stopScanning();

            auto success = radio_SX1278->standby();
            if(success != 0) {
              Serial.printf_P(PSTR("Failed to switch to standby mode (code=%i), we will try to reinitialize it later"), (int) success);
              hardwareProperlyInitialized = false;
            }
            break;
          }

          case Radio_RX: {

            auto success = radio_SX1278->receiveDirect();
            if(success != 0) {
              Serial.printf_P(PSTR("Failed to put hardware in RX mode (code=%i), we will try to reinitialize it later"), (int) success);
              hardwareProperlyInitialized = false;
            }

            pinMode(pins::RX_DATA, INPUT);

            if( RFLink::Signal::params::async_mode_enabled )
              RFLink::Signal::AsyncSignalScanner::startScanning();

            break;
          }

          case Radio_TX: {

            if( RFLink::Signal::params::async_mode_enabled )
              RFLink::Signal::AsyncSignalScanner::stopScanning();

            pinMode(pins::TX_DATA, OUTPUT);

            auto success = radio_SX1278->transmitDirect();
            if(success != 0) {
              Serial.printf_P(PSTR("Failed to put hardware in TX mode (code=%i), we will try to reinitialize it later"), (int) success);
              hardwareProperlyInitialized = false;
            }

            success = radio_SX1278->setOutputPower(13);
            if(success != 0) {
              Serial.printf_P(PSTR("Failed setup hardware TX power (code=%i), we will try to reinitialize it later"), (int) success);
              hardwareProperlyInitialized = false;
            }

            break;
          }

          case Radio_NA:
            break;
        }

        current_State = new_State;
      }
    }

    void set_Radio_mode_SX1276(States new_State, bool force)
    {
      // @TODO : review compatibility with ASYNC mode
      if (current_State != new_State || force)
      {
        switch (new_State)
        {
          case Radio_OFF: {
            if( RFLink::Signal::params::async_mode_enabled )
              RFLink::Signal::AsyncSignalScanner::stopScanning();

            auto success = radio_SX1276->standby();
            if(success != 0) {
              Serial.printf_P(PSTR("Failed to switch to standby mode (code=%i), we will try to reinitialize it later"), (int) success);
              hardwareProperlyInitialized = false;
            }
            break;
          }

          case Radio_RX: {

            auto success = radio_SX1276->receiveDirect();
            if(success != 0) {
              Serial.printf_P(PSTR("Failed to put hardware in RX mode (code=%i), we will try to reinitialize it later"), (int) success);
              hardwareProperlyInitialized = false;
            }

            pinMode(pins::RX_DATA, INPUT);

            if( RFLink::Signal::params::async_mode_enabled )
              RFLink::Signal::AsyncSignalScanner::startScanning();

            break;
          }

          case Radio_TX: {

            if( RFLink::Signal::params::async_mode_enabled )
              RFLink::Signal::AsyncSignalScanner::stopScanning();

            pinMode(pins::TX_DATA, OUTPUT);

            auto success = radio_SX1276->transmitDirect();
            if(success != 0) {
              Serial.printf_P(PSTR("Failed to put hardware in TX mode (code=%i), we will try to reinitialize it later"), (int) success);
              hardwareProperlyInitialized = false;
            }

            success = radio_SX1276->setOutputPower(13);
            if(success != 0) {
              Serial.printf_P(PSTR("Failed setup hardware TX power (code=%i), we will try to reinitialize it later"), (int) success);
              hardwareProperlyInitialized = false;
            }

            break;
          }

          case Radio_NA:
            break;
        }

        current_State = new_State;
      }
    }

    void set_Radio_mode_RFM69(States new_State, bool force)
    {
      // @TODO : review compatibility with ASYNC mode
      if (current_State != new_State || force)
      {
        switch (new_State)
        {
          case Radio_OFF: {
            if( RFLink::Signal::params::async_mode_enabled )
              RFLink::Signal::AsyncSignalScanner::stopScanning();

            auto success = radio_RFM69->standby();
            if(success != 0 ){
              Serial.printf_P(PSTR("Failed to switch to standby mode (code=%i), we will try to reinitialize it later\r\n"), (int) success);
              hardwareProperlyInitialized = false;
            }
            break;
          }

          case Radio_RX: {

            auto success = radio_RFM69->receiveDirect();
            if(success != 0 ) {
              Serial.printf_P(PSTR("ERROR: RFM69 receiveDirect()=%i, we will try to reinitialize it later\r\n"), (int) success);
              hardwareProperlyInitialized = false;
            }

            success = radio_RFM69->disableContinuousModeBitSync();
            if(success != 0 ) {
              Serial.printf_P(PSTR("ERROR: RFM69 disableContinuousModeBitSync()=%i, we will try to reinitialize it later\r\n"), (int) success);
              hardwareProperlyInitialized = false;
            }

            pinMode(pins::RX_DATA, INPUT);

            if( RFLink::Signal::params::async_mode_enabled )
              RFLink::Signal::AsyncSignalScanner::startScanning();

            break;
          }

          case Radio_TX: {

            if( RFLink::Signal::params::async_mode_enabled )
              RFLink::Signal::AsyncSignalScanner::stopScanning();

            pinMode(pins::TX_DATA, OUTPUT);

            auto success = radio_RFM69->transmitDirect();
            if(success != 0 ) {
              Serial.printf_P(PSTR("Failed to switch to TX mode (code=%i), we will try to reinitialize it later\r\n"), (int) success);
              hardwareProperlyInitialized = false;
            }

            if(hardware == HardwareType::HW_RFM69HCW_t)
              radio_RFM69->setOutputPower(13, true);
            else
              radio_RFM69->setOutputPower(13, false);

            break;
          }

          case Radio_NA:
            break;
        }
        current_State = new_State;
      }
    }
    #endif // RFLINK_NO_RADIOLIB_SUPPORT


    float getCurrentRssi() {
      #ifndef RFLINK_NO_RADIOLIB_SUPPORT
      if(hardware == HardwareType::HW_SX1278_t)
        return radio_SX1278->getRSSI(true);
      if(hardware == HardwareType::HW_SX1276_t)
        return radio_SX1276->getRSSI(true);
      if(hardware == HardwareType::HW_RFM69CW_t || hardware == HardwareType::HW_RFM69HCW_t)
        return radio_RFM69->getRSSI();
      #endif

      return -9999.0F;
    }

    void initializeHardware(HardwareType newHardware, bool force) {

      if(newHardware == hardware && !force && hardwareProperlyInitialized) {
        RFLink::sendRawPrint(F("initializeHardware() requested but new and old hardware are the same\r\n"));
        return;
      }

      hardwareProperlyInitialized = false;

      if( newHardware != hardware ) {
        sprintf(printBuf, PSTR("Switching from Radio hardware '%s' to '%s'"), hardwareNames[hardware], hardwareNames[newHardware]);
        RFLink::sendRawPrint(printBuf, true);
      } else {
        sprintf(printBuf, PSTR("Now trying to initialize hardware '%s"), hardwareNames[hardware]);
        RFLink::sendRawPrint(printBuf, true);
      }


      bool success = false;
      hardware = newHardware;

      if(newHardware == HardwareType::HW_basic_t){
        success = true;
      }
      #ifndef RFLINK_NO_RADIOLIB_SUPPORT
      else if(newHardware == HardwareType::HW_SX1278_t){
        success = initialize_SX1278();
      }
      else if(newHardware == HardwareType::HW_SX1276_t){
        success = initialize_SX1276();
      }
      else if(newHardware == HardwareType::HW_RFM69CW_t || newHardware == HardwareType::HW_RFM69HCW_t){
        success = initialize_RFM69();
      }
      #endif
      else {
        RFLink::sendRawPrint(F("Unsupported hardwareId="));
        RFLink::sendRawPrint((int)newHardware);
        RFLink::sendRawPrintln();
      }

      if(!success) {
        RFLink::sendRawPrint(F("Hardware failed to initialize, we will retry later!"), true);
      } else {
        RFLink::sendRawPrint(F("Hardware initialization was successful!"), true);
        hardwareProperlyInitialized = true;
        Radio::set_Radio_mode(Radio::current_State, true);
      }
    }

    #ifndef RFLINK_NO_RADIOLIB_SUPPORT
    bool initialize_SX1278() {
      radioLibModule = Module(pins::RX_CS, -1, pins::RX_RESET, -1);
      if(radio_SX1278 == nullptr)
        radio_SX1278 = new SX1278(&radioLibModule);
      else
        *radio_SX1278 = &radioLibModule;

      int finalResult = 0;

      auto result = radio_SX1278->beginFSK( (float)params::frequency/1000000,
                                            (float)params::bitrate/1000,
                                            50.0F,
                                            (float)params::rxBandwidth/1000,
                                            12, 16, true);
      Serial.printf_P(PSTR("Initialized SX1278(freq=%.2fMhz,br=%.3fkbps,rxbw=%.1fkhz)=%i\r\n"),
                      (float)params::frequency/1000000,
                      (float)params::bitrate/1000,
                      (float)params::rxBandwidth/1000,
                      result);

      finalResult |= result;

      result = radio_SX1278->setOOK(true);
      Serial.printf_P(PSTR("SX1278 setOOK=%i\r\n"), result);
      finalResult |= result;

      result = radio_SX1278->setEncoding(RADIOLIB_ENCODING_NRZ);
      Serial.printf_P(PSTR("SX1278 set encoding result=%i\r\n"), result);
      finalResult |= result;

      result = radio_SX1278->setDataShapingOOK(0);
      Serial.printf_P(PSTR("SX1278 set data shaping result=%i\r\n"), result);
      finalResult |= result;

      RssiThresholdTypesEnum newType = RssiThresholdType_default_SX127X;
      if(params::rssiThresholdType != RssiThresholdTypesEnum::Undefined)
        newType = params::rssiThresholdType;
      if(newType == RssiThresholdTypesEnum::Peak)
        result = radio_SX1278->setOokThresholdType(RADIOLIB_SX127X_OOK_THRESH_PEAK);
      else if(newType == RssiThresholdTypesEnum::Fixed)
        result = radio_SX1278->setOokThresholdType(RADIOLIB_SX127X_OOK_THRESH_FIXED);
      else if(newType == RssiThresholdTypesEnum::Average)
        result = radio_SX1278->setOokThresholdType(RADIOLIB_SX127X_OOK_THRESH_AVERAGE);
      Serial.printf_P(PSTR("SX1278 setOokThresholdType(%i)=%i\r\n"), (int) newType, result);
      finalResult |= result;

      uint16_t newValue = RssiFixedThresholdValue_default_SX127X;
      if(params::fixedRssiThreshold != RssiFixedThresholdValue_undefined)
        newValue = params::fixedRssiThreshold;
      newValue = newValue*2;
      result = radio_SX1278->setOokFixedOrFloorThreshold(newValue);
      Serial.printf_P(PSTR("SX1278 setOokFixedThreshold(0x%.2X)=%i\r\n"), (int) newValue, result);
      finalResult |= result;

      result = radio_SX1278->setOokPeakThresholdDecrement(RADIOLIB_SX127X_OOK_PEAK_THRESH_DEC_1_8_CHIP);
      Serial.printf_P(PSTR("SX1278 setOokPeakThresholdDecrement() result=%i\r\n"), result);
      finalResult |= result;

      result = radio_SX1278->disableBitSync();
      Serial.printf_P(PSTR("SX1278 disableBitSync() result=%i\r\n"), result);
      finalResult |= result;

      result = radio_SX1278->setGain(6);
      Serial.printf_P(PSTR("SX1278 setGain() result=%i\r\n"), result);
      finalResult |= result;

      //result = radio_SX1278->setFrequencyDeviation(200.0F);
      //Serial.printf_P(PSTR("SX1278 setFrequencyDeviation() result=%i\r\n"), result);
      //finalResult |= result;

      return finalResult == 0;
    }

    bool initialize_SX1276() {
      radioLibModule = Module(pins::RX_CS, -1, pins::RX_RESET, -1);
      if(radio_SX1276 == nullptr)
        radio_SX1276 = new SX1276(&radioLibModule);
      else
        *radio_SX1276 = &radioLibModule;

      int finalResult = 0;

      auto result = radio_SX1276->beginFSK( (float)params::frequency/1000000,
                                            (float)params::bitrate/1000,
                                            50.0F,
                                            (float)params::rxBandwidth/1000,
                                            12, 16, true);
      Serial.printf_P(PSTR("Initialized SX1276(freq=%.2fMhz,br=%.3fkbps,rxbw=%.1fkhz)=%i\r\n"),
                      (float)params::frequency/1000000,
                      (float)params::bitrate/1000,
                      (float)params::rxBandwidth/1000,
                      result);

      finalResult |= result;

      result = radio_SX1276->setOOK(true);
      Serial.printf_P(PSTR("SX1276 setOOK=%i\r\n"), result);
      finalResult |= result;

      result = radio_SX1276->setEncoding(RADIOLIB_ENCODING_NRZ);
      Serial.printf_P(PSTR("SX1276 set encoding result=%i\r\n"), result);
      finalResult |= result;

      result = radio_SX1276->setDataShapingOOK(0);
      Serial.printf_P(PSTR("SX1276 set data shaping result=%i\r\n"), result);
      finalResult |= result;

      RssiThresholdTypesEnum newType = RssiThresholdType_default_SX127X;
      if(params::rssiThresholdType != RssiThresholdTypesEnum::Undefined)
        newType = params::rssiThresholdType;
      if(newType == RssiThresholdTypesEnum::Peak)
        result = radio_SX1276->setOokThresholdType(RADIOLIB_SX127X_OOK_THRESH_PEAK);
      else if(newType == RssiThresholdTypesEnum::Fixed)
        result = radio_SX1276->setOokThresholdType(RADIOLIB_SX127X_OOK_THRESH_FIXED);
      else if(newType == RssiThresholdTypesEnum::Average)
        result = radio_SX1276->setOokThresholdType(RADIOLIB_SX127X_OOK_THRESH_AVERAGE);
      Serial.printf_P(PSTR("SX1276 setOokThresholdType(%i)=%i\r\n"), (int) newType, result);
      finalResult |= result;

      uint16_t newValue = RssiFixedThresholdValue_default_SX127X;
      if(params::fixedRssiThreshold != RssiFixedThresholdValue_undefined)
        newValue = params::fixedRssiThreshold;
      newValue = newValue*2;
      result = radio_SX1276->setOokFixedOrFloorThreshold(newValue);
      Serial.printf_P(PSTR("SX1276 setOokFixedThreshold(0x%.2X)=%i\r\n"), (int) newValue, result);
      finalResult |= result;

      result = radio_SX1276->setOokPeakThresholdDecrement(RADIOLIB_SX127X_OOK_PEAK_THRESH_DEC_1_8_CHIP);
      Serial.printf_P(PSTR("SX1276 setOokPeakThresholdDecrement() result=%i\r\n"), result);
      finalResult |= result;

      result = radio_SX1276->disableBitSync();
      Serial.printf_P(PSTR("SX1276 disableBitSync() result=%i\r\n"), result);
      finalResult |= result;

      result = radio_SX1276->setGain(6);
      Serial.printf_P(PSTR("SX1276 setGain() result=%i\r\n"), result);
      finalResult |= result;

      //result = radio_SX1276->setFrequencyDeviation(200.0F);
      //Serial.printf_P(PSTR("SX1276 setFrequencyDeviation() result=%i\r\n"), result);
      //finalResult |= result;

      return finalResult == 0;
    }

    bool initialize_RFM69() {

      radioLibModule = Module(pins::RX_CS, -1, pins::RX_RESET, -1);
      if(radio_RFM69 == nullptr)
        radio_RFM69 = new RF69(&radioLibModule);
      else
        *radio_RFM69 = &radioLibModule;

      int finalResult = 0;

      auto result = radio_RFM69->begin( (float)params::frequency/1000000,
                                            (float)params::bitrate/1000,
                                            50.0F,
                                            (float)params::rxBandwidth/1000,
                                            12, 16);
      Serial.printf_P(PSTR("Initialized RFM69(freq=%.2fMhz,br=%.3fkbps,rxbw=%.1fkhz)=%i\r\n"),
                      (float)params::frequency/1000000,
                      (float)params::bitrate/1000,
                      (float)params::rxBandwidth/1000,
                      result);
      finalResult |= result;

      result = radio_RFM69->setOOK(true);
      Serial.printf_P(PSTR("RFM69 SetOOK()=%i\r\n"), result);
      finalResult |= result;

      result = radio_RFM69->setDataShaping(RADIOLIB_SHAPING_NONE);
      Serial.printf_P(PSTR("RFM69 setDataShapingOOK()=%i\r\n"), result);
      finalResult |= result;

      result = radio_RFM69->setEncoding(RADIOLIB_ENCODING_NRZ);
      Serial.printf_P(PSTR("RFM69 setEncoding()=%i\r\n"), result);
      finalResult |= result;


      RssiThresholdTypesEnum newType = RssiThresholdType_default_RFM69;
      if(params::rssiThresholdType != RssiThresholdTypesEnum::Undefined)
        newType = params::rssiThresholdType;
      if(newType == RssiThresholdTypesEnum::Peak)
        result = radio_RFM69->setOokThresholdType(RADIOLIB_RF69_OOK_THRESH_PEAK);
      else if(newType == RssiThresholdTypesEnum::Fixed)
        result = radio_RFM69->setOokThresholdType(RADIOLIB_RF69_OOK_THRESH_FIXED);
      else if(newType == RssiThresholdTypesEnum::Average)
        result = radio_RFM69->setOokThresholdType(RADIOLIB_RF69_OOK_THRESH_AVERAGE);
      Serial.printf_P(PSTR("RFM69 setOokThresholdType(%i)=%i\r\n"), (int) newType, result);
      finalResult |= result;

      uint16_t newValue = RssiFixedThresholdValue_default_RFM69;
      if(params::fixedRssiThreshold != RssiFixedThresholdValue_undefined)
        newValue = params::fixedRssiThreshold;
      result = radio_RFM69->setOokFixedThreshold(newValue);
      Serial.printf_P(PSTR("RFM69 setOokFixedThreshold(0x%.2X)=%i\r\n"), (int) newValue, result);
      finalResult |= result;

      result = radio_RFM69->setOokPeakThresholdDecrement(RADIOLIB_RF69_OOK_PEAK_THRESH_DEC_1_8_CHIP);
      Serial.printf_P(PSTR("RFM69 setOokPeakThresholdDecrement() result=%i\r\n"), result);
      finalResult |= result;

      result = radio_RFM69->setLnaTestBoost(true);
      Serial.printf_P(PSTR("RFM69 setLnaTestBoost()=%i\r\n"), result);
      finalResult |= result;

      return finalResult == 0;
    }
    #endif // RFLINK_NO_RADIOLIB_SUPPORT


    void mainLoop() {
      static time_t nextEnablementAttemptTime = 0;

      // check if hardware is properly initialized, if not retry every 5 seconds
      if(!hardwareProperlyInitialized) {
        time_t now = time(nullptr);
        if(now > nextEnablementAttemptTime) {
          nextEnablementAttemptTime = now + 5;
          initializeHardware(hardware, true);
        }
      }
    }

  } // end of Radio namespace
} // end of RFLink namespace
