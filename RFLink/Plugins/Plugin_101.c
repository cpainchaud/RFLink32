//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                     Plugin-48 Oregon V1/2/3                                       ##
//#######################################################################################################
/*********************************************************************************************\
 * This protocol takes care of receiving a few 868 Mhz protocols
 *
 * Protocols          : Visonic, 
 *                      ELV EM-Serie: EM-1000S, EM-100-EM, EM-1000-GZ
 *                      ELV KS serie: Thermo (AS3), Thermo/Hygro (AS2000, ASH2000, S2000, S2001A, S2001IA, ASH2200, S300IA) 
 *                                    Rain (S2000R), Wind (S2000W), Thermo/Hygro/Baro (S2001I, S2001ID), UV (S2500H) 
 *                                    Pyrano (Strahlungsleistung), Kombi (KS200, KS300) 
 *                      FS20 FS serie
 *
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v0.1 beta 
 *********************************************************************************************
 * Technical information:
 * Supports Oregon V1, V2 and V3 protocol messages
 * Core code from https://github.com/Cactusbone/ookDecoder/blob/master/ookDecoder.ino
 * Copyright (c) 2014 Charly Koza cactusbone@free.fr Copyright (c) 2012 Olivier Lebrun olivier.lebrun@connectingstuff.net 
 * Copyright (c) 2012 Dominique Pierre (zzdomi) Copyright (c) 2010 Jean-Claude Wippler jcw@equi4.com
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation 
 * files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, 
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software 
 * is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 \*********************************************************************************************/
#define PLUGIN_ID 101
#define PLUGIN_NAME "Few868"

#define OSV3_PULSECOUNT_MIN 126 
#define OSV3_PULSECOUNT_MAX 278


// =====================================================================================================
class DecodeOOK {
protected:
	byte total_bits, bits, flip, state, pos, data[25];
	virtual char decode(word width) = 0;
public:
	enum { UNKNOWN, T0, T1, T2, T3, OK, DONE };
    // -------------------------------------
	DecodeOOK() { resetDecoder(); }
    // -------------------------------------
	bool nextPulse(word width) {
		if (state != DONE)

			switch (decode(width)) {
			case -1: resetDecoder(); break;
			case 1:  done(); break;
		}
		return isDone();
	}
    // -------------------------------------
	bool isDone() const { return state == DONE; }
    // -------------------------------------
	const byte* getData(byte& count) const {
		count = pos;
		return data;
	}
    // -------------------------------------
	void resetDecoder() {
		total_bits = bits = pos = flip = 0;
		state = UNKNOWN;
	}
    // -------------------------------------
	// add one bit to the packet data pbuffer
    // -------------------------------------
	virtual void gotBit(char value) {
		total_bits++;
		byte *ptr = data + pos;
		*ptr = (*ptr >> 1) | (value << 7);

		if (++bits >= 8) {
			bits = 0;
			if (++pos >= sizeof data) {
				resetDecoder();
				return;
			}
		}
		state = OK;
	}
    // -------------------------------------
	// store a bit using Manchester encoding
    // -------------------------------------
	void manchester(char value) {
		flip ^= value; // manchester code, long pulse flips the bit
		gotBit(flip);
	}
    // -------------------------------------
	// move bits to the front so that all the bits are aligned to the end
    // -------------------------------------
	void alignTail(byte max = 0) {
		// align bits
		if (bits != 0) {
			data[pos] >>= 8 - bits;
			for (byte i = 0; i < pos; ++i)
				data[i] = (data[i] >> bits) | (data[i + 1] << (8 - bits));
			bits = 0;
		}
		// optionally shift bytes down if there are too many of 'em
		if (max > 0 && pos > max) {
			byte n = pos - max;
			pos = max;
			for (byte i = 0; i < pos; ++i)
				data[i] = data[i + n];
		}
	}
    // -------------------------------------
	void reverseBits() {
		for (byte i = 0; i < pos; ++i) {
			byte b = data[i];
			for (byte j = 0; j < 8; ++j) {
				data[i] = (data[i] << 1) | (b & 1);
				b >>= 1;
			}
		}
	}
    // -------------------------------------
	void reverseNibbles() {
		for (byte i = 0; i < pos; ++i)
			data[i] = (data[i] << 4) | (data[i] >> 4);
	}
    // -------------------------------------
	void done() {
		while (bits)
			gotBit(0); // padding
		state = DONE;
	}
};

