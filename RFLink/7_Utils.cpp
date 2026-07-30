/** @file
    Various utility functions for use by device drivers.

    Copyright (C) 2015 Tommy Vestermark
    Copyright (C) 2021 Olivier Sannier

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "7_Utils.h"
#include "4_Display.h"
#include "2_Signal.h"
// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>

uint8_t reverse8(uint8_t x)
{
    x = (x & 0xF0) >> 4 | (x & 0x0F) << 4;
    x = (x & 0xCC) >> 2 | (x & 0x33) << 2;
    x = (x & 0xAA) >> 1 | (x & 0x55) << 1;
    return x;
}

void reflect_bytes(uint8_t message[], unsigned num_bytes)
{
    for (unsigned i = 0; i < num_bytes; ++i)
    {
        message[i] = reverse8(message[i]);
    }
}

uint8_t reflect4(uint8_t x)
{
    x = (x & 0xCC) >> 2 | (x & 0x33) << 2;
    x = (x & 0xAA) >> 1 | (x & 0x55) << 1;
    return x;
}

void reflect_nibbles(uint8_t message[], unsigned num_bytes)
{
    for (unsigned i = 0; i < num_bytes; ++i)
    {
        message[i] = reflect4(message[i]);
    }
}

void invert_bytes(uint8_t message[], unsigned num_bytes)
{
    for (unsigned i = 0; i < num_bytes; ++i)
    {
        message[i] = ~message[i];
    }
}

unsigned extract_nibbles_4b1s(uint8_t *message, unsigned offset_bits, unsigned num_bits, uint8_t *dst)
{
    unsigned ret = 0;

    while (num_bits >= 5)
    {
        uint16_t bits = (message[offset_bits / 8] << 8) | message[(offset_bits / 8) + 1];
        bits >>= 11 - (offset_bits % 8); // align 5 bits to LSB
        if ((bits & 1) != 1)
            break; // stuff-bit error
        *dst++ = (bits >> 1) & 0xf;
        ret += 1;
        offset_bits += 5;
        num_bits -= 5;
    }

    return ret;
}

uint8_t crc4(uint8_t const message[], unsigned nBytes, uint8_t polynomial, uint8_t init)
{
    unsigned remainder = init << 4; // LSBs are unused
    unsigned poly = polynomial << 4;
    unsigned bit;

    while (nBytes--)
    {
        remainder ^= *message++;
        for (bit = 0; bit < 8; bit++)
        {
            if (remainder & 0x80)
            {
                remainder = (remainder << 1) ^ poly;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }
    return remainder >> 4 & 0x0f; // discard the LSBs
}

uint8_t crc7(uint8_t const message[], unsigned nBytes, uint8_t polynomial, uint8_t init)
{
    unsigned remainder = init << 1; // LSB is unused
    unsigned poly = polynomial << 1;
    unsigned byte, bit;

    for (byte = 0; byte < nBytes; ++byte)
    {
        remainder ^= message[byte];
        for (bit = 0; bit < 8; ++bit)
        {
            if (remainder & 0x80)
            {
                remainder = (remainder << 1) ^ poly;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }
    return remainder >> 1 & 0x7f; // discard the LSB
}

uint8_t crc8(uint8_t const message[], unsigned nBytes, uint8_t polynomial, uint8_t init)
{
    uint8_t remainder = init;
    unsigned byte, bit;

    for (byte = 0; byte < nBytes; ++byte)
    {
        remainder ^= message[byte];
        for (bit = 0; bit < 8; ++bit)
        {
            if (remainder & 0x80)
            {
                remainder = (remainder << 1) ^ polynomial;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }
    return remainder;
}

uint8_t crc8le(uint8_t const message[], unsigned nBytes, uint8_t polynomial, uint8_t init)
{
    uint8_t remainder = reverse8(init);
    unsigned byte, bit;
    polynomial = reverse8(polynomial);

    for (byte = 0; byte < nBytes; ++byte)
    {
        remainder ^= message[byte];
        for (bit = 0; bit < 8; ++bit)
        {
            if (remainder & 1)
            {
                remainder = (remainder >> 1) ^ polynomial;
            }
            else
            {
                remainder = (remainder >> 1);
            }
        }
    }
    return remainder;
}

uint16_t crc16lsb(uint8_t const message[], unsigned nBytes, uint16_t polynomial, uint16_t init)
{
    uint16_t remainder = init;
    unsigned byte, bit;

    for (byte = 0; byte < nBytes; ++byte)
    {
        remainder ^= message[byte];
        for (bit = 0; bit < 8; ++bit)
        {
            if (remainder & 1)
            {
                remainder = (remainder >> 1) ^ polynomial;
            }
            else
            {
                remainder = (remainder >> 1);
            }
        }
    }
    return remainder;
}

uint16_t crc16(uint8_t const message[], unsigned nBytes, uint16_t polynomial, uint16_t init)
{
    uint16_t remainder = init;
    unsigned byte, bit;

    for (byte = 0; byte < nBytes; ++byte)
    {
        remainder ^= message[byte] << 8;
        for (bit = 0; bit < 8; ++bit)
        {
            if (remainder & 0x8000)
            {
                remainder = (remainder << 1) ^ polynomial;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }
    return remainder;
}

uint8_t lfsr_digest8(uint8_t const message[], unsigned bytes, uint8_t gen, uint8_t key)
{
    uint8_t sum = 0;
    for (unsigned k = 0; k < bytes; ++k)
    {
        uint8_t data = message[k];
        for (int i = 7; i >= 0; --i)
        {
            // fprintf(stderr, "key is %02x\n", key);
            // XOR key into sum if data bit is set
            if ((data >> i) & 1)
                sum ^= key;

            // roll the key right (actually the lsb is dropped here)
            // and apply the gen (needs to include the dropped lsb as msb)
            if (key & 1)
                key = (key >> 1) ^ gen;
            else
                key = (key >> 1);
        }
    }
    return sum;
}

uint8_t lfsr_digest8_reflect(uint8_t const message[], int bytes, uint8_t gen, uint8_t key)
{
    uint8_t sum = 0;
    // Process message from last byte to first byte (reflected)
    for (int k = bytes - 1; k >= 0; --k)
    {
        uint8_t data = message[k];
        // Process individual bits of each byte (reflected)
        for (int i = 0; i < 8; ++i)
        {
            // fprintf(stderr, "key is %02x\n", key);
            // XOR key into sum if data bit is set
            if ((data >> i) & 1)
            {
                sum ^= key;
            }

            // roll the key left (actually the lsb is dropped here)
            // and apply the gen (needs to include the dropped lsb as msb)
            if (key & 0x80)
                key = (key << 1) ^ gen;
            else
                key = (key << 1);
        }
    }
    return sum;
}

uint16_t lfsr_digest16(uint32_t data, int bits, uint16_t gen, uint16_t key)
{
    uint16_t sum = 0;
    for (int bit = bits - 1; bit >= 0; --bit)
    {
        // fprintf(stderr, "key at bit %d : %04x\n", bit, key);
        // if data bit is set then xor with key
        if ((data >> bit) & 1)
            sum ^= key;

        // roll the key right (actually the lsb is dropped here)
        // and apply the gen (needs to include the dropped lsb as msb)
        if (key & 1)
            key = (key >> 1) ^ gen;
        else
            key = (key >> 1);
    }
    return sum;
}

/*
void lfsr_keys_fwd16(int rounds, uint16_t gen, uint16_t key)
{
    for (int i = 0; i <= rounds; ++i) {
        fprintf(stderr, "key at bit %d : %04x\n", i, key);

        // roll the key right (actually the lsb is dropped here)
        // and apply the gen (needs to include the dropped lsb as msb)
        if (key & 1)
            key = (key >> 1) ^ gen;
        else
            key = (key >> 1);
    }
}

void lfsr_keys_rwd16(int rounds, uint16_t gen, uint16_t key)
{
    for (int i = 0; i <= rounds; ++i) {
        fprintf(stderr, "key at bit -%d : %04x\n", i, key);

        // roll the key left (actually the msb is dropped here)
        // and apply the gen (needs to include the dropped msb as lsb)
        if (key & (1 << 15))
            key = (key << 1) ^ gen;
        else
            key = (key << 1);
    }
}
*/

