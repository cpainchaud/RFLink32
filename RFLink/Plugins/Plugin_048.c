//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                    Plugin-48: Oregon V1/2/3                                       ##
//#######################################################################################################
/*********************************************************************************************\
 * This protocol takes care of receiving Oregon Scientific outdoor sensors that use the V1, V2 and V3 protocol
 *
 * models: THC238, THC268, THN132N, THWR288A, THRN122N, THN122N, AW129, AW131, THGR268, THGR122X,
 *         THGN122N, THGN123N, THGR122NX, THGR228N, THGR238, WTGR800, THGR918, THGRN228NX, THGN500,
 *         THGR810, RTGR328N, THGR328N, Huger BTHR918, BTHR918N, BTHR968, RGR126, RGR682, RGR918, PCR122
 *         THWR800, THR128, THR138, THC138, OWL CM119, cent-a-meter, OWL CM113, Electrisave, RGR928 
 *         UVN128, UV138, UVN800, Huger-STR918, WGR918, WGR800, PCR800, WGTR800, RGR126, BTHG968
 *
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (update)   : Sebastien LANGE
 * Support (update)   : https://github.com/basty149/RFLink 
 * Author  (original) : StuntTeam 2015..2016
 * Support (original) : http://sourceforge.net/projects/rflink/
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
#define OSV3_PLUGIN_ID 048
#define PLUGIN_DESC_048 "Oregon V1/2/3"

#define OSV3_PULSECOUNT_MIN 50  // 126
#define OSV3_PULSECOUNT_MAX RAW_BUFFER_SIZE

#ifdef PLUGIN_048
#include "../4_Display.h"

#define OSDATA_SIZE 13

#ifdef PLUGIN_048_DEBUG_OSDATA
#define DISPLAY_DEBUG(a,b) display_DEBUG(a,b)
#else
#define DISPLAY_DEBUG(a,b)
#endif

// Oregon Sciencefic device id from g00gle
#define ID_THGR122N 		0x1d20
#define ID_THGR968  		0x1d30
#define ID_BTHR918  		0x5d50
#define ID_BHTR968  		0x5d60
#define ID_RGR968   		0x2d10
#define ID_RTGN318  		0x0cc3 // warning: id is from 0x0cc3 and 0xfcc3
#define ID_RTGN129  		0x0cc3 // same as RTGN318 but different packet size
#define ID_THGR810  		0xf824 // This might be ID_THGR81, but what's true is lost in (git) history
#define ID_THGR810a 		0xf8b4 // unconfirmed version
#define ID_THN802   		0xc844
#define ID_PCR800   		0x2914
#define ID_PCR800a  		0x2d14 // Different PCR800 ID - AU version I think
#define ID_WGR800   		0x1984
#define ID_WGR800a  		0x1994 // unconfirmed version
#define ID_WGR968   		0x3d00
#define ID_UV800   			0xd874
#define ID_THN129   		0xcc43 // THN129 Temp only
#define ID_RTHN129  		0x0cd3 // RTHN129 Temp, clock sensors
#define ID_BTHGN129 		0x5d53 // Baro, Temp, Hygro sensor
#define ID_UVR128   		0xec70
#define ID_THGR328N   		0xcc23  // Temp & Hygro sensor looks similar to THR228N but with 5 choice channel instead of 3
#define ID_RTGR328N_1		0xdcc3  // RTGR328N_[1-5] RFclock (date &time) & Temp & Hygro sensor looks similar to THGR328N with RF clock (5 channels also) : Temp & hygro part
#define ID_RTGR328N_2 		0xccc3
#define ID_RTGR328N_3 		0xbcc3
#define ID_RTGR328N_4		0xacc3
#define ID_RTGR328N_5 		0x9cc3
#define ID_RTGR328N_6 		0x8ce3  // RTGR328N_6&7 RFclock (date &time) & Temp & Hygro sensor looks similar to THGR328N with RF clock (5 channels also) : RF Time part
#define ID_RTGR328N_7 		0x8ae3

 // Outdoor/Water Temperature Sensors: ;TEMP;BAT;
#define ID_THN132N  		0xea4c	// same as THR228N but different packet size
#define ID_THC238			0xea4c
#define ID_THC268			0xea4c
#define ID_THWR288A			0xea4c
#define ID_THRN122N			0xea4c
#define ID_THN122N			0xea4c
#define ID_AW129			0xea4c
#define ID_AW131			0xea4c
#define ID_THR228N  		0xec40

// Pool (Water) Temperature: ;TEMP;BAT;
#define ID_THWR800			0xca48		

// Indoor Temperature: ;TEMP;BAT;
#define ID_THR128			0x0a4d
#define ID_THR138			0x0a4d
#define ID_THC138			0x0a4d

/*
 * Many devices use 160 bits, known exceptions:
 * 0xEA4c         136 bits  // TH132N
 * 0xEA7c         240 bits  // UV138
 * 0x5A5D / 1A99  176 bits  // THGR918 / WTGR800
 * 0x5A6D         192 bits  // BTHR918N
 * 0x8AEC         192 bits  // RTGR328N
 * 0x9AEC         208 bits  // RTGR328N
 * 0xDA78         144 bits  // UVN800
 * 0x2A19         184 bits  // RCR800
 * 0x2A1d         168 bits  // WGR918
 */
