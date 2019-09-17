//****************************************************************************************************************************************
void(*Reboot)(void) = 0;                                                        // reset function on adres 0.
byte PKSequenceNumber = 0;                                                      // 1 byte packet counter
boolean RFDebug = false;                                                        // debug RF signals with plugin 001
boolean RFUDebug = false;                                                       // debug RF signals with plugin 254
boolean QRFDebug = false;                                                       // debug RF signals with plugin 254 but no multiplication

char pbuffer[PRINT_BUFFER_SIZE];                                                // Buffer for printing data
char InputBuffer_Serial[INPUT_COMMAND_SIZE];                                    // Buffer for Seriel data

// Of all the devices that are compiled, the addresses are stored in a table so that you can jump to them
void PluginInit(void);
void PluginTXInit(void);
boolean (*Plugin_ptr[PLUGIN_MAX])(byte, char*);                                 // Receive plugins
byte Plugin_id[PLUGIN_MAX];
boolean (*PluginTX_ptr[PLUGIN_TX_MAX])(byte, char*);                            // Transmit plugins
byte PluginTX_id[PLUGIN_TX_MAX];

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
} RawSignal = {0, 0, 0, 0, 0L, 0};
// ===============================================================================
unsigned long RepeatingTimer = 0L;
unsigned long SignalCRC = 0L;                                                   // holds the bitstream value for some plugins to identify RF repeats
unsigned long SignalHash = 0L;                                                  // holds the processed plugin number
unsigned long SignalHashPrevious = 0L;                                          // holds the last processed plugin number

void setup() {

  // Low Power Arduino
  ADCSRA = 0;             // disable ADC
  power_all_disable();    // turn off all modules
  power_timer0_enable();  // Timer 0
  power_usart0_enable();  // UART

  Serial.begin(BAUD);                                                           // Initialise the serial port
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

  /*
    pinMode(PIN_BSF_0, OUTPUT);                                                   // rflink board switch signal
    digitalWrite(PIN_BSF_0, HIGH);                                                // rflink board switch signal
  */

  Serial.print(F("20;00;Nodo RadioFrequencyLink - RFLink Gateway V1.1 - "));
  sprintf_P(pbuffer, PSTR("R%02x;"), REVNR);
  Serial.println(pbuffer);

  PKSequenceNumber++;
  PluginInit();
  PluginTXInit();
}

void loop() {
  ScanEvent();
  CheckSerial();
}
/*********************************************************************************************/