// we could use popcount intrinsic, but don't actually need the performance
int parity8(uint8_t byte)
{
    byte ^= byte >> 4;
    byte &= 0xf;
    return (0x6996 >> byte) & 1;
}

int parity_bytes(uint8_t const message[], unsigned num_bytes)
{
    int result = 0;
    for (unsigned i = 0; i < num_bytes; ++i)
    {
        result ^= parity8(message[i]);
    }
    return result;
}

uint8_t xor_bytes(uint8_t const message[], unsigned num_bytes)
{
    uint8_t result = 0;
    for (unsigned i = 0; i < num_bytes; ++i)
    {
        result ^= message[i];
    }
    return result;
}

int add_bytes(uint8_t const message[], unsigned num_bytes)
{
    int result = 0;
    for (unsigned i = 0; i < num_bytes; ++i)
    {
        result += message[i];
    }
    return result;
}

int add_nibbles(uint8_t const message[], unsigned num_bytes)
{
    int result = 0;
    for (unsigned i = 0; i < num_bytes; ++i)
    {
        result += (message[i] >> 4) + (message[i] & 0x0f);
    }
    return result;
}

bool decode_pwm(uint8_t frame[], uint8_t expectedBitCount, uint16_t const pulses[], const int pulsesCount, int pulseIndex, uint16_t shortPulseMinDuration, uint16_t shortPulseMaxDuration, uint16_t longPulseMinDuration, uint16_t longPulseMaxDuration, uint8_t bitOffset)
{
    if (pulseIndex + (expectedBitCount - 1) * 2  > pulsesCount)
    {
        #ifdef PWM_DEBUG
        Serial.print(F("PWM: Not enough pulses: pulseIndex = "));
        Serial.print(pulseIndex);
        Serial.print(F(" - expectedBitCount = "));
        Serial.print(expectedBitCount);
        Serial.print(F(" - pulsesCount = "));
        Serial.print(pulsesCount);
        Serial.print(F(" - min required pulses = "));
        Serial.println(pulseIndex + expectedBitCount * 2);         
        #endif
        return false;
    }

    const uint8_t bitsPerByte = 8;
    //const uint8_t expectedByteCount = expectedBitCount / bitsPerByte;
    const uint8_t endBitCount = expectedBitCount + bitOffset;

    for(uint8_t bitIndex = bitOffset; bitIndex < endBitCount; bitIndex++)
    {
        int currentFrameByteIndex = bitIndex / bitsPerByte;
        uint16_t bitDuration = pulses[pulseIndex];
        uint8_t bitMask = (0x80 >> (bitIndex % bitsPerByte));

        if (value_between(bitDuration, shortPulseMinDuration, shortPulseMaxDuration))
        {
            frame[currentFrameByteIndex] &= ~bitMask;
        }
        else if (value_between(bitDuration, longPulseMinDuration, longPulseMaxDuration))
        {
            frame[currentFrameByteIndex] |= bitMask;
        }
        else
        {
            #ifdef PWM_DEBUG
            Serial.print(F("PWM: Invalid duration at pulse "));
            Serial.print(pulseIndex);
            Serial.print(F(" - bit "));
            Serial.print(bitIndex);
            Serial.print(F(": "));
            Serial.println(bitDuration * RFLink::Signal::RawSignal.Multiply);         
            #endif
            return false; // unexpected bit duration, invalid format
        }


        pulseIndex += 2;
    }

    return true;
}

