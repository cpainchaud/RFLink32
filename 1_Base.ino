//****************************************************************************************************************************************
// void(*Reboot)(void) = 0;                                                        // reset function on adres 0.
byte PKSequenceNumber = 0;                                                      // 1 byte packet counter
boolean RFDebug = false;                                                        // debug RF signals with plugin 001
boolean RFUDebug = false;                                                       // debug RF signals with plugin 254
boolean QRFDebug = false;                                                       // debug RF signals with plugin 254 but no multiplication

char pbuffer[PRINT_BUFFER_SIZE];                                                // Buffer for printing data
char MQTTbuffer[PRINT_BUFFER_SIZE];                                             // Buffer for MQTT message
char InputBuffer_Serial[INPUT_COMMAND_SIZE];                                    // Buffer for Seriel data

// Of all the devices that are compiled, the addresses are stored in a table so that you can jump to them
void PluginInit(void);
boolean (*Plugin_ptr[PLUGIN_MAX])(byte, char*);                                 // Receive plugins
byte Plugin_id[PLUGIN_MAX];

void PrintHex8(uint8_t *data, uint8_t length);                                  // prototype
void PrintHexByte(uint8_t data);                                                // prototype
byte reverseBits(byte data);                                                    // prototype
void RFLinkHW( void );                                                          // prototype

struct RawSignalStruct                                                          // Raw signal variabelen places in a struct
{
  int  Number;                                                                  // Number of pulses, times two as every pulse has a mark and a space.
  byte Repeats;                                                                 // Number of re-transmits on transmit actions.
  byte Delay;                                                                   // Delay in ms. after transmit of a single RF pulse packet
  byte Multiply;                                                                // Pulses[] * Multiply is the real pulse time in microseconds
  unsigned long Time;                                                           // Timestamp indicating when the signal was received (millis())
  byte Pulses[RAW_BUFFER_SIZE + 2];                                             // Table with the measured pulses in microseconds divided by RawSignal.Multiply. (halves RAM usage)
  // First pulse is located in element 1. Element 0 is used for special purposes, like signalling the use of a specific plugin
} RawSignal = {0, 0, 0, 0, 0UL};

// ===============================================================================
unsigned long RepeatingTimer = 0L;
unsigned long SignalCRC = 0L;                                                   // holds the bitstream value for some plugins to identify RF repeats
unsigned long SignalHash = 0L;                                                  // holds the processed plugin number
unsigned long SignalHashPrevious = 0L;                                          // holds the last processed plugin number

void setup() {

  Serial.begin(BAUD);                                                           // Initialise the serial port
  Serial.println(); // ESP "Garbage" message

  pinMode(PIN_RF_RX_DATA, INPUT);                                               // Initialise in/output ports
  pinMode(PIN_RF_TX_DATA, OUTPUT);                                              // Initialise in/output ports
  pinMode(PIN_RF_TX_VCC,  OUTPUT);                                              // Initialise in/output ports
  pinMode(PIN_RF_TX_GND,  OUTPUT);                                              // Initialise in/output ports
  pinMode(PIN_RF_RX_VCC,  OUTPUT);                                              // Initialise in/output ports
  pinMode(PIN_RF_RX_GND,  OUTPUT);                                              // Initialise in/output ports
  digitalWrite(PIN_RF_TX_GND, LOW);                                             // turn GND to TX receiver ON
  digitalWrite(PIN_RF_RX_GND, LOW);                                             // turn GND to RF receiver ON
  digitalWrite(PIN_RF_RX_VCC, HIGH);                                            // turn VCC to RF receiver ON
  digitalWrite(PIN_RF_RX_DATA, INPUT_PULLUP);                                   // pull-up resister on (to prevent garbage)

  PluginInit();

#if defined(MQTT_ACTIVATED) && (defined(ESP32) || defined(ESP8266))
  setup_WIFI();
  setup_MQTT();
#else
  setup_WIFI_OFF();
#endif

  sprintf_P(pbuffer, PSTR("%S"), F("20;00;Nodo RadioFrequencyLink - RFLink Gateway V1.1 - "));
  Serial.print(pbuffer);

#if defined(MQTT_ACTIVATED) && (defined(ESP32) || defined(ESP8266))
  strcpy(MQTTbuffer, pbuffer);
#endif

  sprintf_P(pbuffer, PSTR("R%02x;"), REVNR);
  Serial.println(pbuffer);

#if defined(MQTT_ACTIVATED) && (defined(ESP32) || defined(ESP8266))
  strcat(MQTTbuffer, pbuffer);
  strcat(MQTTbuffer, "\r\n");
#endif

  PKSequenceNumber++;

#if defined(MQTT_ACTIVATED) && (defined(ESP32) || defined(ESP8266))
  publishMsg();
#endif
}

void loop() {

  if (ScanEvent()) {
#if defined(MQTT_ACTIVATED) && (defined(ESP32) || defined(ESP8266))
    publishMsg();
#else
    MQTTbuffer[0] = 0;
#endif
  }
}
/*********************************************************************************************/
