// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef RFLink_h
#define RFLink_h

//
#define BUILDNR 0x04                    // 0x07       // shown in version
#define REVNR 0x00                      // 0X42       // shown in version and startup string
#define BAUD 57600                      // 57600      // Baudrate for serial communication.
#define FOCUS_TIME_MS 50                // 50         // Duration in mSec. that, after receiving serial data from USB only the serial port is checked.
#define PLUGIN_MAX 55                   // 55         // Maximum number of Receive plugins
#define PLUGIN_TX_MAX 5                 // 26         // Maximum number of Transmit plugins
#define INPUT_COMMAND_SIZE 60           // 60         // Maximum number of characters that a command via serial can be.
#define PRINT_BUFFER_SIZE 90            // 90         // Maximum number of characters that a command should print in one go via the print buffer.

#define BUILDNR 0x04 // 0x07       // shown in version
#define REVNR 0x00   // 0X42       // shown in version and startup string

// OLED display, 0.91" SSD1306 I2C
// #define OLED_ENABLED

// WIFI
#if (defined(ESP32) || defined(ESP8266))
#define WIFI_PWR_0 10 // 0~20.5dBm
#define AUTOCONNECT_ENABLED
#endif

// MQTT messages
#define SERIAL_ENABLED // Send RFLink messages over Serial
#if (defined(ESP32) || defined(ESP8266))
#define MQTT_ENABLED          // Send RFLink messages over MQTT
#define MQTT_LOOP_MS 1000     // MQTTClient.loop(); call period (in mSec)
#define MQTT_RETAINED_0 false // Retained option
#endif

// Debug default
#define RFDebug_0 false   // debug RF signals with plugin 001 (no decode)
#define QRFDebug_0 false  // debug RF signals with plugin 001 but no multiplication (faster?, compact)
#define RFUDebug_0 false  // debug RF signals with plugin 254 (decode 1st)
#define QRFUDebug_0 false // debug RF signals with plugin 254 but no multiplication (faster?, compact)

void CallReboot(void);

#endif
