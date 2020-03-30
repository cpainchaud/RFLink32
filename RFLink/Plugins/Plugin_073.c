//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                 Plugin-073 Deltronic Doorbell                                     ##
//#######################################################################################################
/*********************************************************************************************\
 * This Plugin takes care of reception And sending of the Deltronic doorbell
 *
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : StuntTeam & Jonas Jespersen 2015..2016
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Technical information:
 *
 * The doorbell uses the UM3750 circuit which sends out a 12 bit signal:
 *
 * AAAAAAAA BBBB
 *
 * A = Always 1
 * B = Address (Can be either 1, 5 or 9)
 *
 * Address 1
 * 20;0D;DEBUG;Pulses=26;Pulses(uSec)=600,1150,525,1175,500,1175,475,1200,500,1175,500,1200,475,1175,475,1200,475,575,1075,575,1075,575,1075,1225,450;
 * 000000001110 
 * Address 5
 * 20;17;DEBUG;Pulses=26;Pulses(uSec)=550,1075,425,1100,400,1125,425,1100,400,1125,400,1150,375,1125,400,1125,375,550,900,1125,375,550,900,1150,375;
 * 000000001010
 * Address 9
 * 20;1B;DEBUG;Pulses=26;Pulses(uSec)=600,1150,500,1175,525,1175,500,1175,500,1175,500,1175,500,1175,475,1200,500,1200,475,575,1075,600,1075,1200,475;
 * 000000000110
 \*********************************************************************************************/
#define DELTRONIC_PULSECOUNT 26
#define LENGTH_DEVIATION 300

#ifdef PLUGIN_073
boolean Plugin_073(byte function, char *string)
{
    if (RawSignal.Number != DELTRONIC_PULSECOUNT)
        return false;
    //==================================================================================
    unsigned long bitstream = 0L;
    unsigned long checksum = 0L;
    //==================================================================================
    if (RawSignal.Pulses[1] * RAWSIGNAL_SAMPLE_RATE > 675)
        return false; // First pulse is start bit and should be short!
    for (byte x = 2; x < DELTRONIC_PULSECOUNT; x = x + 2)
    {
        if (RawSignal.Pulses[x] * RAWSIGNAL_SAMPLE_RATE > 800)
        { // long pulse  (800-1275)
            if (RawSignal.Pulses[x] * RAWSIGNAL_SAMPLE_RATE > 1275)
                return false; // pulse too long to be valid
            if (RawSignal.Pulses[x + 1] * RAWSIGNAL_SAMPLE_RATE > 675)
                return false;                   // invalid manchestercode (10 01)
            bitstream = (bitstream << 1) | 0x1; // 10 => 1 bit
        }
        else
        { // short pulse
            if (RawSignal.Pulses[x] * RAWSIGNAL_SAMPLE_RATE < 250)
                return false; // too short
            if (RawSignal.Pulses[x + 1] * RAWSIGNAL_SAMPLE_RATE < 700)
                return false;           // invalid manchestercode (10 01)
            bitstream = bitstream << 1; // 01 => 0 bit
        }
    }
    //==================================================================================
    // Prevent repeating signals from showing up
    //==================================================================================
    if ((SignalHash != SignalHashPrevious) || (RepeatingTimer + 2000 < millis()))
    {
        // not seen the RF packet recently
    }
    else
    {
        // already seen the RF packet recently
        return true;
    }
    //==================================================================================
    // all bytes received, make sure checksum is okay
    //==================================================================================
    checksum = (bitstream)&0x00000FF0L;
    if (checksum != 0x00000FF0L)
    {   // First 8 bits should always be 1
        //Serial.println("crc error");
        return false;
    }
    if (bitstream == 0)
        return false; // sanity check
    //==================================================================================
    // Output
    // ----------------------------------
    Serial.print("20;");
    PrintHexByte(PKSequenceNumber++);
    Serial.print(F(";Deltronic;")); // Label
    // ----------------------------------
    sprintf(pbuffer, "ID=%04x;", (bitstream)&0x0000000FL); // ID
    Serial.print(pbuffer);
    Serial.print(F("SWITCH=1;CMD=ON;"));
    Serial.print(F("CHIME=01;"));
    Serial.println();
    //==================================================================================
    RawSignal.Repeats = true; // suppress repeats of the same RF packet
    RawSignal.Number = 0;     // do not process the packet any further
    return true;
}
#endif // PLUGIN_073

#ifdef PLUGIN_TX_073
void Deltronic_Send(unsigned long address);

boolean PluginTX_073(byte function, char *string)
{
    boolean success = false;
    unsigned long bitstream = 0L;
    //10;DELTRONIC;001c33;1;OFF;
    //012345678901234567890123456
    if (strncasecmp(InputBuffer_Serial + 3, "DELTRONIC;", 10) == 0)
    {
        InputBuffer_Serial[11] = 0x30;
        InputBuffer_Serial[12] = 0x78;
        InputBuffer_Serial[19] = 0;
        bitstream = str2int(InputBuffer_Serial + 11);
        bitstream = (bitstream) | 0x00000FF0L;
        Deltronic_Send(bitstream); // Send RF packet
        success = true;
    }
    return success;
}

void Deltronic_Send(unsigned long address)
{
    byte repeatTimes = 16;
    byte repeat, index;
    int periodLong, periodSync;
    unsigned long bitmask;
    int period = 640;

    periodLong = 2 * period;
    periodSync = 36 * period;

    digitalWrite(PIN_RF_RX_VCC, LOW);            // Power off the RF receiver (if wired that way) to protect against interference
    digitalWrite(PIN_RF_TX_VCC, HIGH);           // Enable 433Mhz transmitter
    delayMicroseconds(TRANSMITTER_STABLE_DELAY); // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)

    // Send seperator
    digitalWrite(PIN_RF_TX_DATA, HIGH);
    delayMicroseconds(period);

    // Send sync
    digitalWrite(PIN_RF_TX_DATA, LOW);
    delayMicroseconds(periodSync);
    digitalWrite(PIN_RF_TX_DATA, HIGH);
    delayMicroseconds(period);

    for (repeat = 0; repeat < repeatTimes; repeat++)
    {
        bitmask = 0x00000800L;
        for (index = 0; index < 12; index++)
        {
            if (address & bitmask)
            {
                // Send 1
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(periodLong);
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(period);
            }
            else
            {
                // Send 0
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(period);
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(periodLong);
            }
            bitmask >>= 1;
        }
        // Send sync
        digitalWrite(PIN_RF_TX_DATA, LOW);
        delayMicroseconds(periodSync);
        digitalWrite(PIN_RF_TX_DATA, HIGH);
        delayMicroseconds(period);
    }

    digitalWrite(PIN_RF_TX_DATA, LOW);
    delayMicroseconds(TRANSMITTER_STABLE_DELAY); // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)
    digitalWrite(PIN_RF_TX_VCC, LOW);            // Disable the 433Mhz transmitter
    digitalWrite(PIN_RF_RX_VCC, HIGH);           // Enable the 433Mhz receiver
    RFLinkHW();
}
#endif // PLUGIN_TX_073
