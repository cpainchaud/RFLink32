/// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#include <Arduino.h>
#include "RFLink.h"
#include "3_Serial.h"
#include "4_Display.h"
#include "5_Plugin.h"

char InputBuffer_Serial[INPUT_COMMAND_SIZE];

boolean ReadSerial();
boolean CheckCmd();
/*********************************************************************************************/

boolean CheckSerial()
{
  if (ReadSerial())
    if (CheckCmd())
      return true;
  return false;
}

boolean ReadSerial()
{
  // SERIAL: *************** Check if there is data ready on the serial port **********************
  static byte SerialInByte;        // incoming character value
  static byte SerialInByteCounter; // number of bytes counter
  static unsigned long FocusTimer; // Timer to keep focus on the task during communication

  if (Serial.available())
  {
    SerialInByteCounter = 0;

    FocusTimer = millis() + FOCUS_TIME_MS;
    while (true)
    {
      if (Serial.available())
      {
        SerialInByte = Serial.read();

        if (isprint(SerialInByte))
          InputBuffer_Serial[SerialInByteCounter++] = SerialInByte;

        FocusTimer = millis() + FOCUS_TIME_MS;
      }

      if ((SerialInByte == '\n') || (millis() >= FocusTimer) || (SerialInByteCounter >= (INPUT_COMMAND_SIZE - 1)))
      // new line character or timeout or buffer full
      {
        // if (InputBuffer_Serial[SerialInByteCounter - 1] == ';')
        //   InputBuffer_Serial[SerialInByteCounter - 1] = 0; // remove last ";" char
        // else
        InputBuffer_Serial[SerialInByteCounter] = 0; // serial data is complete
        return true;
      }
    }
  }
  return false;
}

boolean CheckCmd()
{
  static byte ValidCommand = 0;
  if (strlen(InputBuffer_Serial) > 7)
  { // need to see minimal 8 characters on the serial port
    // 10;....;..;ON;
    if (strncmp(InputBuffer_Serial, "10;", 3) == 0)
    { // Command from Master to RFLink
      // -------------------------------------------------------
      // Handle Device Management Commands
      // -------------------------------------------------------
      if (strcasecmp(InputBuffer_Serial + 3, "PING;") == 0)
      {
        display_Header();
        display_Name(PSTR("PONG"));
        display_Footer();
      }
      else if (strcasecmp(InputBuffer_Serial + 3, "REBOOT;") == 0)
      {
        display_Header();
        display_Name(PSTR("REBOOT"));
        display_Footer();
        CallReboot();
      }
      else if (strncasecmp(InputBuffer_Serial + 3, "RFDEBUG=O", 9) == 0)
      {
        if (InputBuffer_Serial[12] == 'N' || InputBuffer_Serial[12] == 'n')
        {
          RFDebug = true;    // full debug on
          QRFDebug = false;  // q full debug off
          RFUDebug = false;  // undecoded debug off
          QRFUDebug = false; // q undecoded debug off
          display_Header();
          display_Name(PSTR("RFDEBUG=ON"));
          display_Footer();
        }
        else
        {
          RFDebug = false; // full debug off
          display_Header();
          display_Name(PSTR("RFDEBUG=OFF"));
          display_Footer();
        }
      }
      else if (strncasecmp(InputBuffer_Serial + 3, "RFUDEBUG=O", 10) == 0)
      {
        if (InputBuffer_Serial[13] == 'N' || InputBuffer_Serial[13] == 'n')
        {
          RFDebug = false;   // full debug off
          QRFDebug = false;  // q debug off
          RFUDebug = true;   // undecoded debug on
          QRFUDebug = false; // q undecoded debug off
          display_Header();
          display_Name(PSTR("RFUDEBUG=ON"));
          display_Footer();
        }
        else
        {
          RFUDebug = false; // undecoded debug off
          display_Header();
          display_Name(PSTR("RFUDEBUG=OFF"));
          display_Footer();
        }
      }
      else if (strncasecmp(InputBuffer_Serial + 3, "QRFDEBUG=O", 10) == 0)
      {
        if (InputBuffer_Serial[13] == 'N' || InputBuffer_Serial[13] == 'n')
        {
          RFDebug = false;   // full debug off
          QRFDebug = true;   // q debug on
          RFUDebug = false;  // undecoded debug off
          QRFUDebug = false; // q undecoded debug off
          display_Header();
          display_Name(PSTR("QRFDEBUG=ON"));
          display_Footer();
        }
        else
        {
          QRFDebug = false; // q debug off
          display_Header();
          display_Name(PSTR("QRFDEBUG=OFF"));
          display_Footer();
        }
      }
      else if (strncasecmp(InputBuffer_Serial + 3, "QRFUDEBUG=O", 11) == 0)
      {
        if (InputBuffer_Serial[13] == 'N' || InputBuffer_Serial[13] == 'n')
        {
          RFDebug = false;  // full debug off
          QRFDebug = false; // q debug off
          RFUDebug = false; // undecoded debug off
          QRFUDebug = true; // q undecoded debug on
          display_Header();
          display_Name(PSTR("QRFUDEBUG=ON"));
          display_Footer();
        }
        else
        {
          QRFUDebug = false; // q undecode debug off
          display_Header();
          display_Name(PSTR("QRFUDEBUG=OFF"));
          display_Footer();
        }
      }
      else if (strncasecmp(InputBuffer_Serial + 3, "VERSION", 7) == 0)
      {
        display_Header();
        display_Splash();
        display_Footer();
      }
      else
      {
        // -------------------------------------------------------
        // Handle Generic Commands / Translate protocol data into Nodo text commands
        // -------------------------------------------------------

        // if (PluginTXCall(0, InputBuffer_Serial))
        // {
        //   ValidCommand = 1;
        // }
        // else
        // {
        // Answer that an invalid command was received?
        ValidCommand = 2;
        //}
      }
    }
  } // if > 7
  if (ValidCommand != 0)
  {
    if (ValidCommand == 1)
    {
      display_Header();
      display_Name(PSTR("OK"));
      display_Footer();
    }
    else
    {
      display_Header();
      display_Name(PSTR("CMD UNKNOWN"));
      display_Footer();
    }
  }
  InputBuffer_Serial[0] = 0; // serial data has been processed.
  ValidCommand = 0;
  return true;
}

/*********************************************************************************************/