bool decode_manchester(uint8_t frame[], uint8_t expectedBitCount, uint16_t const pulses[], const int pulsesCount, int pulseIndex, uint8_t nextBit, bool secondPulse, uint16_t halfBitMinDuration, uint16_t halfBitMaxDuration)
{
    int bitIndex = 0;
    const uint8_t bitsPerByte = 8;
    const uint8_t expectedByteCount = expectedBitCount / bitsPerByte;
    const uint16_t fullBitMinDuration = halfBitMinDuration * 2;
    const uint16_t fullBitMaxDuration = halfBitMaxDuration * 2;

    while ((pulseIndex < pulsesCount) && (bitIndex < expectedBitCount))
    {
        uint16_t pulseDuration = pulses[pulseIndex];
        int currentFrameByteIndex = bitIndex / bitsPerByte;

        if (value_between(pulseDuration, fullBitMinDuration, fullBitMaxDuration))
        {
            if (!secondPulse)
            {
                #ifdef MANCHESTER_DEBUG
                Serial.print(F("Manchester: Cannot have long pulse as a first pulse: index = "));
                Serial.print(pulseIndex);
                Serial.print(" - value = ");
                Serial.println(pulseDuration);
                #endif
                return false;
            }

            frame[currentFrameByteIndex] <<= 1;
            frame[currentFrameByteIndex] |= nextBit;

            nextBit = 1 - nextBit;
            bitIndex++;
        }
        else if (value_between(pulseDuration, halfBitMinDuration, halfBitMaxDuration))
        {
            if (secondPulse)
            {
                frame[currentFrameByteIndex] <<= 1;
                frame[currentFrameByteIndex] |= nextBit;

                bitIndex++;
            }

            secondPulse = !secondPulse;
        }
        else
        {
            #ifdef MANCHESTER_DEBUG
            Serial.print(F("Manchester: Pulse has unexpected duration: index = "));
            Serial.print(pulseIndex);
            Serial.print(" - value = ");
            Serial.println(pulseDuration);
            #endif
            return false;
        }

        pulseIndex++;
    }

    // The low part of the down front gets mixed with the interframe silence and is thus too long to be placed in the pulses
    // This means the last bit is never placed in the frame and we must manually add it into the last byte
    if ((pulseIndex == pulsesCount) && (bitIndex == expectedBitCount - 1))
    {
        frame[expectedByteCount - 1] <<= 1;
        frame[expectedByteCount - 1] |= nextBit;
        bitIndex++;
    }

    return (bitIndex == expectedBitCount);
}

