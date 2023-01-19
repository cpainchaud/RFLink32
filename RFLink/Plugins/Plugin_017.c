#define RTS_PLUGIN_ID 017
#define PLUGIN_DESC_017 "RTS remote controlled devices"
//#define SerialDebugActivated
//#define PLUGIN_017_DEBUG

#ifdef PLUGIN_017
#include "../4_Display.h"
#include "../1_Radio.h"
#include "../7_Utils.h"
#ifdef ESP8266
#include <LittleFS.h>
#else
#include <FS.h>
#include <LITTLEFS.h>
#define LittleFS LITTLEFS
#endif

// Inspired by 
// https://github.com/etimou/SomfyRTS/blob/master/SomfyRTS.cpp
// https://github.com/arendst/Tasmota/issues/3108
// https://github.com/Nickduino/Somfy_Remote
// https://github.com/altelch/Somfy-RTS/blob/master/Somfy-RTS.py
// https://forum.arduino.cc/index.php?topic=208346.30
// https://matdomotique.wordpress.com/2016/04/21/domoticz-rflink-et-somfy/

#define PLUGIN_017_ID "RTS"

#define RTS_CMD_MY       0x1    // Stop or move to favourite position
#define RTS_CMD_UP       0x2    // Move up
#define RTS_CMD_MY_UP    0x3    // Set upper motor limit in initial programming mode
#define RTS_CMD_DOWN     0x4    // Move down
#define RTS_CMD_MY_DOWN  0x5    // Set lower motor limit in initial programming mode
#define RTS_CMD_UP_DOWN  0x6    // Change motor limit and initial programming mode
#define RTS_CMD_PROG     0x8    // Used for (de-)registering remotes
#define RTS_CMD_SUN_FLAG 0x9    // Sun + Flag  Enable sun and wind detector (SUN and FLAG symbol on the Telis Soliris RC)
#define RTS_CMD_FLAG     0xA    // Disable sun detector (FLAG symbol on the Telis Soliris RC)

const int bitsPerByte = 8;

const int RTS_EEPROMBaseAddress = 0;
const int RTS_ExpectedByteCount = 7;
const int RTS_ExpectedBitCount = RTS_ExpectedByteCount * bitsPerByte;
const int RTS_SoftwareSyncPulseDurationBase = 4500;

// Avoid copy/paste, but use a define to prevent the computation from occuring at boot time, we want it to occur when the method is called
#define DeclareRTS_SoftwareSyncPulseDuration const int RTS_SoftwareSyncPulseDuration = RTS_SoftwareSyncPulseDurationBase / RawSignal.Multiply;

