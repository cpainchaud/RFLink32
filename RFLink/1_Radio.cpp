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

#ifdef RFM69_ENABLED
void set_Radio_mode(Radio_State new_State)
{
  if (current_State != new_State)
  {
    switch (new_State)
    {
    case Radio_OFF:
      PIN_RF_RX_DATA = NOT_A_PIN;
      PIN_RF_TX_DATA = NOT_A_PIN;
      radio.reset();
      radio.initialize();
      radio.setFrequency(433920000);
      Serial.print("Freq = ");
      Serial.println(radio.getFrequency());
      //Serial.print("Temp = "); Serial.println(radio.readTemperature());
      radio.setHighPower(true); // for RFM69HW
      // radio.sleep();
      break;

    case Radio_RX:
      radio.transmitEnd();
      PIN_RF_TX_DATA = NOT_A_PIN;
      radio.receiveBegin();
      PIN_RF_RX_DATA = RF69OOK_IRQ_PIN;
      radio.attachUserInterrupt(NULL);
      detachInterrupt(0);
      detachInterrupt(1);
      break;

    case Radio_TX:
      radio.receiveEnd();
      PIN_RF_RX_DATA = NOT_A_PIN;
      radio.transmitBegin();
      PIN_RF_TX_DATA = RF69OOK_IRQ_PIN;
      radio.setPowerLevel(10);
      break;

    case Radio_NA:
      break;
    }
    current_State = new_State;
  }
}
#else
// Prototype
void enableRX();
void disableRX();
void enableTX();
void disableTX();

namespace RFLink { namespace Radio  {

  HardwareType hardware;

  uint8_t PIN_RF_RX_PMOS = PIN_RF_RX_PMOS_0;
  uint8_t PIN_RF_RX_NMOS = PIN_RF_RX_NMOS_0;
  uint8_t PIN_RF_RX_VCC;
  uint8_t PIN_RF_RX_GND = PIN_RF_RX_GND_0;
  uint8_t PIN_RF_RX_NA = PIN_RF_RX_NA_0;
  uint8_t PIN_RF_RX_DATA = PIN_RF_RX_DATA_0;
  boolean PULLUP_RF_RX_DATA = PULLUP_RF_RX_DATA_0;
  bool rx_power_inverted;

  uint8_t PIN_RF_TX_PMOS = PIN_RF_TX_PMOS_0;
  uint8_t PIN_RF_TX_NMOS = PIN_RF_TX_NMOS_0;
  uint8_t PIN_RF_TX_VCC = PIN_RF_TX_VCC_0;
  uint8_t PIN_RF_TX_GND = PIN_RF_TX_GND_0;
  uint8_t PIN_RF_TX_NA = PIN_RF_TX_NA_0;
  uint8_t PIN_RF_TX_DATA = PIN_RF_TX_DATA_0;
  bool tx_power_inverted;

  States current_State = Radio_NA;

