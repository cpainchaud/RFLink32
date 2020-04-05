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
#include "4_Misc.h"
#include "5_Plugin.h"
#include "6_WiFi_MQTT.h"
//****************************************************************************************************************************************

void setup()
{
  Serial.begin(BAUD); // Initialise the serial port
  Serial.println();   // ESP "Garbage" message

  pinMode(PIN_RF_RX_DATA, INPUT);             // Initialise in/output ports
  pinMode(PIN_RF_TX_DATA, OUTPUT);            // Initialise in/output ports
  pinMode(PIN_RF_TX_VCC, OUTPUT);             // Initialise in/output ports
  pinMode(PIN_RF_TX_GND, OUTPUT);             // Initialise in/output ports
  pinMode(PIN_RF_RX_VCC, OUTPUT);             // Initialise in/output ports
  pinMode(PIN_RF_RX_GND, OUTPUT);             // Initialise in/output ports
  digitalWrite(PIN_RF_TX_GND, LOW);           // turn GND to TX receiver ON
  digitalWrite(PIN_RF_RX_GND, LOW);           // turn GND to RF receiver ON
  digitalWrite(PIN_RF_RX_VCC, HIGH);          // turn VCC to RF receiver ON
  digitalWrite(PIN_RF_RX_DATA, INPUT_PULLUP); // pull-up resister on (to prevent garbage)
#ifdef LA_ENABLED
  pinMode(LA_PROBE1, OUTPUT);
  pinMode(LA_PROBE2, OUTPUT);
  pinMode(LA_PROBE3, OUTPUT);
  pinMode(LA_PROBE4, OUTPUT);
  pinMode(LA_PROBE5, OUTPUT);
  pinMode(LA_PROBE6, OUTPUT);
  digitalWrite(LA_PROBE1, LOW);
  digitalWrite(LA_PROBE2, LOW);
  digitalWrite(LA_PROBE3, LOW);
  digitalWrite(LA_PROBE4, LOW);
  digitalWrite(LA_PROBE5, LOW);
  digitalWrite(LA_PROBE6, LOW);
#endif

  PluginInit();

#if defined(MQTT_ENABLED) && (defined(ESP32) || defined(ESP8266))
  setup_WIFI();
  setup_MQTT();
  reconnect();
#else
  setup_WIFI_OFF();
#endif
  display_Start();
  display_Footer();
#if defined(MQTT_ENABLED) && (defined(ESP32) || defined(ESP8266))
#ifdef LA_ENABLED
  digitalWrite(LA_PROBE5, HIGH);
#endif
  publishMsg();
#ifdef LA_ENABLED
  digitalWrite(LA_PROBE5, LOW);
#endif
#endif
}

void loop()
{
#ifdef LA_ENABLED
  digitalWrite(LA_PROBE1, HIGH);
#endif

#if defined(MQTT_ENABLED) && (defined(ESP32) || defined(ESP8266))
#ifdef LA_ENABLED
  digitalWrite(LA_PROBE6, HIGH);
#endif
  checkMQTTloop();
#ifdef LA_ENABLED
  digitalWrite(LA_PROBE6, LOW);
#endif
#endif

  if (ScanEvent())
  {
#if defined(MQTT_ENABLED) && (defined(ESP32) || defined(ESP8266))
#ifdef LA_ENABLED
    digitalWrite(LA_PROBE5, HIGH);
#endif
    publishMsg();
#ifdef LA_ENABLED
    digitalWrite(LA_PROBE5, LOW);
#endif
#else
    MQTTbuffer[0] = 0;
#endif
  }

#ifdef LA_ENABLED
  digitalWrite(LA_PROBE1, LOW);
#endif
}
/*********************************************************************************************/
