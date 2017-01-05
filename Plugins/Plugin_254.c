//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                    Plugin-254: Signal Analyzer                                    ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin shows pulse lengths that have been received on RF and have not been decoded by
 * one of the other plugins. The primary use of this plugin is to provide an easy way to debug and 
 * analyse currently unsupported RF signals
 *
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 ***********************************************************************************************
 * Technical description:
 * This plugin just outputs unsupported  RF packets, use this plugin to find signals from new devices
 * Even if you do not know what to do with the data yourself you might want to share your data so
 * others can analyse it.
 \*********************************************************************************************/
#ifdef PLUGIN_254
boolean Plugin_254(byte function, char *string) {
        if (QRFDebug==true) {                      // debug is on? 
           if(RawSignal.Number<26)return false;        // make sure the packet is long enough to have a meaning 
           // ----------------------------------
           // Output
           // ----------------------------------
           sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
           Serial.print( pbuffer );
           // ----------------------------------
           Serial.print(F("DEBUG;Pulses="));           // debug data
           Serial.print(RawSignal.Number);             // print number of pulses
           Serial.print(F(";Pulses(uSec)="));          // print pulse durations
           //PrintHex8(RawSignal.Pulses+1,RawSignal.Number-1);
           PrintHex8(RawSignal.Pulses+1,RawSignal.Number);
        } else {
           if (RFUDebug==false) return false;          // debug is on? 
           if(RawSignal.Number<26)return false;        // make sure the packet is long enough to have a meaning 
           // ----------------------------------
           // Output
           // ----------------------------------
           sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
           Serial.print( pbuffer );
           // ----------------------------------
           Serial.print(F("DEBUG;Pulses="));           // debug data
           Serial.print(RawSignal.Number);             // print number of pulses
           Serial.print(F(";Pulses(uSec)="));          // print pulse durations
           //for(int x=1;x<RawSignal.Number;x++) {
           for(int x=1;x<RawSignal.Number+1;x++) {
              Serial.print(RawSignal.Pulses[x]*RAWSIGNAL_SAMPLE_RATE); 
              //if (x < RawSignal.Number-1) Serial.write(',');       
              if (x < RawSignal.Number) Serial.write(',');       
           }
        }
        Serial.println(";");
        RawSignal.Number=0;                         // Last plugin, kill packet
        return true;                                // stop processing 
}
#endif // PLUGIN_254
