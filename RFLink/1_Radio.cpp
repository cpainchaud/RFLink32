// ************************************* //
// * Arduino Project RFLink-esp        * //
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
#include "RFM69/RFM69OOK.h"
#include "RFM69/RFM69OOKregisters.h"
RFM69OOK radio;

#include <RadioLib.h>

Module radioLibModule(5, -1, 4, -1);
SX1278 *radio_SX1278 = nullptr;
RF69 *radio_RFM69 = nullptr;



namespace RFLink { namespace Radio  {

    HardwareType hardware = HardwareType::HW_basic_t;
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


    States current_State = Radio_NA;

    const char * hardwareNames[] = {
            "generic",
            "RFM69CW",
            "RFM69HCW",
            "SX1278",
            "RFM69TEST_RADIOLIB",
            "EOF" // this is always the last one and matches index HardareType::HW_EOF_t
    };
#define hardwareNames_count sizeof(hardwareNames)/sizeof(char *)

    static_assert(sizeof(hardwareNames)/sizeof(char *) == HardwareType::HW_EOF_t+1, "hardwareNames has missing/extra names, please compare with HardwareType enum declarations");

// All json variable names
    const char json_name_hardware[] = "hardware";

    const char json_name_rx_data[] =  "rx_data";
    const char json_name_rx_vcc[] =   "rx_vcc";
    const char json_name_rx_nmos[] =  "rx_nmos";
    const char json_name_rx_pmos[] =  "rx_pmos";
    const char json_name_rx_gnd[] =   "rx_gnd";
    const char json_name_rx_na[] =    "rx_na";
    const char json_name_rx_reset[] =    "rx_reset";
    const char json_name_rx_cs[] =    "rx_cs";

    const char json_name_tx_data[] =  "tx_data";
    const char json_name_tx_vcc[] =   "tx_vcc";
    const char json_name_tx_nmos[] =  "tx_nmos";
    const char json_name_tx_pmos[] =  "tx_pmos";
    const char json_name_tx_gnd[] =   "tx_gnd";


    Config::ConfigItem configItems[] =  {
            Config::ConfigItem(json_name_hardware,  Config::SectionId::Radio_id, hardwareNames[HardwareType::HW_basic_t], paramsUpdatedCallback),

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


            Config::ConfigItem()
    };

    void refreshParametersFromConfig() {
      States savedState = current_State;
      bool pinsHaveChanged = false;
      HardwareType currentHardware = hardware;
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
      }

      item = Config::findConfigItem(json_name_rx_data, Config::SectionId::Radio_id);
      if( pins::RX_DATA != item->getLongIntValue() ) {
        changesDetected = true;
        pinsHaveChanged = true;
        pins::RX_DATA = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_rx_vcc, Config::SectionId::Radio_id);
      if( pins::RX_VCC != item->getLongIntValue() ) {
        changesDetected = true;
        pinsHaveChanged = true;
        pins::RX_VCC = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_rx_nmos, Config::SectionId::Radio_id);
      if( pins::RX_NMOS != item->getLongIntValue() ) {
        changesDetected = true;
        pinsHaveChanged = true;
        pins::RX_NMOS = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_rx_pmos, Config::SectionId::Radio_id);
      if( pins::RX_PMOS != item->getLongIntValue() ) {
        changesDetected = true;
        pinsHaveChanged = true;
        pins::RX_PMOS = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_rx_gnd, Config::SectionId::Radio_id);
      if( pins::RX_GND != item->getLongIntValue() ) {
        changesDetected = true;
        pinsHaveChanged = true;
        pins::RX_GND = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_rx_na, Config::SectionId::Radio_id);
      if( pins::RX_NA != item->getLongIntValue() ) {
        changesDetected = true;
        pinsHaveChanged = true;
        pins::RX_NA = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_rx_reset, Config::SectionId::Radio_id);
      if( pins::RX_RESET != item->getLongIntValue() ) {
        changesDetected = true;
        pinsHaveChanged = true;
        pins::RX_RESET = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_rx_cs, Config::SectionId::Radio_id);
      if( pins::RX_CS != item->getLongIntValue() ) {
        changesDetected = true;
        pinsHaveChanged = true;
        pins::RX_CS = item->getLongIntValue();
      }

      // TX

      item = Config::findConfigItem(json_name_tx_data, Config::SectionId::Radio_id);
      if( pins::TX_DATA != item->getLongIntValue() ) {
        changesDetected = true;
        pinsHaveChanged = true;
        pins::TX_DATA = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_tx_vcc, Config::SectionId::Radio_id);
      if( pins::TX_VCC != item->getLongIntValue() ) {
        changesDetected = true;
        pinsHaveChanged = true;
        pins::TX_VCC = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_tx_nmos, Config::SectionId::Radio_id);
      if( pins::TX_NMOS != item->getLongIntValue() ) {
        changesDetected = true;
        pinsHaveChanged = true;
        pins::TX_NMOS = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_tx_pmos, Config::SectionId::Radio_id);
      if( pins::TX_PMOS != item->getLongIntValue() ) {
        changesDetected = true;
        pinsHaveChanged = true;
        pins::TX_PMOS = item->getLongIntValue();
      }

