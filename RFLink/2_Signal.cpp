// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Marc RIVES             * //
// * More details in RFLink.ino file   * //
// ************************************* //

#include <Arduino.h>
#include "1_Radio.h"
#include "2_Signal.h"
#include "5_Plugin.h"

#ifndef USE_ASYNC_RECEIVER
RawSignalStruct RawSignal = {0, 0, 0, 0, 0UL};
#endif // USE_ASYNC_RECEIVER
unsigned long SignalCRC = 0L;   // holds the bitstream value for some plugins to identify RF repeats
unsigned long SignalCRC_1 = 0L; // holds the previous SignalCRC (for mixed burst protocols)
byte SignalHash = 0L;           // holds the processed plugin number
byte SignalHashPrevious = 0L;   // holds the last processed plugin number
unsigned long RepeatingTimer = 0L;

#ifdef USE_ASYNC_RECEIVER
namespace AsyncSignalScanner {
    RawSignalStruct RawSignal = {0, 0, 0, 0, 0UL, false};
    hw_timer_t * nextPulseTimeoutTimer =  timerBegin(3, ESP.getCpuFreqMHz()/1000000000, true);

    void startScanning() {
      RawSignal.Number = 0;
      RawSignal.Time = 0;
      RawSignal.readyForDecoder = false;
      attachInterrupt(PIN_RF_RX_DATA, RX_pin_changed_state, CHANGE);
    }

    void stopScanning() {
      detachInterrupt(PIN_RF_RX_DATA);
      RawSignal.Number = 0;
      RawSignal.Time = 0;
      RawSignal.readyForDecoder = false;
      timerStop(nextPulseTimeoutTimer);
    }

    void IRAM_ATTR RX_pin_changed_state() {
      static unsigned long previousPulseTime_us = 0;
      unsigned long changeTime_us = micros();

      //Serial.println("changed state!");


      if (RawSignal.readyForDecoder) // it means previous packet has not been decoded yet, let's forget about it
        return; 

      unsigned long pulseLength_us = changeTime_us - previousPulseTime_us;
      previousPulseTime_us = changeTime_us;

      if(pulseLength_us < MIN_PULSE_LENGTH_US){ // this is too short, noise?
        timerStop(nextPulseTimeoutTimer);
        RawSignal.Number = 0;
        RawSignal.Time = 0;
      }

      int pinState = digitalRead(PIN_RF_RX_DATA);

      if (RawSignal.Time == 0) {            // this is potentially the beginning of a new signal
        if( pinState != 1)                  // if we get 0 here it means that we are in the middle of a signal, let's forget about it
          return;
        
        RawSignal.Time = millis();          // record when this signal started
        timerAttachInterrupt(nextPulseTimeoutTimer, &onPulseTimerTimeout, true);
        timerAlarmWrite(nextPulseTimeoutTimer, SIGNAL_END_TIMEOUT_US, true);
        timerAlarmEnable(nextPulseTimeoutTimer);
        timerStart(nextPulseTimeoutTimer);

        return;
      }

      if ( pulseLength_us > SIGNAL_END_TIMEOUT_US ) {
        onPulseTimerTimeout();
        return;
      }

      RawSignal.Number++;

      if (RawSignal.Number >= RAW_BUFFER_SIZE ) {      // this signal has too many pulses and will be dicarded
        timerStop(nextPulseTimeoutTimer);
        RawSignal.Number = 0;
        RawSignal.Time = 0;
        Serial.println("this signal has too many pulses and will be dicarded");
        return;
      }

      if (RawSignal.Number == 0 && pulseLength_us < SIGNAL_MIN_PREAMBLE_US) {   // too short preamnble, let's drop it
        timerStop(nextPulseTimeoutTimer);
        RawSignal.Number = 0;
        RawSignal.Time = 0;
        Serial.print("too short preamnble, let's drop it:");Serial.println(pulseLength_us);
        return;
      }

      //Serial.print("found pulse #");Serial.println(RawSignal.Number);
      timerAlarmWrite(nextPulseTimeoutTimer, SIGNAL_END_TIMEOUT_US, true);    // reset the pulse timeout value
      RawSignal.Pulses[RawSignal.Number] = pulseLength_us / RAWSIGNAL_SAMPLE_RATE;


    }

