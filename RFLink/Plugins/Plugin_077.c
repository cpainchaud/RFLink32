#define AVANTEK_PLUGIN_ID 077
#define PLUGIN_DESC_077 "Avantek doorbells"

#ifdef PLUGIN_077
#include "../4_Display.h"
#include "../1_Radio.h"
#include "../7_Utils.h"

#define PLUGIN_077_ID "Avantek"
// #define PLUGIN_077_DEBUG
// #define MANCHESTER_DEBUG

#define AVTK_PULSE_DURATION_MID_D 480
#define AVTK_PULSE_DURATION_MIN_D 380
#define AVTK_PULSE_DURATION_MAX_D 580

#define BUTTONS_BITS_COUNT 4
// #define SWAP_BIT_ORDER_IF_SIGNAL_HAS_CRC

bool decode_manchester(uint8_t frame[], uint8_t expectedBitCount,
                       uint16_t const pulses[], const int pulsesCount,
                       int *pulseIndex, uint16_t shortPulseMinDuration,
                       uint16_t shortPulseMaxDuration,
                       uint16_t longPulseMinDuration,
                       uint16_t longPulseMaxDuration, uint8_t bitOffset,
                       uint8_t bitsPerByte, bool lsb) {
  if (*pulseIndex + (expectedBitCount - 1) * 2 > pulsesCount) {
#ifdef MANCHESTER_DEBUG
    Serial.print(F("MANCHESTER_DEBUG: Not enough pulses: *pulseIndex = "));
    Serial.print(*pulseIndex);
    Serial.print(F(" - expectedBitCount = "));
    Serial.print(expectedBitCount);
    Serial.print(F(" - pulsesCount = "));
    Serial.print(pulsesCount);
    Serial.print(F(" - min required pulses = "));
    Serial.println(*pulseIndex + expectedBitCount * 2);
#endif
    return false;
  }

  const uint8_t endBitCount = expectedBitCount + bitOffset;

  for (uint8_t bitIndex = bitOffset; bitIndex < endBitCount; bitIndex++) {
    bool isLast = bitIndex + 1 == endBitCount;
    int currentFrameByteIndex = bitIndex / bitsPerByte;
    uint8_t offset = bitIndex % bitsPerByte;
    if (offset == 0) {
      frame[currentFrameByteIndex] = 0;
    }
    uint16_t bitDuration0 = pulses[*pulseIndex];
    uint16_t bitDuration1 = pulses[*pulseIndex + 1];

    // TODO we could add parameter of manchester/inversed manchester
    if (value_between(bitDuration0, shortPulseMinDuration,
                       shortPulseMaxDuration) &&
        value_between(bitDuration1, longPulseMinDuration,
                       isLast ? static_cast<uint16_t>(UINT16_MAX) : longPulseMaxDuration)) {
      frame[currentFrameByteIndex] |=
          1 << (lsb ? (bitsPerByte - 1 - offset) : offset);
    } else if (!value_between(bitDuration0, longPulseMinDuration,
                               longPulseMaxDuration) ||
               !value_between(bitDuration1, shortPulseMinDuration,
                               isLast ? static_cast<uint16_t>(UINT16_MAX) : shortPulseMaxDuration)) {
#ifdef MANCHESTER_DEBUG
      Serial.print(F("MANCHESTER_DEBUG: Invalid duration at pulse "));
      Serial.print(*pulseIndex);
      Serial.print(F(" - bit "));
      Serial.print(bitIndex);
      Serial.print(F(": "));
      Serial.println(bitDuration0 * RFLink::Signal::RawSignal.Multiply);
#endif
      return false; // unexpected bit duration, invalid format
    }

    *pulseIndex += 2;
  }

  return true;
}

// TODO why can't  we use the function defined in 7_Utils?
inline bool value_between(uint16_t value, uint16_t min, uint16_t max) {
  return ((value > min) && (value < max));
}

inline bool isLowPulseIndex(const int pulseIndex) { return (pulseIndex % 2 == 0); }