// =====================================================================================================
class DecodeOOK
{
protected:
   byte total_bits, bits, flip, state, pos, data[25];
   virtual signed char decode(word width) = 0;

public:
   enum
   {
      UNKNOWN,
      T0,
      T1,
      T2,
      T3,
      OK,
      DONE
   };
   // -------------------------------------
   DecodeOOK() { resetDecoder(); }
   // -------------------------------------
   bool nextPulse(word width)
   {
      if (state != DONE){
         switch (decode(width))
         {
         case -1:
            resetDecoder();
            break;
         case 1:
            done();
            break;
         }
      }

      return isDone();
   }
   // -------------------------------------
   bool isDone() const { return state == DONE; }
   // -------------------------------------
   const byte *getData(byte &count) const
   {
      count = pos;
      return data;
   }
   // -------------------------------------
   void resetDecoder()
   {
      total_bits = bits = pos = flip = 0;
      state = UNKNOWN;
   }
   // -------------------------------------
   // add one bit to the packet data buffer
   // -------------------------------------
   virtual void gotBit(char value)
   {
      total_bits++;
      byte *ptr = data + pos;
      *ptr = (*ptr >> 1) | (value << 7);

      if (++bits >= 8)
      {
         bits = 0;
         if (++pos >= sizeof data)
         {
            resetDecoder();
            return;
         }
      }
      state = OK;
   }
   // -------------------------------------
   // store a bit using Manchester encoding
   // -------------------------------------
   void manchester(char value)
   {
      flip ^= value; // manchester code, long pulse flips the bit
      gotBit(flip);
   }
   // -------------------------------------
   // move bits to the front so that all the bits are aligned to the end
   // -------------------------------------
   void alignTail(byte max = 0)
   {
      // align bits
      if (bits != 0)
      {
         data[pos] >>= 8 - bits;
         for (byte i = 0; i < pos; ++i)
            data[i] = (data[i] >> bits) | (data[i + 1] << (8 - bits));
         bits = 0;
      }
      // optionally shift bytes down if there are too many of 'em
      if (max > 0 && pos > max)
      {
         byte n = pos - max;
         pos = max;
         for (byte i = 0; i < pos; ++i)
            data[i] = data[i + n];
      }
   }
   // -------------------------------------
   void reverseBits()
   {
      for (byte i = 0; i < pos; ++i)
      {
         byte b = data[i];
         for (byte j = 0; j < 8; ++j)
         {
            data[i] = (data[i] << 1) | (b & 1);
            b >>= 1;
         }
      }
   }
   // -------------------------------------
   void reverseNibbles()
   {
      for (byte i = 0; i < pos; ++i)
         data[i] = (data[i] << 4) | (data[i] >> 4);
   }
   // -------------------------------------
   void done()
   {
      while (bits)
         gotBit(0); // padding
      state = DONE;
   }
};

