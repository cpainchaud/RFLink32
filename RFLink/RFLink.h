// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef RFLink_h
#define RFLink_h

#include <ArduinoJson.h>
#include <time.h>
#include <sys/time.h>

#define BUILDNR 0x05 // 0x07       // shown in version
#define REVNR 0x01   // 0X42       // shown in version and startup string

#define SERIAL_ENABLED // Send RFLink messages over Serial
//#define RFLINK_AUTOOTA_ENABLED // if you want to the device to self-update at boot from a given URKL
                          // dont forget to set the URL in Crendentials.h  

#if (defined(ESP32) || defined(ESP8266))
// OLED display, 0.91" SSD1306 I2C
// #define OLED_ENABLED
#define OLED_CONTRAST 32 // default 255 (max)
#define OLED_FLIP true   // default false

// WIFI
#define WIFI_PWR_0 20 // 0~20.5dBm
//#define RFLINK_SHOW_CONFIG_PORTAL_PIN_BUTTON 32 // if you want start the configuration portal with a button/pin
#ifndef RFLINK_WIFIMANAGER_PORTAL_LONG_PRESS
#define RFLINK_WIFIMANAGER_PORTAL_LONG_PRESS 1000 // milliseconds
#endif

// MQTT messages
//#define MQTT_ENABLED          // Send RFLink messages over MQTT
#define MQTT_LOOP_MS 1000     // MQTTClient.loop(); call period (in mSec)
#define MQTT_RETAINED_0 false // Retained option
#define MQTT_LWT              // Let know if Module is Online or Offline via MQTT Last Will message
// #define MQTT_SSL           // Send MQTT messages over SSL
// #define CHECK_CACERT       // Send MQTT SSL CA Certificate
#endif

// Debug default
#define RFDebug_0 false   // debug RF signals with plugin 001 (no decode)
#define QRFDebug_0 false  // debug RF signals with plugin 001 but no multiplication (faster?, compact)
#define RFUDebug_0 false  // debug RF signals with plugin 254 (decode 1st)
#define QRFUDebug_0 false // debug RF signals with plugin 254 but no multiplication (faster?, compact)

namespace RFLink {

    extern struct timeval timeAtBoot; // used to calculate update

    void setup();

    bool executeCliCommand(const char *cmd);
    void sendMsgFromBuffer();

    void getStatusJsonString(JsonObject &output);
}

void CallReboot(void);

#endif