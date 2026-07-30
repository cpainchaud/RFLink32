//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                            Plugin-051: Hyundai WS Senzor 77(TH)                                   ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding Hyundai WS Senzor 77(TH)
 *
 * Author  (present)  : Michal "Micu" Cieslakiewicz, June 2025
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical Information:
 *
 * Decodes signals from Hyundai WS Senzor 77 & 77TH outdoor units (36 bits, 433.92 MHz).
 * '77' is temperature-only sensor while '77TH' also reports humidity.
 *
 * Product pages (in Polish):
 * https://www.hyundai-electronics.pl/collections/akcesoria-do-stacji-meteo-26427/products/czujniki-dla-stacji-meteo-hyundai-ws-senzor-77-szare-hyuwssenzor77
 * https://www.hyundai-electronics.pl/collections/akcesoria-do-stacji-meteo-26427/products/czujniki-dla-stacji-meteo-hyundai-ws-senzor-77-th-szare-hyuwssenzor77th
 *
 * Weather conditions monitored:
 * + Temperature in Celsius with 0.1 degree precision (on display)
 * + Humidity in percent in 1-point steps (on display) - valid for 77TH only
 * + Temperature trend - down/stable/up (not included in display)
 * Some units have an optional device number (aka channel) switch to select ID 1-3. Otherwise it is hardcoded to 1.
 * Internal switch for temperature units (C/F) affects unit display only and is not reflected in packet payload
 * - temperature is always transmitted in Celsius.
 * Device has an analog input for external temperature probe. There is no indication in data whether unit uses
 * internal or external sensor.
 *
 * Method of detecting unit type based on packet contents is unknown - there are no specific bits that potentially
 * can indicate if device supports humidity reading or not. For '77' humidity field is still used but it does not
 * contain actual measurement - observed values are 0x1D, 0x9D or 0xED with no apparent meaning.
 *
 * Checksum is suspected to be located at last 4 bits but its algorithm is unknown (not standard CRC-4).
 *
 * Timing:
 *
 * '77' unit emits 4 36-bit packets every 68 seconds.
 * '77TH' unit emits 4 36-bit packets every 33 seconds.
 *
 * Modulation:
 *
 *    _        _
 *   | |      | |
 *   | |      | |
 *   | |___   | |______
 *    p  4p    p   8p
 *      0         1
 *
 * Data:
 *
 *   IIII NNII BDDx TTTT TTTT TTTT HHHH HHHH CCCC
 *
 *     I - random system id (6 bits), changed every power cycle (like battery replacement)
 *     N - sensor number aka "channel" (2 bits): 1-3
 *     B - battery low indicator (1 bit): 0 means good power source
 *     D - temperature trend (2 bit): 0-stable, 1-up, 2-down
 *     x - reserved (1 bit): always 0
 *     T - temperature (12 bits): reversed signed u2 * 10
 *     H - humidity (8 bits): reversed signed u2 - 100
 *     C - checksum (4 bits), unknown algorithm
 *
 * Transmission:
 *
 * Most of the time RFLink picks up 4 messages, usually having 74, 64 and two times 66 pulses (see example below).
 * Last high pulse is terminating one.
 *
 * Pulses=74    bits 35..0 (data: 2*36 pulses; terminating high: 1; rflink timeout value: 1) GOOD
 * Pulses=64    bits 30..0, missing 5 MSB (system ID & channel part) (data: 2*31 pulses; terminating high: 1; rflink timeout value: 1)
 * Pulses=66    bits 31..0, missing 4 MSB (system ID part), (data: 2*32 pulses; terminating high: 1; rflink timeout value: 1)
 *
 * Sample (sync):
 * 20;XX;DEBUG;Pulses=74;Pulses(uSec)=609,3905,563,3964,541,3983,543,3969,547,3962,556,1889,533,3977,543,3965,553,1894,531,3978,543,1900,529,1914,515,3993,531,3975,543,1902,528,1914,516,3990,535,3975,541,3968,548,3963,549,1896,532,1910,520,1922,511,1931,503,1935,501,1936,503,1935,502,4001,525,1916,518,1924,509,3993,532,3978,541,1901,525,1917,514,3989,533,2034,521,3957;
