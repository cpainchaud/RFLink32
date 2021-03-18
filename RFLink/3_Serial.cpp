// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#include <Arduino.h>
#include <HardwareSerial.h>
#include "RFLink.h"
#include "1_Radio.h"
#include "3_Serial.h"
#include "4_Display.h"
#include "5_Plugin.h"

char InputBuffer_Serial[INPUT_COMMAND_SIZE];
int serialBufferCursor=0;

/**
 *
 * @return true if some command/data needs to process, false if not
 */
boolean ReadSerial();

boolean CheckCmd();

boolean CopySerial(char *);
/*********************************************************************************************/

using namespace RFLink;

class HardwareSerialExtended: public HardwareSerial {

public:
    size_t readBytesUntilNewLine(char *buffer, size_t length)
    {
        if(length < 1) {
            return 0;
        }
        size_t index = 0;
        while(index < length) {
            int c = timedRead();
            if(c<0)
                break;
            *buffer++ = (char) c;
            index++;
            if(c == 10)
                break;
            if(c == 13) {
                int nextChar = timedPeek();
                if(nextChar < 0)
                    break;
                if(nextChar == 10) // \n newline char found
                    read();
                break;
            }
        }
        return index; // return number of characters, not including null terminator
    }
};

/**
 * @return False if Serial has no data to read and fails to execute command. 
 * */
boolean readSerialAndExecute() {
    if (ReadSerial()) {
#ifdef SERIAL_ENABLED
        RFLink::sendRawPrint(F("\33[2K\r"));
        Serial.flush();
        RFLink::sendRawPrint(PSTR("Message arrived [Serial]:"));
        RFLink::sendRawPrint(InputBuffer_Serial);
        RFLink::sendRawPrint(PSTR("\r\n"));
        Serial.flush();
#endif
        bool success = RFLink::executeCliCommand(InputBuffer_Serial);
        resetSerialBuffer();
        if (success)
            return true;
    }
    return false;
}

boolean CheckMQTT(byte *byte_in) {
    if (CopySerial((char *) byte_in)) {
#ifdef SERIAL_ENABLED
        Serial.flush();
        Serial.print(F("Message arrived [MQTT] "));
        Serial.println(InputBuffer_Serial);
#endif
        if (CheckCmd())
            return true;
    }
    return false;
}

boolean CopySerial(char *src) {
    return (strncpy(InputBuffer_Serial, src, INPUT_COMMAND_SIZE - 2));
}

HardwareSerialExtended *RFL_Serial = (HardwareSerialExtended*) &Serial;

boolean ReadSerial() {
    // SERIAL: *************** Check if there is data ready on the serial port **********************

    static unsigned long FocusTimer; // Timer to keep focus on the task during communication

    int readCount;
    int availableBytes = Serial.available();
    int cursorReference = serialBufferCursor;

    if (availableBytes) {
        FocusTimer = millis() + FOCUS_TIME_MS;

        while (true) {
            availableBytes = Serial.available();
            if (availableBytes) {

                readCount = RFL_Serial->readBytesUntilNewLine(&InputBuffer_Serial[serialBufferCursor], INPUT_COMMAND_SIZE - 1 - serialBufferCursor);

                if(readCount > 0) {

                    serialBufferCursor += readCount;

                    //Serial.printf_P(PSTR("Read %i bytes so far from console and last char=%hu\r\n"), serialBufferCursor, InputBuffer_Serial[serialBufferCursor-1]);

                    if (InputBuffer_Serial[serialBufferCursor - 1] == 13 || InputBuffer_Serial[serialBufferCursor - 1] == 10) {
                        InputBuffer_Serial[serialBufferCursor - 1] = 0;
                        return true;
                    }
                    FocusTimer = millis() + FOCUS_TIME_MS;
                }

            }

            if (millis() >= FocusTimer) { // we will get more characters at next loop
                //Serial.println(F("Exit because of timer"));
                if(cursorReference != serialBufferCursor)
                    Serial.write(&InputBuffer_Serial[cursorReference], serialBufferCursor-cursorReference);
                return false;
            }

            if(serialBufferCursor >= (INPUT_COMMAND_SIZE - 1))
            {
                resetSerialBuffer();
                Serial.println(F("Error: Your command was too long so it was ignored"));
                while (Serial.available()) { // Let's empty Serial so no mistake is made !
                    Serial.read();
                }
                return false;
            }
        }
    }
    return false;
}

