/*********************************************************************************************/
boolean ScanEvent(void) {                                    // Deze routine maakt deel uit van de hoofdloop en wordt iedere 125uSec. doorlopen
  unsigned long Timer = millis() + SCAN_HIGH_TIME_MS;

  while (Timer > millis() || RepeatingTimer > millis()) {
    if (FetchSignal()) {                                     // RF: *** data start ***
      if (PluginRXCall(0, 0)) {                              // Check all plugins to see which plugin can handle the received signal.
        RepeatingTimer = millis() + SIGNAL_REPEAT_TIME_MS;
        return true;
      }
    }
  }// while
  return false;
}

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
/*********************************************************************************************/
