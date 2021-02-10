// *********************************************************************************************************************************
// * Arduino Project RFLink-esp https://github.com/couin3/RFLink (Branch esp)
// * Portions Free Software 2018..2020 StormTeam - Marc RIVES
// * Portions Free Software 2015..2016 StuntTeam - (RFLink R29~R33)
// * Portions © Copyright 2010..2015 Paul Tonkes (original Nodo 3.7 code)
// * Portions GPLv3 2021 Christophe Painchaud Async Receiver (Async Receiver)
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
#include "9_Serial2Net.h"
#include "10_Wifi.h"

#if (defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__))
#include <avr/power.h>
#endif
//****************************************************************************************************************************************
void sendMsgFromBuffer(); // See at bottom

#if (defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__))
void (*Reboot)(void) = 0; // reset function on adress 0.

void CallReboot(void)
{
  RFLink::sendMsgFromBuffer();
  delay(1);
  Reboot();
}
#endif

#if (defined(ESP8266) || defined(ESP32))
void CallReboot(void)
{
  RFLink::sendMsgFromBuffer();
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
#endif
#ifdef ESP8266
  Serial.print(F("ESP CoreVersion :\t"));
  Serial.println(ESP.getCoreVersion());
#endif // ESP8266
  Serial.print(F("Sketch File :\t\t"));
  Serial.println(__FILE__); // "RFLink.ino" version is in 20;00 Message
  Serial.println(F("Compiled on :\t\t" __DATE__ " at " __TIME__));


#if defined(RFLINK_WIFIMANAGER_ENABLED) || defined(RFLINK_WIFI_ENABLED)
RFLink::Wifi::setup();
#endif // RFLINK_WIFIMANAGER_ENABLED

#ifdef MQTT_ENABLED
  RFLink::Mqtt::setup_MQTT();
  RFLink::Mqtt::reconnect(1);
#endif // MQTT_ENABLED

PluginInit();
PluginTXInit();
set_Radio_mode(Radio_OFF);

#if ((defined(ESP8266) || defined(ESP32)) && !defined(RFM69_ENABLED))
  show_Radio_Pin();
#endif // ESP8266 || ESP32

#ifdef OLED_ENABLED
  setup_OLED();
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
  RFLink::Mqtt::publishMsg();
#endif
  pbuffer[0] = 0;
  set_Radio_mode(Radio_RX);

#ifdef RFLINK_SERIAL2NET_ENABLED
RFLink::Serial2Net::startServer();
#endif // RFLINK_SERIAL2NET_ENABLED

}

void loop()
{
  #ifdef MQTT_ENABLED
  RFLink::Mqtt::checkMQTTloop();
  RFLink::sendMsgFromBuffer();
  #endif

  #if defined(RFLINK_WIFIMANAGER_ENABLED) || defined(RFLINK_WIFI_ENABLED)
  RFLink::Wifi::mainLoop();
  #endif

  #ifdef RFLINK_SERIAL2NET_ENABLED
  RFLink::Serial2Net::serverLoop();
  #endif // RFLINK_SERIAL2NET_ENABLED

  #if defined(SERIAL_ENABLED) && PIN_RF_TX_DATA_0 != NOT_A_PIN
  readSerialAndExecute();
  #endif

  if (ScanEvent())
    RFLink::sendMsgFromBuffer();
}

namespace RFLink {

  void sendMsgFromBuffer()
  {
    if (pbuffer[0] != 0)
    {

    #ifdef SERIAL_ENABLED
        Serial.print(pbuffer);
    #endif

    #ifdef MQTT_ENABLED
        RFLink::Mqtt::publishMsg();
    #endif

    #ifdef RFLINK_SERIAL2NET_ENABLED
      RFLink::Serial2Net::broadcastMessage(pbuffer);
    #endif // RFLINK_SERIAL2NET_ENABLED

    #ifdef OLED_ENABLED
        print_OLED();
    #endif

      pbuffer[0] = 0;
    }
  }

