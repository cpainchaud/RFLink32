// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef Radio_h
#define Radio_h

#include <Arduino.h>
#include <11_Config.h>

#define TRANSMITTER_STABLE_DELAY_US 500 // 500        // Delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms).
#define PULLUP_RF_RX_DATA_0 false       // false      // Sometimes a pullup in needed on RX data pin

#undef BUILTIN_LED
#define BUILTIN_LED 9

// PIN Definition
//

#ifdef ESP8266
// ESP8266 D1 Mini
#define PIN_RF_RX_PMOS_0 NOT_A_PIN // High Side P-MOSFET, active on LOW level
#define PIN_RF_RX_NMOS_0 D5        // Low Side N-MOSFET, active on HIGH level
#define PIN_RF_RX_VCC_0 NOT_A_PIN  // Power to the receiver on this pin
#define PIN_RF_RX_GND_0 NOT_A_PIN  // Ground to the receiver on this pin
#define PIN_RF_RX_NA_0 NOT_A_PIN   // Alt. RX_DATA. Forced as input
#define PIN_RF_RX_DATA_0 D6        // On this input, the 433Mhz-RF signal is received. LOW when no signal.
#define PIN_RF_TX_PMOS_0 NOT_A_PIN // High Side P-MOSFET, active on LOW level
#define PIN_RF_TX_NMOS_0 D7        // Low Side N-MOSFET, active on HIGH level
#define PIN_RF_TX_VCC_0 NOT_A_PIN  // +5 volt / Vcc power to the transmitter on this pin
#define PIN_RF_TX_GND_0 NOT_A_PIN  // Ground power to the transmitter on this pin
#define PIN_RF_TX_NA_0 NOT_A_PIN   // Spare RX pin. Forced as input
#define PIN_RF_TX_DATA_0 D4        // Data to the 433Mhz transmitter on this pin
#endif

#ifdef ESP32
#define PIN_RF_RX_PMOS_0 NOT_A_PIN // High Side P-MOSFET, active on LOW level
#define PIN_RF_RX_NMOS_0 NOT_A_PIN // Low Side N-MOSFET, active on HIGH level
#define PIN_RF_RX_VCC_0 NOT_A_PIN         // Power to the receiver on this pin
#define PIN_RF_RX_GND_0 NOT_A_PIN         // Ground to the receiver on this pin
#define PIN_RF_RX_NA_0 NOT_A_PIN          // Alt. RX_DATA. Forced as input
#define PIN_RF_RX_DATA_0 21        // On this input, the 433Mhz-RF signal is received. LOW when no signal.
#define PIN_RF_TX_PMOS_0 NOT_A_PIN // High Side P-MOSFET, active on LOW level
#define PIN_RF_TX_NMOS_0 NOT_A_PIN         // Low Side N-MOSFET, active on HIGH level
#define PIN_RF_TX_VCC_0 4          // +5 volt / Vcc power to the transmitter on this pin
#define PIN_RF_TX_GND_0 NOT_A_PIN         // Ground power to the transmitter on this pin
#define PIN_RF_TX_NA_0 NOT_A_PIN          // Spare RX pin. Forced as input
#define PIN_RF_TX_DATA_0 2         // Data to the 433Mhz transmitter on this pin
#endif

#ifdef __AVR_ATmega328P__
#define PIN_RF_RX_PMOS_0 NOT_A_PIN // High Side P-MOSFET, active on LOW level
#define PIN_RF_RX_NMOS_0 NOT_A_PIN // Low Side N-MOSFET, active on HIGH level
#define PIN_RF_RX_VCC_0 4          // Power to the receiver on this pin
#define PIN_RF_RX_GND_0 NOT_A_PIN  // Ground to the receiver on this pin
#define PIN_RF_RX_NA_0 3           // Alt. RX_DATA. Forced as input
#define PIN_RF_RX_DATA_0 2         // On this input, the 433Mhz-RF signal is received. LOW when no signal.
#define PIN_RF_TX_PMOS_0 NOT_A_PIN // High Side P-MOSFET, active on LOW level
#define PIN_RF_TX_NMOS_0 NOT_A_PIN // Low Side N-MOSFET, active on HIGH level
#define PIN_RF_TX_VCC_0 NOT_A_PIN  // +5 volt / Vcc power to the transmitter on this pin
#define PIN_RF_TX_GND_0 NOT_A_PIN  // Ground power to the transmitter on this pin
#define PIN_RF_TX_NA_0 NOT_A_PIN   // Spare RX pin. Forced as input
#define PIN_RF_TX_DATA_0 NOT_A_PIN // Data to the 433Mhz transmitter on this pin
#endif

