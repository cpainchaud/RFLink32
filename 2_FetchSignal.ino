// ***********************************************************************************
boolean FetchSignal () {
  // *********************************************************************************
  static bool                Toggle;
  static unsigned long       timeStartSeek_ms;
  static unsigned long       timeStartLoop_us;
  static unsigned int        RawCodeLength;
  static unsigned long       PulseLength_us;
  static const bool          Start_Level = LOW;
  // *********************************************************************************


#define RESET_SEEKSTART timeStartSeek_ms = millis();
#define RESET_TIMESTART   timeStartLoop_us = micros();
#define CHECK_RF        ((digitalRead(PIN_RF_RX_DATA) == Start_Level) ^ Toggle)
#define CHECK_TIMEOUT   ((millis() - timeStartSeek_ms) < SIGNAL_SEEK_TIMEOUT_MS)
#define GET_PULSELENGTH PulseLength_us = micros() - timeStartLoop_us
#define SWITCH_TOGGLE   Toggle = !Toggle
#define STORE_PULSE     RawSignal.Pulses[RawCodeLength++] = PulseLength_us / RAWSIGNAL_SAMPLE_RATE

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
    while (CHECK_RF && CHECK_TIMEOUT);
    RESET_TIMESTART;
    SWITCH_TOGGLE;
    while (CHECK_RF && CHECK_TIMEOUT);
    GET_PULSELENGTH;
    SWITCH_TOGGLE;
    if (!CHECK_TIMEOUT) return false;
    }
  //Serial.print ("PulseLength: "); Serial.println (PulseLength);
  STORE_PULSE;

  // ************************
  // ***   Message Loop   ***
  // ************************
  while (RawCodeLength <= RAW_BUFFER_SIZE) {

    // ***   Time Pulse   ***
    RESET_TIMESTART;
    while (CHECK_RF)
    {
      GET_PULSELENGTH;
      if (PulseLength_us > SIGNAL_END_TIMEOUT_US) break;
    }

    // ***   Too short Pulse Check   ***
    if (PulseLength_us < MIN_PULSE_LENGTH_US)
    {
      // NO RawCodeLength++;
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
  //Serial.print ("RawCodeLength: ");
  //Serial.println (RawCodeLength);

  if (RawCodeLength >= MIN_RAW_PULSES)
  {
    RawSignal.Pulses[RawCodeLength] = 0;          // Last element contains the timeout.
    RawSignal.Number = RawCodeLength - 1;         // Number of received pulse times (pulsen *2)
    RawSignal.Multiply = RAWSIGNAL_SAMPLE_RATE;
    RawSignal.Time = millis();                    // Time the RF packet was received (to keep track of retransmits
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
// ***********************************************************************************
