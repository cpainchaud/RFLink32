# RFLink ESP
This is an RF to MQTT gateway build for an ESP8266 board (see #MCU for more). 

It receive OOK 433MHz signals, then it identifies, decodes and push them over MQTT.

Projet is based on RFlink project "R29" (see rflink.nl, latest known open source shard version).

## 1. MCU
We use extensively Wemos ESP8266 D1 mini clone.

This is default settings in Platformio.ini and RFLink.h files.

You may use:
- Other ESP8266/ESP8255 based boards, when no pins limitations. NodeMCUv2 is known working.
- ESP32 based boards should work too. Although we need feedbacks there.
- Arduino board (Uno, Pro Mini, Mega) are working too, of course without WiFi/MQTT part.

## 2. Receiver / Transmitter / Transceiver
We mainly use RXB6 receiver.
It is simple, steady, running on 3.3v, easy to find and cheap.

Many other receivers will do!
Simply *** Please avoid generic noname receiver ***

![Receivers](https://github.com/couin3/RFLink/blob/master/pictures/RFLink-ESP_Receivers.jpg "Receivers")

## 3. OLED display
You can use an OLED display! We used SSD1306 128x64 I2C screen for our testings.

*** This is highly experimental ***, and thus not activated by default.

![OLED](https://github.com/couin3/RFLink/blob/master/pictures/RFLink-ESP_OLED_2.jpg "OLED") 

## 4. IDE
- We strongly recommend using PlatformIO IDE (https://platformio.org/install)
- You may alternatively use Arduino IDE 1.8.10 (https://www.arduino.cc/en/Guide/HomePage)

## 5. Framework
We use Arduino Core for ESP8266 https://github.com/esp8266/Arduino

## 6. Libraries
So far, in addition of core libraries, we use:
- PubSubClient for MQTT messaging https://github.com/knolleary/pubsubclient
- u8g2/u8x8 library for OLED display https://github.com/olikraus/u8g2
- AutoConnect for simplified configuration (incomming v2.0) https://hieromon.github.io/AutoConnect

## 7. COMPILE OPTIONS AND FLAGS
Many features are not enabled by default or can be disabled for various reasons : firmware size, compability etc etc. Here is a listing with some instructions:
### MQTT Server (disabled by default)
- Define compilation flag MQTT_ENABLED or define it in 6_WIFI_MQTT.h
- If you have not enabled WifiManager, make sure you update Wifi settings in 6_Credentials.h

## 8. Additional info
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

![Pinout](https://github.com/couin3/RFLink/blob/master/pictures/RFLink-ESP_Pinout.jpg "Pinout") 

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
