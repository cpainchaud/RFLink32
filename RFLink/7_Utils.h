/** @file
    Various utility functions for use by device drivers.

    Copyright (C) 2015 Tommy Vestermark
    Copyright (C) 2021 Olivier Sannier

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef INCLUDE_UTIL_H_
#define INCLUDE_UTIL_H_

#include <stdint.h>

// Helper macros, collides with MSVC's stdlib.h unless NOMINMAX is used
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

/// Reverse (reflect) the bits in an 8 bit byte.
///
/// @param x input byte
/// @return bit reversed byte
uint8_t reverse8(uint8_t x);

/// Reflect (reverse LSB to MSB) each byte of a number of bytes.
///
/// @param message bytes of message data
/// @param num_bytes number of bytes to reflect
void reflect_bytes(uint8_t message[], unsigned num_bytes);

/// Reflect (reverse LSB to MSB) each nibble in an 8 bit byte, preserves nibble order.
///
/// @param x input byte
/// @return reflected nibbles
uint8_t reflect4(uint8_t x);

/// Reflect (reverse LSB to MSB) each nibble in a number of bytes.
///
/// @param message bytes of nibble message data
/// @param num_bytes number of bytes to reflect
void reflect_nibbles(uint8_t message[], unsigned num_bytes);

/// Unstuff nibbles with 1-bit separator (4B1S) to bytes, returns number of successfully unstuffed nibbles.
///
/// @param message bytes of message data
/// @param offset_bits start offset of message in bits
/// @param num_bits message length in bits
/// @param dst target buffer for extracted nibbles, at least num_bits/5 size
unsigned extract_nibbles_4b1s(uint8_t *message, unsigned offset_bits, unsigned num_bits, uint8_t *dst);

/// CRC-4.
///
/// @param message array of bytes to check
/// @param nBytes number of bytes in message
/// @param polynomial CRC polynomial
/// @param init starting crc value
/// @return CRC value
uint8_t crc4(uint8_t const message[], unsigned nBytes, uint8_t polynomial, uint8_t init);

/// CRC-7.
///
/// @param message array of bytes to check
/// @param nBytes number of bytes in message
/// @param polynomial CRC polynomial
/// @param init starting crc value
/// @return CRC value
uint8_t crc7(uint8_t const message[], unsigned nBytes, uint8_t polynomial, uint8_t init);

/// Generic Cyclic Redundancy Check CRC-8.
///
/// Example polynomial: 0x31 = x8 + x5 + x4 + 1 (x8 is implicit)
/// Example polynomial: 0x80 = x8 + x7 (a normal bit-by-bit parity XOR)
///
/// @param message array of bytes to check
/// @param nBytes number of bytes in message
/// @param polynomial byte is from x^7 to x^0 (x^8 is implicitly one)
/// @param init starting crc value
/// @return CRC value
uint8_t crc8(uint8_t const message[], unsigned nBytes, uint8_t polynomial, uint8_t init);

/// "Little-endian" Cyclic Redundancy Check CRC-8 LE
/// Input and output are reflected, i.e. least significant bit is shifted in first.
///
/// @param message array of bytes to check
/// @param nBytes number of bytes in message
/// @param polynomial CRC polynomial
/// @param init starting crc value
/// @return CRC value
uint8_t crc8le(uint8_t const message[], unsigned nBytes, uint8_t polynomial, uint8_t init);

/// CRC-16 LSB.
/// Input and output are reflected, i.e. least significant bit is shifted in first.
/// Note that poly and init already need to be reflected.
///
/// @param message array of bytes to check
/// @param nBytes number of bytes in message
/// @param polynomial CRC polynomial
/// @param init starting crc value
/// @return CRC value
uint16_t crc16lsb(uint8_t const message[], unsigned nBytes, uint16_t polynomial, uint16_t init);

/// CRC-16.
///
/// @param message array of bytes to check
/// @param nBytes number of bytes in message
/// @param polynomial CRC polynomial
/// @param init starting crc value
/// @return CRC value
uint16_t crc16(uint8_t const message[], unsigned nBytes, uint16_t polynomial, uint16_t init);

/// Digest-8 by "LFSR-based Toeplitz hash".
///
/// @param message bytes of message data
/// @param bytes number of bytes to digest
/// @param gen key stream generator, needs to includes the MSB if the LFSR is rolling
/// @param key initial key
/// @return digest value
uint8_t lfsr_digest8(uint8_t const message[], unsigned bytes, uint8_t gen, uint8_t key);

/// Digest-8 by "LFSR-based Toeplitz hash", byte reflect, bit reflect.
///
/// @param message bytes of message data
/// @param bytes number of bytes to digest
/// @param gen key stream generator, needs to includes the MSB if the LFSR is rolling
/// @param key initial key
/// @return digest value
uint8_t lfsr_digest8_reflect(uint8_t const message[], int bytes, uint8_t gen, uint8_t key);

/// Digest-16 by "LFSR-based Toeplitz hash".
///
/// @param data up to 32 bits data, LSB aligned
/// @param bits number of bits to digest
/// @param gen key stream generator, needs to includes the MSB if the LFSR is rolling
/// @param key initial key
/// @return digest value
uint16_t lfsr_digest16(uint32_t data, int bits, uint16_t gen, uint16_t key);

/// Compute bit parity of a single byte (8 bits).
///
/// @param byte single byte to check
/// @return 1 odd parity, 0 even parity
int parity8(uint8_t byte);

/// Compute bit parity of a number of bytes.
///
/// @param message bytes of message data
/// @param num_bytes number of bytes to sum
/// @return 1 odd parity, 0 even parity
int parity_bytes(uint8_t const message[], unsigned num_bytes);

/// Compute XOR (byte-wide parity) of a number of bytes.
///
/// @param message bytes of message data
/// @param num_bytes number of bytes to sum
/// @return summation value, per bit-position 1 odd parity, 0 even parity
uint8_t xor_bytes(uint8_t const message[], unsigned num_bytes);

/// Compute Addition of a number of bytes.
///
/// @param message bytes of message data
/// @param num_bytes number of bytes to sum
/// @return summation value
int add_bytes(uint8_t const message[], unsigned num_bytes);

/// Compute Addition of a number of nibbles (byte wise).
///
/// @param message bytes (of two nibbles) of message data
/// @param num_bytes number of bytes to sum
/// @return summation value
int add_nibbles(uint8_t const message[], unsigned num_bytes);

/// Returns true if value is between the min and max arguments (excluded)
///
/// @param value   the value to test
/// @param min     the lower value to test against
/// @param max     the upper value to test against
/// @return true if value is between min and max
inline bool value_between(int value, int min, int max);
inline bool value_between(uint8_t value, uint8_t min, uint8_t max);
inline bool value_between(uint16_t value, uint16_t min, uint16_t max);
inline bool value_between(uint32_t value, uint32_t min, uint32_t max);

/**
 *  Decodes the pulses as a PWM encoded series of pulses
 * 
 *  @param frame                  the buffer where to place the decoded bits
 *  @param expectedBitCount       the expected bit count
 *  @param pulses                 the pulses to decode
 *  @param pulsesCount            the numnber of items inside @pulses
 *  @param pulseIndex             the index of the first pulse to be decoded
 *  @param shortPulseMinDuration  the minimum duration of a half bit
 *  @param shortPulseMaxDuration  the maximum duration of a half bit
 *  @param longPulseMinDuration   the minimum duration of a half bit
 *  @param longPulseMaxDuration   the maximum duration of a half bit
 *  @return true if pulses could be decoded, giving enough bits, false if not or pulses were not of valid lengths

    The PWM encoding uses pair of pulses like this:
    
    Long then short : ----__
    Short then long : --____ 
    
    So, for instance, we would receive this: ----__----__----__--____
    This gives 4 pairs, hence 4 bits, long/short, long/short, long/short, short/long 

    This method uses the following truth table

       Pair      |  Value
     ------------+--------  
      long/short |    1
      short/long |    0

    If you need the opposite, simply use the ~ operator on the frame bytes

    Note that for efficiency reasons, this method does not consider the second pulse of the pair and thus does not test
    it for duration validity.
    
    Bits are placed in the frame in the order they appear, ie MSB first. To illustrate, consider the following set of pulses:

    --_-__--_--_--_--_-__--_--_-__-__--_--_--_-__-__-__--_-__--_--_--_-__-__
    1  0  1  1  1  1  0  1  1  0  0  1  1  1  0  0  0  1  0  1  1  1  0  0
         B     |     D     |     9     |     C     |     5     |     C       

    This is decoded as 24 bits and inside the frame parameter, you will  receive the following 3 bytes:
      index    0  1  2  
      value   BD 9C 5C

    Note that if you expect 32 bits or less, you can easily pass a uint32_t as the frame parameter, simply cast like so: 

        uint32_t frame = 0;
        decode_pwm((uint_8t *)&frame, 32, ...

    But because the ESP32 is a little endian architecture, the bytes will be reversed two by two (0 - 3, 1 - 2)
    Using ntohs or ntohl from <netinet/in.h> is thus recommended in that case
*/
bool decode_pwm(uint8_t frame[], uint8_t expectedBitCount, uint16_t const pulses[], const int pulsesCount, int pulseIndex, uint16_t shortPulseMinDuration, uint16_t shortPulseMaxDuration, uint16_t longPulseMinDuration, uint16_t longPulseMaxDuration);

