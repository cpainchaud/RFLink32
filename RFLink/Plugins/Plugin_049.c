//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                               Plugin-049: LaCrosse  TX141                                         ##
//#######################################################################################################
/**
 *
 * LaCrosse TX141-Bv2, TX141TH-Bv2, TX141-Bv3, TX145wsdth sensor.
 *
 * Portions/algos were taken out of RTL_433 https://github.com/merbanan/rtl_433/blob/ade63558fdc85e2571de115c666cf300fd903951/src/devices/lacrosse_tx141x.c
 * Thank you guys for this excellent project. Original authors: Robert Fraczkiewicz <aromring@gmail.com>, Andrew Rivett <veggiefrog@gmail.com>
 *
 * RFLink32 author and maintainer: Christophe Painchaud <shellescape@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 * LaCrosse TX141-Bv2, TX141TH-Bv2, TX141-Bv3, TX145wsdth sensor.
 * Also TFA 30.3221.02 (a TX141TH-Bv2).
 * LaCrosse Color Forecast Station (model C85845), or other LaCrosse product
 * utilizing the remote temperature/humidity sensor TX141TH-Bv2 transmitting
 * in the 433.92 MHz band. Product pages:
 * http://www.lacrossetechnology.com/c85845-color-weather-station/
 * http://www.lacrossetechnology.com/tx141th-bv2-temperature-humidity-sensor
 * The TX141TH-Bv2 protocol is OOK modulated PWM with fixed period of 625 us
 * for data bits, preambled by four long startbit pulses of fixed period equal
 * to ~1666 us. Hence, it is similar to Bresser Thermo-/Hygro-Sensor 3CH.
 * A single data packet looks as follows:
 * 1) preamble - 833 us high followed by 833 us low, repeated 4 times:
 *      ----      ----      ----      ----
 *     |    |    |    |    |    |    |    |
 *           ----      ----      ----      ----
 * 2) a train of 40 data pulses with fixed 625 us period follows immediately:
 *      ---    --     --     ---    ---    --     ---
 *     |   |  |  |   |  |   |   |  |   |  |  |   |   |
 *          --    ---    ---     --     --    ---     -- ....
 * A logical 1 is 417 us of high followed by 208 us of low.
 * A logical 0 is 208 us of high followed by 417 us of low.
 * Thus, in the example pictured above the bits are 1 0 0 1 1 0 1 ....
 * The TX141TH-Bv2 sensor sends 12 of identical packets, one immediately following
 * the other, in a single burst. These 12-packet bursts repeat every 50 seconds. At
 * the end of the last packet there are two 833 us pulses ("post-amble"?).
 * The TX141-Bv3 has a revision which only sends 4 packets per transmission.
 * The data is grouped in 5 bytes / 10 nybbles
 *     [id] [id] [flags] [temp] [temp] [temp] [humi] [humi] [chk] [chk]
 * The "id" is an 8 bit random integer generated when the sensor powers up for the
 * first time; "flags" are 4 bits for battery low indicator, test button press,
 * and channel; "temp" is 12 bit unsigned integer which encodes temperature in degrees
 * Celsius as follows:
 * temp_c = temp/10 - 50
 * to account for the -40 C -- 60 C range; "humi" is 8 bit integer indicating
 * relative humidity in %. The method of calculating "chk", the presumed 8-bit checksum
 * remains a complete mystery at the moment of this writing, and I am not totally sure
 * if the last is any kind of CRC. I've run reveng 1.4.4 on exemplary data with all
 * available CRC algorithms and found no match. Be my guest if you want to
 * solve it - for example, if you figure out why the following two pairs have identical
 * checksums you'll become a hero:
 *     0x87 0x02 0x3c 0x3b 0xe1
 *     0x87 0x02 0x7d 0x37 0xe1
 *     0x87 0x01 0xc3 0x31 0xd8
 *     0x87 0x02 0x28 0x37 0xd8
 * Developer's comment: with unknown CRC (see above) the obvious way of checking the data
 * integrity is making use of the 12 packet repetition. In principle, transmission errors are
 * be relatively rare, thus the most frequent packet should represent the true data.
 * A count enables us to determine the quality of radio transmission.
 *** Addition of TX141 temperature only device, Jan 2018 by Andrew Rivett <veggiefrog@gmail.com>**
 * The TX141-BV2 is the temperature only version of the TX141TH-BV2 sensor.
 * Changes:
 * - Changed minimum bit length to 32 (tx141b is temperature only)
 * - LACROSSE_TX141_BITLEN is 37 instead of 40.
 * - The humidity variable has been removed for TX141.
 * - Battery check bit is inverse of TX141TH.
 * - temp_f removed, temp_c (celsius) is what's provided by the device.
 * - TX141TH-BV3 bitlen is 41
 * The CRC Checksum is not checked. In trying to reverse engineer the
 * CRC, the first nibble can be checked by:
 *     a1 = (bytes[0]&0xF0) >> 4);
 *     b1 = (bytes[1]&0x40) >> 4) - 1;
 *     c1 = (bytes[2]&0xF0) >> 4);
 *     n1 = (a1+a2+c3)&0x0F;
 * The second nibble I could not figure out.
 * Addition of TX141W and TX145wsdth:
 *     PRE5b ID19h BAT1b TEST?1b CH?2h TYPE4h TEMP_WIND12d HUM_DIR12d CHK8h 1x
 * - type 1 has temp+hum (temp is offset 500 and scale 10)
 * - type 2 has wind speed (km/h scale 10) and direction (degrees)
 * - checksum is CRC-8 poly 0x31 init 0x00 over preceeding 7 bytes
 */

