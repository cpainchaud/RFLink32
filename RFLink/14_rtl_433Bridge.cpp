/** @file
    A bridge between RFLink32 and the plugins from rtl_433

    Copyright (C) 2021 Olivier Sannier <obones (a) free (point) fr>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/
#include "2_Signal.h"
#include "3_Serial.h"
#include "4_Display.h"
#include "7_Utils.h"
#include "11_Config.h"
#include "14_rtl_433Bridge.h"
#include "14_rtl_433BridgeFieldMappings.h"

// rtl_433 is built by the C compiler, so we must make sure the method
// definitions are imported unmangled
extern "C" {
#include "data.h"
#include "fatal.h"
#include "list.h"
#include "pulse_detect.h"
#include "pulse_demod.h"
#include "r_api.h"
#include "r_util.h"
#include "r_device.h"
#include "rtl_433_devices.h"
}

namespace RFLink 
{ 
    namespace rtl_433Bridge 
    {
        const r_device r_devices[] = {
#define DECL(name) name,
            DEVICES
#undef DECL
/*            somfy_rts,
            lacrosse_tx35,
            tfa_pool_thermometer*/
        };
        const int num_r_devices = sizeof(r_devices) / sizeof(r_device);

        list_t r_devs = {0};

        void data_acquired_handler(r_device *r_dev, data_t *data)
        {
            /*Serial.print("rtl_433: received data from ");
            Serial.print(r_dev->name);
            Serial.println(":");*/

            display_Header();
            for (data_t *d = data; d; d = d->next) 
            {
                const field_mapping* mapping = get_field_mapping(d->key);
                switch (d->type)
                {
                    case DATA_STRING:
                        if (mapping && mapping->display_functions.str_display_fn)
                        {
                            const char* str = (const char*)(d->value.v_ptr);
                            mapping->display_functions.str_display_fn(str);
                        }
                        else
                        {
                            display_unknown_data(d);
                        }
                        break;
                    case DATA_DOUBLE:
                        if (mapping) 
                        {
                            double dblValue = d->value.v_dbl;

                            // Convert fields ending in _F to _C
                            if (str_endswith(d->key, "_F"))
                                dblValue = fahrenheit2celsius(dblValue);

                            // Convert fields ending in _mph or _mi_h to _kph
                            if (str_endswith(d->key, "_mph") || str_endswith(d->key, "_mi_h")) 
                                dblValue = mph2kmph(dblValue);

                            // Convert fields ending in _in to _mm
                            if (str_endswith(d->key, "_in") || str_endswith(d->key, "_inch"))
                                dblValue = inch2mm(dblValue);

                            // Convert fields ending in _in_h to _mm_h
                            if (str_endswith(d->key, "_in_h"))
                                dblValue = inch2mm(dblValue);

                            // Convert fields ending in _inHg to _hPa
                            if (str_endswith(d->key, "_inHg"))
                                dblValue = inhg2hpa(dblValue);

                            // Convert fields ending in _PSI to _hPa
                            if (str_endswith(d->key, "_PSI")) 
                                dblValue = psi2kpa(dblValue) * 10.0;

                            // Convert fields ending in _m_s in _kph
                            if (str_endswith(d->key, "_m_s"))
                                dblValue = 3.6 * dblValue;

                            if (mapping->display_functions.double_display_fn)
                            {
                                mapping->display_functions.double_display_fn((mapping->multiply_value_by_ten) ? dblValue * 10.0: dblValue);
                            }
                            else 
                            {
                                int intValue = (mapping->multiply_value_by_ten) ? multiply_by_ten(dblValue) : (unsigned int)round(dblValue);

                                if (mapping->display_functions.uint_display_fn)
                                    mapping->display_functions.uint_display_fn(intValue);
                                else if (mapping->display_functions.ulong_display_fn)
                                    mapping->display_functions.ulong_display_fn(intValue);
                                else if (mapping->display_functions.byte_display_fn)
                                    mapping->display_functions.byte_display_fn(intValue);
                                else
                                    display_unknown_data(d);
                            }
                        }
                        else
                        {
                            display_unknown_data(d);
                        }
                        break;
                    case DATA_INT:
                        if (mapping) 
                        {
                            unsigned int intValue = d->value.v_int;
                            if (mapping->multiply_value_by_ten)
                                intValue = intValue * 10;

                            if (mapping->display_functions.uint_display_fn)
                                mapping->display_functions.uint_display_fn(intValue);
                            else if (mapping->display_functions.ulong_display_fn)
                                mapping->display_functions.ulong_display_fn(intValue);
                            else if (mapping->display_functions.byte_display_fn)
                                mapping->display_functions.byte_display_fn(intValue);
                            else
                                display_unknown_data(d);
                        }
                        else
                        {
                            display_unknown_data(d);
                        }
                        break;
                    case DATA_DATA:
                    case DATA_ARRAY:
                    case DATA_COND:
                    case DATA_COUNT:
                    case DATA_FORMAT:
                        break;
                }
            }
            display_Footer();

            const int bufferLength = 1024;
            char buffer[bufferLength];
            data_print_jsons(data, buffer, bufferLength);
            //Serial.println(buffer);

            // Can't use display_XXX, its buffer is way too small
            Serial.print("20;XX;");
            Serial.print("rtl_433;");
            Serial.print("ID=");
            Serial.print(r_dev->protocol_num, 4);
            Serial.print(';');
            Serial.print(buffer);
            Serial.println(';');
            /*display_Header();
            display_Name("rtl_433");
            display_IDn(r_dev->protocol_num, 8);
            display_Name(buffer);
            display_Footer();
            */
            data_free(data);
            
            RFLink::Signal::counters::successfullyDecodedSignalsCount++;
            RFLink::sendMsgFromBuffer();
        }

        void register_protocol(const r_device *r_dev, char *arg)
        {
            r_device *p;
            if (r_dev->create_fn) {
                p = r_dev->create_fn(arg);
            }
            else {
                if (arg && *arg) {
                    fprintf(stderr, "Protocol [%u] \"%s\" does not take arguments \"%s\"!\n", r_dev->protocol_num, r_dev->name, arg);
                }
                p  = (r_device *)malloc(sizeof(*p));
                if (!p)
                    FATAL_CALLOC("register_protocol()");
                *p = *r_dev; // copy
            }

            // cfg->verbosity has these meanings: 0=normal, 1=verbose, 2=verbose decoders, 3=debug decoders, 4=trace decoding.    
            // r_device->verbose is thus the same value, minus one
            p->verbose      = 0; //cfg->verbosity > 0 ? cfg->verbosity - 1 : 0; 
            p->verbose_bits = 0; //cfg->verbose_bits;

            //p->old_model_keys = cfg->old_model_keys; // TODO: temporary allow to change to new style model keys

            p->output_fn  = data_acquired_handler;
            p->output_ctx = NULL;//cfg;
            p->protocol_num = r_devs.len;

            list_push(&r_devs, p);

            /*if (cfg->verbosity) {
                fprintf(stderr, "Registering protocol [%u] \"%s\"\n", r_dev->protocol_num, r_dev->name);
            }*/
        }

        void register_all_protocols(unsigned disabled)
        {
            list_ensure_size(&r_devs, 100);
            for (int i = 0; i < num_r_devices; i++) 
            {
                // register all device protocols that are not disabled
                if (r_devices[i].disabled <= disabled) 
                {
                    register_protocol(&r_devices[i], NULL);
                }
            }
        }

        int processReceivedData()
        {
            //Serial.println("rtl_433 trying to process messages");
            /*for (int deviceIndex = 0; deviceIndex < num_r_devices; deviceIndex++)
              Serial.println(r_devices[deviceIndex].name);*/

            static pulse_data_t pulseData = {0}; 
            int dataPulseIndex = 0;

            for (int pulseIndex = 0; pulseIndex < Signal::RawSignal.Number; pulseIndex++)
            {
                int pulseDuration = Signal::RawSignal.Pulses[pulseIndex] * Signal::RawSignal.Multiply;
                if (PulseIsHigh(pulseIndex))
                {
                    pulseData.pulse[dataPulseIndex] = pulseDuration;
                }
                else
                {
                    pulseData.gap[dataPulseIndex] = pulseDuration;
                    dataPulseIndex++;
                }
            }
            pulseData.num_pulses = dataPulseIndex;
            pulseData.sample_rate = 1.0e6;


            //Serial.println("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
            int decodedCount = run_ook_demods(&r_devs, &pulseData);
            //int decodedCount = pulse_demod_ppm(pulseData, &tfa_pool_thermometer);
            //int decodedCount = pulse_demod_ppm(NULL, &tfa_pool_thermometer);
            //int decodedCount = pulse_demod_ppm(pulseData, NULL);
            //Serial.printf("rtl_433 decoded %d messages", decodedCount);
            //Serial.println();

            //Serial.printf("stack free: %d", uxTaskGetStackHighWaterMark(NULL));
            //Serial.println();

            return decodedCount;
        }
    }
}