u_short countPreamblePairs(const uint16_t pulses[], int *pulseIndex, size_t pulseCount, size_t AVTK_SyncPairsCount, uint16_t AVTK_PulseMinDuration, uint16_t AVTK_PulseMaxDuration) {
    u_short preamblePairsFound = 0;

    for (size_t i = 0; i < (size_t) AVTK_SyncPairsCount && *pulseIndex < (int) pulseCount - 1; i++, (*pulseIndex) += 2) {
        if (value_between(pulses[*pulseIndex], AVTK_PulseMinDuration, AVTK_PulseMaxDuration) &&
            value_between(pulses[(*pulseIndex) + 1], AVTK_PulseMinDuration, AVTK_PulseMaxDuration)) {
            preamblePairsFound++;
        } else if (preamblePairsFound > 0) {
            // if we didn't already have a match, we ignore as mismatch, otherwise we break here
            break;
        }
    }

    return preamblePairsFound;
}

uint8_t decode_bits(uint8_t frame[], const uint16_t *pulses,
                    const size_t pulsesCount, int *pulseIndex,
                    uint16_t pulseDuration, size_t bitsToRead) {
  size_t bitsRead = 0;

    for (; *pulseIndex < (int) pulsesCount && bitsRead < bitsToRead; (*pulseIndex)++) {
    size_t bits =
        (size_t)((pulses[*pulseIndex] + (pulseDuration / 2)) / pulseDuration);

    for (size_t j = 0; j < bits; j++) {
      frame[bitsRead / 8] <<= 1;
      if (!isLowPulseIndex(*pulseIndex)) {
        frame[bitsRead / 8] |= 1;
      }
      bitsRead++;
      if (bitsRead >= bitsToRead) {
        return j + 1;
      }
    }
  }

  // Check if there are enough bits read
  return bitsRead >= bitsToRead ? 0 : -1;
}

bool checkSyncWord(const unsigned char syncword[], const unsigned char pattern[], size_t length) {
  for (size_t i = 0; i < length; i++) {
    if (syncword[i] != pattern[i]) {
      return false;
    }
  }
  return true;
}

// TX
byte hexchar2hexvalue(char c)
{
   if ((c >= '0') && (c <= '9'))
      return c - '0';
   if ((c >= 'A') && (c <= 'F'))
      return c - 'A' + 10;
   if ((c >= 'a') && (c <= 'f'))
      return c - 'a' + 10;
   return -1 ;
}


// TX
bool* convertToBinary(const char* hex, size_t* resultSize)
{
    size_t len = strlen(hex);
    *resultSize = len * 4; // Each hex char is represented by 4 bits

    bool* binaryResult = (bool*)malloc(*resultSize * sizeof(bool));

    if (binaryResult == NULL)
    {
        Serial.print(F(PLUGIN_077_ID));
        Serial.println(F(": Memory allocation failed"));
        exit(EXIT_FAILURE);
    }

    size_t index = 0;

    for (size_t i = 0; i < len; i++)
    {
        char hexChar = hex[i];
        int hexValue = hexchar2hexvalue(hexChar);

        for (int j = 3; j >= 0; j--)
        {
            bool bit = (hexValue >> j) & 1;
            binaryResult[index++] = bit;
        }
    }

    return binaryResult;
}

