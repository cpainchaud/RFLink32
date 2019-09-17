/*********************************************************************************************/
/*********************************************************************************************/
boolean FetchSignal() {

  // Because this is a time critical routine, we use static variables so that
  // variables do not need to be initialized at each function call.
  static const uint8_t Fbit = digitalPinToBitMask(PIN_RF_RX_DATA);
  static const uint8_t Fport = digitalPinToPort(PIN_RF_RX_DATA);
  static const uint8_t FstateMask = (HIGH ? Fbit : 0);                          // When the 433RX is at rest, the output is low. StateSignal must be HIGH
  //
  static const unsigned long LoopsPerMilli = 480UL; // 345UL;                   // Around 480, Arduino IDE 1.8.9 - Arduino Nano
  static const unsigned long Overhead = 2UL;                                    // 6/1000mS*LoopsPerMilli
  static const unsigned long maxloops = SIGNAL_TIMEOUT * LoopsPerMilli;
  //
  static unsigned long PulseLength;
  static unsigned long TimeLength;
  //
  static unsigned int RawCodeLength; // Now unsigned
  static boolean Ftoggle;
  static unsigned long numloops;

  if ((*portInputRegister(Fport) & Fbit) == FstateMask) {                       // If there is a signal
    // If it is a repeating signal, chances are that we will be in this again in a very short time
    // routine return and then end up in the middle of the next repetition. That is why in this
    // case waited until the pulses are over and we start capturing data after a short one
    // rest between the signals. In this way the number of pointless captures is reduced.
    if (RawSignal.Time)                                                         //  First a quick check, because this is in a time-critical part ...
    {
      TimeLength = RawSignal.Time + SIGNAL_REPEAT_TIME;
      if (RawSignal.Repeats && millis() < TimeLength)                           // ... because this check takes a few micros longer!
      {
        PulseLength = micros() + SIGNAL_TIMEOUT * 1000UL;                       // Delay
        while (millis() < TimeLength && micros() < PulseLength);
        {
          if ((*portInputRegister(Fport) & Fbit) == FstateMask)
          {
            PulseLength = micros() + SIGNAL_TIMEOUT * 1000UL;
          }
        }
        while (millis() < TimeLength && (*portInputRegister(Fport) & Fbit) != FstateMask);
      }
    }

    RawCodeLength = 1U;                                                         // Start at 1 for legacy reasons. Element 0 can be used to pass special information like plugin number etc.
    Ftoggle = false;
    do {                                                                        // Read the pulses in microseconds and place them in temporary buffer RawSignal
      numloops = 0UL;
      while (((*portInputRegister(Fport) & Fbit) == FstateMask) ^ Ftoggle)      // while() loop *A*
      {
        if (numloops++ == maxloops) break;                                      // timeout
      }

      PulseLength = ((numloops + Overhead) * 1000UL) / LoopsPerMilli;           // Contains pulselength in microseconds
      if (PulseLength < MIN_PULSE_LENGTH) break;                                // Pulse length too short
      Ftoggle = !Ftoggle;
      RawSignal.Pulses[RawCodeLength++] = PulseLength / (unsigned long)(RAWSIGNAL_SAMPLE_RATE); // store in RawSignal !!!!
    } while (RawCodeLength < RAW_BUFFER_SIZE && numloops <= maxloops);           // For as long as there is space in the buffer, no timeout etc.
    if (RawCodeLength >= MIN_RAW_PULSES) {
      RawSignal.Repeats = 0;                                                    // No repeats
      RawSignal.Multiply = RAWSIGNAL_SAMPLE_RATE;                               // Sample size.
      RawSignal.Number = RawCodeLength - 1;                                     // Number of received pulse times (pulsen *2)
      RawSignal.Pulses[RawSignal.Number + 1] = 0;                               // Last element contains the timeout.
      RawSignal.Time = millis();                                                // Time the RF packet was received (to keep track of retransmits
      return true;
    } else {
      RawSignal.Number = 0;
    }
  }
  return false;
}
/*********************************************************************************************/
