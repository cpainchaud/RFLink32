#include "2_Signal.h"
#include "3_Serial.h"
#include "4_Display.h"
#include "7_Utils.h"
#include "11_Config.h"
#include "14_rtl_433Bridge.h"

// rtl_433 is built by the C compiler, so we must make sure the method
// definitions are imported unmangled
extern "C" {
#include "data.h"
#include "fatal.h"
#include "list.h"
#include "pulse_detect.h"
#include "pulse_demod.h"
#include "r_api.h"
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

        void processReceivedData()
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
        }
    }
}
