//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                          Plugin-41 LaCrosse                                       ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding LaCrosse  weatherstation outdoor sensors
 * It also works for all non LaCrosse sensors that follow this protocol.
 * Lacrosse WS2355, WS3600 and compatibles
 *
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical information:
 * Partially based on http://makin-things.com/articles/decoding-lacrosse-weather-sensor-rf-transmissions/
 *
 * WS2355  
 * Each packet is 52 bits long. 4 different packet formats are transmitted. They are composed of: 
 * 
 * data     0     1    2    3   4    5   6    7   8    9   10
 * 1) 0000 1001 01 00 00100010 01111000 01010011 0011 10101100 0001   0+9+4+2+2+7+8+5+3+3+A+C=41 = 1
 * 2) 0000 1001 00 01 00100010 01111000 01010000 1101 10101111 1000   0+9+1+2+2+7+8+5+0+D+A+F=48 = 8
 * 3) 0000 1001 00 10 00100010 01111000 00001000 1100 11110111 1000
 * 4) 0000 1001 01 11 00100010 01111000 00000000 1100 11111111 1101
 *    SSSS PPPP QR AA BBBBBBBB CCCCCCCC DDDDDDDD dddd EEEEEEEE FFFF    
 *    0000 1001 01 01 01100110 01011110 10001000 1001 01110111 0100
 *    0000 1001 00 11 01100110 01011110 00000000 1101 11111111 0110 
 *    0000 1001 01 00 01100110 01011110 01000110 1000 10111001 0010
 *    0000 1001 00 01 01100110 01011110 01100111 1001 10011000 000    9+1+6+6+5+E+6+7+9+9+8=50  missing bit is 0
      0000 1001 01 11 01100110 01011110 00000000 1001 11111111 011    9+7+6+6+5+E+0+0+9+F+F=56  missing bit is 0
 * S = Sync 
 * P = Preamble (1001 for WS2300, 0110 for WS3600?)
 * A = packet type  00=TEMP, 01=HUM, 10=RAIN, 11=WIND
 * B = Rolling Code
 * C = Flags
 * D = 12 bit value depending on the device/packet type
 * E = Inverted data 
 * F = Checksum
 * Q = 0/1 1=windpacket reports windgust 0=windpacket reports windspeed
 * R = Error checking bit 
 *
 * TEMP Dd = temperature - 30 degrees offset, 12 bits, 0x533 - 0x300 = 0x233 = 23.3 degrees
 * HUM  D  = humidity value, 8 bits, 0x50 = RH of 50 
 * RAIN D = number of tips * 0.508mm (range=0-4095, Once the count reaches 4095 it wraps back to 0. Powerloss results in a reset of the count)  
 * WIND d = Wind direction (0-15 in 22.5 degrees steps) D= wind speed
 *
 * Sample:
 * 20;D3;DEBUG;Pulses=104;Pulses(uSec)=1400,1300,1325,1300,1325,1275,1350,1150,225,1300,1325,1275,1325,1275,225,1300,1325,1275,225,1275,1350,1275,225,1300,1325,1275,225,1300,225,1275,1350,1275,1350,1275,250,1275,225,1275,1350,1275,1350,1300,225,1300,1350,1275,225,1275,225,1275,225,1275,225,1275,1325,1275,225,1300,1325,1275,1325,1275,1325,1275,250,1275,1350,1275,1325,1300,1325,1275,250,1275,1350,1275,1325,1275,250,1275,1325,1275,250,1275,225,1275,225,1275,1350,1275,225,1275,250,1275,225,1275,1325,1275,250,1275,1350,1300,1325;
 * 20;D4;DEBUG;Pulses=104;Pulses(uSec)=1400,1275,1350,1275,1350,1275,1325,1150,250,1275,1350,1275,1325,1275,250,1275,1325,1275,1350,1275,225,1275,225,1275,1350,1300,225,1275,225,1275,1350,1275,1325,1275,225,1275,225,1275,1325,1275,1325,1275,250,1275,1350,1300,225,1275,225,1275,225,1275,225,1275,1350,1275,1325,1275,1350,1275,1325,1275,1350,1275,1325,1275,1350,1275,1325,1300,1325,1275,225,1275,225,1275,1350,1275,225,1275,225,1300,225,1275,250,1275,225,1275,225,1275,250,1275,225,1275,225,1275,1350,1275,250,1275,225,1275,1325;
 * 20;D5;DEBUG;Pulses=104;Pulses(uSec)=1400,1275,1350,1275,1350,1275,1325,1150,225,1275,1350,1275,1325,1275,225,1300,1325,1275,225,1300,1325,1275,1325,1275,1350,1275,225,1300,225,1275,1350,1275,1350,1300,225,1300,225,1275,1350,1275,1325,1275,250,1275,1350,1275,250,1275,225,1275,225,1275,225,1275,1325,1275,1350,1275,250,1275,1325,1275,1350,1275,1350,1275,225,1275,225,1275,1350,1275,225,1300,1325,1275,1325,1275,1350,1275,250,1275,1325,1275,250,1275,250,1275,225,1275,1350,1275,1350,1275,225,1275,1350,1275,1350,1275,225,1275,1325;
   20;C3;DEBUG;Pulses=102;Pulses(uSec)=1400,1275,1325,1275,1325,1275,1325,1175,225,1300,1350,1275,1350,1275,225,1300,1325,1300,1325,1275,1325,1300,225,1300,1325,1275,225,1275,225,1300,1325,1275,1325,1275,250,1275,225,1275,1325,1275,1350,1275,225,1275,1325,1275,225,1225,300,1275,250,1275,225,1275,1325,1275,1325,1300,225,1275,225,1275,1325,1300,1325,1275,225,1275,225,1275,225,1275,225,1275,1325,1275,1325,1275,250,1275,250,1275,1325,1275,1350,1275,225,1275,225,1300,1325,1275,1350,1275,1325,1300,1325,1275,1350,1275,1325;
   20;E1;DEBUG;Pulses=102;Pulses(uSec)=1425,1275,1325,1275,1325,1275,1350,1150,225,1275,1350,1275,1350,1275,250,1275,1350,1275,225,1275,225,1275,250,1275,1350,1275,225,1300,225,1275,1325,1275,1350,1300,225,1275,225,1275,1350,1275,1325,1300,225,1275,1350,1275,250,1275,225,1275,225,1275,250,1275,1325,1275,1350,1275,1325,1275,1325,1275,1325,1275,1350,1275,1350,1275,1325,1300,1325,1275,250,1275,1325,1275,1325,1275,225,1275,250,1275,225,1275,250,1275,225,1300,225,1275,225,1275,225,1300,225,1275,1350,1275,250,1275,225;
  \*********************************************************************************************/