boolean CheckCmd() {
    static byte ValidCommand = 0;
    if (strlen(InputBuffer_Serial) > 7) { // need to see minimal 8 characters on the serial port
        // 10;....;..;ON;
        if (strncmp(InputBuffer_Serial, "10;", 3) == 0) { // Command from Master to RFLink
            // -------------------------------------------------------
            // Handle Device Management Commands
            // -------------------------------------------------------
            if (strncasecmp(InputBuffer_Serial + 3, "PING;", 5) == 0) {
                display_Header();
                display_Name(PSTR("PONG"));
                display_Footer();
            } else if (strncasecmp(InputBuffer_Serial + 3, "REBOOT;", 7) == 0) {
                display_Header();
                display_Name(PSTR("REBOOT"));
                display_Footer();
                CallReboot();
            } else if (strncasecmp(InputBuffer_Serial + 3, "RFDEBUG=O", 9) == 0) {
                if (InputBuffer_Serial[12] == 'N' || InputBuffer_Serial[12] == 'n') {
                    RFDebug = true;    // full debug on
                    QRFDebug = false;  // q full debug off
                    RFUDebug = false;  // undecoded debug off
                    QRFUDebug = false; // q undecoded debug off
                    display_Header();
                    display_Name(PSTR("RFDEBUG=ON"));
                    display_Footer();
                } else {
                    RFDebug = false; // full debug off
                    display_Header();
                    display_Name(PSTR("RFDEBUG=OFF"));
                    display_Footer();
                }
            } else if (strncasecmp(InputBuffer_Serial + 3, "RFUDEBUG=O", 10) == 0) {
                if (InputBuffer_Serial[13] == 'N' || InputBuffer_Serial[13] == 'n') {
                    RFDebug = false;   // full debug off
                    QRFDebug = false;  // q debug off
                    RFUDebug = true;   // undecoded debug on
                    QRFUDebug = false; // q undecoded debug off
                    display_Header();
                    display_Name(PSTR("RFUDEBUG=ON"));
                    display_Footer();
                } else {
                    RFUDebug = false; // undecoded debug off
                    display_Header();
                    display_Name(PSTR("RFUDEBUG=OFF"));
                    display_Footer();
                }
            } else if (strncasecmp(InputBuffer_Serial + 3, "QRFDEBUG=O", 10) == 0) {
                if (InputBuffer_Serial[13] == 'N' || InputBuffer_Serial[13] == 'n') {
                    RFDebug = false;   // full debug off
                    QRFDebug = true;   // q debug on
                    RFUDebug = false;  // undecoded debug off
                    QRFUDebug = false; // q undecoded debug off
                    display_Header();
                    display_Name(PSTR("QRFDEBUG=ON"));
                    display_Footer();
                } else {
                    QRFDebug = false; // q debug off
                    display_Header();
                    display_Name(PSTR("QRFDEBUG=OFF"));
                    display_Footer();
                }
            } else if (strncasecmp(InputBuffer_Serial + 3, "QRFUDEBUG=O", 11) == 0) {
                if (InputBuffer_Serial[14] == 'N' || InputBuffer_Serial[14] == 'n') {
                    RFDebug = false;  // full debug off
                    QRFDebug = false; // q debug off
                    RFUDebug = false; // undecoded debug off
                    QRFUDebug = true; // q undecoded debug on
                    display_Header();
                    display_Name(PSTR("QRFUDEBUG=ON"));
                    display_Footer();
                } else {
                    QRFUDebug = false; // q undecode debug off
                    display_Header();
                    display_Name(PSTR("QRFUDEBUG=OFF"));
                    display_Footer();
                }
            } else if (strncasecmp(InputBuffer_Serial + 3, "VERSION", 7) == 0) {
                display_Header();
                display_Splash();
                display_Footer();
            } else {
                // -------------------------------------------------------
                // Handle Generic Commands / Translate protocol data into Nodo text commands
                // -------------------------------------------------------
                Radio::set_Radio_mode(Radio::States::Radio_TX);

                if (PluginTXCall(0, InputBuffer_Serial))
                    ValidCommand = 1;
                else // Answer that an invalid command was received?
                    ValidCommand = 2;

                Radio::set_Radio_mode(Radio::States::Radio_RX);
            }
        }
    } // if > 7
    if (ValidCommand != 0) {
        display_Header();
        if (ValidCommand == 1)
            display_Name(PSTR("OK"));
        else
            display_Name(PSTR("CMD UNKNOWN"));
        display_Footer();
    }
    resetSerialBuffer();
    ValidCommand = 0;
    return true;
}

void resetSerialBuffer() {
    InputBuffer_Serial[0] = 0;
    serialBufferCursor = 0;
}

/*********************************************************************************************/