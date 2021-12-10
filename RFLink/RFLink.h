// ************************************* //
// * Arduino Project RFLink32        * //
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

#ifndef RFLINK_BUILDNAME
#define RFLINK_BUILDNAME "unknown"
#endif

#ifndef DEFAULT_WIFI_CLIENT_HOSTNAME
#define DEFAULT_WIFI_CLIENT_HOSTNAME "RFLink32"
#endif

#define SERIAL_ENABLED // Send RFLink messages over Serial
//#define RFLINK_AUTOOTA_ENABLED // if you want to the device to self-update at boot from a given URKL
                          // dont forget to set the URL in Crendentials.h

//#define RFLINK_MQTT_DISABLED    // to disable MQTT entirely (not compiled at all)
//#define RFLINK_PORTAL_DISABLED    // to disable Portal/Web UI

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
#ifndef RFLink_default_MQTT_ENABLED
  #define RFLink_default_MQTT_ENABLED false // Send RFLink messages over MQTT
#endif
#ifndef RFLink_default_MQTT_SSL_ENABLED
  #define RFLink_default_MQTT_SSL_ENABLED false // Send MQTT messages over SSL
#endif
#define MQTT_LOOP_MS 1000     // MQTTClient.loop(); call period (in mSec)
#define MQTT_RETAINED_0 false // Retained option
#ifndef RFLink_default_MQTT_LWT              // Let know if Module is Online or Offline via MQTT Last Will message
  #define RFLink_default_MQTT_LWT true
#endif
// #define CHECK_CACERT       // Send MQTT SSL CA Certificate

#endif // (defined(ESP32) || defined(ESP8266))

// Debug default
#define RFDebug_0 false   // debug RF signals with plugin 001 (no decode)
#define QRFDebug_0 false  // debug RF signals with plugin 001 but no multiplication (faster?, compact)
#define RFUDebug_0 false  // debug RF signals with plugin 254 (decode 1st)
#define QRFUDebug_0 false // debug RF signals with plugin 254 but no multiplication (faster?, compact)


namespace RFLink {

    namespace params {
      extern String ntpServer;
    }

    extern struct timeval timeAtBoot; // used to calculate update
    extern struct timeval scheduledRebootTime;

    extern char printBuf[300];

    void setup();
    void mainLoop();

    bool executeCliCommand(char *cmd);
    void sendMsgFromBuffer();
    void sendRawPrint(const char *buf, bool end_of_line=false);
    void sendRawPrint(const __FlashStringHelper *buf, bool end_of_line=false) ;
    void sendRawPrint(long n);
    void sendRawPrint(unsigned long n);
    void sendRawPrint(int n);
    void sendRawPrint(unsigned int n);
    void sendRawPrint(float f);
    void sendRawPrint(char c);
    inline void sendRawPrintln() {sendRawPrint(F("\r\n"));};
    //static void sendRawPrintf(const char *format, va_list args);
    //#define broadcastMessage_P(format, ...) sendRawPrintf(format, __VA_ARGS__)

    //void sendRawPrintf_P(PGM_P, ...);

    void getStatusJsonString(JsonObject &output);

    void scheduleReboot(unsigned int seconds);
}

void CallReboot(void);

#endif