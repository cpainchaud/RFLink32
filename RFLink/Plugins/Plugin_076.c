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
 * Sample signals, usually repeated :
 * 20;XX;DEBUG;Pulses=26;Pulses(uSec)=368,608,384,192,768,224,768,208,800,192,768,624,368,624,352,624,352,208,768,224,752,624,352,240,736,896;
 * 20;XX;DEBUG;Pulses=26;Pulses(uSec)=384,592,384,176,784,208,768,208,784,208,768,640,352,624,352,640,352,224,752,224,752,640,336,224,752,896;
 * 20;XX;DEBUG;Pulses=26;Pulses(uSec)=384,608,368,192,784,192,784,208,768,208,768,624,352,624,352,624,352,224,752,224,752,640,336,224,128,896;
 * 20;XX;DEBUG;Pulses=26;Pulses(uSec)=384,576,400,176,800,192,800,192,784,208,768,624,352,640,352,624,352,224,768,224,752,640,368,208,752,896;
 * 20;XX;DEBUG;Pulses=26;Pulses(uSec)=400,592,464, 96,832,176,784,192,784,208,784,624,352,624,352,624,368,208,768,224,768,640,352,208,768,896;
 * 20;XX;DEBUG;Pulses=26;Pulses(uSec)=384,608,384,256,752,240,736,272,720,256,720,576,400,592,384,592,384,272,720,288,688,624,368,288,688,896;
 * 10;rfdebug=on;
 \*********************************************************************************************/

 #ifdef PLUGIN_076
 #define PLUGIN_DESC_076 "CAME-TOP432"

#define PLUGIN_076_PULSECOUNT 26

#define PLUGIN_076_PREAMBLE 400
#define PLUGIN_076_PREAMBLE_MIN 368
#define PLUGIN_076_PREAMBLE_MAX 432

#define PLUGIN_076_SHORT_PULSE = 270
#define PLUGIN_076_SHORT_PULSE_MIN 192
#define PLUGIN_076_SHORT_PULSE_MAX 464

#define PLUGIN_078_LONG_PULSE 730
#define PLUGIN_076_LONG_PULSE_MIN 570
#define PLUGIN_076_LONG_PULSE_MAX 800

inline short pulse_type_76(byte pulse) {
    
    int pulse_len = pulse*RawSignal.Multiply;

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

boolean Plugin_076(byte function, char *string)
{
    char tmpbuf[60];
    uint16_t code = 0;

    if (RawSignal.Number != PLUGIN_076_PULSECOUNT)
        return false;

    unsigned long duration = ((unsigned long)RawSignal.Pulses[1])*RawSignal.Multiply;
    if(duration <= PLUGIN_076_PREAMBLE_MIN || duration >= PLUGIN_076_PREAMBLE_MAX) {
        //Serial.print("failed on check2 duration=");Serial.print(RawSignal.Pulses[1]);Serial.print("  multiply=");Serial.println(duration);
        return false;
    }


    return false;
}
 #endif // PLUGIN_076

#ifdef PLUGIN_TX_076

#include "1_Radio.h"
boolean PluginTX_076(byte function, char *string)
{
    RawSignalStruct signal;
    if (strncasecmp(InputBuffer_Serial + 3, "CAME-TOP432;", 8) == 0){
        uint16_t code = 2162;
        code = code << 4;

        signal.Number = PLUGIN_076_PULSE_COUNT;
        signal.Repeats = 3;
        signal.Delay = 15;
        signal.Multiply = 10;

        signal.Pulses[1] = PLUGIN_076_PREAMBLE / signal.Multiply; 



    }
    else
        return false;

    return true;
}

#endif PLUGIN_TX_076;