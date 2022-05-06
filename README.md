# RFLink ESP

This is an Radio Frequency to MQTT/Serial/TCP gateway built for an ESP32 and eventually ESP8266 board (see #MCU for more). 

It receives and decodes OOK 433MHz signals from your sensors, alarms push them over MQTT/TCP/Serial.
For some devices it can also send commands to control them.

Project is forked RFLink-ESP (for ESP8266), itself forked from original RFlink project "Release 29" for Arduino only board.

This fork is providing additional features from the others:
- Fully (almost) configurable from a web interface, although CLI is still and will remain available.
- No more recompilation for most options which are configurable at runtime and saved in Flash.
- Far more advanced debugs and troubleshooting helpers.
- Restructured source code with namespaces and classes (Work in Progress)

## 1. MCU
We use extensively ESP32 dev kits.

This is default settings in Platformio.ini and RFLink.h files.

You may use:
- Other ESP8266/ESP8255 based boards, when no pins limitations. NodeMCUv2 is known working.


## 2. Receiver / Transmitter / Transceiver
As a starting point, you can use the RXB6 receiver.
It is simple, steady, running on 3.3v, easy to find and cheap.

Many other receivers will do!
Simply *** Please avoid generic noname receiver ***

![Receivers](https://github.com/cpainchaud/RFLink32/blob/master/pictures/RFLink-ESP_Receivers.jpg "Receivers")

For more advanced behavior and reliability, the following receivers are also supported when used with an ESP32 board:

* SX1278
* SX1276
* RFM69CW
* RFM69HCW
* CC1101

Those advanced receivers require a few pins to be connected to the host, here are the recommended pin assignments:

|  Name         | ESP32 | SX1278/6 | RFM69(H)CW | CC1101 |
|---------------|-------|----------|------------|--------|
|               |  3v3  |   VCC    |  3.3V      | VCC    |
|               |  GND  |   GND    |  GND       | GND    |
|               |  18   |   SCK    |  SCK       | SCK    |
|               |  19   |   MISO   |  MISO      | MISO   |
|               |  23   |   MOSI   |  MOSI      | MOSI   |
| pins::RX_RESET|  4¹   |  NRESET  |  RESET     |        |
| pins::RX_CS   |  5¹   |   NSS    |  NSS       | CSN    |
| pins::RX_DATA<br/>pins::TX_DATA |  26¹  |   DIO2   |  DIO2      | GDO0   |

¹ *These must be configured in the web portal, values suggested here are proven to work reliably.*

## 3. OLED display
You can use an OLED display! We used SSD1306 128x64 I2C screen for our testings.

*** This is highly experimental ***, and thus not activated by default.

![OLED](https://github.com/cpainchaud/RFLink32/blob/master/pictures/RFLink-ESP_OLED_2.jpg "OLED") 

## 4. IDE
- We strongly recommend using PlatformIO IDE (https://platformio.org/install)
- You may alternatively use Arduino IDE 1.8.10 (https://www.arduino.cc/en/Guide/HomePage)

## 5. Framework
We use Arduino Core for ESP8266 https://github.com/esp8266/Arduino

## 6. Libraries
So far, in addition of core libraries, we use:
- PubSubClient for MQTT messaging https://github.com/knolleary/pubsubclient
- u8g2/u8x8 library for OLED display https://github.com/olikraus/u8g2

## 7. COMPILE OPTIONS AND FLAGS
Many features are not enabled by default or can be disabled for various reasons : firmware size, compability etc etc. Here is a listing with some instructions:
### OTA (disabled by default)
There are 3 types of OTA tp update your firwware
#### Arduino/ESP's classic push over UDP (disabled by default)
- RFLINK_OTA_ENABLED we recommaend that you enable a password for this method or anyone on your network could push a new firmware
- RFLINK_OTA_PASSWORD="my_password_here" or RFLINK_OTA_PASSWORD='"'${sysenv.OTA_SEC}'"' in platforomio.ini with an environement variable called OTA_SEC
#### AutoOTA
Your device will download new firmware from a specific URL you specify.
- RFLINK_AUTOOTA_ENABLED
- AutoOTA_URL in Credentials.h or in platformio.ini
#### Config Portal Web Upload
Via WifiManager's Config Portal you can upload a new firmware
**insert screenshot here**

## 8. Additional info
### Default login and password of WebUI
rflink32 / 433mhz

### Pinout
- When WebServer is active (which is default), pin setup has to be done there.
- For safety & simplicity, default WebServer setup is : all pin inactive.
- You may add decoupling capacitors and antenna to improve results.
- This is a simple RX pin wiring :

|  Name         | D1 mini | RXB6  |
|---------------|---------|-------|
| PIN_RF_TX_VCC |   D5    | 5 VCC |
| PIN_RF_TX_NA  |   D6    | 6 DER |
| PIN_RF_TX_DATA|   D7    | 7 DAT |
| PIN_RF_TX_GND |   D8    | 8 GND |

![Pinout](https://github.com/cpainchaud/RFLink32/blob/master/pictures/RFLink-ESP_Pinout.jpg "Pinout") 

### Alternative Pinout
- ESP8266 can't draw more than 12mA on a GPIO pin.
- Some receivers have current spikes bigger than that (eg RXB12).
- This lead to a non working receiver / non starting ESP.
- Here is a more safe wiring

|  Name (Alt)   | D1 mini | RXB6  |
|---------------|---------|-------|
| PIN_RF_TX_VCC |   3v3   | 5 VCC |
| PIN_RF_TX_NA  |   N/A   | 6 DER |
| PIN_RF_TX_DATA|   D7    | 7 DAT |
| PIN_RF_TX_GND |   GND   | 8 GND |

### Thanks
Special thanks to: Axellum, Etimou, Schmurtz, Zoomx 