class OregonDecoderV1 : public DecodeOOK
{
public:
   OregonDecoderV1() {}
   virtual signed char decode(word width)
   {
      if (900 <= width && width < 3400)
      {
         byte w = width >= 2000;
         switch (state)
         {
         case UNKNOWN: // Detect preamble
            if (w == 0)
               ++flip;
            else
               return -1;
            break;
         case OK:
            if (w == 0)
               state = T0;
            else
               manchester(1);
            break;
         case T0:
            if (w == 0)
               manchester(0);
            else
               return -1;
            break;
         default: // Unexpected state
            return -1;
         }
         return (pos == 4) ? 1 : 0; // Messages are fixed-size
      }
      if (width >= 3400)
      {
         if (flip < 10 || flip > 50)
            return -1; // No preamble
         switch (state)
         {
         case UNKNOWN:
            // First sync pulse, lowering edge
            state = T1;
            break;
         case T1:
            // Second sync pulse, lowering edge
            state = T2;
            break;
         case T2:
            // Last sync pulse, determines the first bit!
            if (width <= 5900)
            {
               state = T0;
               flip = 1;
            }
            else
            {
               state = OK;
               flip = 0;
               manchester(0);
            }
            break;
         }
         return 0;
      }
      return -1;
   }
};

class OregonDecoderV2 : public DecodeOOK
{
public:
   OregonDecoderV2() {}

   // -------------------------------------
   // add one bit to the packet data buffer
   // -------------------------------------
   virtual void gotBit(char value)
   {
      if (!(total_bits & 0x01))
      {
         data[pos] = (data[pos] >> 1) | (value ? 0x80 : 00);
      }
      total_bits++;
      pos = total_bits >> 4;
      if (pos >= sizeof data)
      {
         resetDecoder();
         return;
      }
      state = OK;
   }
   // -------------------------------------
   virtual signed char decode(word width)
   {
      //if (200 <= width && width < 1200) {
      //	byte w = width >= 675;
      if (150 <= width && width < 1200)
      {
         byte w = width >= 600;
         switch (state)
         {
         case UNKNOWN:
            if (w != 0)
            {
               // Long pulse
               ++flip;
            }
            else if (w == 0 && 24 <= flip)
            {
               // Short pulse, start bit
               flip = 0;
               state = T0;
            }
            else
            {
               // Reset decoder
               return -1;
            }
            break;
         case OK:
            if (w == 0)
            {
               // Short pulse
               state = T0;
            }
            else
            {
               // Long pulse
               manchester(1);
            }
            break;
         case T0:
            if (w == 0)
            {
               // Second short pulse
               manchester(0);
            }
            else
            {
               // Reset decoder
               return -1;
            }
            break;
         }
      }
      else if (width >= 2500 && pos >= 8)
      {
         return 1;
      }
      else
      {
         return -1;
      }
      return total_bits == 160 ? 1 : 0;
   }
};

class OregonDecoderV3 : public DecodeOOK
{
public:
   OregonDecoderV3() {}
   // -------------------------------------
   // add one bit to the packet data buffer
   // -------------------------------------
   virtual void gotBit(char value)
   {
      data[pos] = (data[pos] >> 1) | (value ? 0x80 : 00);
      total_bits++;
      pos = total_bits >> 3;
      if (pos >= sizeof data)
      {
         resetDecoder();
         return;
      }
      state = OK;
   }
   // -------------------------------------
   virtual signed char decode(word width)
   {
      if (200 <= width && width < 1200)
      {
         byte w = width >= 675;
         switch (state)
         {
         case UNKNOWN:
            if (w == 0)
               ++flip;
            else if (32 <= flip)
            {
               flip = 1;
               manchester(1);
            }
            else
               return -1;
            break;
         case OK:
            if (w == 0)
               state = T0;
            else
               manchester(1);
            break;
         case T0:
            if (w == 0)
               manchester(0);
            else
               return -1;
            break;
         }
      }
      else
      {
         return -1;
      }
      return total_bits == 80 ? 1 : 0;
   }
};

OregonDecoderV1 orscV1;
OregonDecoderV2 orscV2;
OregonDecoderV3 orscV3;

