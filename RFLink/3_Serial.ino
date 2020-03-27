/*********************************************************************************************/
/*
boolean CheckSerial()
{
  static byte SerialInByte = 0;                                               // incoming character value
  static int SerialInByteCounter = 0;                                         // number of bytes counter
  static byte ValidCommand = 0;
  static unsigned long FocusTimer;                                            // Timer to keep focus on the task during communication

  InputBuffer_Serial[0] = 0;                                                  // erase serial buffer string

  // SERIAL: *************** Check if there is data ready on the serial port **********************
  if (Serial.available()) {
    FocusTimer = millis() + FOCUS_TIME_MS;

    while (millis() < FocusTimer) {                                           // standby
      if (Serial.available()) {
        SerialInByte = Serial.read();

        if (isprint(SerialInByte))
          if (SerialInByteCounter < (INPUT_COMMAND_SIZE - 1))
            InputBuffer_Serial[SerialInByteCounter++] = SerialInByte;

        if (SerialInByte == '\n') {                                           // new line character
          InputBuffer_Serial[SerialInByteCounter] = 0;                        // serieel data is complete
          //Serial.print("20;incoming;");
          //Serial.println(InputBuffer_Serial);
          if (strlen(InputBuffer_Serial) > 7) {                               // need to see minimal 8 characters on the serial port
            // 10;....;..;ON;
            if (strncmp (InputBuffer_Serial, "10;", 3) == 0) {               // Command from Master to RFLink
              // -------------------------------------------------------
              // Handle Device Management Commands
              // -------------------------------------------------------
              if (strcasecmp(InputBuffer_Serial + 3, "PING;") == 0) {
                sprintf_P(InputBuffer_Serial, PSTR("20;%02X;PONG;"), PKSequenceNumber++);
                Serial.println(InputBuffer_Serial);
              } else if (strcasecmp(InputBuffer_Serial + 3, "REBOOT;") == 0) {
                strcpy(InputBuffer_Serial, "reboot");
                // Reboot();
              } else if (strncasecmp(InputBuffer_Serial + 3, "RFDEBUG=O", 9) == 0) {
                if (InputBuffer_Serial[12] == 'N' || InputBuffer_Serial[12] == 'n' ) {
                  RFDebug = true;                                        // full debug on
                  RFUDebug = false;                                      // undecoded debug off
                  QRFDebug = false;                                      // undecoded debug off
                  sprintf_P(InputBuffer_Serial, PSTR("20;%02X;RFDEBUG=ON;"), PKSequenceNumber++);
                } else {
                  RFDebug = false;                                        // full debug off
                  sprintf_P(InputBuffer_Serial, PSTR("20;%02X;RFDEBUG=OFF;"), PKSequenceNumber++);
                }
                Serial.println(InputBuffer_Serial);
              } else if (strncasecmp(InputBuffer_Serial + 3, "RFUDEBUG=O", 10) == 0) {
                if (InputBuffer_Serial[13] == 'N' || InputBuffer_Serial[13] == 'n') {
                  RFUDebug = true;                                       // undecoded debug on
                  QRFDebug = false;                                      // undecoded debug off
                  RFDebug = false;                                       // full debug off
                  sprintf_P(InputBuffer_Serial, PSTR("20;%02X;RFUDEBUG=ON;"), PKSequenceNumber++);
                } else {
                  RFUDebug = false;                                       // undecoded debug off
                  sprintf_P(InputBuffer_Serial, PSTR("20;%02X;RFUDEBUG=OFF;"), PKSequenceNumber++);
                }
                Serial.println(InputBuffer_Serial);
              } else if (strncasecmp(InputBuffer_Serial + 3, "QRFDEBUG=O", 10) == 0) {
                if (InputBuffer_Serial[13] == 'N' || InputBuffer_Serial[13] == 'n') {
                  QRFDebug = true;                                       // undecoded debug on
                  RFUDebug = false;                                      // undecoded debug off
                  RFDebug = false;                                       // full debug off
                  sprintf_P(InputBuffer_Serial, PSTR("20;%02X;QRFDEBUG=ON;"), PKSequenceNumber++);
                } else {
                  QRFDebug = false;                                      // undecoded debug off
                  sprintf_P(InputBuffer_Serial, PSTR("20;%02X;QRFDEBUG=OFF;"), PKSequenceNumber++);
                }
                Serial.println(InputBuffer_Serial);
              } else if (strncasecmp(InputBuffer_Serial + 3, "VERSION", 7) == 0) {
                sprintf_P(InputBuffer_Serial, PSTR("20;%02X;VER=1.1;REV=%02x;BUILD=%02x;"), PKSequenceNumber++, REVNR, BUILDNR);
                Serial.println(InputBuffer_Serial);
              } else {
                // -------------------------------------------------------
                // Handle Generic Commands / Translate protocol data into Nodo text commands
                // -------------------------------------------------------
                // check plugins
                if (InputBuffer_Serial[SerialInByteCounter - 1] == ';') InputBuffer_Serial[SerialInByteCounter - 1] = 0; // remove last ";" char
                if (PluginTXCall(0, InputBuffer_Serial)) {
                  ValidCommand = 1;
                } else {
                  // Answer that an invalid command was received?
                  ValidCommand = 2;
                }
              }
            }
          } // if > 7
          if (ValidCommand != 0) {
            if (ValidCommand == 1) {
              sprintf_P(InputBuffer_Serial, PSTR("20;%02X;OK;"), PKSequenceNumber++);
              Serial.println( InputBuffer_Serial );
            } else {
              sprintf_P(InputBuffer_Serial, PSTR("20;%02X;CMD UNKNOWN;"), PKSequenceNumber++); // Node and packet number
              Serial.println( InputBuffer_Serial );
            }
          }
          SerialInByteCounter = 0;
          InputBuffer_Serial[0] = 0;                                          // serial data has been processed.
          ValidCommand = 0;
          FocusTimer = millis() + FOCUS_TIME_MS;
        }// if(SerialInByte
      }// if(Serial.available())
    }// while
  }// if(Serial.available())

  return true;
}
*/

/*********************************************************************************************/
