#define BUILDNR                         0x07                                    // shown in version
#define REVNR                           0x33                                    // shown in version and startup string
#define MIN_RAW_PULSES                    20                                    // =8 bits. Minimal number of bits*2 that need to have been received before we spend CPU time on decoding the signal.
#define RAWSIGNAL_SAMPLE_RATE             30                                    // Sample width / resolution in uSec for raw RF pulses.
#define MIN_PULSE_LENGTH                  40                                    // Pulses shorter than this value in uSec. will be seen as garbage and not taken as actual pulses.
#define SIGNAL_TIMEOUT                     5                                    // Timeout, after this time in mSec. the RF signal will be considered to have stopped.
#define SIGNAL_REPEAT_TIME               250                                    // Time in mSec. in which the same RF signal should not be accepted again. Filters out retransmits.
#define BAUD                           57600                                    // Baudrate for serial communication.
#define TRANSMITTER_STABLE_DELAY         500                                    // delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms).
#define RAW_BUFFER_SIZE                  256                                    // Maximum number of pulses that is received in one go.
#define PLUGIN_MAX                        55                                    // Maximum number of Receive plugins
#define PLUGIN_TX_MAX                     26                                    // Maximum number of Transmit plugins
#define SCAN_HIGH_TIME                    50                                    // tijdsinterval in ms. voor achtergrondtaken snelle verwerking
#define FOCUS_TIME                        50                                    // Duration in mSec. that, after receiving serial data from USB only the serial port is checked. 
#define INPUT_COMMAND_SIZE                60                                    // Maximum number of characters that a command via serial can be.
#define PRINT_BUFFER_SIZE                 60                                   // Maximum number of characters that a command should print in one go via the print buffer.

// to include Config_xx.c files:
#define stringify(x) #x
#define CONFIGFILE2(a, b) stringify(a/Config/b)
#define CONFIGFILE(a, b) CONFIGFILE2(a, b)
#define CONFIG_FILE Config_01.c
#include CONFIGFILE(SKETCH_PATH,CONFIG_FILE)

#define VALUE_PAIR                      44
#define VALUE_ALLOFF                    55
#define VALUE_OFF                       74
#define VALUE_ON                        75
#define VALUE_DIM                       76
#define VALUE_BRIGHT                    77
#define VALUE_UP                        78
#define VALUE_DOWN                      79
#define VALUE_STOP                      80
#define VALUE_CONFIRM                   81
#define VALUE_LIMIT                     82
#define VALUE_ALLON                     141

// PIN Definition 
#define PIN_RF_TX_VCC                4                                          // +5 volt / Vcc spanning naar de zender.
#define PIN_RF_TX_DATA               5                                          // data naar de zender
#define PIN_RF_RX_DATA               2                                          // Op deze input komt het 433Mhz-RF signaal binnen. LOW bij geen signaal.
#define PIN_RF_RX_VCC               12                                          // Spanning naar de ontvanger via deze pin.
#define PIN_SPEAKER                  6                                          // Luidspreker aansluiting
#define PIN_IR_RX_DATA               3						// IR pin

//****************************************************************************************************************************************
byte dummy=1;                                                                   // get the linker going. Bug in Arduino. (old versions?)

void(*Reboot)(void)=0;                                                          // reset function on adres 0.
byte PKSequenceNumber=0;                                                        // 1 byte packet counter
boolean RFDebug=false;                                                          // debug RF signals with plugin 001 
boolean RFUDebug=false;                                                         // debug RF signals with plugin 254 
boolean QRFDebug=false;                                                         // debug RF signals with plugin 254 but no multiplication

uint8_t RFbit,RFport;                                                           // for processing RF signals.

char pbuffer[PRINT_BUFFER_SIZE];                                                // Buffer for printing data
char InputBuffer_Serial[INPUT_COMMAND_SIZE];                                    // Buffer for Seriel data