    void onPulseTimerTimeout() {
      if (RawSignal.readyForDecoder){  // it means previous packet has not been decoded yet, let's forget about it
        Serial.println("previous signal not decoded yet, discarding this one");
        return;
      }

      /*if (digitalRead(PIN_RF_RX_DATA) == HIGH) {   // We have a corrupted packet here
        Serial.println("corrupted signal ends with HIGH");
        RawSignal.Number = 0;
        RawSignal.Time = 0;
        timerStop(nextPulseTimeoutTimer);
        return;
      }*/

      if ( RawSignal.Number == 0) {  // timeout on preamble!
        Serial.println("timeout on preamble");
        timerStop(nextPulseTimeoutTimer);
        RawSignal.Number = 0;
        RawSignal.Time = 0;
        return;
      }

       if ( RawSignal.Number < MIN_RAW_PULSES) {  // not enough pulses, we ignore it
        timerStop(nextPulseTimeoutTimer);
        RawSignal.Number = 0;
        RawSignal.Time = 0;
        return;
      }
      
      // finally we have one!
      RawSignal.Number++;
      RawSignal.Pulses[RawSignal.Number] = SIGNAL_END_TIMEOUT_US / RAWSIGNAL_SAMPLE_RATE;
      Serial.print("found one packet, marking now for decoding. Pulses = ");Serial.println(RawSignal.Number);
      RawSignal.readyForDecoder = true;
    }
};

using namespace AsyncSignalScanner;

boolean ScanEvent(void) {
  if (!AsyncSignalScanner::RawSignal.readyForDecoder)
    return false;
  if (PluginRXCall(0, 0))
  { // Check all plugins to see which plugin can handle the received signal.
      RepeatingTimer = millis() + SIGNAL_REPEAT_TIME_MS;
      AsyncSignalScanner::RawSignal.Number = 0;
      AsyncSignalScanner::RawSignal.Time = 0;
      AsyncSignalScanner::RawSignal.readyForDecoder = false;
      return true;
  }
  AsyncSignalScanner::RawSignal.Number = 0;
  AsyncSignalScanner::RawSignal.Time = 0;
  AsyncSignalScanner::RawSignal.readyForDecoder = false;
  return false;
}
#else
/*********************************************************************************************/
boolean ScanEvent(void)
{ // Deze routine maakt deel uit van de hoofdloop en wordt iedere 125uSec. doorlopen
  unsigned long Timer = millis() + SCAN_HIGH_TIME_MS;

  while (Timer > millis()) // || RepeatingTimer > millis())
  {
    // delay(1); // For Modem Sleep
    if (FetchSignal())
    { // RF: *** data start ***
      if (PluginRXCall(0, 0))
      { // Check all plugins to see which plugin can handle the received signal.
        RepeatingTimer = millis() + SIGNAL_REPEAT_TIME_MS;
        return true;
      }
    }
  } // while
  return false;
}
#endif // USE_ASYNC_RECEIVER_CALL

#if (defined(ESP32) || defined(ESP8266))
// ***********************************************************************************
boolean FetchSignal()
{
  // *********************************************************************************
  static bool Toggle;
  static unsigned long timeStartSeek_ms;
  static unsigned long timeStartLoop_us;
  static unsigned int RawCodeLength;
  static unsigned long PulseLength_us;
  static const bool Start_Level = LOW;
  // *********************************************************************************

#define RESET_SEEKSTART timeStartSeek_ms = millis();
#define RESET_TIMESTART timeStartLoop_us = micros();
#define CHECK_RF ((digitalRead(PIN_RF_RX_DATA) == Start_Level) ^ Toggle)
#define CHECK_TIMEOUT ((millis() - timeStartSeek_ms) < SIGNAL_SEEK_TIMEOUT_MS)
#define GET_PULSELENGTH PulseLength_us = micros() - timeStartLoop_us
#define SWITCH_TOGGLE Toggle = !Toggle
#define STORE_PULSE RawSignal.Pulses[RawCodeLength++] = PulseLength_us / RAWSIGNAL_SAMPLE_RATE

  // ***   Init Vars   ***
  Toggle = true;
  RawCodeLength = 0;
  PulseLength_us = 0;

  // ***********************************
  // ***   Scan for Preamble Pulse   ***
  // ***********************************
  RESET_SEEKSTART;

  while (PulseLength_us < SIGNAL_MIN_PREAMBLE_US)
  {
    while (CHECK_RF && CHECK_TIMEOUT)
      ;
    RESET_TIMESTART;
    SWITCH_TOGGLE;
    while (CHECK_RF && CHECK_TIMEOUT)
      ;
    GET_PULSELENGTH;
    SWITCH_TOGGLE;
    if (!CHECK_TIMEOUT)
      return false;
  }


  RESET_TIMESTART; // next pulse starts now before we do anything else
  //Serial.print ("PulseLength: "); Serial.println (PulseLength);
  STORE_PULSE;

  noInterrupts();

  // ************************
  // ***   Message Loop   ***
  // ************************
  while (RawCodeLength < RAW_BUFFER_SIZE)
  {
   
    while (CHECK_RF)
    {
      GET_PULSELENGTH;
      if (PulseLength_us > SIGNAL_END_TIMEOUT_US)
        break;
    }

    // next Pulse starts now (while we are busy doing calculation) 
    RESET_TIMESTART;

    // ***   Too short Pulse Check   ***
    if (PulseLength_us < MIN_PULSE_LENGTH_US)
    {
      // NO RawCodeLength++;
      interrupts();
      return false; // Or break; instead, if you think it may worth it.
    }

    // ***   Ending Pulse Check   ***
    if (PulseLength_us > SIGNAL_END_TIMEOUT_US) // Again, in main while this time
    {
      RawCodeLength++;
      break;
    }

    // ***   Prepare Next   ***
    SWITCH_TOGGLE;

    // ***   Store Pulse   ***
    STORE_PULSE;
  }
  interrupts();
  //Serial.print ("RawCodeLength: ");
  //Serial.println (RawCodeLength);

  if (RawCodeLength >= MIN_RAW_PULSES)
  {
    RawSignal.Pulses[RawCodeLength] = 0;  // Last element contains the timeout.
    RawSignal.Number = RawCodeLength - 1; // Number of received pulse times (pulsen *2)
    RawSignal.Multiply = RAWSIGNAL_SAMPLE_RATE;
    RawSignal.Time = millis(); // Time the RF packet was received (to keep track of retransmits
    //Serial.print ("D");
    //Serial.print (RawCodeLength);
    return true;
  }
  else
  {
    RawSignal.Number = 0;
  }

  return false;
}
#endif
// ***********************************************************************************