* 20;XX;DEBUG;Pulses=64;Pulses(uSec)=547,1892,531,3977,545,3967,548,1899,527,3980,543,1903,522,1919,514,3990,534,3977,543,1902,526,1916,516,3990,534,3976,541,3972,542,3971,547,1897,530,1913,519,1921,511,1931,507,1935,498,1936,503,1936,500,4001,527,1918,514,1926,507,3994,532,3980,540,1903,524,1917,516,3988,535,2033,519,3978;
* 20;XX;DEBUG;Pulses=66;Pulses(uSec)=542,3962,550,1894,533,3975,543,3969,550,1894,532,3980,540,1903,525,1916,514,3993,531,3977,543,1903,524,1920,511,3993,532,3978,542,3968,546,3965,549,1898,525,1914,519,1923,511,1930,505,1935,500,1935,505,1935,502,4002,522,1919,514,1928,508,3994,531,3977,542,1903,525,1916,515,3993,535,2030,521,1901;
* 20;XX;DEBUG;Pulses=66;Pulses(uSec)=544,3960,552,1892,533,3975,544,3969,548,1895,528,3980,542,1905,520,1922,509,3994,532,3975,545,1900,530,1912,516,3992,532,3976,543,3967,547,3967,549,1895,528,1912,519,1924,508,1933,504,1933,503,1938,496,1940,499,4001,528,1917,511,1927,510,3995,531,3977,542,1906,521,1919,515,3991,530,2036,518,1901;
 * Sample (async):
 * 20;XX;DEBUG;Pulses=74;Pulses(uSec)=609,3920,569,3957,540,3989,540,3971,548,3965,555,1893,536,3978,546,3968,549,1898,534,3978,543,1904,530,1916,518,3994,533,1910,522,3987,542,3972,547,3968,548,1899,531,1916,519,3988,537,1912,521,1921,515,1931,512,1930,506,3999,531,1914,519,3990,539,3976,544,3968,552,1898,529,3982,542,3972,549,1897,535,3978,539,3979,544,4089,552,5000;
 * 20;XX;DEBUG;Pulses=64;Pulses(uSec)=556,1891,539,3973,546,3969,549,1899,530,3980,545,1904,528,1916,518,3992,538,1907,527,3982,540,3975,544,3970,549,1898,535,1915,516,3990,538,1909,521,1923,515,1930,507,1933,507,4000,532,1914,518,3994,532,3976,545,3971,548,1899,531,3981,545,3970,548,1899,534,3976,542,3972,548,4090,552,5000;
 * 20;XX;DEBUG;Pulses=66;Pulses(uSec)=559,3959,559,1888,535,3978,546,3970,549,1897,532,3981,542,1904,529,1918,515,3992,536,1911,521,3991,536,3976,544,3969,548,1902,528,1914,521,3990,538,1908,523,1922,513,1934,506,1932,509,3997,531,1914,524,3986,541,3975,542,3969,548,1901,530,3982,543,3970,547,1901,531,3981,541,3970,548,4090,552,5000;
 * 20;XX;DEBUG;Pulses=66;Pulses(uSec)=558,3961,551,1896,534,3979,546,3968,549,1897,532,3980,542,1908,525,1919,519,3987,538,1911,519,3988,542,3971,551,3970,543,1899,533,1918,515,3989,536,1911,524,1921,513,1929,511,1934,505,4000,531,1914,521,3988,537,3977,543,3974,546,1901,528,3981,542,3972,546,1901,530,3983,541,3971,549,4088,555,5000;
 *
 \*********************************************************************************************/

#define HYWS77TH_PLUGIN_ID 051
#define PLUGIN_DESC_051 "Hyundai WS Senzor 77TH"

#ifdef PLUGIN_051

#include "../4_Display.h"

#define PLUGIN_051_ID "HyWS77TH"
#define HYWS77TH_PULSECOUNT 74

//#define PLUGIN_051_DEBUG

