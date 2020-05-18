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
#ifdef AUTOCONNECT_ENABLED
#include "9_AutoConnect.h"
#else
uint8_t PIN_RF_RX_PMOS = PIN_RF_RX_PMOS_0;
uint8_t PIN_RF_RX_NMOS = PIN_RF_RX_NMOS_0;
uint8_t PIN_RF_RX_VCC = PIN_RF_RX_VCC_0;
uint8_t PIN_RF_RX_GND = PIN_RF_RX_GND_0;
uint8_t PIN_RF_RX_NA = PIN_RF_RX_NA_0;
uint8_t PIN_RF_RX_DATA = PIN_RF_RX_DATA_0;
boolean PULLUP_RF_RX_DATA = PULLUP_RF_RX_DATA_0;
uint8_t PIN_RF_TX_PMOS = PIN_RF_TX_PMOS_0;
uint8_t PIN_RF_TX_NMOS = PIN_RF_TX_NMOS_0;
uint8_t PIN_RF_TX_VCC = PIN_RF_TX_VCC_0;
uint8_t PIN_RF_TX_GND = PIN_RF_TX_GND_0;
uint8_t PIN_RF_TX_NA = PIN_RF_TX_NA_0;
uint8_t PIN_RF_TX_DATA = PIN_RF_TX_DATA_0;
#endif //AUTOCONNECT_ENABLED

Radio_State current_State = Radio_NA;

#ifdef RFM69_ENABLED

#include <SPI.h>
#include <RFM69OOK.h>
#include <RFM69OOKregisters.h>
RFM69OOK radio;

void set_Radio_mode(Radio_State new_State)
{
  if (current_State != new_State)
  {
    switch (new_State)
    {
    case Radio_OFF:
      PIN_RF_RX_DATA = NOT_A_PIN;
      PIN_RF_TX_DATA = NOT_A_PIN;
      radio.initialize();
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

void set_Radio_mode(Radio_State new_State)
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
}

void disableRX()
{
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
#endif // RFM69_ENABLED