#define LACROSSE49_PLUGIN_ID 049
#define PLUGIN_DESC_049 "LaCrosse-TX141"

#define LACROSSE49_MINPULSECOUNT 72 // signal  is repeated without a break so there is a high chance it will be seen a single very large message
#define LACROSSE49_PREAMBLE_PULSE_LENGTH_MIN_D 690
#define LACROSSE49_PREAMBLE_PULSE_LENGTH_MAX_D 920

#define LACROSSE49_SHORT_PULSE_MIN_D 150
#define LACROSSE49_SHORT_PULSE_MAX_D 320
#define LACROSSE49_LONG_PULSE_MIN_D 370
#define LACROSSE49_LONG_PULSE_MAX_D 620

#define LACROSSE_TX141B_BITLEN 32
#define LACROSSE_TX141_BITLEN 37
#define LACROSSE_TX141TH_BITLEN 40
#define LACROSSE_TX141BV3_BITLEN 33
#define LACROSSE_TX141W_BITLEN 65
#define LACROSSE_TX141_BITLEN_MIN 32

//#define PLUGIN_049_DEBUG

#ifdef PLUGIN_049

#include "../4_Display.h"

uint16_t LACROSSE49_PREAMBLE_PULSE_LENGTH_MIN;
uint16_t LACROSSE49_PREAMBLE_PULSE_LENGTH_MAX;

uint16_t LACROSSE49_SHORT_PULSE_MIN;
uint16_t LACROSSE49_SHORT_PULSE_MAX;
uint16_t LACROSSE49_LONG_PULSE_MIN;
uint16_t LACROSSE49_LONG_PULSE_MAX;

inline int findPreamble(const int startPosition, const int endPosition) {
  for(int i=startPosition; i<endPosition; i+=2) {
    bool found = true;
    for(int j=i; j < i+8; j++) {
      if(RawSignal.Pulses[j] < LACROSSE49_PREAMBLE_PULSE_LENGTH_MIN || RawSignal.Pulses[j] > LACROSSE49_PREAMBLE_PULSE_LENGTH_MAX) {
        found = false;
        break;
      }
    }
    if(found)
      return i;
  }

  return -1;
}

inline short int PLUGIN_049_decode_pulse(uint16_t position) {
  if( RawSignal.Pulses[position] < LACROSSE49_SHORT_PULSE_MIN )
    return -1;
  if( RawSignal.Pulses[position] > LACROSSE49_LONG_PULSE_MAX )
    return -1;

  if( RawSignal.Pulses[position+1] < LACROSSE49_SHORT_PULSE_MIN )
    return -1;
  if( RawSignal.Pulses[position+1] > LACROSSE49_LONG_PULSE_MAX )
    return -1;

  return RawSignal.Pulses[position] > RawSignal.Pulses[position+1];

}

/**
 *
 * @param fromPosition
 * @param toPosition
 * @return -1 if failed
 */