// 868 MHz decoders

/// OOK decoder for Visonic devices.
class VisonicDecoder : public DecodeOOK {
public:
    VisonicDecoder () {}
    
    virtual char decode (word width) {
        if (200 <= width && width < 1000) {
            byte w = width >= 600;
            switch (state) {
                case UNKNOWN:
                case OK:
                    state = w == 0 ? T0 : T1;
                    break;
                case T0:
                    gotBit(!w);
                    if (w)
                        return 0;
                    break;
                case T1:
                    gotBit(!w);
                    if (!w)
                        return 0;
                    break;
            }
            // sync error, flip all the preceding bits to resync
            for (byte i = 0; i <= pos; ++i)
                data[i] ^= 0xFF; 
        } else if (width >= 2500 && 8 * pos + bits >= 36 && state == OK) {
            for (byte i = 0; i < 4; ++i)
                gotBit(0);
            alignTail(5); // keep last 40 bits
            // only report valid packets
            byte b = data[0] ^ data[1] ^ data[2] ^ data[3] ^ data[4];
            if ((b & 0xF) == (b >> 4))
                return 1;
        } else
            return -1;
        return 0;
    }
};

/// OOK decoder for FS20 type EM devices.
class EMxDecoder : public DecodeOOK {
public:
    EMxDecoder () : DecodeOOK (30) {} // ignore packets repeated within 3 sec
    
    // see also http://fhz4linux.info/tiki-index.php?page=EM+Protocol
    virtual char decode (word width) {
        if (200 <= width && width < 1000) {
            byte w = width >= 600;
            switch (state) {
                case UNKNOWN:
                    if (w == 0)
                        ++flip;
                    else if (flip > 20)
                        state = OK;
                    else
                        return -1;
                    break;
                case OK:
                    if (w == 0)
                        state = T0;
                    else
                        return -1;
                    break;
                case T0:
                    gotBit(w);
                    break;
            }
        } else if (width >= 1500 && pos >= 9)
            return 1;
        else
            return -1;
        return 0;
    }
};

/// OOK decoder for FS20 type KS devices.
class KSxDecoder : public DecodeOOK {
public:
    KSxDecoder () {}
    
    // see also http://www.dc3yc.homepage.t-online.de/protocol.htm
    virtual char decode (word width) {
        if (200 <= width && width < 1000) {
            byte w = width >= 600;
            switch (state) {
                case UNKNOWN:
                    gotBit(w);
                    bits = pos = 0;
                    if (data[0] != 0x95)
                        state = UNKNOWN;
                    break;
                case OK:
                    state = w == 0 ? T0 : T1;
                    break;
                case T0:
                    gotBit(1);
                    if (!w)
                        return -1;
                    break;
                case T1:
                    gotBit(0);
                    if (w)
                        return -1;
                    break;
            }
        } else if (width >= 1500 && pos >= 6) 
            return 1;
        else
            return -1;
        return 0;
    }
};

/// OOK decoder for FS20 type FS devices.
class FSxDecoder : public DecodeOOK {
public:
    FSxDecoder () {}
    
    // see also http://fhz4linux.info/tiki-index.php?page=FS20%20Protocol
    virtual char decode (word width) {
        if (300 <= width && width < 775) {
            byte w = width >= 500;
            switch (state) {
                case UNKNOWN:
                    if (w == 0)
                        ++flip;
                    else if (flip > 20)
                        state = T1;
                    else
                        return -1;
                    break;
                case OK:
                    state = w == 0 ? T0 : T1;
                    break;
                case T0:
                    gotBit(0);
                    if (w)
                        return -1;
                    break;
                case T1:
                    gotBit(1);
                    if (!w)
                        return -1;
                    break;
            }
        } else if (width >= 1500 && pos >= 5)
            return 1;
        else
            return -1;
        return 0;
    }
};

// =====================================================================================================
// =====================================================================================================


OregonDecoderV1 orscV1;
OregonDecoderV2 orscV2;
OregonDecoderV3 orscV3;

volatile word pulse;
// =====================================================================================================
// =====================================================================================================
byte osdata[13];               