boolean Plugin_017(byte function, const char *string)
{
    const int RTS_MinPulses = 85;
    const int RTS_MaxPulses = 105;

   // ;Pulses=82;Pulses(uSec)=2449,2542,4787,1299,1264,674,612,1311,1263,1301,1261,666,608,1315,1255,683,610,663,615,679,611,1299,1263,1307,611,679,1263,663,611,1309,610,666,1255,1315,1255,682,606,1316,611,665,605,678,1248,679,611,1310,1259,1300,611,679,1259,1311,1259,1311,1263,675,603,679,611,1311,1259,667,611,1311,1259,1311,611,667,611,679,611,667,612,678,1247,1315,608,678,600,678,1260,0

   if (RawSignal.Number >= RTS_MinPulses && RawSignal.Number <= RTS_MaxPulses) 
   {
        const int RTS_HardwareSyncPulseDuration = 2000 / RawSignal.Multiply;
        const int RTS_LongPulseMinDuration = 1000 / RawSignal.Multiply;
        const int RTS_ShortPulseMinDuration = 500 / RawSignal.Multiply;
        const int RTS_ShortPulseMaxDuration = 750 / RawSignal.Multiply;
        DeclareRTS_SoftwareSyncPulseDuration;
        const int RTS_MinRepeatHardwareSyncCount = 5;

        #ifdef PLUGIN_017_DEBUG
        Serial.println(F(PLUGIN_017_ID ": Potential candidate packet"));
        Serial.print(F(PLUGIN_017_ID ": RTS_SoftwareSyncPulseDuration = "));
        Serial.println(RTS_SoftwareSyncPulseDuration);
        Serial.print(F(PLUGIN_017_ID ": RTS_HardwareSyncPulseDuration = "));
        Serial.println(RTS_HardwareSyncPulseDuration);
        Serial.print(F(PLUGIN_017_ID ": RTS_LongPulseMinDuration = "));
        Serial.println(RTS_LongPulseMinDuration);
        Serial.print(F(PLUGIN_017_ID ": RTS_ShortPulseMaxDuration = "));
        Serial.println(RTS_ShortPulseMaxDuration);
        #endif
        // Look for the Software sync pulse (an "on" pulse) preceded by a Hardware sync pulse
        int pulseIndex = 2;  // RawSignal.Pulses[0] is not a pulse but a marker
        int hardwareSyncCount = 0;
        while ((pulseIndex < RawSignal.Number) && ((RawSignal.Pulses[pulseIndex - 1] < RTS_HardwareSyncPulseDuration)  || (RawSignal.Pulses[pulseIndex] < RTS_SoftwareSyncPulseDuration)))
        {
            pulseIndex++;
            if (RawSignal.Pulses[pulseIndex - 1] >= RTS_HardwareSyncPulseDuration && RawSignal.Pulses[pulseIndex - 1] < RTS_SoftwareSyncPulseDuration)
                hardwareSyncCount++;
        }

        if (pulseIndex >= RawSignal.Number)
        {
            #ifdef PLUGIN_017_DEBUG
            Serial.println(F(PLUGIN_017_ID ": No HwSync/SoftSync pulse pair found"));
            #endif
            return false;
        }

        if (pulseIndex % 2 != 1) 
        {
            #ifdef PLUGIN_017_DEBUG
            Serial.print(F(PLUGIN_017_ID ": HwSync/SoftSync pulse pair found, but SoftSync not on an ON Pulse; index = "));
            Serial.print(pulseIndex);
            Serial.print(" - value = ");
            Serial.print(RawSignal.Pulses[pulseIndex]);
            Serial.print(" - previous value = ");
            Serial.println(RawSignal.Pulses[pulseIndex-1]);
            #endif
            return false;
        }

        #ifdef PLUGIN_017_DEBUG
        Serial.print(F(PLUGIN_017_ID ": Found software sync high pulse: index = "));
        Serial.print(pulseIndex);
        Serial.print(" - value = ");
        Serial.print(RawSignal.Pulses[pulseIndex]);
        Serial.print(" - after ");
        Serial.print(hardwareSyncCount);
        Serial.println(" hardware sync pulses");
        #endif

        if (hardwareSyncCount > RTS_MinRepeatHardwareSyncCount)
        {
            #ifdef PLUGIN_017_DEBUG
            Serial.print(F(PLUGIN_017_ID ": Received "));
            Serial.print(hardwareSyncCount);
            Serial.println(" hardware sync pulses, ignoring repeat");
            #endif
            return false;
        }

        //---
        #ifdef PLUGIN_017_DEBUG
        const char markers[2] = {'_', '-'};
        Serial.println(F(PLUGIN_017_ID ": pulses after SWSync high pulse"));
        for (int i = pulseIndex + 1; i < RawSignal.Number; i++)
        {
            uint16_t pulseDuration = RawSignal.Pulses[i];
            char curChar = markers[i % 2];
            if (pulseDuration > RTS_LongPulseMinDuration)
            {
                Serial.print(curChar);
                Serial.print(curChar);
            }
            else if (pulseDuration < RTS_ShortPulseMaxDuration)
            {
                Serial.print(curChar);
            }
        }
        Serial.println();
        #endif
        //---

        // read first pulse after SWSync, to initialize Manchester decoder
        pulseIndex++;
        uint16_t pulseDuration = RawSignal.Pulses[pulseIndex];
        byte nextBit = 0;
        bool secondPulse = false;
        if (pulseDuration > RTS_LongPulseMinDuration)
        {
            #ifdef PLUGIN_017_DEBUG
            Serial.print(F(PLUGIN_017_ID ": Pulse after software sync high is long - "));
            Serial.println(pulseDuration);
            #endif
            secondPulse = true;
            nextBit = 1;
        }
        else if (pulseDuration < RTS_ShortPulseMaxDuration)
        {
            #ifdef PLUGIN_017_DEBUG
            Serial.print(F(PLUGIN_017_ID ": Pulse after software sync high is short - "));
            Serial.println(pulseDuration);
            #endif
            secondPulse = false;
            nextBit = 0;
        }
        else
        {
            #ifdef PLUGIN_017_DEBUG
            Serial.print(F(PLUGIN_017_ID ": Pulse after software sync has unexpected duration: index = "));
            Serial.print(pulseIndex);
            Serial.print(" - value = ");
            Serial.println(pulseDuration);
            #endif
            return false;
        }

        pulseIndex++;
        uint8_t frame[RTS_ExpectedByteCount] = {0, 0, 0, 0, 0, 0, 0};
        if (!decode_manchester(frame, RTS_ExpectedBitCount, RawSignal.Pulses, RawSignal.Number, pulseIndex, nextBit, secondPulse, RTS_ShortPulseMinDuration, RTS_ShortPulseMaxDuration))
        {
            #ifdef PLUGIN_017_DEBUG
            Serial.println(F(PLUGIN_017_ID ": Could not decode Manchester data"));
            #endif
            return false;
        }

        #ifdef PLUGIN_017_DEBUG
        Serial.print(F(PLUGIN_017_ID ": Frame = "));
        for (int i = 0; i < RTS_ExpectedByteCount; i++)
        {
            Serial.print(frame[i], 2);
            Serial.print(" ");
        }
        Serial.println();
        Serial.print(F(PLUGIN_017_ID ": Frame(hex) = "));
        for (int i = 0; i < RTS_ExpectedByteCount; i++)
        {
            Serial.print(frame[i], 16);
            Serial.print(" ");
        }
        Serial.println();
        #endif

        // Now, we deobfuscate the frame
        for (int frameByteIndex = RTS_ExpectedByteCount - 1; frameByteIndex > 0; frameByteIndex--)
            frame[frameByteIndex] ^= frame[frameByteIndex-1];

        #ifdef PLUGIN_017_DEBUG
        Serial.print(F(PLUGIN_017_ID ": clearFrame = "));
        for (int i = 0; i < RTS_ExpectedByteCount; i++)
        {
            Serial.print(frame[i], 16);
            Serial.print(" ");
        }
        Serial.println();
        #endif

        uint32_t remoteAddress = ((uint32_t)frame[4] << 16) | ((uint32_t)frame[5] << 8) | (uint32_t)frame[6];
        uint16_t rollingCode = ((uint16_t)frame[2] << 8) | frame[3];
        uint8_t command = (frame[1] >> 4) & 0xf;

        #ifdef PLUGIN_017_DEBUG
        Serial.println("Group       A       B       C       D       F               G                    ");
        Serial.println("Byte:       0H      0L      1H      1L      2       3       4       5       6    ");
        Serial.println("        +-------+-------+-------+-------+-------+-------+-------+-------+-------+");
        Serial.println("        ! 0xA   + R-KEY ! C M D ! C K S !  Rollingcode  ! Remote Handheld Addr. !");
        Serial.print(  "        !   ");
        Serial.print((frame[0] >> 4) & 0xf, 16);
        Serial.print(               "   +   ");
        Serial.print(frame[0] & 0xf, 16);
        Serial.print(                       "   !   ");
        Serial.print(command, 16);
        Serial.print(                               "   !   ");
        Serial.print(frame[1] & 0xf, 16);
        Serial.print(                                       "   !     ");
        Serial.print(rollingCode, 16);
        Serial.print(                                                    "      !         ");
        Serial.print(remoteAddress, 16);
        Serial.println(                                                                        "        !");
        Serial.println("        +-------+-------+-------+-------+MSB----+----LSB+LSB----+-------+----MSB+");
        #endif

        // verify checksum
        uint8_t cksum = 0;
        for (int i = 0; i < 7; ++i) 
            cksum ^= frame[i] ^ (frame[i] >> 4);
        cksum &= 0x0F;

        if (cksum != 0) 
        {
            #ifdef PLUGIN_017_DEBUG
            Serial.print(F(PLUGIN_017_ID ": Invalid Checksum : "));
            Serial.println(cksum);
            #endif
            return false;
        }

        #ifdef PLUGIN_017_DEBUG
        Serial.println(F(PLUGIN_017_ID ": Checksum OK"));
        #endif

        // map RTS command to RFLink command
        switch (command)
        {
            case RTS_CMD_MY:
                command = CMD_Stop;
                break;
            case RTS_CMD_UP:
                command = CMD_Up;
                break;       
            case RTS_CMD_DOWN:
                command = CMD_Down;
                break;
            case RTS_CMD_PROG:
                command = CMD_Pair;
                break;
            case RTS_CMD_MY_UP:
            case RTS_CMD_MY_DOWN:
            case RTS_CMD_UP_DOWN:
            case RTS_CMD_SUN_FLAG:
            case RTS_CMD_FLAG:
            default:
                command = CMD_Unknown;
        }

         // all is good, output the received packet
        display_Header();
        display_Name(PLUGIN_017_ID);
        display_IDn(remoteAddress, 6);  
        display_IDn(rollingCode, 4);  
        display_CMD(false, command);
        display_Footer();
        RawSignal.Repeats = true; // suppress repeats of the same RF packet
        return true;
    }
    return false;
}
#endif //PLUGIN_017

