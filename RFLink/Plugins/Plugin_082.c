//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                     Plugin-082 Mertik Maxitrol                                    ##
//#######################################################################################################
/*********************************************************************************************\
 * This Plugin takes care of reception of Mertik Maxitrol / DRU for fireplaces
 * PCB markings: G6R H4T1.
 *
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical information:
 *
 * 0001100101101001011001101
 *   ----------------------- data bits (10=1 01=0)
 * --                        preamble, always 00?   
 * Shortened: (10=1 01=0)
 * 01100101101001011001101
 * 0 1 0 0 1 1 0 0 1 0 1 1 
 * 
 * 010011001011  
 *         ----   command => 4 bits
 * --------       address => 8 bits 
 *
 * command bits:
 * 0111 7 off 
 * 0011 3 on  
 * 1011 b up
 * 1101 d down
 * 1000 8 stop
 * 1010 a go up
 * 1100 c go down
 *
 * Sample RF packet: 
 * Pulses=26;Pulses(uSec)=475,300,325,700,325,700,325,700,325,700,725,300,725,300,725,300,725,300,725,300,325,700,725,300,725;
 \*********************************************************************************************/
#define MAXITROL_PULSECOUNT     26

#define PLUGIN_082_RFSTART      100
#define PLUGIN_082_RFSPACE      250
#define PLUGIN_082_RFLOW        400
#define PLUGIN_082_RFHIGH       750

#ifdef PLUGIN_082
boolean Plugin_082(byte function, char *string) {
      if (RawSignal.Number !=MAXITROL_PULSECOUNT) return false;
      unsigned int bitstream=0L;
      byte address=0;
      byte command=0;
      byte status=0;
      //==================================================================================
      // get bits
      for(int x=3;x <= MAXITROL_PULSECOUNT-1;x=x+2) {
         if (RawSignal.Pulses[x]*RAWSIGNAL_SAMPLE_RATE < 550) {
            if (RawSignal.Pulses[x+1]*RAWSIGNAL_SAMPLE_RATE < 550) return false;
            bitstream = (bitstream << 1);           // 0
         } else {
            if (RawSignal.Pulses[x]*RAWSIGNAL_SAMPLE_RATE > 900) return false;
            if (RawSignal.Pulses[x+1]*RAWSIGNAL_SAMPLE_RATE > 550) return false;
            bitstream = (bitstream << 1) | 0x1;     // 1
         }
      }
      //==================================================================================
      // all bytes received, make sure packet is valid
      if (RawSignal.Pulses[1]*RAWSIGNAL_SAMPLE_RATE > 550) return false;
      if (RawSignal.Pulses[2]*RAWSIGNAL_SAMPLE_RATE > 550) return false;
      //==================================================================================
      // Prevent repeating signals from showing up
      //==================================================================================
      if( (SignalHash!=SignalHashPrevious) || (RepeatingTimer<millis()) ) { 
         // not seen the RF packet recently
         if (bitstream == 0) return false;            // sanity check
      } else {
         // already seen the RF packet recently
         return true;
      }      
      //==================================================================================
      command=(bitstream) & 0x0f;                   // get address from pulses
      address=((bitstream)>>4)&0xff;
      if (command == 0xB) status=1;                 // up
      else if (command == 0xD) status=2;            // down
      else if (command == 0x7) status=3;            // off
      else if (command == 0x3) status=4;            // on
      else if (command == 0x8) status=5;            // stop
      else if (command == 0xa) status=6;            // go up
      else if (command == 0xc) status=7;            // go down
      else {
        return false;
      }
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( pbuffer );
      Serial.print(F("Mertik;"));                   // Label
      sprintf(pbuffer, "ID=%02x;", address);        // ID
      Serial.print( pbuffer );
      sprintf(pbuffer, "SWITCH=%02x;", status);
      Serial.print( pbuffer );
      Serial.print(F("CMD="));                         
      if (status==1) Serial.print(F("UP;"));           
      if (status==2) Serial.print(F("DOWN;"));         
      if (status==3) Serial.print(F("OFF;"));          
      if (status==4) Serial.print(F("ON;"));           
      if (status==5) Serial.print(F("STOP;"));         
      if (status==6) Serial.print(F("GOUP;"));       
      if (status==7) Serial.print(F("GODOWN;"));     
      Serial.println();
      //==================================================================================
      RawSignal.Repeats=true;                    // suppress repeats of the same RF packet         
      RawSignal.Number=0;
      return true;
}
#endif // PLUGIN_082