      item = Config::findConfigItem(json_name_tx_gnd, Config::SectionId::Radio_id);
      if( pins::TX_GND != item->getLongIntValue() ) {
        changesDetected = true;
        pinsHaveChanged = true;
        pins::TX_GND = item->getLongIntValue();
      }

      initializeHardware(newHardwareId);

      // restore to RX state
      if( savedState != States::Radio_OFF && savedState != States::Radio_NA) {
        set_Radio_mode(States::Radio_RX);
        //Serial.println("Changes applied, bringing radio back to RX mode");
      }

    }

    void paramsUpdatedCallback() {
      refreshParametersFromConfig();
    }

    HardwareType hardwareIDFromString(const char *name) {
      for(int i=0; i<hardwareNames_count; i++) {
        if(strcmp(hardwareNames[i], name) == 0)
          return (HardwareType) i;
      }

      return HardwareType::HW_EOF_t;
    }

    void set_Radio_mode_generic(States new_State)
    {
      if (current_State != new_State)
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

    void set_Radio_mode(States new_State)
    {
      if(hardware == HardwareType::HW_basic_t)
        set_Radio_mode_generic(new_State);
      else if( hardware == HardwareType::HW_RFM69CW_t || hardware == HardwareType::HW_RFM69HCW_t )
        //set_Radio_mode_RFM69_new(new_State);
        set_Radio_mode_RFM69(new_State);
      else if( hardware == HardwareType::HW_SX1278_t )
        set_Radio_mode_SX1278(new_State);
      else
        Serial.printf_P(PSTR("Error while trying to switch Radio state: unknown hardware id '%i'"), new_State);
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

    void set_Radio_mode_RFM69(States new_State)
    {
      // @TODO : review compatibility with ASYNC mode
      if (current_State != new_State)
      {
        switch (new_State)
        {
          case Radio_OFF:
            if( RFLink::Signal::params::async_mode_enabled )
              RFLink::Signal::AsyncSignalScanner::stopScanning();
            radio.sleep();
            break;

          case Radio_RX:
            radio.transmitEnd();
            pinMode(pins::RX_DATA, INPUT);
            radio.receiveBegin();
            if( RFLink::Signal::params::async_mode_enabled )
              RFLink::Signal::AsyncSignalScanner::startScanning();
            //detachInterrupt(0);
            //detachInterrupt(1);
            break;

          case Radio_TX:
            if( RFLink::Signal::params::async_mode_enabled )
              RFLink::Signal::AsyncSignalScanner::stopScanning();
            radio.receiveEnd();
            pinMode(pins::TX_DATA, OUTPUT);
            radio.transmitBegin();
            radio.setPowerLevel(31);
            break;

          case Radio_NA:
            break;
        }
        current_State = new_State;
      }
    }

    void set_Radio_mode_SX1278(States new_State)
    {
      // @TODO : review compatibility with ASYNC mode
      if (current_State != new_State)
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
              Serial.printf_P(PSTR("Failed to put hardware in in receive mode (code=%i), we will try to reinitialize it later"), (int) success);
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
              Serial.printf_P(PSTR("Failed to put hardware in transmit mode (code=%i), we will try to reinitialize it later"), (int) success);
              hardwareProperlyInitialized = false;
            }

            success = radio_SX1278->setOutputPower(13);
            if(success != 0) {
              Serial.printf_P(PSTR("Failed setup hardware transmit power (code=%i), we will try to reinitialize it later"), (int) success);
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

    void set_Radio_mode_RFM69_new(States new_State)
    {
      // @TODO : review compatibility with ASYNC mode
      if (current_State != new_State)
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
              Serial.printf_P(PSTR("Failed to switch to RX mode (code=%i), we will try to reinitialize it later\r\n"), (int) success);
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

            break;
          }

          case Radio_NA:
            break;
        }
        current_State = new_State;
      }
    }


    float getCurrentRssi() {
      if(hardware == HardwareType::HW_SX1278_t)
        return radio_SX1278->getRSSI(true);

      return -9999.0F;
    }

