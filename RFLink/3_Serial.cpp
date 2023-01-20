// ************************************* //
// * Arduino Project RFLink32        * //
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
        //Serial.flush();
        RFLink::sendRawPrint(F("Message arrived [Serial]:"));
        RFLink::sendRawPrint(InputBuffer_Serial);
        RFLink::sendRawPrint(F("\r\n"));
        //Serial.flush();
#endif
        return RFLink::executeCliCommand(InputBuffer_Serial);
    }
    return false;
}

boolean CheckMQTT(byte *byte_in) {
    if (CopySerial((char *) byte_in)) {
#ifdef SERIAL_ENABLED
        //Serial.flush();
        Serial.print(F("Message arrived [MQTT] "));
        Serial.println(InputBuffer_Serial);
#endif
        return executeCliCommand(InputBuffer_Serial);
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

void resetSerialBuffer() {
    InputBuffer_Serial[0] = 0;
    serialBufferCursor = 0;
}

/*********************************************************************************************/