#ifdef PLUGIN_TX_082
boolean PluginTX_082(byte function, char *string) {
        boolean success=false;
        unsigned long bitstream=0L;
        //10;MERTIK;64;UP;
        //0123456789012345
        if (strncasecmp(InputBuffer_Serial+3,"MERTIK;",7) == 0) { // KAKU Command eg.
           if (InputBuffer_Serial[12] != ';') return false;
           unsigned int bitstream2=0;                  // holds last 8 bits
           
           InputBuffer_Serial[8]=0x30;
           InputBuffer_Serial[9]=0x78;                             // Get address from hexadecimal value 
           InputBuffer_Serial[12]=0x00;                            // Get address from hexadecimal value 
           bitstream=str2int(InputBuffer_Serial+8);                // Address (first 16 bits)

           if(strcasecmp(InputBuffer_Serial+13,"stop")==0) bitstream2=0x8;
           else if(strcasecmp(InputBuffer_Serial+13,"on")==0) bitstream2=0x3;
           else if(strcasecmp(InputBuffer_Serial+13,"off")==0) bitstream2=0x7;
           else if(strcasecmp(InputBuffer_Serial+13,"up")==0) bitstream2=0xB;
           else if(strcasecmp(InputBuffer_Serial+13,"down")==0) bitstream2=0xD;
           else if(strcasecmp(InputBuffer_Serial+13,"go_up")==0) bitstream2=0xA;
           else if(strcasecmp(InputBuffer_Serial+13,"go_down")==0) bitstream2=0xC;
           if (bitstream2==0) return false;
           //-----------------------------------------------
           RawSignal.Multiply=50;
           RawSignal.Repeats=10;
           RawSignal.Delay=20;
           RawSignal.Pulses[1]=PLUGIN_082_RFLOW/RawSignal.Multiply;
           RawSignal.Pulses[2]=PLUGIN_082_RFLOW/RawSignal.Multiply;
           for(byte x=18;x>=3;x=x-2) {
              if ((bitstream & 1) == 1) {
                 RawSignal.Pulses[x] = PLUGIN_082_RFLOW/RawSignal.Multiply;
                 RawSignal.Pulses[x-1] = PLUGIN_082_RFHIGH /RawSignal.Multiply;
              } else {
                 RawSignal.Pulses[x] = PLUGIN_082_RFHIGH /RawSignal.Multiply;
                 RawSignal.Pulses[x-1] = PLUGIN_082_RFLOW/RawSignal.Multiply;
              }
              bitstream = bitstream >> 1;
           }
           for(byte x=26;x>=19;x=x-2) {
              if ((bitstream2 & 1) == 1) {
                 RawSignal.Pulses[x] = PLUGIN_082_RFLOW/RawSignal.Multiply;
                 RawSignal.Pulses[x-1] = PLUGIN_082_RFHIGH /RawSignal.Multiply;
              } else {
                 RawSignal.Pulses[x] = PLUGIN_082_RFHIGH /RawSignal.Multiply;
                 RawSignal.Pulses[x-1] = PLUGIN_082_RFLOW/RawSignal.Multiply;
              }
              bitstream2 = bitstream2 >> 1;
           }
           RawSignal.Pulses[27]=PLUGIN_082_RFSTART/RawSignal.Multiply;
           RawSignal.Number=27;
           RawSendRF();
           success=true;        
           //-----------------------------------------------
        }
        return success;
}
#endif // PLUGIN_082
