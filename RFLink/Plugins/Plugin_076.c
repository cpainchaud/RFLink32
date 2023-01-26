//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                          Plugin-076: Came TOP-432 Door/gate Opener Remotes                        ##
//#######################################################################################################

/*********************************************************************************************\
 * This plugin takes care of decoding the CAME TOP-342 protocol
 *   
 * Author  (present)  : 2021 Christophe Painchaud
 * Support (present)  : https://github.com/couin3/RFLink 
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical Information:
 * 
 * CAME-TOP-432 is a common Gate/Door system
 * Their radio receiver is usually equiped with a 8 bit switch to generate a unique code
 * 
 * Sample signals, 26 pulses repeated at least 4 times with 20ms delay between pulses:
 * 20;XX;DEBUG;Pulses=26;Pulses(uSec)=368,608,384,192,768,224,768,208,800,192,768,624,368,624,352,624,352,208,768,224,752,624,352,240,736,896;
 * 20;XX;DEBUG;Pulses=26;Pulses(uSec)=384,592,384,176,784,208,768,208,784,208,768,640,352,624,352,640,352,224,752,224,752,640,336,224,752,896;
 * 20;XX;DEBUG;Pulses=26;Pulses(uSec)=384,608,368,192,784,192,784,208,768,208,768,624,352,624,352,624,352,224,752,224,752,640,336,224,128,896;
 * 20;XX;DEBUG;Pulses=26;Pulses(uSec)=384,576,400,176,800,192,800,192,784,208,768,624,352,640,352,624,352,224,768,224,752,640,368,208,752,896;
 * 20;XX;DEBUG;Pulses=26;Pulses(uSec)=400,592,464, 96,832,176,784,192,784,208,784,624,352,624,352,624,368,208,768,224,768,640,352,208,768,896;
 * 20;XX;DEBUG;Pulses=26;Pulses(uSec)=384,608,384,256,752,240,736,272,720,256,720,576,400,592,384,592,384,272,720,288,688,624,368,288,688,896;
 *
 \*********************************************************************************************/


#define PLUGIN_DESC_076 "CAME-TOP432"

#define PLUGIN_076_PULSE_COUNT 26

#define PLUGIN_076_PREAMBLE 290
// #define PLUGIN_076_PREAMBLE_MIN 368 <--- RXB6 sees fat pulses
#define PLUGIN_076_PREAMBLE_MIN 280
#define PLUGIN_076_PREAMBLE_MAX 432

#define PLUGIN_076_SHORT_PULSE 290
#define PLUGIN_076_SHORT_PULSE_MIN 180
#define PLUGIN_076_SHORT_PULSE_MAX 464

#define PLUGIN_076_LONG_PULSE 710
#define PLUGIN_076_LONG_PULSE_MIN 570
#define PLUGIN_076_LONG_PULSE_MAX 850

#define PLUGIN_076_REPEAT_IGNORE_TIME_MS 1500

#if defined(PLUGIN_076)
#include "1_Radio.h"
#include "2_Signal.h"

#if defined(DEBUG) || defined(RFLINK_DEBUG)
#define PLUGIN_076_DEBUG
#endif

inline short pulse_type_76(uint16_t pulse) {
    
    uint32_t pulse_len = pulse*RawSignal.Multiply;

    if(pulse_len < PLUGIN_076_SHORT_PULSE_MIN)
        return -1;
    if(pulse_len <= PLUGIN_076_SHORT_PULSE_MAX)
        return 0;
    if(pulse_len < PLUGIN_076_LONG_PULSE_MIN)
        return -1;
    if(pulse_len > PLUGIN_076_LONG_PULSE_MAX)
        return -1;
    return 1;
}