#ifdef PLUGIN_TX_017
const char RTS_ConfigFileName[] = "/rts.bin";
const byte RTS_AddressSize = 3;
const byte RTS_RollingCodeSize = 2;
const byte RTS_ConfigFileRecordSize = RTS_AddressSize + RTS_RollingCodeSize;
const byte RTS_ConfigFileRecordCount = 32;

void sendFrame(uint8_t* frame, bool isFirst);
void saveRTSRecord(uint8_t eepromRecordNumber, uint32_t address, uint16_t rollingCode);

boolean PluginTX_017(byte function, const char *string)
{
    // Original RFLink has these commands
    //10;RTSCLEAN; => Clean Rolling code table stored in internal EEPROM
    //10;RTSRECCLEAN=9 => Clean Rolling code record number (value from 0 - 31)
    //10;RTSSHOW; => Show Rolling code table stored in internal EEPROM (includes RTS settings)
    //10;RTSINVERT; => Toggle RTS ON/OFF inversion
    //10;RTSLONGTX; => Toggle RTS long transmit ON/OFF 
    //10;RTS;1a602a;0;UP; => RTS protocol, address, unused, command
    //10;RTS;1b602b;0123;PAIR; => Pairing for RTS rolling code: RTS protocol, address, rolling code number (hex), PAIR command (eeprom record number is set to 0)
    //10;RTS;1b602b;0123;0;PAIR; => Extended Pairing for RTS rolling code: RTS protocol, address, rolling code number (hex), eeprom record number (hex), PAIR command    

    // create default file if it does not exist
    if (!LittleFS.exists(RTS_ConfigFileName))
    {
        File file = LittleFS.open(RTS_ConfigFileName, "w");
        const byte emptyRecord[RTS_ConfigFileRecordSize] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        for (int recordNumber = 0; recordNumber < RTS_ConfigFileRecordCount; recordNumber++)
            file.write(emptyRecord, RTS_ConfigFileRecordSize);
        file.close();
    }

    retrieve_Init();

    if (!retrieve_Name("10"))
        return false;
    
    if (retrieve_Name("RTSCLEAN"))
    {
        LittleFS.remove(RTS_ConfigFileName);
        return true;
    }
    else if (retrieve_hasPrefix("RTSRECCLEAN="))
    {
        unsigned long eepromRecordNumber;
        if (!retrieve_decimalNumber(eepromRecordNumber, 2))
            return false;

        if (eepromRecordNumber >= RTS_ConfigFileRecordCount)
            return false;

        saveRTSRecord(eepromRecordNumber, 0xFFFFFF, 0xFFFF);
        return true;
    }
    else if (retrieve_Name("RTSSHOW"))
    {
        File file = LittleFS.open(RTS_ConfigFileName, "r");
        for (int recordNumber = 0; recordNumber < RTS_ConfigFileRecordCount; recordNumber++)
        {
            uint32_t addressInFile;
            if (file.read((uint8_t*)&addressInFile, RTS_AddressSize) != RTS_AddressSize)
            {
                #ifdef PLUGIN_017_DEBUG
                Serial.println(F(PLUGIN_017_ID ": Storage file too short for address!"));
                #endif 
                return false;
            }
            
            uint16_t codeInFile;
            if (file.read((uint8_t*)&codeInFile, RTS_RollingCodeSize) != RTS_RollingCodeSize)
            {
                #ifdef PLUGIN_017_DEBUG
                Serial.println(F(PLUGIN_017_ID ": Storage file too short for code!"));
                #endif
                return false;
            }

            sprintf(printBuf, PSTR("RTS Record: %d  Address: %06X  RC: %04X"), recordNumber, (addressInFile & 0xFFFFFF), codeInFile);
            sendRawPrint(printBuf, true);
        }
        file.close();
        return true;
    }
    else if (retrieve_Name("RTSINVERT"))
    {
        // NOT IMPLEMENTED: Invert On/Off
        return false;
    }
    else if (retrieve_Name("RTSLONGTX"))
    {
        // NOT IMPLEMENTED: Long Tx
        return false;
    }
    else if (!retrieve_Name(PLUGIN_017_ID))  // last try, an order
        return false;

    unsigned long address = 0;
    uint16_t value = 0;
    byte command = 0;
    byte eepromRecordNumber = 0;

    //10;RTS;1a602a;0;UP;
    //10;RTS;1b602b;0;UP;
    //10;RTS;1b602b;0123;PAIR;
    //10;RTS;1b602b;0123;4;PAIR;
    if (!retrieve_hexNumber(address, 6))
        return false;
    if (!retrieve_word(value))
        return false;
    retrieve_byte(eepromRecordNumber); // may fail, but that's fine, it does not touch the token "pointer" in this case
    if (!retrieve_Command(command))
        return false;

    uint16_t code = 0;

    // not pairing? retrieve the next code from the address by checking all records, along with the record number for future reuse
    if (command != VALUE_PAIR)
    {
        File file = LittleFS.open(RTS_ConfigFileName, "r");

        uint32_t addressInFile = 0xFFFFFFFF;
        for (int recordNumber = 0; recordNumber < RTS_ConfigFileRecordCount; recordNumber++)
        {
            if (file.read((uint8_t*)&addressInFile, RTS_AddressSize) != RTS_AddressSize)
            {
                #ifdef PLUGIN_017_DEBUG
                Serial.println(F(PLUGIN_017_ID ": Storage file too short for address!"));
                #endif 
                return false;
            }
            
            if (file.read((uint8_t*)&code, sizeof(code)) != sizeof(code))
            {
                #ifdef PLUGIN_017_DEBUG
                Serial.println(F(PLUGIN_017_ID ": Storage file too short for code!"));
                #endif
                return false;
            }

            if ((addressInFile & 0xFFFFFF) == address)
            {
                eepromRecordNumber = recordNumber;
                break;
            }
            else if (recordNumber == RTS_ConfigFileRecordCount - 1)
            {
                #ifdef PLUGIN_017_DEBUG
                Serial.println(F(PLUGIN_017_ID ": Address not found in storage file, issue PAIR command first!"));
                #endif
                return false;
            }
        }

        file.close();
    }

    // map command to button value
    /*   
      RFLink        Byte       RTS         Description
    VALUE_STOP      0x1     My          Stop or move to favourite position
    VALUE_UP        0x2     Up          Move up
                    0x3     My + Up     Set upper motor limit in initial programming mode
    VALUE_DOWN      0x4     Down        Move down
                    0x5     My + Down   Set lower motor limit in initial programming mode
                    0x6     Up + Down   Change motor limit and initial programming mode
    VALUE_PAIR      0x8     Prog        Used for (de-)registering remotes
                    0x9     Sun + Flag  Enable sun and wind detector (SUN and FLAG symbol on the Telis Soliris RC)
                    0xA     Flag        Disable sun detector (FLAG symbol on the Telis Soliris RC)
    */
    uint8_t button = 0;
    switch(command)
    {
        case VALUE_STOP:
            button = RTS_CMD_MY;
            break;
        case VALUE_UP:
            button = RTS_CMD_UP;
            break;
        case VALUE_DOWN:
            button = RTS_CMD_DOWN;
            break;
        case VALUE_PAIR:
            button = RTS_CMD_PROG;
            code = value; // use as initial code value
            break;
        default:
            return false;
    }

    uint8_t frame[RTS_ExpectedByteCount];

    #ifdef PLUGIN_017_DEBUG
    Serial.print(F(PLUGIN_017_ID ": Address = "));
    Serial.print(address, 16);
    Serial.print(" ; Button = ");
    Serial.print(button, 16);
    Serial.print(" ; RollingCode = ");
    Serial.print(code, 16);
    Serial.print(" ; eepromRecordNumber = ");
    Serial.println(eepromRecordNumber, 16);
    #endif

    // build frame
    frame[0] = 0xAC; // Encryption key. Doesn't matter much
    frame[1] = button << 4;  // Which button did  you press? The 4 LSB will be the checksum
    frame[2] = code >> 8;    // Rolling code (big endian)
    frame[3] = code;         // Rolling code
    frame[4] = address >> 16; // Remote address
    frame[5] = address >>  8; // Remote address
    frame[6] = address;       // Remote address

    // create checksum and place it in low nibble of frame[1]
    uint8_t checksum = 0;
    for(uint8_t i = 0; i < RTS_ExpectedByteCount; i++) 
        checksum ^= frame[i] ^ (frame[i] >> 4);
    checksum &= 0x0F;
    frame[1] |= checksum;

    #ifdef PLUGIN_017_DEBUG
    Serial.print(F(PLUGIN_017_ID ": Frame = "));
    for (uint8_t i = 0; i < RTS_ExpectedByteCount; i++)
    {
        Serial.print(frame[i], 16);
        Serial.print(" ");
    }
    Serial.println();
    #endif

    // obfuscate
    for(uint8_t i = 1; i < RTS_ExpectedByteCount; i++) 
        frame[i] ^= frame[i-1];

    // send first occurence
    sendFrame(frame, true);

    // send repeats
    for (uint8_t i = 0; i < 2; i++)
        sendFrame(frame, false);

    // store next code 
    saveRTSRecord(eepromRecordNumber, address, ++code);

    return true;
}

