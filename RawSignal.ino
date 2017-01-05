/*********************************************************************************************/
boolean ScanEvent(void) {                                         // Deze routine maakt deel uit van de hoofdloop en wordt iedere 125uSec. doorlopen
  unsigned long Timer=millis()+SCAN_HIGH_TIME;

#ifdef PLUGIN_250
  Plugin_250_Scan();
#endif

  while(Timer>millis() || RepeatingTimer>millis()) {

    if (FetchSignal(PIN_IR_RX_DATA,LOW)) {                    // IR
      if ( PluginRXCall(0,0) ) {                              // Check all plugins to see which plugin can handle the received signal.
        RepeatingTimer=millis()+SIGNAL_REPEAT_TIME;
        return true;
      }
    }

    if (FetchSignal(PIN_RF_RX_DATA,HIGH)) {                    // RF: *** data start ***
      if ( PluginRXCall(0,0) ) {                                // Check all plugins to see which plugin can handle the received signal.
        RepeatingTimer=millis()+SIGNAL_REPEAT_TIME;
        return true;
      }
    }
  }// while

    return false;
}
/**********************************************************************************************\
 * Haal de pulsen en plaats in buffer. 
 * bij de TSOP1738 is in rust is de uitgang hoog. StateSignal moet LOW zijn
 * bij de 433RX is in rust is de uitgang laag. StateSignal moet HIGH zijn
 * 
 \*********************************************************************************************/
const unsigned long LoopsPerMilli=345;
const unsigned long Overhead=0;  

// Because this is a time critical routine, we use global variables so that the variables 
// do not need to be initialized at each function call. 
int RawCodeLength=0;
unsigned long PulseLength=0L;
unsigned long numloops=0L;
unsigned long maxloops=0L;

boolean Ftoggle=false;
uint8_t Fbit=0;
uint8_t Fport=0;
uint8_t FstateMask=0;
/*********************************************************************************************/
boolean FetchSignal(byte DataPin, boolean StateSignal) {
  uint8_t Fbit = digitalPinToBitMask(DataPin);
  uint8_t Fport = digitalPinToPort(DataPin);
  uint8_t FstateMask = (StateSignal ? Fbit : 0);

  if ((*portInputRegister(Fport) & Fbit) == FstateMask) {                       // Als er signaal is
    // Als het een herhalend signaal is, dan is de kans groot dat we binnen hele korte tijd weer in deze
    // routine terugkomen en dan midden in de volgende herhaling terecht komen. Daarom wordt er in dit
    // geval gewacht totdat de pulsen voorbij zijn en we met het capturen van data beginnen na een korte 
    // rust tussen de signalen.Op deze wijze wordt het aantal zinloze captures teruggebracht.
    if (RawSignal.Time) {                                                       //  Eerst een snelle check, want dit bevindt zich in een tijdkritisch deel...
      if (RawSignal.Repeats && (RawSignal.Time+SIGNAL_REPEAT_TIME)>millis()) { // ...want deze check duurt enkele micro's langer!
        PulseLength=micros()+SIGNAL_TIMEOUT*1000;                             // delay
        while ((RawSignal.Time+SIGNAL_REPEAT_TIME)>millis() && PulseLength>micros())
          if ((*portInputRegister(Fport) & Fbit) == FstateMask)
            PulseLength=micros()+SIGNAL_TIMEOUT*1000;
        while((RawSignal.Time+SIGNAL_REPEAT_TIME)>millis() &&  (*portInputRegister(Fport) & Fbit) != FstateMask);
      }
    }
    RawCodeLength=1;                                                            // Start at 1 for legacy reasons. Element 0 can be used to pass special i    formation like plugin number etc.
      Ftoggle=false;                  
    maxloops = (SIGNAL_TIMEOUT * LoopsPerMilli);  
    do{                                                                         // read the pulses in microseconds and place them in temporay buffer RawSignal
        numloops = 0;
      while (((*portInputRegister(Fport) & Fbit) == FstateMask) ^ Ftoggle)      // while() loop *A*
        if (numloops++ == maxloops) break;                                        // timeout 
      PulseLength=((numloops + Overhead)* 1000) / LoopsPerMilli;                // Contains pulslength in microseconds
      if (PulseLength<MIN_PULSE_LENGTH) break;                                  // Pulse length too short
      Ftoggle=!Ftoggle;    
      RawSignal.Pulses[RawCodeLength++]=PulseLength/(unsigned long)(RAWSIGNAL_SAMPLE_RATE); // store in RawSignal !!!! 
    } 
    while (RawCodeLength<RAW_BUFFER_SIZE && numloops<=maxloops);               // For as long as there is space in the buffer, no timeout etc.
    if (RawCodeLength>=MIN_RAW_PULSES) {
      RawSignal.Repeats=0;                                                      // no repeats
      RawSignal.Multiply=RAWSIGNAL_SAMPLE_RATE;                                 // sample size.
      RawSignal.Number=RawCodeLength-1;                                         // Number of received pulse times (pulsen *2)
      RawSignal.Pulses[RawSignal.Number+1]=0;                                   // Last element contains the timeout. 
      RawSignal.Time=millis();                                                  // Time the RF packet was received (to keep track of retransmits
      return true;
    } 
    else {
      RawSignal.Number=0;    
    }
  }
  return false;
}


void RFLinkHW()
{
  // not in use on Nodo Small boards.
}


/*********************************************************************************************\
 * Send rawsignal buffer to RF  * DEPRICATED * DO NOT USE *
 \*********************************************************************************************/
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
}
/*********************************************************************************************/