  const char * hardwareNames[] = {
    "generic",
    "RFM69",
    "EOF" // this is always the last one and matches index HardareType::HW_EOF_t
};
#define jsonSections_count sizeof(jsonSections)/sizeof(char *)

static_assert(sizeof(hardwareNames)/sizeof(char *) == HardwareType::HW_EOF_t+1, "hardwareNames has missing/extra names, please compare with HardwareType enum declarations");

// All json variable names
const char json_name_hardware[] = "hardware";

const char json_name_rx_data_pin[] = "rx_data_pin";
const char json_name_rx_power_pin[] = "rx_power_pin";
const char json_name_rx_power_inverted[] = "rx_power_inverted";

const char json_name_tx_data_pin[] = "tx_data_pin";
const char json_name_tx_power_pin[] = "tx_power";
const char json_name_tx_power_inverted[] = "tx_power_inverted";

Config::ConfigItem configItems[] =  {
  Config::ConfigItem(json_name_hardware,    Config::SectionId::Radio_id, hardwareNames[HardwareType::HW_basic_t], paramsUpdatedCallback),

  Config::ConfigItem(json_name_rx_data_pin,      Config::SectionId::Radio_id, PIN_RF_RX_DATA_0, paramsUpdatedCallback),
  Config::ConfigItem(json_name_rx_power_pin,     Config::SectionId::Radio_id, PIN_RF_RX_VCC_0, paramsUpdatedCallback),
  Config::ConfigItem(json_name_rx_power_inverted,Config::SectionId::Radio_id, false, paramsUpdatedCallback),

  Config::ConfigItem(json_name_tx_data_pin,      Config::SectionId::Radio_id, PIN_RF_TX_DATA_0, paramsUpdatedCallback),
  Config::ConfigItem(json_name_tx_power_pin,     Config::SectionId::Radio_id, PIN_RF_TX_VCC_0, paramsUpdatedCallback),
  Config::ConfigItem(json_name_tx_power_inverted,Config::SectionId::Radio_id, false, paramsUpdatedCallback),
  
  Config::ConfigItem()
};

void refreshParametersFromConfig() {

  States savedState = current_State;

  if( savedState != States::Radio_OFF && savedState != States::Radio_NA) {
    set_Radio_mode(States::Radio_OFF);
  }

  Config::ConfigItem *item;
  bool changesDetected = false;

  item = Config::findConfigItem(json_name_rx_data_pin, Config::SectionId::Wifi_id);
  if( PIN_RF_RX_DATA != item->getLongIntValue() ) {
    changesDetected = true;
    PIN_RF_RX_DATA = item->getBoolValue();
  }

  item = Config::findConfigItem(json_name_rx_power_pin, Config::SectionId::Wifi_id);
  if( rx_power_inverted != item->getBoolValue() ) {
    changesDetected = true;
    rx_power_inverted = item->getBoolValue();
  }

  item = Config::findConfigItem(json_name_rx_power_inverted, Config::SectionId::Wifi_id);
  if( PIN_RF_RX_VCC != item->getLongIntValue() ) {
    changesDetected = true;
    PIN_RF_RX_VCC = item->getBoolValue();
  }

  item = Config::findConfigItem(json_name_tx_data_pin, Config::SectionId::Wifi_id);
  if( PIN_RF_TX_DATA != item->getLongIntValue() ) {
    changesDetected = true;
    PIN_RF_TX_DATA = item->getBoolValue();
  }

  item = Config::findConfigItem(json_name_tx_power_pin, Config::SectionId::Wifi_id);
  if( PIN_RF_TX_VCC != item->getLongIntValue() ) {
    changesDetected = true;
    PIN_RF_TX_VCC = item->getBoolValue();
  }

  item = Config::findConfigItem(json_name_tx_power_pin, Config::SectionId::Wifi_id);
  if( tx_power_inverted != item->getBoolValue() ) {
    changesDetected = true;
    tx_power_inverted = item->getBoolValue();
  }


  // restore to RX state
  if( savedState != States::Radio_OFF && savedState != States::Radio_NA) {
    set_Radio_mode(States::Radio_RX);
  }

}

void paramsUpdatedCallback() {
  void refreshParametersFromConfig();
}



void set_Radio_mode(States new_State)
{
  if (current_State != new_State)
  {
    switch (new_State)
    {
    case Radio_OFF:
      disableTX();
      disableRX();
      break;

    case Radio_RX:
      disableTX();
      enableRX();
      break;

    case Radio_TX:
      disableRX();
      enableTX();
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
  if (PIN_RF_RX_PMOS != (uint8_t)NOT_A_PIN)
  {
    Serial.print(F("Radio pin RF_RX_PMOS :\t"));
    Serial.println(GPIO2String(PIN_RF_RX_PMOS));
    Serial.println(PIN_RF_RX_PMOS);
  }
  if (PIN_RF_RX_NMOS != (uint8_t)NOT_A_PIN)
  {
    Serial.print(F("Radio pin RF_RX_NMOS :\t"));
    Serial.println(GPIO2String(PIN_RF_RX_NMOS));
  }
  if (PIN_RF_RX_VCC != (uint8_t)NOT_A_PIN)
  {
    Serial.print(F("Radio pin RF_RX_VCC :\t"));
    Serial.println(GPIO2String(PIN_RF_RX_VCC));
  }
  if (PIN_RF_RX_GND != (uint8_t)NOT_A_PIN)
  {
    Serial.print(F("Radio pin RF_RX_GND :\t"));
    Serial.println(GPIO2String(PIN_RF_RX_GND));
  }
  if (PIN_RF_RX_NA != (uint8_t)NOT_A_PIN)
  {
    Serial.print(F("Radio pin RF_RX_NA :\t"));
    Serial.println(GPIO2String(PIN_RF_RX_NA));
  }
  if (PIN_RF_RX_DATA != (uint8_t)NOT_A_PIN)
  {
    Serial.print(F("Radio pin RF_RX_DATA :\t"));
    Serial.print(GPIO2String(PIN_RF_RX_DATA));
    if (PULLUP_RF_RX_DATA)
      Serial.println(F(" (Pullup enabled)"));
    else
      Serial.println();
  }
  //
  if (PIN_RF_TX_PMOS != (uint8_t)NOT_A_PIN)
  {
    Serial.print(F("Radio pin RF_TX_PMOS :\t"));
    Serial.println(GPIO2String(PIN_RF_TX_PMOS));
  }
  if (PIN_RF_TX_NMOS != (uint8_t)NOT_A_PIN)
  {
    Serial.print(F("Radio pin RF_TX_NMOS :\t"));
    Serial.println(GPIO2String(PIN_RF_TX_NMOS));
  }
  if (PIN_RF_TX_VCC != (uint8_t)NOT_A_PIN)
  {
    Serial.print(F("Radio pin RF_TX_VCC :\t"));
    Serial.println(GPIO2String(PIN_RF_TX_VCC));
  }
  if (PIN_RF_TX_GND != (uint8_t)NOT_A_PIN)
  {
    Serial.print(F("Radio pin RF_TX_GND :\t"));
    Serial.println(GPIO2String(PIN_RF_TX_GND));
  }
  if (PIN_RF_TX_DATA != (uint8_t)NOT_A_PIN)
  {
    Serial.print(F("Radio pin RF_TX_DATA :\t"));
    Serial.println(GPIO2String(PIN_RF_TX_DATA));
  }
  if (PIN_RF_TX_NA != (uint8_t)NOT_A_PIN)
  {
    Serial.print(F("Radio pin RF_TX_NA :\t"));
    Serial.println(GPIO2String(PIN_RF_TX_NA));
  }
}
#endif // ESP8266 || ESP32

void enableRX()
{
  // RX pins
  pinMode(PIN_RF_RX_NA, INPUT);       // Initialise in/output ports
  pinMode(PIN_RF_RX_DATA, INPUT);     // Initialise in/output ports
  pinMode(PIN_RF_RX_NMOS, OUTPUT);    // MOSFET, always output
  pinMode(PIN_RF_RX_PMOS, OUTPUT);    // MOSFET, always output
  digitalWrite(PIN_RF_RX_NMOS, HIGH); // turn GND to RF receiver ON
  digitalWrite(PIN_RF_RX_PMOS, LOW);  // turn VCC to RF receiver ON
  pinMode(PIN_RF_RX_GND, OUTPUT);     // Initialise in/output ports
  pinMode(PIN_RF_RX_VCC, OUTPUT);     // Initialise in/output ports
  digitalWrite(PIN_RF_RX_GND, LOW);   // turn GND to RF receiver ON
  digitalWrite(PIN_RF_RX_VCC, HIGH);  // turn VCC to RF receiver ON
  if (PULLUP_RF_RX_DATA)
    pinMode(PIN_RF_RX_DATA, INPUT_PULLUP); // Initialise in/output ports
  delayMicroseconds(TRANSMITTER_STABLE_DELAY_US);
  if(RFLink::Signal::params::async_mode_enabled)
    RFLink::Signal::AsyncSignalScanner::startScanning();
 
}

void disableRX()
{
  if( RFLink::Signal::params::async_mode_enabled )
    RFLink::Signal::AsyncSignalScanner::stopScanning();
  
  // RX pins
  pinMode(PIN_RF_RX_DATA, INPUT);
  pinMode(PIN_RF_RX_NA, INPUT);
  pinMode(PIN_RF_RX_PMOS, OUTPUT);    // MOSFET, always output
  pinMode(PIN_RF_RX_NMOS, OUTPUT);    // MOSFET, always output
  digitalWrite(PIN_RF_RX_PMOS, HIGH); // turn VCC to RF receiver OFF
  digitalWrite(PIN_RF_RX_NMOS, LOW);  // turn GND to RF receiver OFF
  pinMode(PIN_RF_RX_VCC, INPUT);
  pinMode(PIN_RF_RX_GND, INPUT);
}

void enableTX()
{
  // TX Pins
  pinMode(PIN_RF_TX_NA, INPUT);       // Initialise in/output ports
  pinMode(PIN_RF_TX_DATA, OUTPUT);    // Initialise in/output ports
  digitalWrite(PIN_RF_TX_DATA, LOW);  // No signal yet
  pinMode(PIN_RF_TX_NMOS, OUTPUT);    // MOSFET, always output
  pinMode(PIN_RF_TX_PMOS, OUTPUT);    // MOSFET, always output
  digitalWrite(PIN_RF_TX_NMOS, HIGH); // turn GND to TX receiver ON
  digitalWrite(PIN_RF_TX_PMOS, LOW);  // turn VCC to TX receiver ON
  pinMode(PIN_RF_TX_GND, OUTPUT);     // Initialise in/output ports
  pinMode(PIN_RF_TX_VCC, OUTPUT);     // Initialise in/output ports
  digitalWrite(PIN_RF_TX_GND, LOW);   // turn GND to TX receiver ON
  digitalWrite(PIN_RF_TX_VCC, HIGH);  // turn VCC to TX receiver ON
  delayMicroseconds(TRANSMITTER_STABLE_DELAY_US);
}

void disableTX()
{
  // TX Pins
  delayMicroseconds(TRANSMITTER_STABLE_DELAY_US);
  digitalWrite(PIN_RF_TX_DATA, LOW);  // No more signal
  pinMode(PIN_RF_TX_DATA, INPUT);     //
  pinMode(PIN_RF_TX_NA, INPUT);       //
  pinMode(PIN_RF_TX_NMOS, OUTPUT);    // MOSFET, always output
  pinMode(PIN_RF_TX_PMOS, OUTPUT);    // MOSFET, always output
  digitalWrite(PIN_RF_TX_PMOS, HIGH); // turn VCC to TX receiver OFF
  digitalWrite(PIN_RF_TX_NMOS, LOW);  // turn GND to TX receiver OFF
  pinMode(PIN_RF_TX_VCC, INPUT);
  pinMode(PIN_RF_TX_GND, INPUT);
}


}}


#endif // not RFM69_ENABLED