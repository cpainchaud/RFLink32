/**
Fine Offset Electronics WH2 Temperature/Humidity sensor protocol,
also Agimex Rosenborg 66796 (sold in Denmark), collides with WH5,
also ClimeMET CM9088 (Sold in UK),
also TFA Dostmann/Wertheim 30.3157 (Temperature only!) (sold in Germany).

The sensor sends two identical packages of 48 bits each ~48s. The bits are PWM modulated with On Off Keying.

The data is grouped in 6 bytes / 12 nibbles.

    [pre] [pre] [type] [id] [id] [temp] [temp] [temp] [humi] [humi] [crc] [crc]

There is an extra, unidentified 7th byte in WH2A packages.

- pre is always 0xFF
- type is always 0x4 (may be different for different sensor type?)
- id is a random id that is generated when the sensor starts
- temp is 12 bit signed magnitude scaled by 10 celsius
- humi is 8 bit relative humidity percentage

Based on 
http://lucsmall.com/2012/04/29/weather-station-hacking-part-2/
and
https://github.com/lucsmall/WH2-Weather-Sensor-Library-for-Arduino

Comment detailing protocol from rtl_433 project.
*/
#define FINEOFFSET_PLUGIN_ID 050
#define PLUGIN_DESC_050 "FineOffset Temp/Humidity sensors"

#ifdef PLUGIN_050
#include "../4_Display.h"
#include "../1_Radio.h"
#include "../7_Utils.h"

#define PLUGIN_050_ID "FineOffset"

#define FINEOFFSET_PULSE_COUNT 96

#ifdef PLUGIN_050_DEBUG
#define SerialDebugActivated
#endif

boolean Plugin_050(byte function, const char *string)
{
    if (RawSignal.Number == FINEOFFSET_PULSE_COUNT) 
    {
        const int CM_LongLowMinDuration = 1200 / RawSignal.Multiply;
        const int CM_LongLowMaxDuration = 1600 / RawSignal.Multiply;
        const int CM_ShortHighMinDuration = 300 / RawSignal.Multiply;
        const int CM_ShortHighMaxDuration = 600 / RawSignal.Multiply;
        byte data[6] = {0, 0, 0, 0, 0, 0};
        //Skip the first bit as the lengths tend to vary...
        if(!decode_pwm(data, 47, RawSignal.Pulses,RawSignal.Number, 3, CM_ShortHighMinDuration, CM_ShortHighMaxDuration, CM_LongLowMinDuration, CM_LongLowMaxDuration, 1))
            return false;
        invert_bytes(data, 6);
        byte calculated_crc = crc8(data + 1, 4, 0x31, 0);
#ifdef PLUGIN_050_DEBUG
        const size_t buflen = sizeof(PLUGIN_050_ID ": packet = ") + 32;
        char printbuf[buflen];
        snprintf(printbuf, buflen, "%s%02x%02x%02x%02x%02x%02x, CRC=%02x", PLUGIN_088_ID ": packet = ", data[0], data[1], data[2], data[3],data[4], data[5], calculated_crc);
        SerialDebugPrintln(printbuf);
#endif
        if(calculated_crc != data[5])
        {
#ifdef PLUGIN_050_DEBUG
            SerialDebugPrintln(PLUGIN_050_ID ": CRC Check Failed");
#endif
            return false;
        }
        
        uint16_t unitid = (data[1] << 4) | (data[2] >> 4);
        uint16_t temperature = ((data[2] & 0xF) << 8) | data[3];
        //MSB of 12-bit number actually indicates sign, this isn't standard two's complement
        if(temperature & 0x800)
            temperature = - (temperature & 0x7FF);
        byte humidity = data[4];
        display_Header();
        display_Name(PLUGIN_050_ID);
        display_IDn(unitid, 4);  // unit id
        display_TEMP(temperature); 
        display_HUM(humidity); 
        display_Footer();
    }
    return false;
}
#endif
