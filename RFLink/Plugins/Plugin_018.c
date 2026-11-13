#define PLUGIN_DESC_018 "Louvolite R1492-6CH-WH remote controlled blinds"

#ifdef PLUGIN_018
#include "../4_Display.h"
#include "../1_Radio.h"
#include "../7_Utils.h"
#ifdef ESP8266
#include <LittleFS.h>
#else
#include <FS.h>
#include <LittleFS.h>
#endif

#define PLUGIN_018_ID "LOUVO"

#define LOUVO_BYTE_COUNT (9)
#define LOUVO_BIT_COUNT (65)

#define LOUVO_HEADER    0xac

#define LOUVO_CMD_UP        0x000b
#define LOUVO_CMD_STOP      0x0023
#define LOUVO_CMD_RELEASE   0x0024  // End of button press
#define LOUVO_CMD_DOWN      0x0043

#define LOUVO_PWM_SYNC  4750
#define LOUVO_PWM_LONG  665
#define LOUVO_PWM_SHORT 266
#define LOUVO_PWM_TOLERANCE 50

#define LOUVO_NUM_REPEATS   9

//#define PLUGIN_018_DEBUG

/*
 * 20;XX;DEBUG;Pulses=130;Pulses(uSec)=645,313,254,674,658,278,261,653,675,272,659,269,268,663,271,659,667,269,265,672,659,268,674,255,269,673,659,265,265,675,264,
 * 677,248,680,662,254,672,268,661,266,265,671,266,660,670,271,670,261,670,263,265,674,264,661,265,664,674,256,268,661,684,261,262,663,260,676,268,657,266,672,268,
 * 659,265,659,264,672,667,273,259,672,260,661,270,674,267,662,263,664,274,653,269,677,255,665,270,672,266,659,264,664,673,268,258,672,269,661,264,663,664,272,660,
 * 270,674,255,675,262,271,661,665,269,271,659,660,275,668,265,273,663,649,5000;RSSI=-9999;REASON=Unknown
*/

boolean Plugin_018(byte function, const char *string)
{
    const int ExpectedPulses = 130;

    if (RawSignal.Number == ExpectedPulses)
    {
        const int PWMShortMin = (LOUVO_PWM_SHORT - LOUVO_PWM_TOLERANCE) / RawSignal.Multiply;
        const int PWMShortMax = (LOUVO_PWM_SHORT + LOUVO_PWM_TOLERANCE) / RawSignal.Multiply;
        const int PWMLongMin = (LOUVO_PWM_LONG - LOUVO_PWM_TOLERANCE) / RawSignal.Multiply;
        const int PWMLongMax = (LOUVO_PWM_LONG + LOUVO_PWM_TOLERANCE) / RawSignal.Multiply;

        const uint8_t pkt_header = LOUVO_HEADER;
        uint32_t pkt_uuid = 0;
        uint16_t pkt_node = 0;
        uint16_t pkt_cmd = 0;
        uint8_t pkt_crc = 0;

        byte packet[LOUVO_BYTE_COUNT] = {0};

        //Skip the first bit as the lengths tend to vary...
        if(!decode_pwm(packet, 65, RawSignal.Pulses, RawSignal.Number, 1, PWMShortMin, PWMShortMax, PWMLongMin, PWMLongMax, 0))
        {
            Serial.println(F(PLUGIN_018_ID ": Failed to decode PWM"));
            return false;
        }

        if (packet[0] == pkt_header)
        {
            int i;

            // Calculate CSUM excluding header
            for (i=1; i<7; ++i)
                pkt_crc += packet[i];

            if (packet[7] != pkt_crc)
            {
                Serial.println(F(PLUGIN_018_ID ": CSUM Check Failed"));
                return false;
            }

            pkt_uuid = packet[1] << 16 | packet[2] << 8 | packet[3];
            pkt_node = packet[4];
            switch (packet[5] << 8 | packet[6])
            {
                case LOUVO_CMD_UP:
                    pkt_cmd = CMD_Up;
                break;

                case LOUVO_CMD_STOP:
                    pkt_cmd = CMD_Stop;
                break;

                case LOUVO_CMD_RELEASE:
                    return false;
                break;

                case LOUVO_CMD_DOWN:
                    pkt_cmd = CMD_Down;
                break;

                default:
                    pkt_cmd = CMD_Unknown;
                break;
            }
        }
        else
            return false; // not a Louvolite packet, let other plugins try it

        // all is good, output the received packet
        display_Header();
        display_Name(PLUGIN_018_ID);
        display_IDn(pkt_uuid, 6);
        display_IDn(pkt_node, 4);
        display_CMD(false, pkt_cmd);
        display_Footer();

        RawSignal.Repeats = true; // suppress repeats of the same RF packet
        return true;
    }
    return false;
}
#endif //PLUGIN_018