  bool executeCliCommand(const char *cmd) {
    static byte ValidCommand = 0;
    if (strlen(cmd) > 7)
    { // need to see minimal 8 characters on the serial port
      // 10;....;..;ON;
      if (strncmp(cmd, "10;", 3) == 0)
      { // Command from Master to RFLink
        // -------------------------------------------------------
        // Handle Device Management Commands
        // -------------------------------------------------------
        if (strncasecmp(cmd + 3, "PING;",5) == 0)
        {
          display_Header();
          display_Name(PSTR("PONG"));
          display_Footer();
        }
        else if (strncasecmp(cmd + 3, "REBOOT;",7) == 0)
        {
          display_Header();
          display_Name(PSTR("REBOOT"));
          display_Footer();
          CallReboot();
        }
        else if (strncasecmp(cmd + 3, "RFDEBUG=O", 9) == 0)
        {
          if (cmd[12] == 'N' || cmd[12] == 'n')
          {
            RFDebug = true;    // full debug on
            QRFDebug = false;  // q full debug off
            RFUDebug = false;  // undecoded debug off
            QRFUDebug = false; // q undecoded debug off
            display_Header();
            display_Name(PSTR("RFDEBUG=ON"));
            display_Footer();
          }
          else
          {
            RFDebug = false; // full debug off
            display_Header();
            display_Name(PSTR("RFDEBUG=OFF"));
            display_Footer();
          }
        }
        else if (strncasecmp(cmd + 3, "RFUDEBUG=O", 10) == 0)
        {
          if (cmd[13] == 'N' || cmd[13] == 'n')
          {
            RFDebug = false;   // full debug off
            QRFDebug = false;  // q debug off
            RFUDebug = true;   // undecoded debug on
            QRFUDebug = false; // q undecoded debug off
            display_Header();
            display_Name(PSTR("RFUDEBUG=ON"));
            display_Footer();
          }
          else
          {
            RFUDebug = false; // undecoded debug off
            display_Header();
            display_Name(PSTR("RFUDEBUG=OFF"));
            display_Footer();
          }
        }
        else if (strncasecmp(cmd + 3, "QRFDEBUG=O", 10) == 0)
        {
          if (cmd[13] == 'N' || cmd[13] == 'n')
          {
            RFDebug = false;   // full debug off
            QRFDebug = true;   // q debug on
            RFUDebug = false;  // undecoded debug off
            QRFUDebug = false; // q undecoded debug off
            display_Header();
            display_Name(PSTR("QRFDEBUG=ON"));
            display_Footer();
          }
          else
          {
            QRFDebug = false; // q debug off
            display_Header();
            display_Name(PSTR("QRFDEBUG=OFF"));
            display_Footer();
          }
        }
        else if (strncasecmp(cmd + 3, "QRFUDEBUG=O", 11) == 0)
        {
          if (cmd[14] == 'N' || cmd[14] == 'n')
          {
            RFDebug = false;  // full debug off
            QRFDebug = false; // q debug off
            RFUDebug = false; // undecoded debug off
            QRFUDebug = true; // q undecoded debug on
            display_Header();
            display_Name(PSTR("QRFUDEBUG=ON"));
            display_Footer();
          }
          else
          {
            QRFUDebug = false; // q undecode debug off
            display_Header();
            display_Name(PSTR("QRFUDEBUG=OFF"));
            display_Footer();
          }
        }
        else if (strncasecmp(cmd + 3, "VERSION", 7) == 0)
        {
          display_Header();
          display_Splash();
          display_Footer();
        }
        else
        {
          // -------------------------------------------------------
          // Handle Generic Commands / Translate protocol data into Nodo text commands
          // -------------------------------------------------------
          set_Radio_mode(Radio_TX);

          if (PluginTXCall(0, cmd))
            ValidCommand = 1;
          else // Answer that an invalid command was received?
            ValidCommand = 2;

          set_Radio_mode(Radio_RX);
        }
      }
    } // if > 7
    if (ValidCommand != 0)
    {
      display_Header();
      if (ValidCommand == 1)
        display_Name(PSTR("OK"));
      else
        display_Name(PSTR("CMD UNKNOWN"));
      display_Footer();
    }
    ValidCommand = 0;
    sendMsgFromBuffer(); // in case there is a response waiting to be sent
    return true;
  }


}

/*********************************************************************************************/