namespace RFLink {
  namespace Utils {

    const uint8_t BitArray::_masks[8] = {128, 64, 32, 16, 8, 4, 2, 1};

    BitArray::BitArray() {
      currentSize = 0;

      /*
      storage[0]= 0x40;
      storage[1]= 0xf2;
      storage[2]= 0xa5;
      storage[3]= 0x49;

      *((uint32_t*) (&storage[4])) = 0x12345678;

      Serial.printf("BitArrayDebug: %.8X\r\n", getUInt(0, 32));
      Serial.printf("BitArrayDebug: %.8X\r\n", getUInt(32, 32));*/

    }

    uint32_t BitArray::getUInt(const uint16_t firstBitPosition, const uint16_t length) {

      int32_t result = 0;
      for (uint16_t i = firstBitPosition; i < firstBitPosition + length; i++) {
        result <<= 1;
        //Serial.printf("byte %i bit %i = %i\r\n", (int) i / 8, (int) i % 8, (storage[i / 8] & (0x80 >> (i%8)) ) != 0);
        if ((storage[i / 8] & (0x80 >> (i%8)) ) != 0)
          result += 1;
      }

      return result;
    }

  } //end of Utils namespace
} // end of RFLink namespace


// Unit testing
#ifdef _TEST
int main(int argc, char **argv)
{
    fprintf(stderr, "util:: test\r\n");

    uint8_t msg[] = {0x08, 0x0a, 0xe8, 0x80};

    fprintf(stderr, "util::crc8(): odd parity:  %02X\r\n", crc8(msg, 3, 0x80, 0x00));
    fprintf(stderr, "util::crc8(): even parity: %02X\r\n", crc8(msg, 4, 0x80, 0x00));

    return 0;
}
#endif /* _TEST */
