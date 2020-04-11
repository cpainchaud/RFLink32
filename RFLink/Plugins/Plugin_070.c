//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                             Plugin-70 Select Plus Wireless Doorbell                               ##
//#######################################################################################################
/*********************************************************************************************\
 * This Plugin takes care of reception of the Select Plus wireless doorbell (Sold at Action for 6 euro's)
 * PCB markings: Quhwa QH-C-CE-3V aka QH-832AC
 * Also sold as "1 by One" and "Delta" wireless doorbell.
 *
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : StuntTeam 2015..2016
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical information:
 *
 * There are two known models:
 * SelectPlus (200689103 - Black - Datecode:0614) also sold as "1 by One" (O00 QH-0031)
 *            PCB details: Quhwa, QH-C-CE-3V, 09.10.16, CPU: MC908T
 * SelectPlus (200689101 - White - Datecode:0914)	
 *            PCB details: Quhwa, QH-C-CE-3V, REV1.0, CPU: MC908T
 *
 * Each frame is 35 pulses long. It is composed of: 
 * 101011001010110010110011010 10101010
 * The first block appears to be an unique ID
 * The second block appears to be some kind of identifier which always is 0xAA (10101010) 
 * Converting the pulses into bits results in a 13 bit unique address and a 4 bit identifier: 
 *
 * B) 1110000110011 0000  => 1C33 0      B)lack push button
 * W) 1101110110100 0000  => 1BB4 0      W)hite push button
 *
 * Note: The transmitter sends 43 times the same packet when the bell button is pressed
 * the retransmit is killed to prevent reporting the same press multiple times
 *
 * Sample:
 * B) 20;62;DEBUG;Pulses=36;Pulses(uSec)=1000,1000,225,1000,225,1000,225,300,900,300,900,300,900,300,900,1000,225,1000,225,300,925,300,900,1000,225,1000,225,275,900,300,900,300,900,300,900;
 * W) 20;A2;DEBUG;Pulses=36;Pulses(uSec)=325,950,250,950,250,250,925,950,250,950,250,950,250,275,925,950,250,950,250,250,925,950,250,275,925,250,925,275,925,250,925,275,925,275,925;
 * w) 20;66;DEBUG;Pulses=36;Pulses(uSec)=650,2000,550,2000,550,550,2000,2000,550,2000,550,2000,550,550,2000,2000,550,2000,550,550,2000,2000,550,550,2000,550,2000,550,1950,550,2000,550,2000,550,2000;
 * b) 20;05;DEBUG;Pulses=36;Pulses(uSec)=2100,2100,500,2050,500,2100,500,600,1950,600,1950,600,1950,600,1950,2050,500,2050,500,600,1950,600,1950,2100,500,2050,500,600,1950,600,1950,600,1950,600,1950;
 \*********************************************************************************************/
#define SELECTPLUS_PULSECOUNT 36
#define SELECTPLUS_PULSEMID 650 / RAWSIGNAL_SAMPLE_RATE
#define SELECTPLUS_PULSEMAX 2125 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_070
#include "../4_Display.h"