#define LACROSSE_PULSECOUNT 104  // also handles 102 pulses!
#define LACROSSE_PULSEMID  1000/RAWSIGNAL_SAMPLE_RATE
#define LACROSSE_MIDLO  1100/RAWSIGNAL_SAMPLE_RATE
#define LACROSSE_MIDHI  1400/RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_002
boolean Plugin_002(byte function, char *string) {
   if ((RawSignal.Number != LACROSSE_PULSECOUNT) && (RawSignal.Number != (LACROSSE_PULSECOUNT-2))) return false; 
      unsigned long bitstream1=0L;                  // holds first 16 bits 
      unsigned long bitstream2=0L;                  // holds last 28 bits

      unsigned int sensordata=0;
      byte checksum=0;
      byte bitcounter=0;                            // counts number of received bits (converted from pulses)
      byte sensortype=0;
      byte data[12];
      //==================================================================================
      // get bytes 
      for(int x=1;x < RawSignal.Number;x+=2) {
         if ((RawSignal.Pulses[x+1] < LACROSSE_MIDLO) || (RawSignal.Pulses[x+1] > LACROSSE_MIDHI)) {
            if (x+1 < RawSignal.Number) return false; // in between pulse check
         }
         if (RawSignal.Pulses[x] > LACROSSE_PULSEMID ){
            if (bitcounter < 20) {
               bitstream1 = (bitstream1 << 1);
               bitcounter++;                     // only need to count the first 20 bits
            } else {
               bitstream2 = (bitstream2 << 1);
            }
         } else {
            if (bitcounter < 20) {
               bitstream1 = (bitstream1 << 1) | 0x1; 
               bitcounter++;                     // only need to count the first 20 bits
            } else {
               bitstream2 = (bitstream2 << 1) | 0x1; 
            }
         }
      }
      if (RawSignal.Number == (LACROSSE_PULSECOUNT-2)) bitstream2 = (bitstream2 << 1); // add missing zero bit
      //==================================================================================
      // all bytes received, sort data, do sanity checks and make sure checksum is okay
      //==================================================================================
      if (bitstream1 == 0) return false;  
      //if ((bitstream1 == 0) && (bitstream2 == 0)) return false;  
      //data[0] = (bitstream1 >> 16) & 0x0f;          // prepare nibbles from bit stream
      //if (data[0] != 0) return false;               // just a quick check to make sure the sync bits are correct. they are not needed for the checksum
      data[0] = (bitstream1 >> 12) & 0x0f;          // First nibble 
      if ( (data[0] != 0x09) && (data[0] != 0x06) ) { // type verification
         return false;                              // 1001 for WS2300, 0110 for WS3600
      }
      data[1] = (bitstream1 >>  8) & 0x0f;          // Various other checks are possible
      data[2] = (bitstream1 >>  4) & 0x0f;          // Like parity checks and bit tests
      data[3] = (bitstream1 >>  0) & 0x0f;          // but false positives do not seem to be a problem

      data[4] = (bitstream2 >> 28) & 0x0f;
      data[5] = (bitstream2 >> 24) & 0x0f;
      data[6] = (bitstream2 >> 20) & 0x0f;
      data[7] = (bitstream2 >> 16) & 0x0f;
      data[8] = (bitstream2 >> 12) & 0x0f;
      data[9] = (bitstream2 >>  8) & 0x0f;
      data[10]= (bitstream2 >>  4) & 0x0f;
      //==================================================================================
      for (byte i=0;i<11;i++){ 
          checksum=checksum + data[i];              // max. value = A5
      }
      checksum=checksum & 0x0f;
      if (checksum != ((bitstream2)&0x0f)) return false;
      //==================================================================================
      // Prevent repeating signals from showing up
      //==================================================================================
      sensortype=(data[1])&0x03;                    // get sensor type from bitstream
      unsigned long tempval=((checksum)<<8)+sensortype;   // sensor type + checksum
      if( (SignalHash!=SignalHashPrevious) || (RepeatingTimer<millis()) || (SignalCRC != tempval)  ){ 
         SignalCRC=tempval;                         // not seen this RF packet recently, save value for later
      } else {
         return true;                               // already seen the RF packet recently
      }  
 	  //==================================================================================
      // now process the various sensor types      
      //==================================================================================
      // Output
      // ----------------------------------
      if (sensortype == 0x00) {                     // Temperature
         sensordata = data[6]*100;                  // 468
         sensordata = sensordata + data[7]*10;
         sensordata = sensordata + data[8];
         if ( (data[0]) == 0x09) {                  // WS2300
            sensordata = sensordata-300;
         } else {                                   // WS3600
            sensordata = sensordata-400;
         }
         // ----------------------------------
         Serial.print("20;");
         PrintHexByte(PKSequenceNumber++);
         Serial.print(F(";LaCrosseV2;ID="));        // Label
         PrintHex8( data+2,2);
         // ----------------------------------
         sprintf(pbuffer, ";TEMP=%04x;", sensordata);     
         Serial.println( pbuffer );
      } else
      if (sensortype == 0x01) {                     // Humidity
         sensordata=((data[6])<<4)+data[7];
         if (sensordata==0) return false;           // Humidity should not be 0
         // ----------------------------------
         Serial.print("20;");
         PrintHexByte(PKSequenceNumber++);
         Serial.print(F(";LaCrosseV2;ID="));         // Label
         PrintHex8( data+2,2);
         // ----------------------------------
         sprintf(pbuffer, ";HUM=%02x;", (sensordata)&0xff);     
         Serial.println( pbuffer );
      } else
      if (sensortype == 0x02) {                     // Rain
         unsigned long rain=((data[6])<<8)+((data[7])<<4)+(data[8]);
         if ( (data[0]) == 0x09) {                  // WS2300
            rain=rain*508;                          // 0-4095 * 0.508mm
            rain=rain/350;                          // divide by 3.5                      
         } else {
            rain=rain*518;                          // 0-4095 * 0.518mm
            rain=rain/100;                             
         }                                          
         // ----------------------------------      // 8c = 140 = 140*0,518=72,5mm   
         Serial.print("20;");
         PrintHexByte(PKSequenceNumber++);
         Serial.print(F(";LaCrosseV2;ID=00"));         // Label
         PrintHex8( data+2,1);
         //PrintHex8( data+2,2);
         // ----------------------------------
         sprintf(pbuffer, ";RAIN=%04x;", (rain)&0xffff);     
         Serial.println( pbuffer );
      } else
      if (sensortype == 0x03) {                     // Wind
         // ----------------------------------
         Serial.print("20;");
         PrintHexByte(PKSequenceNumber++);
         Serial.print(F(";LaCrosseV2;ID="));         // Label
         PrintHex8( data+2,2);
         // ----------------------------------
         sensordata=(data[8])&0x0f;                 // wind direction in 22.5 degree steps 
         sprintf(pbuffer, ";WINDIR=%04d;", sensordata);     
         Serial.print( pbuffer ); 
         
         sensordata=((data[6])<<4)+data[7];         // possibly 9 bits?
         sensordata=sensordata * 36;                // go from m/s to km/hr 6*36=216 = 21,6 km hr = 6 m/s 
         if ( (data[0]) == 0x09) {                  // WS2300
            sensordata=sensordata / 10;             // divide by 10
         }
         // 216 * 0,0277778   
         sprintf(pbuffer, "WINGS=%04x;WINSP=%04x;", sensordata, sensordata);     
         Serial.println( pbuffer );
      } else {
         Serial.print("20;");
         PrintHexByte(PKSequenceNumber++);
         Serial.print(F(";LaCrosseV2;DEBUG="));         // Label
         PrintHex8( data,11);
         Serial.println();
         //return false;
      }
      //==================================================================================
      RawSignal.Repeats=false;
      RawSignal.Number=0;
      return true;
}
#endif // PLUGIN_002