#if (defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__))
// ***********************************************************************************
boolean FetchSignal()
{

  // Because this is a time critical routine, we use static variables so that
  // variables do not need to be initialized at each function call.
  static const uint8_t Fbit = digitalPinToBitMask(PIN_RF_RX_DATA);
  static const uint8_t Fport = digitalPinToPort(PIN_RF_RX_DATA);
  static const uint8_t FstateMask = (HIGH ? Fbit : 0); // When the 433RX is at rest, the output is low. StateSignal must be HIGH
  //

#if F_CPU == 16000000U
#warning "F_CPU @16MHz, LoopsPerMilli = 480UL"
  // Arduino IDE 1.8.9 - Pro-mini @16MHz
  static const unsigned long LoopsPerMilli = 480UL;
  static const unsigned long Overhead = 2UL; // 6/1000mS*LoopsPerMilli
#define LoopsPerMilliSet
#endif
#if F_CPU == 8000000U
#warning "F_CPU @8MHz, LoopsPerMilli = 240UL"
  // Arduino IDE 1.8.9 - Pro-mini @8MHz
  static const unsigned long LoopsPerMilli = 240UL;
  static const unsigned long Overhead = 2UL; // 6/1000mS*LoopsPerMilli
#define LoopsPerMilliSet
#endif
#ifndef LoopsPerMilliSet
#error "F_CPU not set or unexpected Frequency"
#endif

  static const unsigned long maxloops = (SIGNAL_END_TIMEOUT_US * LoopsPerMilli) / 1000UL;
  //
  static unsigned long PulseLength;
  static unsigned long TimeLength;
  //
  static unsigned int RawCodeLength; // Now unsigned
  static boolean Ftoggle;
  static unsigned long numloops;

  if ((*portInputRegister(Fport) & Fbit) == FstateMask)
  { // If there is a signal
    // If it is a repeating signal, chances are that we will be in this again in a very short time
    // routine return and then end up in the middle of the next repetition. That is why in this
    // case waited until the pulses are over and we start capturing data after a short one
    // rest between the signals. In this way the number of pointless captures is reduced.
    if (RawSignal.Time) //  First a quick check, because this is in a time-critical part ...
    {
      TimeLength = RawSignal.Time + SIGNAL_REPEAT_TIME_MS;
      if (RawSignal.Repeats && millis() < TimeLength) // ... because this check takes a few micros longer!
      {
        PulseLength = micros() + SIGNAL_END_TIMEOUT_US; // Delay
        while (millis() < TimeLength && micros() < PulseLength)
          ;
        {
          if ((*portInputRegister(Fport) & Fbit) == FstateMask)
          {
            PulseLength = micros() + SIGNAL_END_TIMEOUT_US;
          }
        }
        while (millis() < TimeLength && (*portInputRegister(Fport) & Fbit) != FstateMask)
          ;
      }
    }

    RawCodeLength = 1U; // Start at 1 for legacy reasons. Element 0 can be used to pass special information like plugin number etc.
    Ftoggle = false;
    do
    { // Read the pulses in microseconds and place them in temporary buffer RawSignal
      numloops = 0UL;
      while (((*portInputRegister(Fport) & Fbit) == FstateMask) ^ Ftoggle) // while() loop *A*
      {
        if (numloops++ == maxloops)
          break; // timeout
      }

      PulseLength = ((numloops + Overhead) * 1000UL) / LoopsPerMilli; // Contains pulselength in microseconds
      if (PulseLength < MIN_PULSE_LENGTH_US)
        break; // Pulse length too short
      Ftoggle = !Ftoggle;
      RawSignal.Pulses[RawCodeLength++] = PulseLength / (unsigned long)(RAWSIGNAL_SAMPLE_RATE); // store in RawSignal !!!!
    } while (RawCodeLength < RAW_BUFFER_SIZE && numloops <= maxloops);                          // For as long as there is space in the buffer, no timeout etc.
    if (RawCodeLength >= MIN_RAW_PULSES)
    {
      RawSignal.Repeats = 0;                      // No repeats
      RawSignal.Multiply = RAWSIGNAL_SAMPLE_RATE; // Sample size.
      RawSignal.Number = RawCodeLength - 1;       // Number of received pulse times (pulsen *2)
      RawSignal.Pulses[RawSignal.Number + 1] = 0; // Last element contains the timeout.
      RawSignal.Time = millis();                  // Time the RF packet was received (to keep track of retransmits
      return true;
    }
    else
    {
      RawSignal.Number = 0;
    }
  }
  return false;
}
// *********************************************************************************