boolean Plugin_070(byte function, char *string)
{
    if (RawSignal.Number != SELECTPLUS_PULSECOUNT)
        return false;
    unsigned long bitstream = 0L;
    byte checksum = 0;
    //==================================================================================
    // get bytes
    for (byte x = 2; x < SELECTPLUS_PULSECOUNT; x = x + 2)
    {
        if (RawSignal.Pulses[x] < SELECTPLUS_PULSEMID)
        {
            if (RawSignal.Pulses[x + 1] < SELECTPLUS_PULSEMID)
                return false; // invalid pulse sequence 10/01
            bitstream = (bitstream << 1);
        }
        else
        {
            if (RawSignal.Pulses[x] > SELECTPLUS_PULSEMAX)
                return false; // invalid pulse duration, pulse too long
            if (RawSignal.Pulses[x + 1] > SELECTPLUS_PULSEMID)
                return false; // invalid pulse sequence 10/01
            bitstream = (bitstream << 1) | 0x1;
        }
    }
    if (bitstream == 0)
        return false; // sanity check
    //==================================================================================
    // Prevent repeating signals from showing up
    //==================================================================================
    if ((SignalHash != SignalHashPrevious) || ((RepeatingTimer + 2000) < millis()) || (SignalCRC != bitstream))
    {
        SignalCRC = bitstream; // not seen the RF packet recently
    }
    else
    {
        return true; // already seen the RF packet recently
    }
    //==================================================================================
    // all bytes received, make sure checksum is okay
    //==================================================================================
    checksum = (bitstream)&0xf; // Second block
    if (checksum != 0)
        return false; // last 4 bits should always be 0
    //==================================================================================
    // Output
    // ----------------------------------
    Serial.print("20;");
    PrintHexByte(PKSequenceNumber++);
    Serial.print(F(";SelectPlus;")); // Label
    // ----------------------------------
    sprintf(pbuffer, "ID=%04x;", ((bitstream) >> 4) & 0xffff); // ID
    Serial.print(pbuffer);
    Serial.print(F("SWITCH=1;CMD=ON;"));
    Serial.print(F("CHIME=01;"));
    Serial.println();
    //==================================================================================
    RawSignal.Repeats = true; // suppress repeats of the same RF packet
    RawSignal.Number = 0;     // do not process the packet any further
    return true;
}
#endif // PLUGIN_070

#ifdef PLUGIN_TX_070
void SelectPlus_Send(unsigned long address);

boolean PluginTX_070(byte function, char *string)
{
    boolean success = false;
    unsigned long bitstream = 0L;
    //10;SELECTPLUS;001c33;1;OFF;
    //012345678901234567890123456
    if (strncasecmp(InputBuffer_Serial + 3, "SELECTPLUS;", 11) == 0)
    {
        InputBuffer_Serial[12] = 0x30;
        InputBuffer_Serial[13] = 0x78;
        InputBuffer_Serial[20] = 0;
        bitstream = str2int(InputBuffer_Serial + 12);
        bitstream = bitstream << 4;
        SelectPlus_Send(bitstream); // Send RF packet
        success = true;
    }
    return success;
}

void SelectPlus_Send(unsigned long address)
{
    int fpulse = 364;  // Pulse witdh in microseconds
    int fretrans = 16; // number of RF packet retransmissions
    uint32_t fdatabit;
    uint32_t fdatamask = 0x10000;
    uint32_t fsendbuff;

    digitalWrite(PIN_RF_RX_VCC, LOW);            // Power off the RF receiver (if wired that way) to protect against interference
    digitalWrite(PIN_RF_TX_VCC, HIGH);           // Enable 433Mhz transmitter
    delayMicroseconds(TRANSMITTER_STABLE_DELAY); // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)

    for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++)
    {
        fsendbuff = address;
        // send SYNC 3P High
        digitalWrite(PIN_RF_TX_DATA, HIGH);
        delayMicroseconds(fpulse * 3);
        // end send SYNC
        // Send command
        for (int i = 0; i < 17; i++)
        { // SelectPlus address is only 13 bits, last 4 bits are always zero
            // read data bit7
            fdatabit = fsendbuff & fdatamask; // Get most left bit
            fsendbuff = (fsendbuff << 1);     // Shift left

            if (fdatabit != fdatamask)
            {                                      // Write 0
                digitalWrite(PIN_RF_TX_DATA, LOW); // short low
                delayMicroseconds(fpulse * 1);
                digitalWrite(PIN_RF_TX_DATA, HIGH); // long high
                delayMicroseconds(fpulse * 3);
            }
            else
            { // Write 1
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 3); // long low
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 1); // short high
            }
        }
        digitalWrite(PIN_RF_TX_DATA, LOW); // and lower the signal
        if (nRepeat < fretrans)
        {
            delayMicroseconds(fpulse * 16); // delay between RF transmits
        }
    }
    delayMicroseconds(TRANSMITTER_STABLE_DELAY); // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)
    digitalWrite(PIN_RF_TX_VCC, LOW);            // Disable the 433Mhz transmitter
    digitalWrite(PIN_RF_RX_VCC, HIGH);           // Enable the 433Mhz receiver
    RFLinkHW();
}
#endif // PLUGIN_070