boolean Plugin_077(byte function, const char *string)
{
   const u_short AVTK_SyncPairsCount = 8;
   const u_short AVTK_MinSyncPairs = 5;

  const int syncWordSize = 8;
  unsigned char syncwordChars[] = {0xCA, 0xCA, 0x53, 0x53};
  size_t syncwordLength = sizeof(syncwordChars) / sizeof(syncwordChars[0]);
  uint8_t syncword[syncwordLength];

  if (RawSignal.Number == 0)
  {
    return false;
  }

  const u_int16_t AVTK_PulseDuration = AVTK_PULSE_DURATION_MID_D / RawSignal.Multiply;
  const u_int16_t AVTK_PulseMinDuration = AVTK_PULSE_DURATION_MIN_D / RawSignal.Multiply;
  const u_int16_t AVTK_PulseMaxDuration = AVTK_PULSE_DURATION_MAX_D / RawSignal.Multiply;

  int pulseIndex = 1;

  while (pulseIndex + (int)(2 * AVTK_SyncPairsCount + syncWordSize) <
         RawSignal.Number) {
    u_short preamblePairsFound = countPreamblePairs(RawSignal.Pulses, &pulseIndex, RawSignal.Number, AVTK_SyncPairsCount, AVTK_PulseMinDuration, AVTK_PulseMaxDuration);

    if (preamblePairsFound < AVTK_MinSyncPairs) {
#ifdef PLUGIN_077_DEBUG
      Serial.print(F(PLUGIN_077_ID));
      Serial.print(F(": Preamble not found ("));
      Serial.print(preamblePairsFound);
      Serial.print(F(" < "));
      Serial.print(AVTK_MinSyncPairs);
      Serial.print(F("), pulseIndex is "));
      Serial.println(pulseIndex);
#endif
      continue;
    }
#ifdef PLUGIN_077_DEBUG
    Serial.print(F(PLUGIN_077_ID));
    Serial.print(F(": Preamble found ("));
    Serial.print(preamblePairsFound);
    Serial.print(F(" >= "));
    Serial.print(AVTK_MinSyncPairs);
    Serial.println(F("), pulseIndex is "));
    Serial.println(pulseIndex);
#endif

    for (size_t i = 0; i < syncwordLength; i++) syncword[i] = 0;
    uint8_t bitsProccessed =
        decode_bits(syncword, RawSignal.Pulses, RawSignal.Number, &pulseIndex,
                    AVTK_PULSE_DURATION_MID_D, 8 * syncwordLength);
    if (!bitsProccessed) {
#ifdef PLUGIN_077_DEBUG
      Serial.print(F(PLUGIN_077_ID));
      Serial.println(F(": Error on syncword decode"));
#endif
      continue;
    }

#ifdef PLUGIN_077_DEBUG
    Serial.print(F(PLUGIN_077_ID));
    Serial.print(F(": Syncword 0x"));
    for (size_t i = 0; i < syncwordLength; i++) {
      Serial.print(syncwordChars[i], HEX);
    }
#endif

    if (!checkSyncWord(syncword, syncwordChars, syncwordLength)) {
#ifdef PLUGIN_077_DEBUG
      Serial.println(F(" not found"));
#endif
      continue;
    }
#ifdef PLUGIN_077_DEBUG
      Serial.println(F(" found"));
#endif

    int alteredIndex = pulseIndex;
    uint16_t alteredValue = RawSignal.Pulses[alteredIndex];
    if (!isLowPulseIndex(pulseIndex)) {
      // the last pulse "decode_bits" processed was high
      RawSignal.Pulses[pulseIndex] =
          RawSignal.Pulses[pulseIndex] - bitsProccessed * AVTK_PulseDuration;
    }

    byte address[] = { 0, 0, 0, 0 };

    bool decodeResult = decode_manchester(
        address, 32, RawSignal.Pulses, RawSignal.Number, &pulseIndex, AVTK_PulseMinDuration,
        AVTK_PulseMaxDuration, 2 * AVTK_PulseMinDuration,
        2 * AVTK_PulseMaxDuration, 0, 8, true);
    RawSignal.Pulses[alteredIndex] = alteredValue;

    if (!decodeResult) {
#ifdef PLUGIN_077_DEBUG
      Serial.print(F(PLUGIN_077_ID));
      Serial.println(F(": Could not decode address manchester data"));
#endif
      continue;
    }
#ifdef PLUGIN_077_DEBUG
    Serial.print(F(PLUGIN_077_ID));
    Serial.print(F(": Address (lsb): "));
    Serial.print(address[0], HEX);
    Serial.print(F(" "));
    Serial.print(address[1], HEX);
    Serial.print(F(" "));
    Serial.print(address[2], HEX);
    Serial.print(F(" "));
    Serial.println(address[3], HEX);

    Serial.print(F(PLUGIN_077_ID));
    Serial.print(F(": pulseIndex is "));
    Serial.println(pulseIndex);
#endif

    byte buttons[] = { 0 };
    if (!decode_manchester(buttons, BUTTONS_BITS_COUNT, RawSignal.Pulses, RawSignal.Number, &pulseIndex,
                           AVTK_PulseMinDuration, AVTK_PulseMaxDuration,
                           2 * AVTK_PulseMinDuration, 2 * AVTK_PulseMaxDuration,
                           0, BUTTONS_BITS_COUNT, true)) {
#ifdef PLUGIN_077_DEBUG
      Serial.print(F(PLUGIN_077_ID));
      Serial.println(F(": Could not decode buttons manchester data"));
#endif
      continue;
    }

// TODO we would have to shift back the result because we shifted it too much to
// the left because we think that everything has 8 bits

    bool hasCrc = false;
    if (pulseIndex + 2 * AVTK_SyncPairsCount < RawSignal.Number) {
      short savedPulseIndex = pulseIndex;
      preamblePairsFound = countPreamblePairs(RawSignal.Pulses, &pulseIndex, RawSignal.Number, AVTK_SyncPairsCount, AVTK_PulseMinDuration, AVTK_PulseMaxDuration);
      pulseIndex = savedPulseIndex;
      hasCrc = preamblePairsFound < AVTK_SyncPairsCount;
    }

#ifdef SWAP_BIT_ORDER_IF_SIGNAL_HAS_CRC
    if (hasCrc) {
      pulseIndex -= (2 * BUTTONS_BITS_COUNT);
      if (!decode_manchester(buttons, BUTTONS_BITS_COUNT, RawSignal.Pulses, RawSignal.Number, &pulseIndex,
                            AVTK_PulseMinDuration, AVTK_PulseMaxDuration,
                            2 * AVTK_PulseMinDuration, 2 * AVTK_PulseMaxDuration,
                            0, BUTTONS_BITS_COUNT, false)) {
#ifdef PLUGIN_077_DEBUG
        Serial.print(F(PLUGIN_077_ID));
        Serial.println(F(": Could not (re)decode buttons manchester data"));
#endif
        continue;
      }
    }
#endif

#ifdef PLUGIN_077_DEBUG
    Serial.print(F(PLUGIN_077_ID));
    Serial.print(F(": Buttons: "));
    Serial.println(buttons[0], HEX);

    Serial.print(F(PLUGIN_077_ID));
    Serial.print(F(": pulseIndex is "));
    Serial.println(pulseIndex);
#endif

    if (hasCrc) {
      pulseIndex--;

      alteredIndex = pulseIndex;
      alteredValue = RawSignal.Pulses[alteredIndex];
      bool bitNr4IsSet = buttons[0] & 0b00010000; // 4th bit to the left, 0=110 (2x 1x), 1=100 (1x 2x)
      RawSignal.Pulses[alteredIndex] -= ((bitNr4IsSet ? 2 : 1) * AVTK_PulseDuration);

      byte crc[] = { 0 };
      decodeResult = decode_bits(crc, RawSignal.Pulses, RawSignal.Number, &pulseIndex, AVTK_PULSE_DURATION_MID_D, 8);
      RawSignal.Pulses[alteredIndex] = alteredValue;

      if (!decodeResult) {
#ifdef PLUGIN_077_DEBUG
        printf("Error on crc decode\n");
#endif
        continue;
      }
      pulseIndex += 2;

#ifdef PLUGIN_077_DEBUG
      Serial.print(F(PLUGIN_077_ID));
      Serial.print(F(": pulseIndex is "));
      Serial.println(pulseIndex);
      Serial.print(F(PLUGIN_077_ID));
      Serial.print(F(": CRC: 0x"));
      Serial.print(crc[0], HEX);
      Serial.println();

      Serial.print(F(PLUGIN_077_ID));
      Serial.print(F(": pulseIndex is "));
      Serial.println(pulseIndex);
#endif
    }

   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   unsigned long tmpval = (uint32_t)(((((uint32_t)address[0] << 24) | ((uint32_t)address[1] << 16) | ((uint32_t)address[2] << 8) | address[3]) << 8) | buttons[0]);
   if ((SignalHash != SignalHashPrevious) || ((RepeatingTimer + 1000) < millis()) || (SignalCRC != tmpval))
      SignalCRC = tmpval; // not seen the RF packet recently
   else
      return true; // already seen the RF packet recently
   //==================================================================================
   // Output
   //==================================================================================
    display_Header();
    display_Name(PLUGIN_077_ID);
    char c_ID[4 * 2 + 1];
    sprintf(c_ID, "%02x%02x%02x%02x", address[0], address[1], address[2], address[3]);
    display_IDc(c_ID);
    display_SWITCH(buttons[0]);
    display_Footer();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;     // do not process the packet any further

    return true;
  }

  return false;
}
#endif //PLUGIN_077

