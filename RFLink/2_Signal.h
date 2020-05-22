/// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef Signal_h
#define Signal_h

#include <Arduino.h>

#define RAW_BUFFER_SIZE 292         // 292        // Maximum number of pulses that is received in one go.
#define MIN_RAW_PULSES 50           // 50         // Minimal number of bits that need to have been received before we spend CPU time on decoding the signal.
#define RAWSIGNAL_SAMPLE_RATE 32    // 32         // =8 bits. Sample width / resolution in uSec for raw RF pulses.
#define SIGNAL_SEEK_TIMEOUT_MS 25   // 25         // After this time in mSec, RF signal will be considered absent.
#define SIGNAL_MIN_PREAMBLE_US 300  // 300        // After this time in uSec, a RF signal will be considered to have started.
#define MIN_PULSE_LENGTH_US 100     // 250        // Pulses shorter than this value in uSec. will be seen as garbage and not taken as actual pulses.
#define SIGNAL_END_TIMEOUT_US 5000  // 4500       // After this time in uSec, the RF signal will be considered to have stopped.
#define SIGNAL_REPEAT_TIME_MS 250   // 500        // Time in mSec. in which the same RF signal should not be accepted again. Filters out retransmits.
#define SCAN_HIGH_TIME_MS 50        // 50         // time interval in ms. fast processing for background tasks

struct RawSignalStruct // Raw signal variabelen places in a struct
{
  int Number;                       // Number of pulses, times two as every pulse has a mark and a space.
  byte Repeats;                     // Number of re-transmits on transmit actions.
  byte Delay;                       // Delay in ms. after transmit of a single RF pulse packet
  byte Multiply;                    // Pulses[] * Multiply is the real pulse time in microseconds
  unsigned long Time;               // Timestamp indicating when the signal was received (millis())
  byte Pulses[RAW_BUFFER_SIZE + 1]; // Table with the measured pulses in microseconds divided by RawSignal.Multiply. (halves RAM usage)
  // First pulse is located in element 1. Element 0 is used for special purposes, like signalling the use of a specific plugin
};

extern RawSignalStruct RawSignal;
extern unsigned long SignalCRC;   // holds the bitstream value for some plugins to identify RF repeats
extern unsigned long SignalCRC_1; // holds the previous SignalCRC (for mixed burst protocols)
extern byte SignalHash;           // holds the processed plugin number
extern byte SignalHashPrevious;   // holds the last processed plugin number
extern unsigned long RepeatingTimer;

boolean FetchSignal();
boolean ScanEvent(void);
// void RFLinkHW(void);
// void RawSendRF(void);

void AC_Send(unsigned long data, byte cmd);

#endif