inline bool PLUGIN_049_decode(int fromPosition, int toPosition ) {

  int messageLength = toPosition - (fromPosition+8);
  int bitCount = messageLength/2;

  #ifdef PLUGIN_049_DEBUG
  sprintf(printBuf, PSTR("LaCrosseTX141 message length=%i bits=%i"), messageLength, bitCount/2);
  sendRawPrint(printBuf, true);
  #endif

  int deviceType = -1;

  if (bitCount >= 64) {
    deviceType = LACROSSE_TX141W_BITLEN;
  }
  else if (bitCount > 41) {
    #ifdef PLUGIN_049_DEBUG
    sprintf(printBuf, PSTR("LaCrosseTX141 False-Positive, we're out!"));
    sendRawPrint(printBuf, true);
    #endif
    return false;
  }
  else if (bitCount >= 41) {
    deviceType = LACROSSE_TX141TH_BITLEN; // actually TX141TH-BV3
  }
  else if (bitCount >= 40) {
    deviceType = LACROSSE_TX141TH_BITLEN;
  }
  else if (bitCount >= 37) {
    deviceType = LACROSSE_TX141_BITLEN;
  }
  else if (bitCount == 32) {
    deviceType = LACROSSE_TX141B_BITLEN;
  } else {
    deviceType = LACROSSE_TX141BV3_BITLEN;
  }

  BitArray data;
  #ifdef PLUGIN_049_DEBUG
  data.storage[0] = 0;
  sprintf(printBuf, PSTR("LaCrosseTX141 %.2X"), (int) data.storage[0]);
  sendRawPrint(printBuf, true);
  #endif

  if(!data.fillFromPwmPulses(deviceType, RawSignal.Pulses, RawSignal.Number, fromPosition + 8, LACROSSE49_SHORT_PULSE_MIN, LACROSSE49_SHORT_PULSE_MAX, LACROSSE49_LONG_PULSE_MIN, LACROSSE49_LONG_PULSE_MAX)){
    #ifdef PLUGIN_049_DEBUG
    sprintf(printBuf, PSTR("LaCrosseTX141 failed to decode PWM"));
    sendRawPrint(printBuf, true);
    #endif
  }

  #ifdef PLUGIN_049_DEBUG
  sprintf(printBuf, PSTR("LaCrosseTX141 %.2X"), (int) data.storage[0]);
  sendRawPrint(printBuf, true);
  #endif

  display_Header();

  if(deviceType == LACROSSE_TX141B_BITLEN) {
    display_Name(PSTR("LaCrosse-TX141B"));
  } else if(deviceType == LACROSSE_TX141_BITLEN) {
    display_Name(PSTR("LaCrosse-TX141Bv2"));
  } else if(deviceType == LACROSSE_TX141BV3_BITLEN) {
    display_Name(PSTR("LaCrosse-TX141Bv3"));
  } else if(deviceType == LACROSSE_TX141W_BITLEN) {
    display_Name(PSTR("LaCrosse-TX141W"));
  } else {
    display_Name(PSTR("LaCrosse-TX141THBv2"));
    if (lfsr_digest8_reflect(data.storage, 4, 0x31, 0xf4) != data.storage[4]) {
      #ifdef PLUGIN_049_DEBUG
      sprintf(printBuf, PSTR("LACROSSE_TX141THBv2 Failed CRC"));
      sendRawPrint(printBuf, true);
      #endif
      return false;
    }
  }

  if (deviceType == LACROSSE_TX141W_BITLEN) {
    if (crc8(data.storage, 8, 0x31, 0x00) ){
      #ifdef PLUGIN_049_DEBUG
      sprintf(printBuf, PSTR("LACROSSE_TX141W Failed CRC"));
      sendRawPrint(printBuf, true);
      #endif
      return false;
    }

    uint32_t id = data.getUInt(0*8 + 5, 19);
    display_IDn(id, 4);

    display_CHAN(data.getUInt(3*8 + 2, 2));

    bool battery_low = data.getUInt(3*8 + 0, 1);
    uint8_t type = data.getUInt(3*8 + 4, 4);
    int16_t temp_raw = (int16_t)data.getUInt(4*8, 12);
    uint8_t humidity = data.getUInt(5*8 + 4, 12);


    if (type == 1) {
      // Temp/Hum

      display_TEMP(temp_raw-500);
      display_HUM(humidity, true);

    } else if (type == 2) {
      // wind direction is in humidity field

      display_WINDIR(humidity);
      display_WINSP(temp_raw);
    } else {
      #ifdef PLUGIN_049_DEBUG
      sprintf(printBuf, PSTR("LACROSSE_TX141W failed packet type=%i"), (int) type);
      sendRawPrint(printBuf, true);
      #endif
      return false;
    }

    display_BAT(!battery_low);

    display_Footer();

    return true;
  }
  
  // sensor ID
  unsigned short int sensorId = data.getUInt(0, 8);
  display_IDn(sensorId, 4);

  // temperature reading
  int16_t temperatureRaw = sensorId = data.getUInt(12, 12);;


  //Serial.printf("%.4X\r\n", (long int) temperatureRaw);
  temperatureRaw = temperatureRaw - 500;
  //Serial.println( ((float)temperatureRaw) * 0.1f );

  display_TEMP(temperatureRaw);

  // battery status
  display_BAT( (data.getBit(8)) ^ !(deviceType == LACROSSE_TX141_BITLEN || deviceType == LACROSSE_TX141BV3_BITLEN) );

  display_Footer();

  return true;
}