void reportSerial(class DecodeOOK& decoder) {
	byte pos;
	const byte* data = decoder.getData(pos);
	for (byte i = 0; i < pos; ++i) {
        if (i < 13) osdata[i]=data[i];
	}
	decoder.resetDecoder();
}
// =====================================================================================================
// calculate a packet checksum by performing a 
byte checksum(byte type, int count, byte check) {
     byte calc=0;
     // type 1, add all nibbles, deduct 10
     if (type == 1) {
        for(byte i = 0; i<count;i++) {
           calc += (osdata[i]&0xF0) >> 4;
           calc += (osdata[i]&0xF);
        }
        calc=calc-10;
     } else 
     // type 2, add all nibbles up to count, add the 13th nibble , deduct 10
     if (type == 2) {
        for(byte i = 0; i<count;i++) {
           calc += (osdata[i]&0xF0) >> 4;
           calc += (osdata[i]&0xF);
        }
        calc += (osdata[6]&0xF);
        calc=calc-10;
     } else 
     // type 3, add all nibbles up to count, subtract 10 only use the low 4 bits for the compare
     if (type == 3) {
        for(byte i = 0; i<count;i++) {
           calc += (osdata[i]&0xF0) >> 4;
           calc += (osdata[i]&0xF);
        }
        calc=calc-10;
        calc=(calc&0x0f);
     } else 
     if (type == 4) {
        for(byte i = 0; i<count;i++) {
           calc += (osdata[i]&0xF0) >> 4;
           calc += (osdata[i]&0xF);
        }
        calc=calc-10;
        
     } 
     if (check == calc ) return 0;     
     return 1;
}
// =====================================================================================================
boolean Plugin_048(byte function, char *string) {
  boolean success=false;

#ifdef PLUGIN_048_CORE
      if ((RawSignal.Number < OSV3_PULSECOUNT_MIN) || (RawSignal.Number > OSV3_PULSECOUNT_MAX) ) return false; 

      byte basevar=0;
      byte rc=0;
      byte found = 0;
      byte channel = 0;
      
      int temp = 0;
      byte hum = 0;
      int comfort = 0;
      int baro = 0;
      int forecast = 0;
      int uv = 0;
      int wdir = 0;
      int wspeed = 0;
      int awspeed = 0;
      int rain = 0;
      int raintot = 0;

      word p = pulse;
      // ==================================================================================
      for (int x = 0; x < RawSignal.Number; x++) {
          p = RawSignal.Pulses[x]*RAWSIGNAL_SAMPLE_RATE;
          if (p != 0) {
             if (orscV1.nextPulse(p)) {
                reportSerial(orscV1);
                found=1; 
             } 
             if (orscV2.nextPulse(p)) { 
                reportSerial(orscV2);
                found=2;
             } 
             if (orscV3.nextPulse(p)) { 
                reportSerial(orscV3);
                found=3;
             } 
          }
      }
      if (found == 0) break;
      
      // ==================================================================================
      // Protocol and device info:
      // ==================================================================================
      //Serial.print("Oregon V");
      //Serial.print(found);
      //Serial.print(": ");
      //for(byte x=0; x<13;x++) {
      //    Serial.print( osdata[x],HEX ); 
      //    Serial.print((" ")); 
      //}
      //Serial.println();  
      //Serial.print("Oregon ID="); 
      unsigned int id=(osdata[0]<<8)+ (osdata[1]);
      rc=osdata[0];
      //Serial.println(id,HEX);  
      // ==================================================================================
      // Process the various device types:
      // ==================================================================================
      // Oregon V1 packet structure
      // SL-109H, AcuRite 09955
      // TEMP + CRC
      // ==================================================================================
      // 8487101C
      // 84+87+10=11B > 1B+1 = 1C
      if (found==1) {       // OSV1 
         int sum = osdata[0]+osdata[1]+osdata[2];  // max. value is 0x2FD
         sum= (sum &0xff) + (sum>>8);              // add overflow to low byte
         if (osdata[3] != (sum & 0xff) ) {
            //Serial.println("CRC Error"); 
            break;
         }
         // -------------       
         temp = ((osdata[2]>>4) * 100)  + ((osdata[1] & 0x0F) * 10) + ((osdata[1] >> 4));
         if ((osdata[2] & 0x02) == 2) temp=temp | 0x8000;  // bit 1 set when temp is negative, set highest bit on temp valua
        // ----------------------------------
        // Output
        // ----------------------------------
        sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
        Serial.print( pbuffer );
        // ----------------------------------
        Serial.print("OregonV1;");                       // Label
        sprintf(pbuffer, "ID=00%02x;", rc);               // ID    
        Serial.print( pbuffer );
        sprintf(pbuffer, "TEMP=%04x;", temp);     
        Serial.print( pbuffer );
        Serial.println();
      } 
      // ==================================================================================
      // ea4c  Outside (Water) Temperature: THC238, THC268, THN132N, THWR288A, THRN122N, THN122N, AW129, AW131
      // TEMP + BAT + CRC
      // ca48  Pool (Water) Temperature: THWR800
      // 0a4d  Indoor Temperature: THR128, THR138, THC138 
      // ==================================================================================
      // OSV2 EA4C20725C21D083 // THN132N
      // OSV2 EA4C101360193023 // THN132N
      // OSV2 EA4C40F85C21D0D4 // THN132N
      // OSV2 EA4C20809822D013
      //      0123456789012345
      //      0 1 2 3 4 5 6 7
      if(id == 0xea4c || id == 0xca48 || id == 0x0a4d) {
        byte sum=(osdata[7]&0x0f) <<4; 
        sum=sum+(osdata[6]>>4);
        if ( checksum(2,6, sum) !=0) {  // checksum = all nibbles 0-11+13 results is nibbles 15 <<4 + 12
            //Serial.println("CRC Error"); 
            break;
        }
        // -------------       
        temp = ((osdata[5]>>4) * 100)  + ((osdata[5] & 0x0F) * 10) + ((osdata[4] >> 4));
        if ((osdata[6] & 0x0F) >= 8) temp=temp | 0x8000;
        // ----------------------------------
        // Output
        // ----------------------------------
        sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
        Serial.print( pbuffer );
        // ----------------------------------
        Serial.print("Oregon Temp;");                    // Label
        sprintf(pbuffer, "ID=%02x%02x;", rc,osdata[2]);   // ID    
        Serial.print( pbuffer );
        sprintf(pbuffer, "TEMP=%04x;", temp);     
        Serial.print( pbuffer );
        if ((osdata[3] & 0x0F) >= 4) {
           Serial.print("BAT=LOW;"); 
        } else {        
           Serial.print("BAT=OK;"); 
        }        
        Serial.println();
      } else
      // ==================================================================================
      // 1a2d  Indoor Temp/Hygro: THGN122N, THGN123N, THGR122NX, THGR228N, THGR238, THGR268, THGR122X
      // 1a3d  Outside Temp/Hygro: THGR918, THGRN228NX, THGN500
      // fa28  Indoor Temp/Hygro: THGR810
      // *aac  Outside Temp/Hygro: RTGR328N
      // ca2c  Outside Temp/Hygro: THGR328N
      // fab8  Outside Temp/Hygro: WTGR800
      // TEMP + HUM sensor + BAT + CRC
      // ==================================================================================
      // OSV2 AACC13783419008250AD[RTGR328N,...] Id:78 ,Channel:0 ,temp:19.30 ,hum:20 ,bat:10
      // OSV2 1A2D40C4512170463EE6[THGR228N,...] Id:C4 ,Channel:3 ,temp:21.50 ,hum:67 ,bat:90
      // OSV2 1A2D1072512080E73F2C[THGR228N,...] Id:72 ,Channel:1 ,temp:20.50 ,hum:78 ,bat:90
      // OSV2 1A2D103742197005378E // THGR228N
      // OSV3 FA28A428202290834B46 // 
      // OSV2 1A2D1002 02060552A4C
      //      1A3D10D91C273083..
      //      1A3D10D90C284083..
      // OSV3 FA2814A93022304443BE // THGR810
      //      01234567890123456789
      //      0 1 2 3 4 5 
      // F+A+2+8+1+4+A+9+3+0+2+2+3+0+4+4=4d-a=43 
      if(id == 0xfa28 || id == 0x1a2d || id == 0x1a3d || (id&0xfff)==0xACC || id == 0xca2c || id == 0xfab8 ) {
        if ( checksum(1,8,osdata[8]) !=0) break;   // checksum = all nibbles 0-15 results is nibbles 16.17
        // -------------       
        temp = ((osdata[5]>>4) * 100)  + ((osdata[5] & 0x0F) * 10) + ((osdata[4] >> 4));
        if ((osdata[6] & 0x0F) >= 8) temp=temp | 0x8000;
        // -------------       
        hum = ((osdata[7] & 0x0F)*10)+ (osdata[6] >> 4);
        // ----------------------------------
        // Output
        // ----------------------------------
        sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
        Serial.print( pbuffer );
        // ----------------------------------
        Serial.print("Oregon TempHygro;");               // Label
        sprintf(pbuffer, "ID=%02x%02x;", rc,osdata[2]);   // ID    
        Serial.print( pbuffer );
        sprintf(pbuffer, "TEMP=%04x;", temp);     
        Serial.print( pbuffer );
        sprintf(pbuffer, "HUM=%02x;", hum);     
        Serial.print( pbuffer );
        if ((osdata[3] & 0x0F) >= 4) {
           Serial.print("BAT=LOW;"); 
        } else {        
           Serial.print("BAT=OK;"); 
        }        
        Serial.println();
      } else
      // ==================================================================================
      // 5a5d  Indoor Temp/Hygro/Baro: Huger - BTHR918
      // 5a6d  Indoor Temp/Hygro/Baro: BTHR918N, BTHR968. BTHG968 
      // TEMP + HUM + BARO + FORECAST + BAT
      // NO CRC YET
      // ==================================================================================
      // 5A 6D 00 7A 10 23 30 83 86 31
      // 5+a+6+d+7+a+1+2+3+3+8+3=47 -a=3d  +8=4f +8+6=55      
      // 5+a+6+d+7+a+1+2+3+3=3c-a=32
      // 0  1  2  3  4  5  6  7  8  9
      if(id == 0x5a6d || id == 0x5a5d || id == 0x5d60) {
        // -------------       
        temp = ((osdata[5]>>4) * 100)  + ((osdata[5] & 0x0F) * 10) + ((osdata[4] >> 4));
        if ((osdata[6] & 0x0F) >= 8) temp=temp | 0x8000;
        // -------------       
        hum = ((osdata[7] & 0x0F)*10)+ (osdata[6] >> 4);
        
        //0: normal, 4: comfortable, 8: dry, C: wet
        int tmp_comfort = osdata[7] >> 4;
        if (tmp_comfort == 0x00)
			comfort=0;
		else if (tmp_comfort == 0x04)
			comfort=1;
		else if (tmp_comfort == 0x08)
			comfort=2;
		else if (tmp_comfort == 0x0C)
			comfort=3;
			
        // -------------       
        baro = (osdata[8] + 856);  // max value = 1111 / 0x457
        
        //2: cloudy, 3: rainy, 6: partly cloudy, C: sunny
        int tmp_forecast = osdata[9]>>4;
        if (tmp_forecast == 0x02)
			forecast = 3;
        else if (tmp_forecast == 0x03)
			forecast = 4;
        else if (tmp_forecast == 0x06)
			forecast = 2;
        else if (tmp_forecast == 0x0C)
			forecast = 1;
        
        // ----------------------------------
        // Output
        // ----------------------------------
        sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
        Serial.print( pbuffer );
        // ----------------------------------
        Serial.print("Oregon BTHR;");                    // Label
        sprintf(pbuffer, "ID=%02x%02x;", rc,osdata[2]);   // ID    
        Serial.print( pbuffer );
        sprintf(pbuffer, "TEMP=%04x;", temp);     
        Serial.print( pbuffer );
        sprintf(pbuffer, "HUM=%02x;", hum);     
        Serial.print( pbuffer );
        sprintf(pbuffer, "HSTATUS=%d;", comfort);
        Serial.print( pbuffer );
        sprintf(pbuffer, "BARO=%04x;", baro);     
        Serial.print( pbuffer );
        sprintf(pbuffer, "BFORECAST=%d;", forecast);
        Serial.print( pbuffer );

		//below is not correct, and for now discarded
        //if (((osdata[3] & 0x0F) & 0x04) != 0) {
        //   Serial.print("BAT=LOW;"); 
        //} else {        
        //   Serial.print("BAT=OK;"); 
        //}        
        Serial.println();
      } else
      // ==================================================================================
      // 2914  Rain Gauge:
      // 2d10  Rain Gauge:
      // 2a1d  Rain Gauge: RGR126, RGR682, RGR918, PCR122
      // 2A1D0065502735102063 
      // 2+A+1+D+0+0+6+5+5+0+2+7+3+5+1+0+2+0=3e-a=34 != 63 
      // ==================================================================================
      if(id == 0x2a1d || id == 0x2d10 || id == 0x2914) { // Rain sensor
        //Checksum - add all nibbles from 0 to 8, subtract 9 and compare
        /*
        int cs = 0;
        for (byte i = 0; i < pos-2; ++i) { //all but last byte
            cs += data[i] >> 4;
            cs += data[i] & 0x0F;
        }
        int csc = (data[8] >> 4) + ((data[9] & 0x0F)*16);    
        cs -= 9;  //my version as A fails?
        Serial.print(csc); 
        Serial.print(" vs ");   
        Serial.println(cs);
        */
        rain = ((osdata[5]>>4) * 100)  + ((osdata[5] & 0x0F) * 10) + (osdata[4] >> 4);
        raintot = ((osdata[7]  >> 4) * 10)  + (osdata[6]>>4);
        // ----------------------------------
        // Output
        // ----------------------------------
        sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
        Serial.print( pbuffer );
        // ----------------------------------
        Serial.print("Oregon Rain;");                    // Label
        sprintf(pbuffer, "ID=%02x%02x;", rc,osdata[3]);   // ID    
        Serial.print( pbuffer );
        sprintf(pbuffer, "RAIN=%04x;", rain);     
        Serial.print( pbuffer );
        sprintf(pbuffer, "RAINTOT=%04x;", raintot);     
        Serial.print( pbuffer );
        if ((osdata[3] & 0x0F) >= 4) {
           Serial.print("BAT=LOW;"); 
        } else {        
           Serial.print("BAT=OK;"); 
        }        
        Serial.println();
      } else
      // ==================================================================================
      // 2a19  Rain Gauge: PCR800
      // RAIN + BAT + CRC
      // ==================================================================================
      // OSV3 2A19048E399393250010 
      //      01234567890123456789
      //      0 1 2 3 4 5 6 7 8 9
      // 2+A+1+9+0+4+8+E+3+9+9+3+9+3+2+5=5b-A=51 => 10 
      if(id == 0x2a19) { // Rain sensor
        int sum = (osdata[9] >> 4);  
        if ( checksum(3,9,sum) !=0) { // checksum = all nibbles 0-17 result is nibble 18
            //Serial.print("CRC Error, "); 
            break;
        }
        rain = ((osdata[5]>>4) * 100)  + ((osdata[5] & 0x0F) * 10) + (osdata[4] >> 4);
        //Serial.print(" RainTotal=");
        //raintot = ((osdata[7]  >> 4) * 10)  + (osdata[6]>>4);
        //Serial.print(raintot); 
        // ----------------------------------
        // Output
        // ----------------------------------
        sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
        Serial.print( pbuffer );
        // ----------------------------------
        Serial.print("Oregon Rain2;");                   // Label
        sprintf(pbuffer, "ID=%02x%02x;", rc,osdata[4]);   // ID    
        Serial.print( pbuffer );
        sprintf(pbuffer, "RAIN=%04x;", rain);     
        Serial.print( pbuffer );
        //sprintf(pbuffer, "RAINTOT=%04x;", raintot);     
        //Serial.print( pbuffer );
        if ((osdata[3] & 0x0F) >= 4) {
           Serial.print("BAT=LOW;"); 
        } else {        
           Serial.print("BAT=OK;"); 
        }        
        Serial.println();        
      } else
      // ==================================================================================
      // 1a89  Anemometer: WGR800
      // WIND DIR + SPEED + AV SPEED + CRC
      // ==================================================================================
      // OSV3 1A89048800C026400543
      // OSV3 1A89048800C00431103B
      // OSV3 1a89048848c00000003e W
      // OSV3 1a890488c0c00000003e E     
      //      1A89042CB0C047000147      
      //      0 1 2 3 4 5 6 7 8 9 
      // 1+A+8+9+0+4+8+8+0+0+C+0+0+4+3+1+1+0=45-a=3b
      if(id == 0x1a89) { // Wind sensor
        if ( checksum(1,9,osdata[9]) !=0) break;
        Serial.print(" WINDIR=");
        wdir=(osdata[4] >> 4);
        wdir=wdir*225;
        wdir=wdir/10;
        Serial.print(wdir); 
        // -------------       
        wspeed = (osdata[6] >> 4) * 10;
        wspeed = wspeed + (osdata[6] &0x0f) * 100;
        wspeed = wspeed + (osdata[5] &0x0f);
        // -------------       
        awspeed = (osdata[8] >> 4) * 100;
        awspeed = awspeed + (osdata[7] &0x0f) * 10;
        awspeed = awspeed + (osdata[7] >> 4);
        // ----------------------------------
        // Output
        // ----------------------------------
        sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
        Serial.print( pbuffer );
        // ----------------------------------
        Serial.print("Oregon Wind;");                    // Label
        sprintf(pbuffer, "ID=%02x%02x;", rc,osdata[2]);   // ID    
        Serial.print( pbuffer );
        sprintf(pbuffer, "WINDIR=%04x;", wdir);     
        Serial.print( pbuffer );
        sprintf(pbuffer, "WINSP=%04x;", wspeed);     
        Serial.print( pbuffer );
        sprintf(pbuffer, "AWINSP=%04x;", awspeed);     
        Serial.print( pbuffer );
        if ((osdata[3] & 0x0F) >= 4) {
           Serial.print("BAT=LOW;"); 
        } else {        
           Serial.print("BAT=OK;"); 
        }        
        Serial.println();                
      } else
      // ==================================================================================
      // 3a0d  Anemometer: Huger-STR918, WGR918
      // 1984  Anemometer:
      // 1994  Anemometer:
      // WIND DIR + SPEED + AV SPEED + BAT + CRC
      // 3A0D006F400800000031
      // ==================================================================================
      if(id == 0x3A0D || id == 0x1984 || id == 0x1994 ) {
        if ( checksum(1,9,osdata[9]) !=0) {
            Serial.print("CRC Error, "); 
            //break;
        }
        wdir = ((osdata[5]>>4) * 100)  + ((osdata[5] & 0x0F * 10) ) + (osdata[4] >> 4);    
        wspeed = ((osdata[7] & 0x0F) * 100)  + ((osdata[6]>>4) * 10)  + ((osdata[6] & 0x0F)) ;
        awspeed = ((osdata[8]>>4) * 100)  + ((osdata[8] & 0x0F) * 10)+((osdata[7] >>4)) ;      
        // ----------------------------------
        // Output
        // ----------------------------------
        sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
        Serial.print( pbuffer );
        // ----------------------------------
        Serial.print("Oregon Wind2;");                   // Label
        sprintf(pbuffer, "ID=%02x%02x;", rc,osdata[2]);   // ID    
        Serial.print( pbuffer );
        sprintf(pbuffer, "WINDIR=%04x;", wdir);     
        Serial.print( pbuffer );
        sprintf(pbuffer, "WINSP=%04x;", wspeed);     
        Serial.print( pbuffer );
        sprintf(pbuffer, "AWINSP=%04x;", awspeed);     
        Serial.print( pbuffer );
        if ((osdata[3] & 0x0F) >= 4) {
           Serial.print("BAT=LOW;"); 
        } else {        
           Serial.print("BAT=OK;"); 
        }        
        Serial.println();                      
	  } else
      // ==================================================================================
      // ea7c  UV Sensor: UVN128, UV138
      // UV + BAT
      // NO CRC YET
      // ==================================================================================
      if(id == 0xea7c) { 
        uv=((osdata[5] & 0x0F) * 10)  + (osdata[4] >> 4);
        Serial.print(uv); 
        // ----------------------------------
        // Output
        // ----------------------------------
        sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
        Serial.print( pbuffer );
        // ----------------------------------
        Serial.print("Oregon UVN128/138;");              // Label
        sprintf(pbuffer, "ID=%02x%02x;", rc,osdata[2]);   // ID    
        Serial.print( pbuffer );
        sprintf(pbuffer, "UV=%04x;", uv);     
        Serial.print( pbuffer );
        if ((osdata[3] & 0x0F) >= 4) {
           Serial.print("BAT=LOW;"); 
        } else {        
           Serial.print("BAT=OK;"); 
        }        
        Serial.println();                
      } else
      // ==================================================================================
      // da78  UV Sensor: UVN800
      // UV 
      // NO CRC YET
      // ==================================================================================
      if( id == 0xda78) { 
        uv=(osdata[6] & 0xf0) + (osdata[5] &0x0f) ;
        // ----------------------------------
        // Output
        // ----------------------------------
        sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
        Serial.print( pbuffer );
        // ----------------------------------
        Serial.print("Oregon UVN800;");                  // Label
        sprintf(pbuffer, "ID=%02x%02x;", rc,osdata[2]);   // ID    
        Serial.print( pbuffer );
        sprintf(pbuffer, "UV=%04x;", uv);     
        Serial.print( pbuffer );
        if ((osdata[3] & 0x0F) >= 4) {
           Serial.print("BAT=LOW;"); 
        } else {        
           Serial.print("BAT=OK;"); 
        }        
        Serial.println();     
      } else
      // ==================================================================================
      // *aec  Date&Time: RTGR328N
      // NO CRC YET
      // ==================================================================================
      // 8AEA1378077214924242C16CBD  21:49 29/04/2014 
      // 0 1 2 3 4 5 6 7 8 9 0 1 2
      // 8+A+E+A+1+3+7+8+0+7+7+2+1+4+9+2+4+2+4+2+C+1+6+C=88 !=   BD
      // Date & Time
      //if( (id &0xfff) == 0xaea) { 
      //  Serial.print("RTGR328N RC: ");
      //  Serial.print(osdata[3], HEX);
      //  Serial.print(" Date: ");
      //  Serial.print( (osdata[9] >> 4) + 10 * (osdata[10] & 0xf) );
      //  Serial.print(" - ");
      //  Serial.print( osdata[8] >> 4 );
      //  Serial.print(" - ");
      //  Serial.print( (osdata[7] >> 4) + 10 * (osdata[8] & 0xf) );
      //  Serial.print(" Time: ");
      //  Serial.print( (osdata[6] >> 4) + 10 * (osdata[7] & 0xf) );
      //  Serial.print(":");
      //  Serial.print((osdata[5] >> 4) + 10 * (osdata[6] & 0xf));
      //  Serial.print(":");
      //  Serial.print((osdata[4] >> 4) + 10 * (osdata[5] & 0xf));
      //  Serial.println();
      //  break;
      //}  
      // ==================================================================================
      // eac0  Ampere meter: cent-a-meter, OWL CM113, Electrisave
      // ==================================================================================
      if(id == 0xeac0) { 
        //Serial.println("UNKNOWN LAYOUT");
        // ----------------------------------
        // Output
        // ----------------------------------
        sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
        Serial.print( pbuffer );
        // ----------------------------------
        Serial.print("Oregon Unknown;");                   // Label
        for(byte x=0; x<13;x++) {
           Serial.print( osdata[x],HEX ); 
           Serial.print((" ")); 
        }
        Serial.println(";");  
        return success;
      } else  
      // ==================================================================================
      // 0x1a* / 0x2a* 0x3a** Power meter: OWL CM119
      // ==================================================================================
      // ==================================================================================
      // 1a99  Anemometer: WGTR800
      // WIND + TEMP + HUM + CRC
      // ==================================================================================
      if(id == 0x1a99) { // Wind sensor
        //Serial.println("UNKNOWN LAYOUT");
        // ----------------------------------
        // Output
        // ----------------------------------
        sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
        Serial.print( pbuffer );
        // ----------------------------------
        Serial.print("Oregon Unknown;");                   // Label
        for(byte x=0; x<13;x++) {
           Serial.print( osdata[x],HEX ); 
           Serial.print((" ")); 
        }
        Serial.println(";");  
        return success;
      } else    
      // ==================================================================================
      if( (id&0xf00)==0xA00 ) { // Wind sensor
        // ----------------------------------
        // Output
        // ----------------------------------
        sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
        Serial.print( pbuffer );
        // ----------------------------------
        Serial.print("Oregon Unknown;");                   // Label
        for(byte x=0; x<13;x++) {
           Serial.print( osdata[x],HEX ); 
           Serial.print((" ")); 
        }
        Serial.println(";");  
      }
      // ==================================================================================
      RawSignal.Repeats=true;                    // suppress repeats of the same RF packet 
      RawSignal.Number=0;
      success = true;
#endif // PLUGIN_048_CORE
  return success;
}
