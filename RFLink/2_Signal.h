/// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef Signal_h
#define Signal_h

#include <Arduino.h>
#include "RFLink.h"

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