volatile word pulse;
byte osdata[OSDATA_SIZE];

void reportSerial(class DecodeOOK &decoder)
{
	byte pos;
	const byte *data = decoder.getData(pos);
	for (byte i = 0; i < pos; ++i) {
		if (i < OSDATA_SIZE)
			osdata[i] = data[i];
	}
	decoder.resetDecoder();
}
// =====================================================================================================
// calculate a packet checksum by performing a
byte checksum(byte type, int count, byte check)
{
   byte calc = 0;
   // type 1, add all nibbles, deduct 10
   if (type == 1)
   {
      for (byte i = 0; i < count; i++)
      {
         calc += (osdata[i] & 0xF0) >> 4;
         calc += (osdata[i] & 0xF);
      }
      calc = calc - 10;
   }
   else
       // type 2, add all nibbles up to count, add the 13th nibble , deduct 10
       if (type == 2)
   {
      for (byte i = 0; i < count; i++)
      {
         calc += (osdata[i] & 0xF0) >> 4;
         calc += (osdata[i] & 0xF);
      }
      calc += (osdata[6] & 0xF);
      calc = calc - 10;
   }
   else
       // type 3, add all nibbles up to count, subtract 10 only use the low 4 bits for the compare
       if (type == 3)
   {
      for (byte i = 0; i < count; i++)
      {
         calc += (osdata[i] & 0xF0) >> 4;
         calc += (osdata[i] & 0xF);
      }
      calc = calc - 10;
      calc = (calc & 0x0f);
   }
   else if (type == 4)
   {
      for (byte i = 0; i < count; i++)
      {
         calc += (osdata[i] & 0xF0) >> 4;
         calc += (osdata[i] & 0xF);
      }
      calc = calc - 10;
   }
   if (check == calc)
      return 0;
   return 1;
}

int oregon_getTemperature() {
	int _t = ((osdata[5] >> 4) * 100) + ((osdata[5] & 0x0F) * 10) + ((osdata[4] >> 4) & 0x0f);
	if ((osdata[6] & 0x0F) >= 8) _t = _t | 0x8000;
	return _t; /* 8312 => -21.3  | 0281 => 18.2 */
}

unsigned long oregon_getRollingCodeAndChannel() {
	return (((osdata[2] & 0x0F) + (osdata[3] & 0xF0)) << 8) | (osdata[2] >> 4); // A004 => RC A0, CH = 04 (1=1,2=2,4=3)
}

boolean oregon_getBatteryOk() {
	return !((osdata[3] & 0x08) == 0x08); // 1000b
}