#ifdef PLUGIN_TX_018

void PluginTX_018_sendPacket(uint32_t uuid, byte node, uint16_t cmd, bool wake)
{
    const int PWMShort = LOUVO_PWM_SHORT;
    const int PWMLong  = LOUVO_PWM_LONG;
    const int PWMSync  = LOUVO_PWM_SYNC;

    uint8_t packet[LOUVO_BYTE_COUNT];

    // build frame
    packet[0] = LOUVO_HEADER;
    packet[1] = (uuid >> 16) & 0xff;
    packet[2] = (uuid >> 8) & 0xff;
    packet[3] = uuid & 0xff;
    packet[4] = node;
    packet[5] = (cmd >> 8) & 0xff;
    packet[6] = cmd & 0xff;
    packet[7] = 0; // Checksum
    packet[8] = 0x80;

    // create checksum
    for(uint8_t i = 1; i < 7; i++)
        packet[7] += packet[i];

    #ifdef PLUGIN_018_DEBUG
    Serial.print(F(PLUGIN_018_ID ": Frame = "));
    for (uint8_t i = 0; i < LOUVO_BYTE_COUNT; i++)
    {
        Serial.print(packet[i], 16);
        Serial.print(" ");
    }
    Serial.println();
    #endif

    // wake up pulse, only for first frame
    if (wake)
    {
        for (int i=0; i < 8; i++) {
            digitalWrite(Radio::pins::TX_DATA, HIGH);
            delayMicroseconds(PWMShort);
            digitalWrite(Radio::pins::TX_DATA, LOW);
            delayMicroseconds(PWMLong);
        }
    }

    // Sync Pulse
    digitalWrite(Radio::pins::TX_DATA, HIGH);
    delayMicroseconds(PWMSync);
    digitalWrite(Radio::pins::TX_DATA, LOW);
    delayMicroseconds(PWMLong);

    // Data: bits are sent one by one, starting with the MSB.
    for(byte i = 0; i < LOUVO_BIT_COUNT; i++)
    {
        if(((packet[i/8] >> (7 - (i%8))) & 1) == 1)
        {
            digitalWrite(Radio::pins::TX_DATA, HIGH);
            delayMicroseconds(PWMLong);
            digitalWrite(Radio::pins::TX_DATA, LOW);
            delayMicroseconds(PWMShort);
        }
        else
        {
            digitalWrite(Radio::pins::TX_DATA, HIGH);
            delayMicroseconds(PWMShort);
            digitalWrite(Radio::pins::TX_DATA, LOW);
            delayMicroseconds(PWMLong);
        }
    }

    digitalWrite(Radio::pins::TX_DATA, LOW);
    delayMicroseconds(PWMSync);
}

boolean PluginTX_018(byte function, const char *string)
{
    retrieve_Init();

    if (!retrieve_Name("10"))
        return false;

    if (!retrieve_Name(PLUGIN_018_ID))  // last try, an order
        return false;

    unsigned long uuid = 0;
    byte node = 0;
    byte cmd = 0;

    //10;LOUVO;1a602a;1;UP;
    if (!retrieve_hexNumber(uuid, 6))
        return false;
    if (!retrieve_byte(node))
        return false;
    if (!retrieve_Command(cmd))
        return false;

    // map command to button value
    uint16_t pkt_cmd = 0;
    switch(cmd)
    {
        case VALUE_STOP:
            pkt_cmd = LOUVO_CMD_STOP;
            break;
        case VALUE_UP:
            pkt_cmd = LOUVO_CMD_UP;
            break;
        case VALUE_DOWN:
            pkt_cmd = LOUVO_CMD_DOWN;
            break;
        default:
            return false;
    }

    #ifdef PLUGIN_018_DEBUG
    Serial.print(F(PLUGIN_018_ID ": UUID = "));
    Serial.print(uuid, 16);
    Serial.print(" ; Node = ");
    Serial.print(node, 16);
    Serial.print(" ; Command = ");
    Serial.print(cmd, 8);
    Serial.println();
    #endif

    // send first packet with preamble
    PluginTX_018_sendPacket(uuid, node, pkt_cmd, true);

    // send repeats
    for (uint8_t i = 0; i < LOUVO_NUM_REPEATS; i++)
        PluginTX_018_sendPacket(uuid, node, pkt_cmd, false);

    // UP and DOWN have an associated 'release' burst
    if (pkt_cmd != LOUVO_CMD_STOP)
    {
        delay(500);

        // send first occurence
        PluginTX_018_sendPacket(uuid, node, LOUVO_CMD_RELEASE, true);

        // send repeats
        for (uint8_t i = 0; i < LOUVO_NUM_REPEATS/2; i++)
            PluginTX_018_sendPacket(uuid, node, LOUVO_CMD_RELEASE, false);
    }

    return true;
}

#endif //PLUGIN_TX_018