boolean Plugin_049(byte function, const char *string) {

  if (RawSignal.Number < LACROSSE49_MINPULSECOUNT) {
    return false;
  }

  LACROSSE49_PREAMBLE_PULSE_LENGTH_MIN = LACROSSE49_PREAMBLE_PULSE_LENGTH_MIN_D / RawSignal.Multiply;
  LACROSSE49_PREAMBLE_PULSE_LENGTH_MAX = LACROSSE49_PREAMBLE_PULSE_LENGTH_MAX_D / RawSignal.Multiply;
  LACROSSE49_SHORT_PULSE_MIN = LACROSSE49_SHORT_PULSE_MIN_D / RawSignal.Multiply;
  LACROSSE49_SHORT_PULSE_MAX = LACROSSE49_SHORT_PULSE_MAX_D / RawSignal.Multiply;
  LACROSSE49_LONG_PULSE_MIN = LACROSSE49_LONG_PULSE_MIN_D / RawSignal.Multiply;
  LACROSSE49_LONG_PULSE_MAX = LACROSSE49_LONG_PULSE_MAX_D / RawSignal.Multiply;

  uint16_t startPosition = 1;

  while(true) {

    int preamblePosition = findPreamble(startPosition, RawSignal.Number - LACROSSE_TX141_BITLEN_MIN * 2 -
                                           8); // 32 is the number of bits of the smallest message + 8 pulses for preamble

    if (preamblePosition < 0) {
      #ifdef PLUGIN_049_DEBUG
      //sprintf(printBuf, PSTR("Failed to find a preamble"));
      //sendRawPrint(printBuf, true);
      #endif
      return false;
    }

    #ifdef PLUGIN_049_DEBUG
    sprintf(printBuf, PSTR("LaCrosseTX141 found a suitable preamble at position %i"), preamblePosition);
    sendRawPrint(printBuf, true);
    #endif

    int secondPreamblePosition = -1;

    if (RawSignal.Number > (preamblePosition) + (LACROSSE_TX141_BITLEN_MIN * 2 + 8) * 2) {
      #ifdef PLUGIN_049_DEBUG
      sendRawPrint(F("LaCrosseTX141 enough Pulses to discover a new another Preamble"), true);
      #endif
      secondPreamblePosition = findPreamble(preamblePosition + LACROSSE_TX141_BITLEN_MIN * 2, RawSignal.Number - 8);
    }

    if (secondPreamblePosition > 0) {
      #ifdef PLUGIN_049_DEBUG
      sprintf(printBuf, PSTR("LaCrosseTX141 a second Preamble was found at position %i"), secondPreamblePosition);
      sendRawPrint(printBuf, true);
      #endif
    } else {
      secondPreamblePosition = RawSignal.Number + 1;
    }

    if(PLUGIN_049_decode(preamblePosition, secondPreamblePosition))
      return true;

    pbuffer[0] = 0;

    startPosition = secondPreamblePosition;

    if((int)RawSignal.Number - startPosition < 72) {
      break;
    }

    #ifdef PLUGIN_049_DEBUG
    sprintf(printBuf, PSTR("LaCrosseTX141: looking for a second preamble!"));
    sendRawPrint(printBuf, true);
    #endif
  }


  return true;

}

#endif // PLUGIN_049
