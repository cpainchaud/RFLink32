//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                       Plugin-087: NOX Security Alarm  Control+Sensors                             ##
//#######################################################################################################


/****************************
 * 
 * 
 * 20;XX;DEBUG;Pulses=66;Pulses(uSec)=841,1972,857,866,424,863,866,856,867,865,429,853,438,848,877,848,440,856,872,848,439,847,879,846,883,844,452,835,882,844,446,844,445,843,884,844,450,837,883,840,882,843,450,836,453,839,450,842,453,836,880,843,885,842,880,844,451,835,452,838,454,836,451,843,445,5000;
 * 20;XX;DEBUG;Pulses=66;Pulses(uSec)=919,1908,915,820,466,815,908,820,902,825,462,824,466,828,894,827,464,829,892,834,457,832,888,837,888,838,453,836,888,841,446,839,454,836,887,844,444,840,886,841,880,842,447,841,451,847,443,841,446,848,876,848,878,844,879,847,443,846,443,844,444,844,447,851,438,5000;
 * 20;XX;DEBUG;Pulses=66;Pulses(uSec)=912,1914,906,827,463,820,900,821,901,834,458,838,451,834,886,834,456,838,884,839,451,837,887,840,881,848,442,847,876,844,445,845,448,844,881,842,448,845,874,856,871,847,443,846,443,844,444,855,434,848,879,855,870,847,873,852,438,851,440,851,436,855,433,852,441,5000;
 * 
 * *************************/

#define NOXALARMPLUGIN_ID 087
#define PLUGIN_DESC_087 "NOXALARM"


#define NOX_SUPERPREAMBLE_PULSE_LEN 20000 

#define NOX_HEADER_PULSE_LEN 2000 
#define NOX_HEADER_PULSE_LEN_MIN 1900
#define NOX_HEADER_PULSE_LEN_MAX 2250

#define NOX_PULSE_SHORT_LEN 430
#define NOX_PULSE_SHORT_LEN_MIN 380
#define NOX_PULSE_SHORT_LEN_MAX 530

#define NOX_PULSE_LONG_LEN 800
#define NOX_PULSE_LONG_LEN_MIN 740
#define NOX_PULSE_LONG_LEN_MAX 1050

#define NOX_CONTROL_PULSECOUNT 66


#ifdef PLUGIN_087

inline short pulse_type(byte pulse) {
    
    int pulse_len = pulse*RawSignal.Multiply;

    if(pulse_len < NOX_PULSE_SHORT_LEN_MIN)
        return -1;
    if(pulse_len <= NOX_PULSE_SHORT_LEN_MAX)
        return 0;
    if(pulse_len < NOX_PULSE_LONG_LEN_MIN)
        return -1;
    if(pulse_len > NOX_PULSE_LONG_LEN_MAX)
        return -1;
    return 1;
}

boolean Plugin_087(byte function, const char *string)
{
    char tmpbuf[60];
    uint32_t code = 0;

    if (RawSignal.Number != NOX_CONTROL_PULSECOUNT)
        return false;

    //Serial.println("NOX check");
    //sprintf(plugin_087_tmpbuf, "%i %i %i", RawSignal.Multiply, RawSignal.Pulses[1]*RawSignal.Multiply, RawSignal.Pulses[2]*RawSignal.Multiply );
    //Serial.println(plugin_087_tmpbuf);

    unsigned long duration = ((unsigned long)RawSignal.Pulses[1])*RawSignal.Multiply;
    if(duration <= NOX_PULSE_LONG_LEN_MIN || duration >= NOX_PULSE_LONG_LEN_MAX) {
        //Serial.print("failed on check2 duration=");Serial.print(RawSignal.Pulses[1]);Serial.print("  multiply=");Serial.println(duration);
        return false;
    }

    //Serial.println("NOX check3");
    
    duration = RawSignal.Pulses[2]*RawSignal.Multiply;
    if(duration <= NOX_HEADER_PULSE_LEN_MIN || duration >= NOX_HEADER_PULSE_LEN_MAX)
        return false;

    //Serial.println("NOX found");

    int i=0;
    for(; i<NOX_CONTROL_PULSECOUNT-4;i+=2){
        int x=i+3;
        short first_type = pulse_type(RawSignal.Pulses[x]);
        short second_type = pulse_type(RawSignal.Pulses[x+1]);

        if(first_type == -1 || second_type == -1 || second_type == 0) {
            //sprintf(plugin_087_tmpbuf, "NOX decoding error at pair #%u = %u %u", i, RawSignal.Pulses[x]*RawSignal.Multiply, RawSignal.Pulses[x+1]*RawSignal.Multiply );
            //Serial.println(plugin_087_tmpbuf);
            return false;
        }

        if(first_type == 1) 
            code += 1;
        
        code = code << 1;
    }

    sprintf(tmpbuf, "***** code= 0x%04x  %u+%u ********", code, RawSignal.Pulses[i+3]*RawSignal.Multiply, RawSignal.Pulses[i+4]*RawSignal.Multiply );
    Serial.println(tmpbuf);

    RawSignal.Repeats = 3;
    RawSignal.Delay = 15;

    return true;
}
#endif // PLUGIN_087


#ifdef PLUGIN_TX_087

#include "1_Radio.h"
#include "2_Signal.h"

boolean PluginTX_087(byte function, const char *string)
{
    char tmpbuf[60];
    boolean success = false;
    RawSignalStruct signal;

    if (strncasecmp(InputBuffer_Serial + 3, "NOXALARM;", 8) == 0){
       Serial.println(F("NOX TX Requested"));
       //RawSignalStruct RawSignal = {0, 0, 0, 0, 0UL};

       signal.Number = NOX_CONTROL_PULSECOUNT;
       signal.Repeats = 3;
       signal.Delay = 15;
       signal.Multiply = 10; 
       
       signal.Pulses[1] = NOX_PULSE_LONG_LEN / signal.Multiply;
       signal.Pulses[2] = NOX_HEADER_PULSE_LEN / signal.Multiply;

       //uint32_t code = 0xb2b4b0e0;
       uint32_t code= 0x10101a8;

       for(int i=3; i < 65; i+=2) {

            if( (code & 0x80000000) == 0 )
                signal.Pulses[i] = NOX_PULSE_SHORT_LEN / signal.Multiply;
            else
                signal.Pulses[i] = NOX_PULSE_LONG_LEN / signal.Multiply;

            signal.Pulses[i+1] = NOX_PULSE_LONG_LEN / signal.Multiply;

            sprintf(tmpbuf, "writing pair #%u: %u,%u", (i-1)/2, signal.Pulses[i]*signal.Multiply, signal.Pulses[i+1]*signal.Multiply );
            Serial.println(tmpbuf);

            itoa (code, tmpbuf, 2);
            Serial.println(tmpbuf);

            code = code << 1;
       }

       signal.Pulses[65] = NOX_PULSE_SHORT_LEN / signal.Multiply;
       signal.Pulses[66] = NOX_PULSE_SHORT_LEN / signal.Multiply;

       //Serial.println(signal.Number);

       //Plugin_087(0,"");
       //enableTX_generic();
       Signal::RawSendRF(&signal);
       
       signal.Number=0;
       success = true;
   }

   return success;
}

#endif // PLUGIN_TX_087