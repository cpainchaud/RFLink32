//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-33 Conrad                                           ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding Conrad Pool Thermomether model 9771
 * 
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical information:
 * Decodes signals from a Conrad Model 9771 Pool Thermometer, (80 pulses, 40 bits, 433 MHz).
 * Message Format: 
 * AAAAAAAA BBBBBBBB CCCCCCCCCC DD EEEE FFFF GGGG
 * 00000000 00010010 1100101101 01 1001 0001 1001
 *
 * A = Always 0 ?
 * B = Device id ?
 * C = Temperature digits 
 * D = Temperature ones
 * E = Temperature tens
 * F = Always 1?
 * G = Unknown
 *
 * Sample:
 * 20;8D;DEBUG;Pulses=80;Pulses(uSec)=1890,5760,1890,5730,1890,5760,1890,5730,1890,5760,1890,5760,1890,5760,1890,5760,1890,5760,1890,5760,1890,5760,5910,1830,1890,5640,1890,5760,5910,1830,1890,5640,5910,1830,1860,5640,5910,1830,1890,5610,5910,1830,5910,1830,5910,1830,1890,5400,5910,1830,1890,5610,1890,5760,5910,1830,5910,1830,1890,5520,1890,5760,5910,1860,1890,5610,1890,5760,1890,5760,5910,1830,1890,5610,5910,1830,5910,1830,1860,6990;
 \*********************************************************************************************/
#define CONRAD_PULSECOUNT 80
#define CONRAD_PULSEMAX  5000/RAWSIGNAL_SAMPLE_RATE
#define CONRAD_PULSEMIN  2300/RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_033
boolean Plugin_033(byte function, char *string) {
      if (RawSignal.Number != CONRAD_PULSECOUNT) return false;
      unsigned long bitstream=0L;
      unsigned int temperature=0;
      unsigned int rc=0;
      byte checksum=0;
      byte bitcount=0;                              // bit counter (counting first 8 bits that need 
      //==================================================================================
      // get all 28 bits
      for(byte x=1;x <=CONRAD_PULSECOUNT-1;x+=2) {
         if (RawSignal.Pulses[x] > CONRAD_PULSEMAX) {
            if (RawSignal.Pulses[x+1] > CONRAD_PULSEMAX) if ( (x+1) < CONRAD_PULSECOUNT ) return false; // invalid pulse length
            if (bitcount > 7) { 
               bitstream = (bitstream << 1) | 0x1; 
            } else {
               return false;                        // first 8 bits should all be zeros
            }
            bitcount++;
         } else {
            if (RawSignal.Pulses[x] > CONRAD_PULSEMIN) return false; // invalid pulse length
            if (RawSignal.Pulses[x+1] < CONRAD_PULSEMIN) return false; // invalid pulse length
            if (bitcount > 7) bitstream = (bitstream << 1);
            bitcount++;
         }
      }
      //==================================================================================
      // Prevent repeating signals from showing up
      //==================================================================================
      Serial.println(bitstream,HEX);
      if( (SignalHash!=SignalHashPrevious) || (RepeatingTimer+1000<millis() ) || (SignalCRC != bitstream) ) { 
         SignalCRC=bitstream;                       // not seen the RF packet recently
         if (bitstream == 0) return false;          // Perform a sanity check
      } else {
         return true;                               // already seen the RF packet recently
      }
      //==================================================================================
      checksum = (bitstream >>  4) & 0x0f;
      if (checksum != 0x01) return false;
      
      rc=(bitstream >> 24);
      temperature=(bitstream >> 14) & 0x3ff; 
      temperature=temperature-500;
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( pbuffer );
      Serial.print(F("Conrad;"));                   // Label
      sprintf(pbuffer, "ID=%04x;", rc);             // ID    
      Serial.print( pbuffer );
      sprintf(pbuffer, "TEMP=%04x;", temperature);     
      Serial.print( pbuffer );
      Serial.println();
      //==================================================================================
      RawSignal.Repeats=true;                       // suppress repeats of the same RF packet
      RawSignal.Number=0;
      return true;
}
#endif // PLUGIN_033
