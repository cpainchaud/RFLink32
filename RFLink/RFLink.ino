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
#include "1_Radio.h"
#include "2_Signal.h"
#include "3_Serial.h"
#include "4_Display.h"
#include "5_Plugin.h"
#include "6_WiFi_MQTT.h"
#include "8_OLED.h"
#include "9_AutoConnect.h"

#if (defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__))
#include <avr/power.h>
#endif
//****************************************************************************************************************************************
void sendMsg(); // See at bottom

#if (defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__))
void (*Reboot)(void) = 0; // reset function on adress 0.

void CallReboot(void)
{
  sendMsg();
  delay(1);
  Reboot();
}
#endif

#if (defined(ESP8266) || defined(ESP32))
void CallReboot(void)
{
  sendMsg();
  delay(1);
  ESP.restart();
}
#endif

void setup()
{
#if (defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__))
  // Low Power Arduino
  ADCSRA = 0;            // disable ADC
  power_all_disable();   // turn off all modules
  power_timer0_enable(); // Timer 0
  power_usart0_enable(); // UART
  delay(250);            // Wait ESP-01S
#ifdef RFM69_ENABLED
  power_spi_enable(); // SPI
#endif
#elif defined(ESP32)
  btStop();
#endif

  delay(250);         // Time needed to switch back from Upload to Console
  Serial.begin(BAUD); // Initialise the serial port

#if (defined(ESP32) || defined(ESP8266))
  Serial.println(); // ESP "Garbage" message
  Serial.print(F("Arduino IDE Version :\t"));
  Serial.println(ARDUINO);
#ifdef ESP8266
  Serial.print(F("ESP CoreVersion :\t"));
  Serial.println(ESP.getCoreVersion());
#endif // ESP8266
  Serial.print(F("Sketch File :\t\t"));
  Serial.println(__FILE__); // "RFLink.ino" version is in 20;00 Message
  Serial.println(F("Compiled on :\t\t" __DATE__ " at " __TIME__));

#if (!defined(AUTOCONNECT_ENABLED) && !defined(MQTT_ENABLED))
  setup_WIFI_OFF();
#endif
#endif // ESP32 || ESP8266

#if (!defined(AUTOCONNECT_ENABLED) && defined(MQTT_ENABLED))
  setup_WIFI();
  setup_MQTT();
  reconnect();
#endif

#ifdef AUTOCONNECT_ENABLED
  setup_AutoConnect();
#endif
  set_Radio_mode(Radio_OFF);
#if (defined(ESP8266) || defined(ESP32))
  show_Radio_Pin();
#endif // ESP8266 || ESP32

  PluginInit();
  PluginTXInit();

#ifdef OLED_ENABLED
  setup_OLED();
#endif
#ifdef MQTT_ENABLED
  setup_MQTT();
  reconnect();
#endif

  display_Header();
  display_Splash();
  display_Footer();

#ifdef SERIAL_ENABLED
  Serial.print(pbuffer);
#endif
#ifdef OLED_ENABLED
  splash_OLED();
#endif
#ifdef MQTT_ENABLED
  publishMsg();
#endif
  pbuffer[0] = 0;
  set_Radio_mode(Radio_RX);
}

void loop()
{
#ifdef AUTOCONNECT_ENABLED
  loop_AutoConnect();
  if (WiFi.status() == WL_CONNECTED)
  {
#endif
#ifdef MQTT_ENABLED
    checkMQTTloop();
    sendMsg();
#endif

#ifdef SERIAL_ENABLED
#if PIN_RF_TX_DATA_0 != NOT_A_PIN
    if (CheckSerial())
      sendMsg();
#endif
#endif

#ifdef AUTOCONNECT_ENABLED
    if (CheckWeb(CmdMsg))
      sendMsg();
#endif

    if (ScanEvent())
      sendMsg();

#ifdef AUTOCONNECT_ENABLED
  }
#endif
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
#ifdef AUTOCONNECT_ENABLED
    LastMsg = pbuffer;
#endif
#ifdef OLED_ENABLED
    print_OLED();
#endif
    pbuffer[0] = 0;
  }
}

/*********************************************************************************************/