#define HYWS77TH_HIGH_PULSE_MIN_D 480
#define HYWS77TH_HIGH_PULSE_MAX_D 580
#define HYWS77TH_HIGH_1ST_PULSE_MIN_D 560
#define HYWS77TH_HIGH_1ST_PULSE_MAX_D 640
#define HYWS77TH_LOW_SHORT_PULSE_MIN_D 1800
#define HYWS77TH_LOW_SHORT_PULSE_MAX_D 2100
#define HYWS77TH_LOW_LONG_PULSE_MIN_D 3800
#define HYWS77TH_LOW_LONG_PULSE_MAX_D 4200

boolean Plugin_051(byte function, const char *string)
{
  uint16_t hdr;
  uint8_t id, ch, batlow, trend;
  uint16_t tmpu;
  int16_t temp;
  uint8_t hum;
  uint16_t mask;
  uint32_t bitstream;

  if (RawSignal.Number != HYWS77TH_PULSECOUNT)
    return false;

  #ifdef PLUGIN_051_DEBUG
    Serial.println(PLUGIN_051_ID ": Pulsecount OK");
  #endif

  const unsigned int HYWS77TH_HIGH_PULSE_MIN = HYWS77TH_HIGH_PULSE_MIN_D / RawSignal.Multiply;
  const unsigned int HYWS77TH_HIGH_PULSE_MAX = HYWS77TH_HIGH_PULSE_MAX_D / RawSignal.Multiply;
  const unsigned int HYWS77TH_HIGH_1ST_PULSE_MIN = HYWS77TH_HIGH_1ST_PULSE_MIN_D / RawSignal.Multiply;
  const unsigned int HYWS77TH_HIGH_1ST_PULSE_MAX = HYWS77TH_HIGH_1ST_PULSE_MAX_D / RawSignal.Multiply;
  const unsigned int HYWS77TH_LOW_SHORT_PULSE_MIN = HYWS77TH_LOW_SHORT_PULSE_MIN_D / RawSignal.Multiply;
  const unsigned int HYWS77TH_LOW_SHORT_PULSE_MAX = HYWS77TH_LOW_SHORT_PULSE_MAX_D / RawSignal.Multiply;
  const unsigned int HYWS77TH_LOW_LONG_PULSE_MIN = HYWS77TH_LOW_LONG_PULSE_MIN_D / RawSignal.Multiply;
  const unsigned int HYWS77TH_LOW_LONG_PULSE_MAX = HYWS77TH_LOW_LONG_PULSE_MAX_D / RawSignal.Multiply;

  #ifdef PLUGIN_051_DEBUG
    #define DBGBUFLEN 80
    char dbgbuf[DBGBUFLEN];
  #endif

  //==================================================================================
  // Perform a pre sanity check
  //==================================================================================
  if (RawSignal.Pulses[1] < HYWS77TH_HIGH_1ST_PULSE_MIN || RawSignal.Pulses[1] > HYWS77TH_HIGH_1ST_PULSE_MAX) {
    #ifdef PLUGIN_051_DEBUG
      snprintf(dbgbuf, DBGBUFLEN, "%s: 1st H pulse out of range: %d (%d<=x<=%d)", PLUGIN_051_ID, RawSignal.Pulses[1], HYWS77TH_HIGH_1ST_PULSE_MIN, HYWS77TH_HIGH_1ST_PULSE_MAX);
      Serial.println(dbgbuf);
    #endif
    return false;  // First decoded pulse (high) is a little longer one (by ~70-80 usecs)
  }
  byte i;
  for (i = 2; i < RawSignal.Number; i += 2)
  {
    if (RawSignal.Pulses[i] < HYWS77TH_LOW_SHORT_PULSE_MIN || RawSignal.Pulses[i] > HYWS77TH_LOW_LONG_PULSE_MAX) {
      #ifdef PLUGIN_051_DEBUG
        snprintf(dbgbuf, DBGBUFLEN, "%s: L pulse #%d out of range: %d (%d<=x<=%d)", PLUGIN_051_ID, i, RawSignal.Pulses[i], HYWS77TH_LOW_SHORT_PULSE_MIN, HYWS77TH_LOW_LONG_PULSE_MAX);
        Serial.println(dbgbuf);
      #endif
      return false;
    }
    if (RawSignal.Pulses[i] > HYWS77TH_LOW_SHORT_PULSE_MAX && RawSignal.Pulses[i] < HYWS77TH_LOW_LONG_PULSE_MIN) {
      #ifdef PLUGIN_051_DEBUG
        snprintf(dbgbuf, DBGBUFLEN, "%s: L pulse #%d invalid length: %d (x<=%d||x>=%d)", PLUGIN_051_ID, i, RawSignal.Pulses[i], HYWS77TH_LOW_SHORT_PULSE_MAX, HYWS77TH_LOW_LONG_PULSE_MIN);
        Serial.println(dbgbuf);
      #endif
      return false;
    }
    if (RawSignal.Pulses[i + 1] < HYWS77TH_HIGH_PULSE_MIN || RawSignal.Pulses[i + 1] > HYWS77TH_HIGH_PULSE_MAX) {
      #ifdef PLUGIN_051_DEBUG
        snprintf(dbgbuf, DBGBUFLEN, "%s: H pulse #%d out of range: %d (%d<=x<=%d)", PLUGIN_051_ID, i + 1, RawSignal.Pulses[i + 1], HYWS77TH_HIGH_PULSE_MIN, HYWS77TH_HIGH_PULSE_MAX);
        Serial.println(dbgbuf);
      #endif
      return false;
    }
  }

  //==================================================================================
  // Get all 36 bits
  //==================================================================================

  // (skipping 4-bit CRC for now)
  // humidity: 8-bit reversed
  mask = 1;
  hum = 0;
  for (i = RawSignal.Number - 24; i < RawSignal.Number - 8 ; i += 2)
  {
    if (RawSignal.Pulses[i] >= HYWS77TH_LOW_LONG_PULSE_MIN && RawSignal.Pulses[i] <= HYWS77TH_LOW_LONG_PULSE_MAX)
      hum |= mask;
    mask <<= 1;
  }
  hum = 100 - (~hum + 1);

  // temperature: 12-bit reversed
  mask = 1;
  tmpu = 0;
  for (i = RawSignal.Number - 48; i < RawSignal.Number - 24 ; i += 2)
  {
    if (RawSignal.Pulses[i] >= HYWS77TH_LOW_LONG_PULSE_MIN && RawSignal.Pulses[i] <= HYWS77TH_LOW_LONG_PULSE_MAX)
      tmpu |= mask;
    mask <<= 1;
  }
  if (tmpu & 0x0800)
    temp = -(~tmpu + 1);
  else
    temp = tmpu;

  // remaining fields in header: 12-bit
  mask = 1;
  hdr = 0;
  for (i = RawSignal.Number - 50; i > 0 ; i -= 2)
  {
    if (RawSignal.Pulses[i] >= HYWS77TH_LOW_LONG_PULSE_MIN && RawSignal.Pulses[i] <= HYWS77TH_LOW_LONG_PULSE_MAX)
      hdr |= mask;
    mask <<= 1;
  }
  id = (hdr & 0x0ff0) >> 4;  // device ID includes channel
  ch = (hdr & 0x00c0) >> 6 ;
  batlow = (hdr & 0x0008) >> 3;
  trend = (hdr & 0x0006) >> 1;

  //==================================================================================
  // Prevent repeating signals from showing up
  //==================================================================================
  bitstream = (hdr << 20) | (tmpu << 8) | hum;  // reconstruct bitsteam-like word
  if ((SignalHash != SignalHashPrevious) || (RepeatingTimer + 600 < millis()) || (SignalCRC != bitstream))
    SignalCRC = bitstream; // not seen the RF packet recently
  else {
    #ifdef PLUGIN_051_DEBUG
      Serial.println(PLUGIN_051_ID ": Duplicated packet detected");
    #endif
    return true; // already seen the RF packet recently
  }

  //==================================================================================
  // Output
  //==================================================================================
  display_Header();
  display_Name(PSTR(PLUGIN_051_ID));
  display_IDn(id, 2);
  display_CHAN(ch);
  display_TEMPD(temp);
  display_TREND(trend);
  display_HUM(hum);
  display_BAT(batlow == 0);
  display_Footer();

  RawSignal.Repeats = true; // suppress repeats of the same RF packet
  RawSignal.Number = 0;
  return true;
}

#endif // PLUGIN_051