// =====================================================================================================
boolean Plugin_048(byte function, const char *string) {
	if ((RawSignal.Number < OSV3_PULSECOUNT_MIN) || (RawSignal.Number > OSV3_PULSECOUNT_MAX)) return false;

	byte proto = 0;
#ifdef PLUGIN_048_DEBUG_STANDALONE   
	if (string && strlen(string) == 8) proto = 1;
#else
	memset(osdata, 0, OSDATA_SIZE);
	word p = pulse;
	
	for (int x = 1; x < RawSignal.Number; ++x) {
		p = RawSignal.Pulses[x] * RawSignal.Multiply;
		if (p != 0) {
			if (orscV1.nextPulse(p)) {
				reportSerial(orscV1);
				proto = 1;
			}
			if (orscV2.nextPulse(p)) {
				reportSerial(orscV2);
				proto = 2;
			}
			if (orscV3.nextPulse(p)) {
				reportSerial(orscV3);
				proto = 3;
			}
		}
	}	
	if (0 == proto)  return false;	
#endif
	
	byte rc = 0;
	unsigned int id = 1 == proto ? 0x0001 : (osdata[0] << 8) + (osdata[1]); // ID=XXXX;
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

#ifdef PLUGIN_048_DEBUG_RAWSIGNAL
	//debugRawSignal(RawSignal.Number);
	Serial.print("XX;XX;Oregon=");
	Serial.print(id, HEX);
	Serial.print(";PROTO=");
	Serial.print(proto);
	Serial.print(";DEBUG=");
	for(byte x=0; x<OSDATA_SIZE;x++)
		Serial.print(osdata[x], HEX);
	Serial.println();
#endif
	
	if (id == ID_THR228N || id == ID_THN132N || id == ID_THWR800 || id == ID_THR128) {
		/*
		   FIXED: ID_THN132N:20230131
		   ==================================================================================
		   ea4c  Outdoor (Water) Temperature: THC238, THC268, THN132N, THWR288A, THRN122N, THN122N, AW129, AW131
		   0a4d  Indoor Temperature: THR128, THR138, THC138
		   ca48  Pool (Water) Temperature: THWR800 0xca48		   
		   TEMP + BAT + CRC
		   ==================================================================================
		   OSV2 EA4C20725C21D083 // THN132N
		   OSV2 EA4C101360193023 // THN132N
		   OSV2 EA4C40F85C21D0D4 // THN132N
		   OSV2 EA4C20809822D013
				0123456789012345 checksum = all nibbles 0-11+13 results is nibbles 15 <<4 + 12
				0 1 2 3 4 5 6 7		
	   */

	   byte sum = (osdata[7] & 0x0f) << 4;
	   sum = sum + (osdata[6] >> 4);
	   
	   if (checksum(2, 6, sum) != 0) return false;

		display_Header();
		display_NameEx(PSTR("Oregon"), id);
		display_IDn(oregon_getRollingCodeAndChannel(), 4);
		display_TEMP(oregon_getTemperature());
		display_BAT(oregon_getBatteryOk());
		display_DEBUG(osdata, OSDATA_SIZE);
		display_Footer();
	}
	else if (id == 0xfa28 || id == 0x1a2d || id == 0x1a3d || (id & 0xfff) == 0xACC || id == 0xca2c || id == 0xfab8) {
	   /*
		   1a2d  Indoor Temp/Hygro: THGN122N, THGN123N, THGR122NX, THGR228N, THGR238, THGR268, THGR122X
		   1a3d  Outdoor Temp/Hygro: THGR918, THGRN228NX, THGN500
		   fa28  Indoor Temp/Hygro: THGR810
		   *aac  Outdoor Temp/Hygro: RTGR328N
		   ca2c  Outdoor Temp/Hygro: THGR328N
		   fab8  Outdoor Temp/Hygro: WTGR800
		   TEMP + HUM + BAT + CRC
		   ==================================================================================
		   OSV2 AACC13783419008250AD RTGR328N, ... Id:78 ,Channel:0 ,temp:19.30 ,hum:20 ,bat:10
		   OSV2 1A2D40C4512170463EE6 THGR228N, ... Id:C4 ,Channel:3 ,temp:21.50 ,hum:67 ,bat:90
		   OSV2 1A2D1072512080E73F2C THGR228N, ... Id:72 ,Channel:1 ,temp:20.50 ,hum:78 ,bat:90
		   OSV2 1A2D103742197005378E THGR228N
		   OSV3 FA28A428202290834B46
		   OSV3 FA2814A93022304443BE THGR810
		   OSV2 1A2D1002 02060552A4C
				1A3D10D91C273083..
				1A3D10D90C284083..
				01234567890123456789
				0 1 2 3 4 5
		   F+A+2+8+1+4+A+9+3+0+2+2+3+0+4+4=4d-a=43
	   */

		if (checksum(1, 8, osdata[8]) != 0) return false; // checksum = all nibbles 0-15 results is nibbles 16.17			 

		temp = oregon_getTemperature();
		hum = ((osdata[7] & 0x0F) * 16) + (osdata[6] >> 4);

		display_Header();
		display_NameEx(PSTR("Oregon"), id);
		display_IDn((osdata[1] << 8 | osdata[3]), 4);
		display_TEMP(temp);
		display_HUM(hum);
		boolean batOK = !((osdata[4] & 0x0F) >= 4);
		display_BAT(batOK);
		display_Footer();
	}
	else if (id == 0x5a6d || id == 0x5a5d || id == ID_BHTR968) {
	/*
		5a5d  Indoor Temp/Hygro/Baro: Huger - BTHR918
		5a6d  Indoor Temp/Hygro/Baro: BTHR918N, BTHR968. BTHG968
		TEMP + HUM + BARO + FORECAST + BAT
		NO CRC YET
		==================================================================================
		5A 6D 00 7A 10 23 30 83 86 31
		5+a+6+d+7+a+1+2+3+3+8+3=47 -a=3d  +8=4f +8+6=55
		5+a+6+d+7+a+1+2+3+3=3c-a=32
		5+a+6+d+7+a+1+2+3+3+0+8+3+8+6=55 -a =4b +3=4e !=1
		0  1  2  3  4  5  6  7  8  9
	*/

		temp = ((osdata[5] >> 4) * 100) + ((osdata[5] & 0x0F) * 10) + ((osdata[4] >> 4));
		if ((osdata[6] & 0x0F) >= 8) temp = temp | 0x8000;
		hum = ((osdata[7] & 0x0F) * 10) + (osdata[6] >> 4);

		// 0: normal, 4: comfortable, 8: dry, C: wet
		int tmp_comfort = osdata[7] >> 4;
		if (tmp_comfort == 0x00) comfort = 0;
		else if (tmp_comfort == 0x04) comfort = 1;
		else if (tmp_comfort == 0x08) comfort = 2;
		else if (tmp_comfort == 0x0C) comfort = 3;

		baro = (osdata[8] + 856); // max value = 1111 / 0x457

		// 2: cloudy, 3: rainy, 6: partly cloudy, C: sunny
		int tmp_forecast = osdata[9] >> 4;
		if (tmp_forecast == 0x02) forecast = 3;
		else if (tmp_forecast == 0x03) forecast = 4;
		else if (tmp_forecast == 0x06) forecast = 2;
		else if (tmp_forecast == 0x0C) forecast = 1;

		display_Header();
		display_NameEx(PSTR("Oregon"), id);
		display_IDn((rc << 8 | osdata[2]), 4);
		display_TEMP(temp);
		display_HUM(hum);
		display_HSTATUS(comfort);
		display_BARO(baro);
		display_BFORECAST(forecast);
	/*
		below is not correct, and for now discarded
		if (((osdata[3] & 0x0F) & 0x04) != 0) {
			Serial.print("BAT=LOW;");
		} else {
			Serial.print("BAT=OK;");
		}
		boolean batOK=!((osdata[4] & 0x0F) >= 4);
		display_BAT(batOK);
	*/
		display_Footer();
	}
	else if (id == 0x2a1d || id == ID_RGR968 || id == ID_PCR800){
	/*
		2914  Rain Gauge:
		2d10  Rain Gauge:
		2a1d  Rain Gauge: RGR126, RGR682, RGR918, RGR928, PCR122
		==================================================================================
		2A1D0065502735102063
		2+A+1+D+0+0+6+5+5+0+2+7+3+5+1+0+2+0=3e-a=34 != 63
		2+A+1+D+0+0+6+5+5+0+2+7+3+5+1+0+2+0+6=44-a=3A
	*/

	/*
		// Checksum - add all nibbles from 0 to 8, subtract 9 and compare
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

		rain = ((osdata[5] >> 4) * 100) + ((osdata[5] & 0x0F) * 10) + (osdata[4] >> 4);
		raintot = ((osdata[7] >> 4) * 10) + (osdata[6] >> 4);

		display_Header();
		display_NameEx(PSTR("Oregon"), id);
		display_IDn((rc << 8 | osdata[3]), 4);
		display_RAIN(rain);
		display_RAINTOT(raintot);
		boolean batOK = !((osdata[3] & 0x0F) >= 4);
		display_BAT(batOK);
		display_Footer();
   }
   else if (id == 0x2a19) {
	   /*
		   2a19  Rain Gauge: PCR800
		   RAIN + BAT + CRC
		   ==================================================================================
		   OSV3 2A19048E399393250010
				01234567890123456789
				0 1 2 3 4 5 6 7 8 9
		   2+A+1+9+0+4+8+E+3+9+9+3+9+3+2+5=5b-A=51 => 10
	   */

		int sum = (osdata[9] >> 4);
		if (checksum(3, 9, sum) != 0) {
			// checksum = all nibbles 0-17 result is nibble 18
			return false;
		}

		rain = ((osdata[5] >> 4) * 100) + ((osdata[5] & 0x0F) * 10) + (osdata[4] >> 4);
		//Serial.print(" RainTotal=");
		//raintot = ((osdata[7]  >> 4) * 10)  + (osdata[6]>>4);
		//Serial.print(raintot);

		display_Header();
		display_NameEx(PSTR("Oregon"), id);
		display_IDn((rc << 8 | osdata[3]), 4);
		display_RAIN(rain);
		display_RAINTOT(raintot);
		boolean batOK = !((osdata[3] & 0x0F) >= 4);
		display_BAT(batOK);
		display_Footer();
	}
	else if (id == 0x1a89) {
		/*
			1a89  Anemometer: WGR800
			WIND DIR + SPEED + AV SPEED + CRC
			==================================================================================
			OSV3 1A89048800C026400543
			OSV3 1A89048800C00431103B
			OSV3 1a89048848c00000003e W
			OSV3 1a890488c0c00000003e E
				1A89042CB0C047000147
				0 1 2 3 4 5 6 7 8 9
			1+A+8+9+0+4+8+8+0+0+C+0+0+4+3+1+1+0=45-a=3b
		*/
		if (checksum(1, 9, osdata[9]) != 0)return false;

		wdir = ((osdata[4] >> 4) & 0x0f);
		wspeed = (osdata[6] >> 4) * 10;
		wspeed = wspeed + (osdata[6] & 0x0f) * 100;
		wspeed = wspeed + (osdata[5] & 0x0f);
		awspeed = (osdata[8] >> 4) * 100;
		awspeed = awspeed + (osdata[7] & 0x0f) * 10;
		awspeed = awspeed + (osdata[7] >> 4);

		display_Header();
		display_NameEx(PSTR("Oregon"), id);
		display_IDn((rc << 8 | osdata[2]), 4);
		display_WINDIR(wdir);
		display_WINSP(wspeed);
		display_AWINSP(awspeed);
		boolean batOK = !((osdata[3] & 0x0F) >= 4);
		display_BAT(batOK);
		display_Footer();
	}
	else if (id == 0x3A0D || id == ID_WGR800 || id == ID_WGR800a) {
		/*
			3a0d  Anemometer: Huger-STR918, WGR918
			1984  Anemometer:
			1994  Anemometer:
			WIND DIR + SPEED + AV SPEED + BAT + CRC
			3A0D006F400800000031
		*/
		if (checksum(1, 9, osdata[9]) != 0) return false;

		wdir = ((osdata[5] >> 4) * 100) + ((osdata[5] & 0x0F * 10)) + (osdata[4] >> 4);
		wdir = (int)(wdir / 22.5);
		wspeed = ((osdata[7] & 0x0F) * 100) + ((osdata[6] >> 4) * 10) + ((osdata[6] & 0x0F));
		awspeed = ((osdata[8] >> 4) * 100) + ((osdata[8] & 0x0F) * 10) + ((osdata[7] >> 4));

		display_Header();
		display_NameEx(PSTR("Oregon"), id);
		display_IDn((rc << 8 | osdata[2]), 4);
		display_WINDIR(wdir);
		display_WINSP(wspeed);
		display_AWINSP(awspeed);
		boolean batOK = !((osdata[3] & 0x0F) >= 4);
		display_BAT(batOK);
		display_Footer();
	}
	else if (id == 0xea7c) {
		/*
			ea7c  UV Sensor: UVN128, UV138
			UV + BAT
			NO CRC YET
		*/

		uv = ((osdata[5] & 0x0F) * 10) + (osdata[4] >> 4);

		display_Header();
		display_NameEx(PSTR("Oregon"), id);
		display_IDn((rc << 8 | osdata[2]), 4);
		display_UV(uv);
		boolean batOK = !((osdata[3] & 0x0F) >= 4);
		display_BAT(batOK);
		display_Footer();
	}
	else if (id == 0xda78) {
		/*
			da78  UV Sensor: UVN800
			UV
			NO CRC YET
		*/
		uv = (osdata[6] & 0xf0) + (osdata[5] & 0x0f);


		display_Header();
		display_NameEx(PSTR("Oregon"), id);
		display_IDn((rc << 8 | osdata[2]), 4);
		display_UV(uv);
		boolean batOK = !((osdata[3] & 0x0F) >= 4);
		display_BAT(batOK);
		display_Footer();
	}
	else if (1 == proto) {              	   
	   /*
		   	Oregon protocol v1
		   	==================================================================================
		   	SL-109H, AcuRite 09955
		   	TEMP + CRC
		   	==================================================================================
		   	8487101C
			88190AAB
		   	01234567	/ 8
			0 1 2 3		/ 4
		   	RcTTTx$$	/ $ => 84+87+10=11B > 1B+1 = 1C
			     - -> 0010 0x02
				 B -> 1000 0x04
		*/   
		int sum = osdata[0] + osdata[1] + osdata[2]; // max. value is 0x2FD
		sum = (sum & 0xff) + (sum >> 8);             // add overflow to low byte
		if (osdata[3] != (sum & 0xff)) return false;

		temp = (((osdata[2] & 0xF0) >> 4) * 100) + ((osdata[1] & 0x0F) * 10) + ((osdata[1] >> 4));
		if ((osdata[2] & 0x02) == 0x02) temp = temp | 0x8000;

		display_Header();
		display_NameEx(PSTR("Oregon"), id);
		display_IDn((((unsigned long)(osdata[0] & 0xF0)) << 8) + (osdata[0] & 0x0F), 4);
		display_TEMP(temp);
		display_BAT(!((osdata[2] & 0x08) == 0x08));
		display_DEBUG(osdata, OSDATA_SIZE);
		display_Footer();
	}
#ifndef PLUGIN_048_DEBUG_UNKNOWN
	else return false;
#else	
	else {
		/*
			*aec  Date&Time: RTGR328N

			20;06;Oregon Unknown;DEBUG=8A EC 13 FC 60 81 43 91 11 30 0 0 0 ;
			20;20;Oregon Unknown;DEBUG=8A EC 13 FC 40 33 44 91 11 30 0 0 0 ;

			8A EC 13 FC 60 81 43 91 11 30 0 0 0 ;
			8+A+E+C+1+3+F+C+6+0+8+1+4+3+9+1=6B -A=61  != 30
			8+A+E+C+1+3+F+C+6+0+8+1+4+3+9=6a-A=60 0=0
			NO CRC YET

			OSV3 2A19048E399393250010
				01234567890123456789
				0 1 2 3 4 5 6 7 8 9
			2+A+1+9+0+4+8+E+3+9+9+3+9+3+2+5=5b-A=51 => 10

			==================================================================================
			8AEA1378077214924242C16CBD  21:49 29/04/2014
			0 1 2 3 4 5 6 7 8 9 0 1 2
			8+A+E+A+1+3+7+8+0+7+7+2+1+4+9+2+4+2+4+2+C+1+6+C=88 !=   BD
			Date & Time
			==================================================================================
			eac0  Ampere meter: cent-a-meter, OWL CM113, Electrisave
			==================================================================================
			0x1a* / 0x2a* 0x3a** Power meter: OWL CM119
			==================================================================================
			0x628* Power meter: OWL CM180
			==================================================================================
			OSV3 6284 3C 7801 D0
			OSV3 6280 3C 2801 A0A8BA05 00 00 ?? ?? ??
			==================================================================================
			1a99  Anemometer: WGTR800
			WIND + TEMP + HUM + CRC
		*/

		display_Header();
		display_NameEx(PSTR("Oregon"), id);
		display_DEBUG(osdata, OSDATA_SIZE);
		display_Footer();
	}   
#endif // PLUGIN_048_DEBUG_UNKNOWN

	// suppress repeats of the same RF packet
	RawSignal.Repeats = true;
	RawSignal.Number = 0;

	return true;
}

#endif // PLUGIN_048
