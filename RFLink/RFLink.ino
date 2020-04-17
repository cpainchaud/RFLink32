// *********************************************************************************************************************************
// * Arduino Project RFLink-esp https://github.com/couin3/RFLink (Branch esp)
// * Portions Free Software 2018..2020 StormTeam - Marc RIVES
// * Portions Free Software 2015..2016 StuntTeam - (RFLink R29~R33)
// * Portions © Copyright 2010..2015 Paul Tonkes (original Nodo 3.7 code)
// *
// *                                       RFLink-esp
// *
// ********************************************************************************************************************************
// * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
// * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// * You received a copy of the GNU General Public License along with this program in file 'COPYING.TXT'.
// * For more information on GPL licensing: http://www.gnu.org/licenses
// ********************************************************************************************************************************

// ****************************************************************************
#include <Arduino.h>
#include "RFLink.h"
#include "2_Signal.h"
#include "4_Display.h"
#include "5_Plugin.h"
#include "9_AutoConnect.h"
#if (defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__))
#include <avr/power.h>
#else
#include "6_WiFi_MQTT.h"
#endif
#ifdef OLED_ENABLED
#include "8_OLED.h"
#endif
//****************************************************************************************************************************************

#if (defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__))
void (*Reboot)(void) = 0; // reset function on adress 0.
#endif

void sendMsg(); // See at bottom

void setup()
{
#if (defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__))
  // Low Power Arduino
  ADCSRA = 0;            // disable ADC
  power_all_disable();   // turn off all modules
  power_timer0_enable(); // Timer 0
  power_usart0_enable(); // UART
#endif

  Serial.begin(BAUD); // Initialise the serial port
  Serial.println();   // ESP "Garbage" message

  // RX pins
  pinMode(PIN_RF_RX_VCC, OUTPUT);             // Initialise in/output ports
  pinMode(PIN_RF_RX_NA, INPUT);               // Initialise in/output ports
  pinMode(PIN_RF_RX_DATA, INPUT);             // Initialise in/output ports
  pinMode(PIN_RF_RX_GND, OUTPUT);             // Initialise in/output ports
  digitalWrite(PIN_RF_RX_GND, LOW);           // turn GND to RF receiver ON
  digitalWrite(PIN_RF_RX_VCC, HIGH);          // turn VCC to RF receiver ON
  digitalWrite(PIN_RF_RX_DATA, INPUT_PULLUP); // pull-up resistor on (to prevent garbage)

  // TX Pins
  // pinMode(PIN_RF_TX_VCC, OUTPUT);    // Initialise in/output ports
  // pinMode(PIN_RF_TX_DATA, OUTPUT);   // Initialise in/output ports
  // pinMode(PIN_RF_TX_GND, OUTPUT);    // Initialise in/output ports
  // digitalWrite(PIN_RF_TX_GND, LOW);  // turn GND to TX receiver ON
  // digitalWrite(PIN_RF_TX_VCC, HIGH); // turn VCC to TX receiver ON
  //delayMicroseconds(TRANSMITTER_STABLE_DELAY_US);

  delay(100);

#if (!defined(AUTOCONNECT_ENABLED) && !defined(MQTT_ENABLED))
#if (defined(ESP32) || defined(ESP8266))
  setup_WIFI_OFF();
#endif
#endif

#if (!defined(AUTOCONNECT_ENABLED) && defined(MQTT_ENABLED))
  setup_WIFI();
  setup_MQTT();
  reconnect();
#endif

#ifdef AUTOCONNECT_ENABLED
  setup_AutoConnect();
#endif

#ifdef MQTT_ENABLED
  setup_MQTT();
  reconnect();
#endif

#ifdef OLED_ENABLED
  setup_OLED();
#endif
  display_Header();
  display_Splash();
  display_Footer();
#ifdef SERIAL_ENABLED
  Serial.print(pbuffer);
#endif
#if defined(MQTT_ENABLED) && (defined(ESP32) || defined(ESP8266))
  publishMsg();
#endif
#ifdef OLED_ENABLED
  splash_OLED();
#endif
  pbuffer[0] = 0;

  PluginInit();
  delay(100);
}

void loop()
{
#ifdef AUTOCONNECT_ENABLED
  loop_AutoConnect();
#endif
#ifdef MQTT_ENABLED
  checkMQTTloop();
#endif
  if (ScanEvent())
    sendMsg();
}

void sendMsg()
{
  if (pbuffer[0] != 0)
  {
#ifdef SERIAL_ENABLED
    Serial.print(pbuffer);
#endif
#ifdef MQTT_ENABLED
    publishMsg();
#endif
#ifdef OLED_ENABLED
    print_OLED();
#endif
    pbuffer[0] = 0;
  }
}
/*********************************************************************************************/