boolean Plugin_076(byte function, const char *string)
{
    static uint16_t lastSeenID = 0;
    static unsigned long lastSeenTime = 0;
    
    uint16_t code = 0;

    if (RawSignal.Number != PLUGIN_076_PULSE_COUNT)
        return false;

    unsigned long duration = ((unsigned long)RawSignal.Pulses[1])*RawSignal.Multiply;
    if(duration <= PLUGIN_076_PREAMBLE_MIN || duration >= PLUGIN_076_PREAMBLE_MAX) {
        #ifdef PLUGIN_076_DEBUG
        Serial.print("failed on check2 duration=");Serial.print(RawSignal.Pulses[1]);Serial.print("  multiply=");Serial.println(duration);
        #endif
        return false;
    }

    for(int i=0; i < 12*2; i+=2) { // 12bits,
        code = code << 1;

        #ifdef PLUGIN_076_DEBUG
        sprintf(tmpbuf, "Pulse pair: %u / %u", RawSignal.Pulses[2+i], RawSignal.Pulses[2+i+1]);
        Serial.println(tmpbuf);
        #endif
        
        short firstPulseType = pulse_type_76(RawSignal.Pulses[2+i]);
        if(firstPulseType < 0){
            #ifdef PLUGIN_076_DEBUG
            Serial.print("failed on check3 duration=");Serial.print(RawSignal.Pulses[2+i]);Serial.print("  multiply=");Serial.println(RawSignal.Multiply);
            #endif
            return false;
        }
        short secondPulseType = pulse_type_76(RawSignal.Pulses[2+i+1]);
        if(secondPulseType < 0){
            #ifdef PLUGIN_076_DEBUG
            Serial.print("failed on check4 duration=");Serial.print(RawSignal.Pulses[2+i+1]);Serial.print("  multiply=");Serial.println(RawSignal.Multiply);
            #endif
            return false;
        }

        if( firstPulseType == secondPulseType){
            #ifdef PLUGIN_076_DEBUG
            Serial.print("failed on check5 durations are equal:");Serial.print(RawSignal.Pulses[2+i]);Serial.print(" / "); Serial.println(RawSignal.Pulses[2+i+1]);
            Serial.print("failed on check5 durations types:");Serial.print(firstPulseType);Serial.print(" / "); Serial.println(secondPulseType);
            Serial.print("failed on check5 durations multiply:");Serial.print(RawSignal.Multiply);Serial.print(" at loop #");Serial.println(i);
            #endif
            return false;
        }

        if(firstPulseType == 0) {
            code += 1;
             #ifdef PLUGIN_076_DEBUG
             Serial.println("Pair is a 1");
             #endif
        }
        else {
            #ifdef PLUGIN_076_DEBUG
            Serial.println("Pair is a 0");
            #endif
        }
    }

    #ifdef PLUGIN_076_DEBUG
    sprintf(tmpbuf, "***** CAME-TOP432 code= 0x%02x ********", code );
    Serial.println(tmpbuf);
    #endif

    auto now = millis();

    if(lastSeenID == code) {
        if(now - lastSeenTime < PLUGIN_076_REPEAT_IGNORE_TIME_MS)
            return true;
    } else
        lastSeenID = code;

    lastSeenTime = now;

    display_Header();
    display_Name(PSTR("CAME-TOP432"));
    display_IDn(code, 4);
    display_SWITCH(1);
    display_CMD(CMD_Single, CMD_On); // #ALL #ON
    display_Footer();

    RawSignal.Repeats = 4;
    RawSignal.Delay = 20;


    return true;
}
 #endif // PLUGIN_076

#ifdef PLUGIN_TX_076


boolean  PluginTX_076(byte function, const char *string)
{
    RawSignalStruct signal;

    if (strncasecmp(InputBuffer_Serial + 3, "CAME-TOP432;", 11) == 0){

        uint16_t code = 0x78d;

        signal.Number = PLUGIN_076_PULSE_COUNT;
        signal.Repeats = 5;
        signal.Delay = 15;
        signal.Multiply = RFLink::Signal::params::sample_rate;

        signal.Pulses[1] = PLUGIN_076_PREAMBLE / signal.Multiply;

        for(int i=0; i<12; i++){
            unsigned long mask = 1;
            mask = mask << (12 - i -1);
            
            if( (code & mask) != 0 ) {
                signal.Pulses[2+i*2] = PLUGIN_076_SHORT_PULSE / signal.Multiply;
                signal.Pulses[2+i*2+1] = PLUGIN_076_LONG_PULSE / signal.Multiply;
            }
            else {
                signal.Pulses[2+i*2] = PLUGIN_076_LONG_PULSE / signal.Multiply;
                signal.Pulses[2+i*2+1] = PLUGIN_076_SHORT_PULSE / signal.Multiply;
            }

        }

       RawSendRF(&signal);
       
       signal.Number=0;
    }
    else
        return false;

    return true;
}

#endif // PLUGIN_TX_076;