/**
 *  Decodes the pulses as a Manchester encoded series of pulses
 * 
 *  @param frame               the buffer where to place the decoded bits
 *  @param expectedBitCount    the expected bit count
 *  @param pulses              the pulses to decode
 *  @param pulsesCount         the numnber of items inside @pulses
 *  @param pulseIndex          the index of the first pulse to be decoded
 *  @param nextBit             the value of the next bit to use when a transition occurs
 *  @param secondPulse         true if the first pulse (as indicated by pulseIndex) is the second part of the first transition
 *  @param halfBitMinDuration  the minimum duration of a half bit
 *  @param halfBitMaxDuration  the maximum duration of a half bit
 *  @return true if pulses could be decoded, giving enough bits

    The manchester encoding uses transitions to encode a bit value like this:
    
    From low to high : _- 
    From high to low : -_
    
    So, for instance, we would receive this: _-_--__-
    This gives 4 transitions, hence 4 bits, low to high, low to high, high to low, low to high

    As we are receiving pulses, you can see that these four bits are encoded as 6 pulses which means we have to consider short/long changes.
    Usually, the encoded part is preceded by other pulses which means that we have 4 possible situations:
   
    Pulses before | Manchester data
               ___|_-
               ___|-_
               ---|_-
               ---|-_

    As you can see, in two of the four cases, the pulse before is "mixed" with the first manchester encoded half bit.
    You, the caller, have to figure this out and set secondPulse to true if that is the case, along with making sure pulseIndex indicates the 
    pulse that is the second half bit.
    If the pulse is not mixed with the halfbit, then secondPulse is set to false and pulseIndex indicates the pulse that is the first half bit.

    The value of nextBit is also the responsibility of the caller to accomodate the two possible representations of a low to high transition.
    It can be either a 1 or a 0, the high to low transition being the opposite. Obviously, its value will also depend on the above pulse mixing
    at the start of the manchester encoded pulses.

    Here is a truth table for the most common case, the one where low to high is interpred as a 1:
     
            pulseIndex  secondPulse  nextBit
    ___|_-       1          true        1
    ___|-_       1         false        0
    ---|_-       1         false        1
    ---|-_       1          true        0

    If you wanted the low to high transition to represent a 0, simply invert the values of nextBit in the above table.

    When pulses are received, the last one is always a high one, the usual silence at the end being ignored and not given in the pulses
    array which means we will not have the final transition if it is from high to low.
    This decoder method takes this into account by considering that if there is one last bit to decode but no pulses left, then a final
    transition is implied and the next bit gets pushed into the frame.

    Bits are placed in the frame in the order they appear, ie MSB first. To illustrate, consider the following set of pulses:

    _-_-_-_--_-__--__-_--__-_-_--_-__-_-_-_-_-_-_--_-_-__-_--__-_--__--_-__--_-_-_-_-_-_-_-__-_-_-_-_--__--__--__--
     1 1 1 1 0 0 1 0 1 1 0 1 1 1 0 0 1 1 1 1 1 1 1 0 0 0 1 1 0 1 1 0 1 0 0 1 0 0 0 0 0 0 0 0 1 1 1 1 1 0 1 0 1 0 1 0
        F   |   2   |   D   |   C   |   F   |   E   |   3   |   6   |   9   |   0   |   0   |   F   |   A   |   A

    This is decoded as 56 bits, with the last one being decoded because of the "last pulse" rule mentionned above.
    Inside the frame parameter, you will thus receive the following 7 bytes:
      index    0  1  2  3  4  5  6 
      value   F2 DC FE 36 90 0F AA

*/
bool decode_manchester(uint8_t frame[], uint8_t expectedBitCount, uint16_t const pulses[], const int pulsesCount, int pulseIndex, uint8_t nextBit, bool secondPulse, uint16_t halfBitMinDuration, uint16_t halfBitMaxDuration);