#ifdef PLUGIN_TX_077
#include "../1_Radio.h"
#include "../2_Signal.h"
#include "../3_Serial.h"
#include <stdlib.h>

inline void sendBit(boolean state)
{
   digitalWrite(Radio::pins::TX_DATA, state ? HIGH : LOW);
   delayMicroseconds(AVTK_PULSE_DURATION_MID_D);
}

inline void sendManchesterBit(bool bit)
{
  if (bit) {
    sendBit(true);
    sendBit(false);
    sendBit(false);
  } else {
    sendBit(true);
    sendBit(true);
    sendBit(false);
  }
}

void sendManchester(const char* data)
{
    for (size_t i = 0; i < strlen(data); i++) {
        int hexValue = hexchar2hexvalue(data[i]);
        for (int j = 3; j >= 0; j--) {
            bool bit = (hexValue >> j) & 1;
            sendManchesterBit(bit);
        }
    }
}

size_t preambleSize;
size_t syncWordSize;
bool* preamble = convertToBinary("aaaa", &preambleSize);
bool* syncWord = convertToBinary("caca5353", &syncWordSize);

const short sendTimes = 3;

boolean PluginTX_077(byte function, const char *string)
{
   //10;AVANTEK;71f1100080;1
   //01234567890123456789012
  if (string != NULL && strncasecmp(string + 3, "AVANTEK;", 8) == 0) {
    char *strings[4];
    char *ptr = NULL;

    // Tokenize input string
    int index = 0;
    ptr = strtok(const_cast<char*>(string), ";");
    while (ptr != NULL) {
        strings[index++] = ptr;
        ptr = strtok(NULL, ";");
    }

    char *address = strings[2];
    char *buttons = strings[3];

		noInterrupts();

#ifdef PLUGIN_077_DEBUG
    Serial.println(F(PLUGIN_077_ID ": Sending preamble"));
#endif
		for (u_short count = 0; count < sendTimes; count++) {
         for (u_int i = 0; i < preambleSize; i++) {
            sendBit(preamble[i]);
         }

#ifdef PLUGIN_077_DEBUG
         Serial.println(F(PLUGIN_077_ID ": Sending syncword"));
#endif
         for (u_int i = 0; i < syncWordSize; i++) {
            sendBit(syncWord[i]);
         }

#ifdef PLUGIN_077_DEBUG
         Serial.print(F(PLUGIN_077_ID ": Sending address "));
         Serial.println(address);
#endif

      sendManchester(address);
#ifdef PLUGIN_077_DEBUG
         Serial.print(F(PLUGIN_077_ID ": Sending buttons "));
         Serial.println(buttons);
#endif
      sendManchester(buttons);
		}
		interrupts();

    return true;
	}
  return false;
}

#endif //PLUGIN_TX_077