#endif
// ***********************************************************************************

/*********************************************************************************************/
/*
  // RFLink Board specific: Generate a short pulse to switch the Aurel Transceiver from TX to RX mode.
  void RFLinkHW( void) {
     delayMicroseconds(36);
     digitalWrite(PIN_BSF_0,LOW);
     delayMicroseconds(16);
     digitalWrite(PIN_BSF_0,HIGH);
     return;
  }
*/
/*********************************************************************************************\
   Send rawsignal buffer to RF  * DEPRICATED * DO NOT USE
\*********************************************************************************************/
/*
  void RawSendRF(void) {                                                    // * DEPRICATED * DO NOT USE *
  int x;
  digitalWrite(PIN_RF_RX_VCC,LOW);                                        // Spanning naar de RF ontvanger uit om interferentie met de zender te voorkomen.
  digitalWrite(PIN_RF_TX_VCC,HIGH);                                       // zet de 433Mhz zender aan
  delayMicroseconds(TRANSMITTER_STABLE_DELAY);                            // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)

  RawSignal.Pulses[RawSignal.Number]=1;                                   // due to a bug in Arduino 1.0.1

  for(byte y=0; y<RawSignal.Repeats; y++) {                               // herhaal verzenden RF code
     x=1;
     noInterrupts();
     while(x<RawSignal.Number) {
        digitalWrite(PIN_RF_TX_DATA,HIGH);
        delayMicroseconds(RawSignal.Pulses[x++]*RawSignal.Multiply-5);    // min een kleine correctie
        digitalWrite(PIN_RF_TX_DATA,LOW);
        delayMicroseconds(RawSignal.Pulses[x++]*RawSignal.Multiply-7);    // min een kleine correctie
     }
     interrupts();
     if (y+1 < RawSignal.Repeats) delay(RawSignal.Delay);                 // Delay buiten het gebied waar de interrupts zijn uitgeschakeld! Anders werkt deze funktie niet.
  }

  delayMicroseconds(TRANSMITTER_STABLE_DELAY);                            // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)
  digitalWrite(PIN_RF_TX_VCC,LOW);                                        // zet de 433Mhz zender weer uit
  digitalWrite(PIN_RF_RX_VCC,HIGH);                                       // Spanning naar de RF ontvanger weer aan.
  // RFLinkHW();
  }
*/