void saveRTSRecord(uint8_t eepromRecordNumber, uint32_t address, uint16_t rollingCode)
{
    File file = LittleFS.open(RTS_ConfigFileName, "r+");
    file.seek(RTS_ConfigFileRecordSize * eepromRecordNumber, SeekSet);
    file.write((uint8_t*)&address, RTS_AddressSize);
    file.write((uint8_t*)&rollingCode, RTS_RollingCodeSize);
    file.close();
}

void sendFrame(uint8_t* frame, bool isFirst)
{
    #ifndef RFLINK_NO_RADIOLIB_SUPPORT
    uint32_t originalFrequency = Radio::setFrequency(433420000);
    #endif
    RawSignal.Multiply = 1;

    const int RTS_HalfBitPulseDuration = 640 / RawSignal.Multiply;
    const int RTS_WakeUpPulseDuration = 9415 / RawSignal.Multiply;
    const int RTS_WakeUpSilenceDuration = 89565 / RawSignal.Multiply;
    const int RTS_InterframeSilenceDuration = 30415 / RawSignal.Multiply;
    DeclareRTS_SoftwareSyncPulseDuration;

    // wake up pulse, only for first frame
    if (isFirst) 
    { 
        digitalWrite(Radio::pins::TX_DATA, HIGH);
        delayMicroseconds(RTS_WakeUpPulseDuration);
        digitalWrite(Radio::pins::TX_DATA, LOW);
        delayMicroseconds(RTS_WakeUpSilenceDuration);
    }

    // Hardware sync: two sync for the first frame, seven for the following ones.
    for (int i = 0; i < (isFirst ? 2 : 7) ; i++) {
        digitalWrite(Radio::pins::TX_DATA, HIGH);
        delayMicroseconds(4 * RTS_HalfBitPulseDuration);
        digitalWrite(Radio::pins::TX_DATA, LOW);
        delayMicroseconds(4 * RTS_HalfBitPulseDuration);
    }

    // Software sync
    digitalWrite(Radio::pins::TX_DATA, HIGH);
    delayMicroseconds(RTS_SoftwareSyncPulseDuration);
    digitalWrite(Radio::pins::TX_DATA, LOW);
    delayMicroseconds(RTS_HalfBitPulseDuration);

    // Data: bits are sent one by one, starting with the MSB.
    for(byte i = 0; i < RTS_ExpectedBitCount; i++) 
    {
        if(((frame[i/8] >> (7 - (i%8))) & 1) == 1) 
        {
            digitalWrite(Radio::pins::TX_DATA, LOW);
            delayMicroseconds(RTS_HalfBitPulseDuration);
            digitalWrite(Radio::pins::TX_DATA, HIGH);
            delayMicroseconds(RTS_HalfBitPulseDuration);
        }
        else 
        {
            digitalWrite(Radio::pins::TX_DATA, HIGH);
            delayMicroseconds(RTS_HalfBitPulseDuration);
            digitalWrite(Radio::pins::TX_DATA, LOW);
            delayMicroseconds(RTS_HalfBitPulseDuration);
        }
    }

    digitalWrite(Radio::pins::TX_DATA, LOW);
    delayMicroseconds(RTS_InterframeSilenceDuration); // Inter-frame silence

    RawSignal.Multiply = RFLink::Signal::params::sample_rate; // restore setting
    #ifndef RFLINK_NO_RADIOLIB_SUPPORT
    Radio::setFrequency(originalFrequency);
    #endif
}

#endif //PLUGIN_TX_017

