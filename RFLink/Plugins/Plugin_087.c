//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                       Plugin-087: NOX Security Alarm  Control+Sensors                             ##
//#######################################################################################################


#define NOXALARMPLUGIN_ID 087
#define PLUGIN_DESC_087 PSTR("NOXALARM")

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

    RawSignal.Repeats = true;

    RawSignal.Number = 0;

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
       Serial.println("NOX TX Requested");
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
       //enableTX();
       Signal::RawSendRF(&signal);
       
       signal.Number=0;
       success = true;
   }

   return success;
}

#endif // PLUGIN_TX_087