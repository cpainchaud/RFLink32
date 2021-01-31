//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                     Plugin-083: CAME TOP-432  door/gate opener                    ##
//#######################################################################################################


#define OXOALARMPLUGIN_ID 087
#define PLUGIN_DESC_087 PSTR("OXOALARM")

#define OXO_HEADER_PULSE_LEN 2000 
#define OXO_HEADER_PULSE_LEN_MIN 1900
#define OXO_HEADER_PULSE_LEN_MAX 2250

#define OXO_PULSE_SHORT_LEN 430
#define OXO_PULSE_SHORT_LEN_MIN 380
#define OXO_PULSE_SHORT_LEN_MAX 530

#define OXO_PULSE_LONG_LEN 800
#define OXO_PULSE_LONG_LEN_MIN 740
#define OXO_PULSE_LONG_LEN_MAX 1050

#define OXO_CONTROL_PULSECOUNT 66


#ifdef PLUGIN_087

char plugin_087_tmpbuf[50];

inline short pulse_type(byte pulse) {
    
    int pulse_len = pulse*RawSignal.Multiply;

    if(pulse_len < OXO_PULSE_SHORT_LEN_MIN)
        return -1;
    if(pulse_len <= OXO_PULSE_SHORT_LEN_MAX)
        return 0;
    if(pulse_len < OXO_PULSE_LONG_LEN_MIN)
        return -1;
    if(pulse_len > OXO_PULSE_LONG_LEN_MAX)
        return -1;
    return 1;
}

boolean Plugin_087(byte function, char *string)
{
    uint32_t code = 0;

    if (RawSignal.Number != OXO_CONTROL_PULSECOUNT)
        return false;

     Serial.println("OXO check2");

    //Serial.println("OXO check");
    //sprintf(plugin_087_tmpbuf, "%i %i %i", RawSignal.Multiply, RawSignal.Pulses[1]*RawSignal.Multiply, RawSignal.Pulses[2]*RawSignal.Multiply );
    //Serial.println(plugin_087_tmpbuf);

    long duration = RawSignal.Pulses[1]*RawSignal.Multiply;
    if(duration <= OXO_PULSE_LONG_LEN_MIN || duration >= OXO_PULSE_LONG_LEN_MAX)
        return false;

         Serial.println("OXO check3");
    
    duration = RawSignal.Pulses[2]*RawSignal.Multiply;
    if(duration <= OXO_HEADER_PULSE_LEN_MIN || duration >= OXO_HEADER_PULSE_LEN_MAX)
        return false;

    Serial.println("OXO found");

    int i=0;
    for(; i<OXO_CONTROL_PULSECOUNT-4;i+=2){
        int x=i+3;
        short first_type = pulse_type(RawSignal.Pulses[x]);
        short second_type = pulse_type(RawSignal.Pulses[x+1]);

        if(first_type == -1 || second_type == -1 || second_type == 0) {
            sprintf(plugin_087_tmpbuf, "OXO decoding error at pair #%u = %u %u", i, RawSignal.Pulses[x]*RawSignal.Multiply, RawSignal.Pulses[x+1]*RawSignal.Multiply );
            Serial.println(plugin_087_tmpbuf);
            return false;
        }

        if(first_type == 1) 
            code += 1;
        
        code = code << 1;
    }

    sprintf(plugin_087_tmpbuf, "***** code= 0x%04x  %u+%u ********", code, RawSignal.Pulses[i+3]*RawSignal.Multiply, RawSignal.Pulses[i+4]*RawSignal.Multiply );
    Serial.println(plugin_087_tmpbuf);

    RawSignal.Repeats = true;

    RawSignal.Number = 0;

    return true;
}
#endif // PLUGIN_087


#ifdef PLUGIN_TX_087

#include "1_Radio.h"

boolean PluginTX_087(byte function, char *string)
{
   boolean success = false;
   //10;BYRON;112233;01;OFF;
   //01234567890123456789012
   if (strncasecmp(InputBuffer_Serial + 3, "OXOALARM;", 8) == 0){
       Serial.println("OXO TX Requested");
       //RawSignalStruct RawSignal = {0, 0, 0, 0, 0UL};

       RawSignal.Number = OXO_CONTROL_PULSECOUNT;
       RawSignal.Repeats = 3;
       RawSignal.Delay = 20;
       RawSignal.Multiply = RAWSIGNAL_SAMPLE_RATE; 
       
       RawSignal.Pulses[1] = OXO_PULSE_LONG_LEN / RawSignal.Multiply;
       RawSignal.Pulses[2] = OXO_HEADER_PULSE_LEN / RawSignal.Multiply;

       //uint32_t code = 0xb2b4b0e0;
       uint32_t code= 0x10101a8;

       for(int i=3; i < 65; i+=2) {

            if( (code & 0x80000000) == 0 )
                RawSignal.Pulses[i] = OXO_PULSE_SHORT_LEN / RawSignal.Multiply;
            else
                RawSignal.Pulses[i] = OXO_PULSE_LONG_LEN / RawSignal.Multiply;

            RawSignal.Pulses[i+1] = OXO_PULSE_LONG_LEN / RawSignal.Multiply;

            sprintf(plugin_087_tmpbuf, "writing pair #%u: %u,%u", (i-1)/2, RawSignal.Pulses[i]*RawSignal.Multiply, RawSignal.Pulses[i+1]*RawSignal.Multiply );
            Serial.println(plugin_087_tmpbuf);

            itoa (code, plugin_087_tmpbuf, 2);
            Serial.println(plugin_087_tmpbuf);

            code = code << 1;
       }

       RawSignal.Pulses[65] = OXO_PULSE_SHORT_LEN / RawSignal.Multiply;
       RawSignal.Pulses[66] = OXO_PULSE_SHORT_LEN / RawSignal.Multiply;

       Serial.println(RawSignal.Number);

       //Plugin_087(0,"");
       //enableTX();
       RawSendRF();
       
       /*delay(2000);
       digitalWrite(PIN_RF_TX_DATA, LOW);
       delay(2000);
       digitalWrite(PIN_RF_TX_DATA, HIGH);
       delay(2000);
       digitalWrite(PIN_RF_TX_DATA, LOW);
       delay(2000);
       digitalWrite(PIN_RF_TX_DATA, HIGH);
       delay(2000);
       digitalWrite(PIN_RF_TX_DATA, LOW);
       delay(2000);
       digitalWrite(PIN_RF_TX_DATA, HIGH);
       delay(2000);
       digitalWrite(PIN_RF_TX_DATA, LOW);
       delay(2000);
       digitalWrite(PIN_RF_TX_DATA, HIGH);
       delay(2000);
       digitalWrite(PIN_RF_TX_DATA, LOW);
       delay(2000);
       digitalWrite(PIN_RF_TX_DATA, HIGH);
       disableTX();
       Serial.println("** pushed ***");*/

       RawSignal.Number=0;
       success = true;
   }

   return success;
}

#endif // PLUGIN_TX_087