/*********************************************************************************************\
   Send bitstream to RF - Plugin 004 (Newkaku) special version
\*********************************************************************************************/
void AC_Send(unsigned long data, byte cmd)
{
#define AC_FPULSE 260 // Pulse width in microseconds
#define AC_FRETRANS 5 // Number of code retransmissions

  // Serial.println("Send AC");
  // Serial.println(data, HEX);
  // Serial.println(cmd, HEX);

  unsigned long bitstream = 0L;
  byte command = 0;
  // prepare data to send
  for (unsigned short i = 0; i < 32; i++)
  { // reverse data bits
    bitstream <<= 1;
    bitstream |= (data & B1);
    data >>= 1;
  }
  if (cmd != 0xff)
  { // reverse dim bits
    for (unsigned short i = 0; i < 4; i++)
    {
      command <<= 1;
      command |= (cmd & B1);
      cmd >>= 1;
    }
  }
  // send bits
  for (byte nRepeat = 0; nRepeat < AC_FRETRANS; nRepeat++)
  {
    data = bitstream;
    if (cmd != 0xff)
      cmd = command;
    digitalWrite(PIN_RF_TX_DATA, HIGH);
    //delayMicroseconds(fpulse);  //335
    delayMicroseconds(335);
    digitalWrite(PIN_RF_TX_DATA, LOW);
    delayMicroseconds(AC_FPULSE * 10 + (AC_FPULSE >> 1)); //335*9=3015 //260*10=2600
    for (unsigned short i = 0; i < 32; i++)
    {
      if (i == 27 && cmd != 0xff)
      { // DIM command, send special DIM sequence TTTT replacing on/off bit
        digitalWrite(PIN_RF_TX_DATA, HIGH);
        delayMicroseconds(AC_FPULSE);
        digitalWrite(PIN_RF_TX_DATA, LOW);
        delayMicroseconds(AC_FPULSE);
        digitalWrite(PIN_RF_TX_DATA, HIGH);
        delayMicroseconds(AC_FPULSE);
        digitalWrite(PIN_RF_TX_DATA, LOW);
        delayMicroseconds(AC_FPULSE);
      }
      else
        switch (data & B1)
        {
        case 0:
          digitalWrite(PIN_RF_TX_DATA, HIGH);
          delayMicroseconds(AC_FPULSE);
          digitalWrite(PIN_RF_TX_DATA, LOW);
          delayMicroseconds(AC_FPULSE);
          digitalWrite(PIN_RF_TX_DATA, HIGH);
          delayMicroseconds(AC_FPULSE);
          digitalWrite(PIN_RF_TX_DATA, LOW);
          delayMicroseconds(AC_FPULSE * 5); // 335*3=1005 260*5=1300  260*4=1040
          break;
        case 1:
          digitalWrite(PIN_RF_TX_DATA, HIGH);
          delayMicroseconds(AC_FPULSE);
          digitalWrite(PIN_RF_TX_DATA, LOW);
          delayMicroseconds(AC_FPULSE * 5);
          digitalWrite(PIN_RF_TX_DATA, HIGH);
          delayMicroseconds(AC_FPULSE);
          digitalWrite(PIN_RF_TX_DATA, LOW);
          delayMicroseconds(AC_FPULSE);
          break;
        }
      //Next bit
      data >>= 1;
    }
    // send dim bits when needed
    if (cmd != 0xff)
    { // need to send DIM command bits
      for (unsigned short i = 0; i < 4; i++)
      { // 4 bits
        switch (cmd & B1)
        {
        case 0:
          digitalWrite(PIN_RF_TX_DATA, HIGH);
          delayMicroseconds(AC_FPULSE);
          digitalWrite(PIN_RF_TX_DATA, LOW);
          delayMicroseconds(AC_FPULSE);
          digitalWrite(PIN_RF_TX_DATA, HIGH);
          delayMicroseconds(AC_FPULSE);
          digitalWrite(PIN_RF_TX_DATA, LOW);
          delayMicroseconds(AC_FPULSE * 5); // 335*3=1005 260*5=1300
          break;
        case 1:
          digitalWrite(PIN_RF_TX_DATA, HIGH);
          delayMicroseconds(AC_FPULSE);
          digitalWrite(PIN_RF_TX_DATA, LOW);
          delayMicroseconds(AC_FPULSE * 5);
          digitalWrite(PIN_RF_TX_DATA, HIGH);
          delayMicroseconds(AC_FPULSE);
          digitalWrite(PIN_RF_TX_DATA, LOW);
          delayMicroseconds(AC_FPULSE);
          break;
        }
        //Next bit
        cmd >>= 1;
      }
    }
    //Send termination/synchronisation-signal. Total length: 32 periods
    digitalWrite(PIN_RF_TX_DATA, HIGH);
    delayMicroseconds(AC_FPULSE);
    digitalWrite(PIN_RF_TX_DATA, LOW);
    delayMicroseconds(AC_FPULSE * 40); //31*335=10385 40*260=10400
  }
  // End transmit
}
/*********************************************************************************************/