// Van alle devices die worden mee gecompileerd, worden in een tabel de adressen opgeslagen zodat hier naar toe gesprongen kan worden
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
  byte Pulses[RAW_BUFFER_SIZE+2];                                               // Table with the measured pulses in microseconds divided by RawSignal.Multiply. (halves RAM usage)
                                                                                // First pulse is located in element 1. Element 0 is used for special purposes, like signalling the use of a specific plugin
} RawSignal={0,0,0,0,0,0L};
// ===============================================================================
unsigned long RepeatingTimer=0L;
unsigned long SignalCRC=0L;                                                     // holds the bitstream value for some plugins to identify RF repeats
unsigned long SignalHash=0L;                                                    // holds the processed plugin number
unsigned long SignalHashPrevious=0L;                                            // holds the last processed plugin number

void setup() {
  Serial.begin(BAUD);                                                           // Initialise the serial port
  pinMode(PIN_RF_RX_DATA, INPUT);                                               // Initialise in/output ports
  pinMode(PIN_RF_TX_DATA, OUTPUT);                                              // Initialise in/output ports
  pinMode(PIN_RF_TX_VCC,  OUTPUT);                                              // Initialise in/output ports
  pinMode(PIN_RF_RX_VCC,  OUTPUT);                                              // Initialise in/output ports    
  digitalWrite(PIN_RF_RX_VCC,HIGH);                                             // turn VCC to RF receiver ON
  digitalWrite(PIN_RF_RX_DATA,INPUT_PULLUP);                                    // pull-up resister on (to prevent garbage)

  RFbit=digitalPinToBitMask(PIN_RF_RX_DATA);
  RFport=digitalPinToPort(PIN_RF_RX_DATA);
  Serial.print(F("20;00;Nodo RadioFrequencyLink - RFLink Gateway V1.1 - "));
  sprintf(InputBuffer_Serial,"R%02x;",REVNR);
  Serial.println(InputBuffer_Serial); 

  PKSequenceNumber++;
  PluginInit();
  PluginTXInit();
}