namespace RFLink {
  namespace Utils {

    class BitArray {

    private:
      static const uint8_t _masks[8];
      static constexpr int _sizeOfStorageInBytes = 64;

    public:

      uint8_t storage[_sizeOfStorageInBytes];
      uint16_t currentSize;

      BitArray();

      inline bool fillFromPwmPulses(uint8_t expectedBitCount,
                                    uint16_t const pulses[],
                                    const int pulsesCount,
                                    int pulseIndex,
                                    uint16_t shortPulseMinDuration,
                                    uint16_t shortPulseMaxDuration,
                                    uint16_t longPulseMinDuration,
                                    uint16_t longPulseMaxDuration) {

        return decode_pwm(this->storage, expectedBitCount,
                          pulses, pulsesCount,
                          pulseIndex,
                          shortPulseMinDuration, shortPulseMaxDuration,
                          longPulseMinDuration, longPulseMaxDuration);

      }

      inline bool getBit(const uint16_t bitNumber) {
        return (storage[bitNumber / 8] & (0x80 >> (bitNumber%8))) != 0;
      }

      uint32_t getUInt(const uint16_t firstBitPosition, const uint16_t length);

    }; // end of BitArray class


  } // end of Utils namespace
} //end of RFLink namespace

#endif /* INCLUDE_UTIL_H_ */
