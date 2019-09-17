/*********************************************************************************************/
boolean ScanEvent(void) {                                         // Deze routine maakt deel uit van de hoofdloop en wordt iedere 125uSec. doorlopen
  unsigned long Timer = millis() + SCAN_HIGH_TIME;

  while (Timer > millis() || RepeatingTimer > millis()) {
    //       if (FetchSignal(PIN_RF_RX_DATA,HIGH)) {                    // RF: *** data start ***
    if (FetchSignal()) {                                         // RF: *** data start ***
      if ( PluginRXCall(0, 0) ) {                               // Check all plugins to see which plugin can handle the received signal.
        RepeatingTimer = millis() + SIGNAL_REPEAT_TIME;
        return true;
      }
    }
  }// while
  return false;
}

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
/*
  // RFLink Board specific: Generate a short pulse to switch the Aurel Transceiver from TX to RX mode.
  void RFLinkHW( void ) {
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
/*********************************************************************************************/
