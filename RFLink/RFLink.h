// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef RFLink_h
#define RFLink_h

//
#define BUILDNR 0x07                    // shown in version
#define REVNR 0x42                      // shown in version and startup string
#define BAUD 115200                     // Baudrate for serial communication.
#define MIN_RAW_PULSES 50               // Minimal number of bits that need to have been received before we spend CPU time on decoding the signal.
#define RAW_BUFFER_SIZE 512             // 148        // Maximum number of pulses that is received in one go.
#define RAWSIGNAL_SAMPLE_RATE 32        // =8 bits. Sample width / resolution in uSec for raw RF pulses.
#define SIGNAL_SEEK_TIMEOUT_MS 25       // After this time in mSec. RF signal will be considered absent.
#define SIGNAL_MIN_PREAMBLE_US 3000     //
#define MIN_PULSE_LENGTH_US 50          // 25         // Pulses shorter than this value in uSec. will be seen as garbage and not taken as actual pulses.
#define SIGNAL_END_TIMEOUT_US 3500      // After this time in uSec. the RF signal will be considered to have stopped.
#define SIGNAL_REPEAT_TIME_MS 250       // 500        // Time in mSec. in which the same RF signal should not be accepted again. Filters out retransmits.
#define TRANSMITTER_STABLE_DELAY_US 500 // delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms).
#define SCAN_HIGH_TIME_MS 50            // time interval in ms. fast processing for background tasks
#define FOCUS_TIME_MS 50                // Duration in mSec. that, after receiving serial data from USB only the serial port is checked.
#define PLUGIN_MAX 10                   // 55         // Maximum number of Receive plugins
#define PLUGIN_TX_MAX 0                 // 26         // Maximum number of Transmit plugins
#define INPUT_COMMAND_SIZE 60           // Maximum number of characters that a command via serial can be.
#define PRINT_BUFFER_SIZE 60            // Maximum number of characters that a command should print in one go via the print buffer.

/*
#define VALUE_PAIR                     44
#define VALUE_ALLOFF                   55
#define VALUE_OFF                      74
#define VALUE_ON                       75
#define VALUE_DIM                      76
#define VALUE_BRIGHT                   77
#define VALUE_UP                       78
#define VALUE_DOWN                     79
#define VALUE_STOP                     80
#define VALUE_CONFIRM                  81
#define VALUE_LIMIT                    82
#define VALUE_ALLON                   141
*/

// PIN Definition

// NodeMCUv2
#define PIN_RF_TX_VCC NOT_A_PIN  // +5 volt / Vcc power to the transmitter on this pin
#define PIN_RF_TX_GND NOT_A_PIN  // Ground power to the transmitter on this pin
#define PIN_RF_TX_DATA NOT_A_PIN // Data to the 433Mhz transmitter on this pin
#define PIN_RF_RX_VCC NOT_A_PIN  // Power to the receiver on this pin
#define PIN_RF_RX_GND NOT_A_PIN  // Ground to the receiver on this pin
#define PIN_RF_RX_DATA D1        // On this input, the 433Mhz-RF signal is received. LOW when no signal.

// MQTT messages
#define MQTT_ACTIVATED
#define MQTT_LOOP_MS 10000 // MQTTClient.loop(); call period (in mSec)

#endif