//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                     Plugin-75 Lidl doorbell                                       ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding the Lidl/SilverCrest Doorbell protocol for model Z31370-TX
 * 
 * Author             : StuntTeam
 * Support            : www.nodo-domotica.nl
 * Date               : 9-02-2015
 * Version            : 1.0
 * Compatibility      : RFLink 1.0
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical Information:
 * Decodes signals from a Lidl Doorbell, (90 pulses, 40 bits, 433 MHz).
 * Lidl Doorbell Message Format: 
 * 1010 1111 1000 0010 0001 1011 0011 0000 0000 0000    AF821B3000
 *                                    -------------- always zero
 *  
 * Sample packet: (Nodo Pulse timing)
 * 1850,1200,1400,1200,1400,1200,1400,1200,1400,  (Preamble)
 * 700,400,200,900,700,400,200,900,700,400,700,400,700,400,700,400,700,400,200,900,200,900,200,900,200,900,200,900,700,400,250,900,200,900,200,900,700,
 * 400,700,400,700,400,200,900,700,400,700,400,200,900,200,900,700,400,700,400,200,900,200,900,200,900,200,900,200,900,200,900,200,900,200,900,200,900,
 * 200,900,200,900,250,900,200
 *********************************************************************************************
 * Technical Information:
 * Decodes signals from a Lidl SilverCrest Z31370-TX Doorbell, (114 pulses, 48 bits, 433 MHz).
 * SilverCrest Doorbell Message Format: 
 * 01011100 10100101 10001000 | 00000100 11010110 10111011 10010111    5CA588 04D6BB97
 * always the same            | device unique number
 *  
 * Sample packet:
 * 20;14;DEBUG;Pulses=114;Pulses(uSec)=360,60,60,390,360,60,60,390,60,390,60,390,390,60,360,60,60,390,360,60,60,390,360,60,360,60,60,390,360,60,60,390,60,390,360,60,360,60,360,60,30,390,360,60,360,60,360,60,390,60,360,60,60,390,390,60,360,60,360,60,390,60,360,60,390,60,360,60,360,60,390,60,60,390,60,390,30,390,60,390,60,390,360,60,60,390,60,390,60,390,60,390,360,60,360,60,60,390,60,390,60,390,360,60,60,390,60,390,360,60,60,390,360,1260;
 \*********************************************************************************************/
   // ==================================================================================
#define LIDL_PULSECOUNT 90      // type 0
#define LIDL_PULSECOUNT2 114    // type 1
#define PLUGIN_ID 75

#ifdef PLUGIN_075
boolean Plugin_075(byte function, char *string) {
      if ((RawSignal.Number != LIDL_PULSECOUNT) && (RawSignal.Number != LIDL_PULSECOUNT2)) return false;
      unsigned long bitstream=0;
      unsigned long bitstream2=0;
      
      int bitcount=0;      
      byte type=0;
      //==================================================================================
      // get all bits
      if (RawSignal.Number == LIDL_PULSECOUNT) {
         if (RawSignal.Pulses[1]*RawSignal.Multiply > 1000 && RawSignal.Pulses[2]*RawSignal.Multiply > 1000 &&
            RawSignal.Pulses[3]*RawSignal.Multiply > 1000 && RawSignal.Pulses[4]*RawSignal.Multiply > 1000 && 
            RawSignal.Pulses[5]*RawSignal.Multiply > 1000 && RawSignal.Pulses[6]*RawSignal.Multiply > 1000 &&
            RawSignal.Pulses[7]*RawSignal.Multiply > 1000 && RawSignal.Pulses[8]*RawSignal.Multiply > 1000 && RawSignal.Pulses[9]*RawSignal.Multiply > 1000 ) {  
            //
         } else {
            return false;
         }
         for(int x=10;x <=90;x+=2) {
            if (bitcount < 28){
               if (RawSignal.Pulses[x]*RawSignal.Multiply > 550) {
                  bitstream = (bitstream << 1);
                  bitcount++;
               } else {
                  bitstream = (bitstream << 1) | 0x1; 
                  bitcount++;
               }
            } else {
               if (RawSignal.Pulses[x]*RawSignal.Multiply > 550) {
                  bitstream2 = (bitstream2 << 1);
                  bitcount++;
               } else {
                  bitstream2 = (bitstream2 << 1) | 0x1; 
                  bitcount++;
               }
            }
         }
      } else {
         if (RawSignal.Pulses[0] != PLUGIN_ID) return false; // only accept plugin1 translated packets
         type=1;
         for(byte x=1;x < LIDL_PULSECOUNT2-1;x+=2) {
            if (RawSignal.Pulses[x]*RawSignal.Multiply > 200) {
               if (RawSignal.Pulses[x+1]*RawSignal.Multiply > 200) return false; // invalid pulse length
               if (bitcount > 23) { 
                  bitstream2 = (bitstream2 << 1);
               } else {
                  bitstream = (bitstream << 1);
               }
               bitcount++;
            } else {
               if (RawSignal.Pulses[x+1]*RawSignal.Multiply < 200) return false; // invalid pulse length
               if (bitcount > 23) {
                  bitstream2 = (bitstream2 << 1) | 0x1; 
               } else {
                  bitstream = (bitstream << 1) | 0x1; 
               }
               bitcount++;
            }
         }
      }
      //==================================================================================
      // all bytes received, make sure checksum is okay
      //==================================================================================
      if (type==0) {
         if ((bitstream2 &0xfff) != 0x00) return false; // these 8 bits are always 0
      } else {
         if (bitstream != 0x5ca588) return false;      
      }
      //==================================================================================
      // Prevent repeating signals from showing up
      //==================================================================================
      if((SignalHash!=SignalHashPrevious) || ((RepeatingTimer+2000)<millis()) || (SignalCRC != bitstream) ) { 
         SignalCRC=bitstream;                       // not seen the RF packet recently
      } else {
         return true;                               // already seen the RF packet recently
      }      
      //==================================================================================
      // Output
      // ----------------------------------
      Serial.print("20;");
      PrintHexByte(PKSequenceNumber++);
      // ----------------------------------
      if (type == 0) {
         Serial.print(";SilverCrest;");              // Label
         sprintf(pbuffer, "ID=%08lx;", bitstream);     // ID      
      } else {
         Serial.print(";SilverCrest;");              // Label
         sprintf(pbuffer, "ID=%08lx;", bitstream2);    // ID      
      }
      Serial.print( pbuffer );
      Serial.print(F("SWITCH=1;CMD=ON;"));  
      Serial.print(F("CHIME=01;"));
      Serial.println();
      //==================================================================================
      RawSignal.Repeats=true;                          // suppress repeats of the same RF packet
      RawSignal.Number=0;                              // do not process the packet any further
      return true;
}
#endif // PLUGIN_075_CORE