    void initializeHardware(HardwareType newHardware, bool force) {

      if(newHardware == hardware && !force) {
        hardwareProperlyInitialized = true;
        RFLink::sendRawPrint(F("initializeHardware() requested but new and old hardware are the same\r\n"));
        return;
      }

      hardwareProperlyInitialized = false;


      RFLink::sendRawPrint(F("Switching from Radio hardware "));
      RFLink::sendRawPrint(hardwareNames[hardware]);
      RFLink::sendRawPrint(F(" to "));
      RFLink::sendRawPrint(hardwareNames[newHardware]);
      RFLink::sendRawPrintln();

      //RFLink::sendRawPrintf_P(PSTR("Switching from Radio hardware '%s' to '%s'\r\n"), "hello", hardwareNames[newHardware]);
      //RFLink::sendRawPrintf("Switching from Radio hardware '%s' to '%s'\r\n", "hello", hardwareNames[newHardware]);

      bool success = false;
      hardware = newHardware;

      if(newHardware == HardwareType::HW_SX1278_t){
        success = initialize_SX1278();
      }
      else if(newHardware == HardwareType::HW_RFM69CW_t || newHardware == HardwareType::HW_RFM69HCW_t){
        success = initialize_RFM69_legacy();
      }
      else if(newHardware == HardwareType::HW_basic_t){
        success = true;
      }
      else {
        RFLink::sendRawPrint(F("Unsupported hardwareId="));
        RFLink::sendRawPrint((int)newHardware);
        RFLink::sendRawPrintln();
      }

      if(!success) {
        Serial.println(F("Hardware failed to initialize, we will retry later!"));
      } else {
        Serial.println(F("Hardware initialization was successful!"));
        hardwareProperlyInitialized = true;
      }
    }

    bool initialize_SX1278() {
      radioLibModule = Module(pins::RX_CS, -1, pins::RX_RESET, -1);
      if(radio_SX1278 == nullptr)
        radio_SX1278 = new SX1278(&radioLibModule);
      else
        *radio_SX1278 = &radioLibModule;

      int finalResult = 0;

      auto result = radio_SX1278->beginFSK( 433.92F, 9.600F, 50.0F, 250.0F, 12, 16, true);
      Serial.printf_P(PSTR("Initialized SX1278, return code %i\r\n"), result);
      finalResult |= result;

      result = radio_SX1278->setOOK(true);
      Serial.printf_P(PSTR("SX1278, setOOK=%i\r\n"), result);
      finalResult |= result;

      result = radio_SX1278->setEncoding(RADIOLIB_ENCODING_NRZ);
      Serial.printf_P(PSTR("SX1278, set encoding result=%i\r\n"), result);
      finalResult |= result;

      result = radio_SX1278->setDataShapingOOK(0);
      Serial.printf_P(PSTR("SX1278, set data shaping result=%i\r\n"), result);
      finalResult |= result;

      result = radio_SX1278->setGain(6);
      Serial.printf_P(PSTR("SX1278, setGain() result=%i\r\n"), result);
      finalResult |= result;

      //result = radio_SX1278->setOokThresholdType(SX127X_OOK_THRESH_FIXED);
      //Serial.printf("SX1278, setOokThresholdType result=%i\r\n", result);
      //finalResult |= result;

      //result = radio_SX1278->setOokFixedOrFloorThreshold(0x0C);
      //Serial.printf("SX1278, setOokFixedOrFloorThreshold() result=%i\r\n", result);
      //finalResult |= result;

      result = radio_SX1278->setOokPeakThresholdDecrement(SX127X_OOK_PEAK_THRESH_DEC_1_4_CHIP);
      Serial.printf_P(PSTR("SX1278, setOokPeakThresholdDecrement() result=%i\r\n"), result);
      finalResult |= result;

      //result = radio_SX1278->startReceive(0, SX127X_RXCONTINUOUS);
      //Serial.printf("sx1278, receive start code %i\r\n", result);
      //finalResult |= result;

      //result = radio_SX1278->startDirect();
      //Serial.printf("sx1278, startDirect code %i\r\n", result)
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

      auto result = radio_RFM69->begin(433.92F, 19.200F, 50.0F, 250.0F, 12, 16);
      Serial.printf_P(PSTR("RFM69 being()=%i\r\n"), result);
      finalResult |= result;

      radio_RFM69->setOOK(true);
      Serial.printf_P(PSTR("RFM69 SetOOK()=%i\r\n"), result);
      finalResult |= result;

      result = radio_RFM69->setDataShaping(RADIOLIB_SHAPING_NONE);
      Serial.printf_P(PSTR("RFM69 setDataShapingOOK()=%i\r\n"), result);
      finalResult |= result;

      result = radio_RFM69->setEncoding(RADIOLIB_ENCODING_NRZ);
      Serial.printf_P(PSTR("RFM69 setEncoding()=%i\r\n"), result);
      finalResult |= result;

      return finalResult == 0;
    }

    bool initialize_RFM69_legacy() {
      radio.reset();
      radio.initialize();
      radio.setFrequency(433920000);
      Serial.printf_P(PSTR("RFM69 initialized with Freq = %.2f\r\n"), (double)radio.getFrequency()/1000000);
      //Serial.print("Temp = "); Serial.println(radio.readTemperature());
      if(hardware == HardwareType::HW_RFM69HCW_t)
        radio.setHighPower(true);
      return true;
    }

    void mainLoop() {
      if(!hardwareProperlyInitialized) {
        initializeHardware(hardware, true);
      }
    }

  } // end of Radio namespace
} // end of RFLink namespace