#ifdef __AVR_ATmega2560__
#define PIN_RF_RX_PMOS_0 NOT_A_PIN // High Side P-MOSFET, active on LOW level
#define PIN_RF_RX_NMOS_0 NOT_A_PIN // Low Side N-MOSFET, active on HIGH level
#define PIN_RF_RX_VCC_0 16         // Power to the receiver on this pin
#define PIN_RF_RX_GND_0 NOT_A_PIN  // Ground to the receiver on this pin
#define PIN_RF_RX_NA_0 NOT_A_PIN   // Alt. RX_DATA. Forced as input
#define PIN_RF_RX_DATA_0 19        // On this input, the 433Mhz-RF signal is received. LOW when no signal.
#define PIN_RF_TX_PMOS_0 NOT_A_PIN // High Side P-MOSFET, active on LOW level
#define PIN_RF_TX_NMOS_0 NOT_A_PIN // Low Side N-MOSFET, active on HIGH level
#define PIN_RF_TX_VCC_0 15         // +5 volt / Vcc power to the transmitter on this pin
#define PIN_RF_TX_GND_0 NOT_A_PIN  // Ground power to the transmitter on this pin
#define PIN_RF_TX_NA_0 NOT_A_PIN   // Spare RX pin. Forced as input
#define PIN_RF_TX_DATA_0 14        // Data to the 433Mhz transmitter on this pin
#endif



namespace RFLink { namespace Radio {

    enum States
    {
        Radio_OFF,
        Radio_RX,
        Radio_TX,
        Radio_NA
    };

    enum HardwareType
    {
        HW_basic_t,
        HW_RFM69CW_t,
        HW_RFM69HCW_t,
        HW_SX7218_t,
        HW_EOF_t,
    };

    extern HardwareType hardware;
    extern States current_State;

    extern Config::ConfigItem configItems[];

    namespace pins {
        extern uint8_t RX_PMOS;
        extern uint8_t RX_NMOS;
        extern uint8_t RX_VCC;
        extern uint8_t RX_GND;
        extern uint8_t RX_NA;
        extern uint8_t RX_DATA;
        extern boolean PULLUP_RX_DATA;

        extern uint8_t TX_PMOS;
        extern uint8_t TX_NMOS;
        extern uint8_t TX_VCC;
        extern uint8_t TX_GND;
        extern uint8_t TX_NA;
        extern uint8_t TX_DATA;
    }
    

    
    void setup();
    void paramsUpdatedCallback();
    void refreshParametersFromConfig();


    /**
     * return HardwareType::HW_EOF_t when not found
     * */
    HardwareType hardwareIDFromString(const char *);

    void set_Radio_mode(States new_state);
    void show_Radio_Pin();

    /**
     * don't use directly unless you know what you are doing.
     * */
    void set_Radio_mode_generic(States new_state);
    /**
     * don't use directly unless you know what you are doing.
     * */
    void set_Radio_mode_RFM69(States new_state);
    /**
     * don't use directly unless you know what you are doing.
     * */
    void set_Radio_mode_SX7218(States new_state);
    
    /**
     * don't use directly unless you know what you are doing.
     * */
    void enableRX_generic();
    /**
     * don't use directly unless you know what you are doing.
     * */
    void disableRX_generic();
    /**
     * don't use directly unless you know what you are doing.
     * */
    void enableTX_generic();
    /**
     * don't use directly unless you know what you are doing.
     * */
    void disableTX_generic();

}}


#endif // Radio_h