void loop() {
  byte SerialInByte=0;                                                          // incoming character value
  int SerialInByteCounter=0;                                                    // number of bytes counter 

  byte ValidCommand=0;
  unsigned long FocusTimer=0L;                                                  // Timer to keep focus on the task during communication
  InputBuffer_Serial[0]=0;                                                      // erase serial buffer string 

  while(true) {
    ScanEvent();                                                                // Scan for RF events
    // SERIAL: *************** kijk of er data klaar staat op de seriele poort **********************
    if(Serial.available()) {
      
      if (Serial.peek() == 0x84)
        {
          pinMode(6, OUTPUT);
          digitalWrite(6, LOW);
        }
          
      FocusTimer=millis()+FOCUS_TIME;

      while(FocusTimer>millis()) {                                              // standby 
        if(Serial.available()) {
          SerialInByte=Serial.read();                
          
          if(isprint(SerialInByte))
            if(SerialInByteCounter<(INPUT_COMMAND_SIZE-1))
              InputBuffer_Serial[SerialInByteCounter++]=SerialInByte;
              
          if(SerialInByte=='\n') {                                              // new line character
            InputBuffer_Serial[SerialInByteCounter]=0;                          // serieel data is complete
            if (strlen(InputBuffer_Serial) > 7){                                // need to see minimal 8 characters on the serial port
               if (strncmp (InputBuffer_Serial,"10;",3) == 0) {                 // Command from Master to RFLink
                  // -------------------------------------------------------
                  // Handle Device Management Commands
                  // -------------------------------------------------------
                  if (strcasecmp(InputBuffer_Serial+3,"PING;")==0) {
                     sprintf_P(InputBuffer_Serial,PSTR("20;%02X;PONG;"),PKSequenceNumber++);
                     Serial.println(InputBuffer_Serial); 
                  } else
                  if (strcasecmp(InputBuffer_Serial+3,"REBOOT;")==0) {
                     strcpy(InputBuffer_Serial,"reboot");
                     Reboot();
                  } else
                  if (strncasecmp(InputBuffer_Serial+3,"RFDEBUG=O",9) == 0) {
                     if (InputBuffer_Serial[12] == 'N' || InputBuffer_Serial[12] == 'n' ) {
                        RFDebug=true;                                           // full debug on
                        RFUDebug=false;                                         // undecoded debug off 
                        QRFDebug=false;                                        // undecoded debug off
                        sprintf_P(InputBuffer_Serial,PSTR("20;%02X;RFDEBUG=ON;"),PKSequenceNumber++);
                     } else {
                        RFDebug=false;                                          // full debug off
                        sprintf_P(InputBuffer_Serial,PSTR("20;%02X;RFDEBUG=OFF;"),PKSequenceNumber++);
                     }
                     Serial.println(InputBuffer_Serial); 
                  } else                 
                  if (strncasecmp(InputBuffer_Serial+3,"RFUDEBUG=O",10) == 0) {
                     if (InputBuffer_Serial[13] == 'N' || InputBuffer_Serial[13] == 'n') {
                        RFUDebug=true;                                          // undecoded debug on 
                        QRFDebug=false;                                        // undecoded debug off
                        RFDebug=false;                                          // full debug off
                        sprintf_P(InputBuffer_Serial,PSTR("20;%02X;RFUDEBUG=ON;"),PKSequenceNumber++);
                     } else {
                        RFUDebug=false;                                         // undecoded debug off
                        sprintf_P(InputBuffer_Serial,PSTR("20;%02X;RFUDEBUG=OFF;"),PKSequenceNumber++);
                     }
                     Serial.println(InputBuffer_Serial); 
                  } else                 
                  if (strncasecmp(InputBuffer_Serial+3,"QRFDEBUG=O",10) == 0) {
                     if (InputBuffer_Serial[13] == 'N' || InputBuffer_Serial[13] == 'n') {
                        QRFDebug=true;                                         // undecoded debug on 
                        RFUDebug=false;                                         // undecoded debug off 
                        RFDebug=false;                                          // full debug off
                        sprintf_P(InputBuffer_Serial,PSTR("20;%02X;QRFDEBUG=ON;"),PKSequenceNumber++);
                     } else {
                        QRFDebug=false;                                        // undecoded debug off
                        sprintf_P(InputBuffer_Serial,PSTR("20;%02X;QRFDEBUG=OFF;"),PKSequenceNumber++);
                     }
                     Serial.println(InputBuffer_Serial); 
                  } else                 
                  if (strncasecmp(InputBuffer_Serial+3,"VERSION",7) == 0) {
                      sprintf_P(InputBuffer_Serial,PSTR("20;%02X;VER=1.1;REV=%02x;BUILD=%02x;"),PKSequenceNumber++,REVNR, BUILDNR);
                      Serial.println(InputBuffer_Serial); 
                  } else {
                     // -------------------------------------------------------
                     // Handle Generic Commands / Translate protocol data into Nodo text commands 
                     // -------------------------------------------------------
                     // check plugins
                     if (InputBuffer_Serial[SerialInByteCounter-1]==';') InputBuffer_Serial[SerialInByteCounter-1]=0;  // remove last ";" char
                     if(PluginTXCall(0, InputBuffer_Serial)) {
                        ValidCommand=1;
                     } else {
                        // Answer that an invalid command was received?
                        ValidCommand=2;
                     }
                  }
               }
            } // if > 7
            if (ValidCommand != 0) {
               if (ValidCommand==1) {
                  sprintf_P(InputBuffer_Serial,PSTR("20;%02X;OK;"),PKSequenceNumber++);
                  Serial.println( InputBuffer_Serial ); 
               } else {
                  sprintf_P(InputBuffer_Serial, PSTR("20;%02X;CMD UNKNOWN;"), PKSequenceNumber++); // Node and packet number 
                  Serial.println( InputBuffer_Serial );
               }   
            }
            SerialInByteCounter=0;  
            InputBuffer_Serial[0]=0;                                            // serial data has been processed. 
            ValidCommand=0;
            FocusTimer=millis()+FOCUS_TIME;                                             
          }// if(SerialInByte
       }// if(Serial.available())
    }// while 
   }// if(Serial.available())
  }// while 
} // void
/*********************************************************************************************/

