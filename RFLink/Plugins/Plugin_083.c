//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                     Plugin-083: Brel Motor                                            ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding and encoding the Brel Motor / Dooya protocol (DC318/DC114)
 * 
 * Author  (present)  : Sebastien LANGE
 * Support (present)  : https://github.com/basty149/RFLink 
 * Author  (original) : Sebastien LANGE
 * Support (original) : https://github.com/basty149/RFLink 
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical Information:
 * Decodes signals from a DOOYA receiver
 * DOOYA protocol
 * 1101 0110 1000 0000 1101 1111 1111 0000
 * AAAA AAAA AAAA AAAA AAAA AAAA BBBB CCCC
 *
 * A = Remote ID
 * B = Remote channel
 * C = Command (UP/STOP/DOWN/RESET)
 * 
 * Sample:
 * 20;XX;DEBUG;Pulses=82;Pulses(uSec)=4640,1504,192,640,192,640,512,320,192,608,512,320,192,640,480,320,480,352,160,640,512,320,192,640,160,640,192,640,160,640,160,640,192,640,512,320,480,320,160,640,160,640,480,320,160,640,160,640,160,672,160,640,192,640,192,640,192,640,192,640,192,640,192,640,512,352,160,640,160,640,160,640,512,320,512,320,512,320,480,320,160,4992;
 \*********************************************************************************************/
#define DOOYA_PLUGIN_ID 083
#define PLUGIN_DESC_083 PSTR("BRELMOTOR")
#define DOOYA_PULSECOUNT_1 82

#define DOOYA_MIDVALUE_D 384

#define DOOYA_UP_COMMAND    0x11 // 0001 0001
#define DOOYA_STOP_COMMAND  0x55 // 0101 0101
#define DOOYA_DOWN_COMMAND  0x33 // 0011 0011
#define DOOYA_SETUP_COMMAND 0xcc // 1100 1100


#ifdef PLUGIN_083
#include "../4_Display.h"

boolean Plugin_083(byte function, const char *string)
{
   const long DOOYA_MIDVALUE = DOOYA_MIDVALUE_D / RawSignal.Multiply;
   
   char dbuffer[64];

      if ( RawSignal.Number == DOOYA_PULSECOUNT_1 ) 
      {
         byte bbuffer[128];

#ifdef PLUGIN_083_DEBUG
         sprintf_P(dbuffer, PSTR("Pulses %04d Multiply %04d Delay %04d : "), 
            RawSignal.Number, RawSignal.Multiply, RawSignal.Delay);
         Serial.print(dbuffer);
#endif

         int buffer_index=0;

         // // Skip start [1] & [2] and end sequence
         for (int j = 3; j < RawSignal.Number-1; j+=8)
         {
            byte short_bitstream=0;
   
            for (int i = 0; i < 8; i+=2)
            {
               short_bitstream <<= 1; // Always shift

               // if (RawSignal.Pulses[j+i] < RawSignal.Pulses[j+i+1]) 
               if (RawSignal.Pulses[j+i] < DOOYA_MIDVALUE) 
               {
                  short_bitstream |= 0x0; // long bit = 0
               }
               else
               {
                  short_bitstream |= 0x1; // long bit = 1
               }
            }

            bbuffer[buffer_index++]=short_bitstream;

#ifdef PLUGIN_083_DEBUG
            sprintf(dbuffer, "0x%x ", short_bitstream);
            Serial.print(dbuffer);
#endif
         }

#ifdef PLUGIN_083_DEBUG
         sprintf_P(dbuffer, PSTR("END %02x%02x"), RawSignal.Pulses[RawSignal.Number-1], RawSignal.Pulses[RawSignal.Number]);
         Serial.print(dbuffer);
         Serial.print(F(";\r\n"));
#endif
         /**
          * bbuffer :
          * [0-5] : ID
          * [6-7] : Channel
          * [8-9] : COMMAND
          **/

         long address = bbuffer[0]*0x100000
                        +bbuffer[1]*0x10000
                        +bbuffer[2]*0x1000
                        +bbuffer[3]*0x100
                        +bbuffer[4]*0x10
                        +bbuffer[5];

         int command = bbuffer[8]*0x10+bbuffer[9];
         char * commandstring;

         switch (command)
         {
         case DOOYA_UP_COMMAND:
            commandstring = PSTR("CMD=UP");
            break;
         case DOOYA_DOWN_COMMAND:
            commandstring = PSTR("CMD=DOWN");
            break;
         case DOOYA_STOP_COMMAND:
            commandstring = PSTR("CMD=STOP");
            break;
         case DOOYA_SETUP_COMMAND:
            commandstring = PSTR("CMD=SETUP");
            break;
         default :
            return false;
         } 

         display_Header();
         display_Name(PSTR("BrelMotor"));
         display_IDn(address, 6);
         display_SWITCH(bbuffer[7]);
         display_Name(commandstring);
         display_Footer();
      } 
      else 
      {
#ifdef PLUGIN_083_DEBUG
         sprintf_P(dbuffer, PSTR("Pulses %04d END %02x%02x"), RawSignal.Number, RawSignal.Pulses[RawSignal.Number-1], RawSignal.Pulses[RawSignal.Number]);
         Serial.println(dbuffer);
#endif
         return false;
      }  

   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // PLUGIN_0083

#ifdef PLUGIN_TX_083
#include "../1_Radio.h"
#include "../2_Signal.h"
#include "../3_Serial.h"
#include <stdlib.h>

#define DOOYA_RFSTART_0 4512
#define DOOYA_RFSTART_1 1472
#define DOOYA_RFEND_0 4992
#define DOOYA_RFLOW   192
#define DOOYA_RFHIGH  608

/**
 * Convert Hex character to Hex value
 **/
byte hexchar2hexvalue(char c)
{
   if ((c>='0') && (c<='9'))
      return c-'0' ;
   if ((c>='A') && (c<='F'))
      return c+10-'A' ;
   if ((c>='a') && (c<='f'))
      return c+10-'a' ;
   return -1 ;
}

/*
 * Add High et Low signal pulses to pulses array.
 * Each bit from hex string will be encoded as High pulse and Low pulse
 * hexvalue : Hex String to encode.
 */
void addHighLowSignalPulses(unsigned long high, unsigned long low, 
   char * hexvalue, int *currrentPulses)  {

   int i=0;
   while (*(hexvalue+i) != '\0') 
   {
#ifdef PLUGIN_083_DEBUG
      Serial.print(*(hexvalue+i));
      Serial.print("(");
#endif

      byte value = hexchar2hexvalue(*(hexvalue+i));
      for (int x=0; x < 4; x++)
      {
         if ((value & B1000) == B1000)
         {
#ifdef PLUGIN_083_DEBUG
            Serial.print("1");
#endif
            RawSignal.Pulses[(*currrentPulses)++] = high / RawSignal.Multiply;
            RawSignal.Pulses[(*currrentPulses)++] = low / RawSignal.Multiply;
         }
         else
         {
#ifdef PLUGIN_083_DEBUG
            Serial.print("0");
#endif
            RawSignal.Pulses[(*currrentPulses)++] = low / RawSignal.Multiply;
            RawSignal.Pulses[(*currrentPulses)++] = high / RawSignal.Multiply;
         }
         value = value << 1;
      }
#ifdef PLUGIN_083_DEBUG
      Serial.print(")");
#endif

      i++;
   }
}

void debugRawSignal(int size) 
{
   char dbuffer[64];

   sprintf_P(dbuffer, PSTR("Pulses %04d Multiply %04d: "), 
            RawSignal.Number, RawSignal.Multiply, RawSignal.Time);
   Serial.print(dbuffer);

   for (int i=0; i<size; i++) 
   {
      sprintf_P(dbuffer, PSTR("%d "), RawSignal.Pulses[i]*RawSignal.Multiply);
      Serial.print(dbuffer);
   }

   Serial.println();
}

void sendRF(int currentPulses) 
{
   noInterrupts();
 
   for (int j=0; j < RawSignal.Repeats; j++) {
      for (int i = 0; i < currentPulses; i=i+2) 
      {
      digitalWrite(TX_DATA, HIGH);
      delayMicroseconds(RawSignal.Pulses[i]*RawSignal.Multiply);
      digitalWrite(TX_DATA, LOW);
      delayMicroseconds(RawSignal.Pulses[i+1]*RawSignal.Multiply);
      }
   }

   interrupts();
}

void addSinglePulse(unsigned long value, int *currrentPulses)
{
   RawSignal.Pulses[(*currrentPulses)++] = value / RawSignal.Multiply;
}

boolean  PluginTX_083(byte function, const char *string)
{
#ifdef PLUGIN_083_DEBUG
   Serial.println(F("PluginTX_083"));
#endif

   boolean success = false;
   unsigned long bitstream = 0L;

#ifdef PLUGIN_083_DEBUG
   char dbuffer[30] ;
   sprintf_P(dbuffer, PSTR("%s"), string);
   Serial.println(dbuffer);
#endif

   //20;XX;DEBUG;Pulses=82;Pulses(uSec)=4704,1472,192,608,192,608,512,288,192,608,512,288,192,608,512,288,512,320,192,608,512,288,192,608,192,608,192,608,192,608,192,608,192,640,512,288,512,320,192,640,192,640,512,288,192,640,192,608,192,640,192,640,192,608,192,640,192,640,512,320,512,320,512,320,512,352,160,640,192,640,192,640,512,320,512,320,512,320,512,320,192,4992;

   //20;XX;DEBUG;Pulses=82;Pulses(uSec)=4512,1472,192,608,192,608,512,288,192,608,512,288,192,608,512,288,512,320,192,608,512,288,192,608,192,608,192,608,192,608,192,608,192,640,512,288,512,288,192,608,192,608,512,288,192,608,192,608,192,640,192,608,192,608,192,608,192,608,512,288,512,288,512,288,512,320,192,608,512,288,192,608,512,288,192,608,512,288,192,608,512,4992;

   //10;BrelMotor;2b40c8;01;UP;
   //0123456789012345678901234567890123456789
   //0         10        20        30        40
   //
   // Pulses 0082 Multiply 0032 : 0x2 0xb 0x4 0x0 0xc 0x8 0x0 0xf 0x1 0xe END 069c;

   if (strncasecmp(string + 3, "BrelMotor;", 10) == 0)
   {
      int command;
      char *strings[10];
      char *ptr = NULL;

      // Tokenize input string
      byte index = 0;
      ptr = strtok(string, ";");  // takes a list of delimiters
      while(ptr != NULL)
      {
         strings[index++] = ptr;
         ptr = strtok(NULL, ";");  // takes a list of delimiters
      }

      char * address = strings[2]; 
      char * subaddress = strings[3];
      char * commandstring = strings[4];

#ifdef PLUGIN_083_DEBUG
      sprintf_P(dbuffer, PSTR("Send BrelMotor %s %s"), address, subaddress);
      Serial.println(dbuffer);
#endif

      if (strncasecmp(commandstring, "on", 2) == 0)
         command = DOOYA_UP_COMMAND;
      else if (strncasecmp(commandstring, "up", 2) == 0)
         command = DOOYA_UP_COMMAND;
      else if (strncasecmp(commandstring, "off", 3) == 0)
         command = DOOYA_DOWN_COMMAND;
      else if (strncasecmp(commandstring, "stop", 4) == 0)
         command = DOOYA_STOP_COMMAND;
      else if (strncasecmp(commandstring, "down", 4) == 0)
         command = DOOYA_DOWN_COMMAND;
      else if (strncasecmp(commandstring, "go_up", 5) == 0)
         command = DOOYA_UP_COMMAND;
      else if (strncasecmp(commandstring, "go_down", 7) == 0)
         command = DOOYA_DOWN_COMMAND;
      if (command == 0)
         return false;

#ifdef PLUGIN_083_DEBUG
      sprintf_P(dbuffer, PSTR("Send BrelMotor %s %s %s"), address, subaddress, command);
      Serial.println(dbuffer);
#endif

      RawSignal.Repeats = 5;
      RawSignal.Multiply = 32;

      int currentPulses=0;

      // add Headers
      addSinglePulse(DOOYA_RFSTART_0, &currentPulses);
      addSinglePulse(DOOYA_RFSTART_1, &currentPulses);
      // add Body
      addHighLowSignalPulses(DOOYA_RFHIGH, DOOYA_RFLOW, address, &currentPulses);
      addHighLowSignalPulses(DOOYA_RFHIGH, DOOYA_RFLOW, subaddress, &currentPulses);
      char buffercommand[3];
      sprintf_P(buffercommand, PSTR("%2x"), command);
      addHighLowSignalPulses(DOOYA_RFHIGH, DOOYA_RFLOW, buffercommand, &currentPulses);
      // add Footer
      addSinglePulse(0, &currentPulses);
      addSinglePulse(DOOYA_RFEND_0, &currentPulses);

      RawSignal.Number = currentPulses;
#ifdef PLUGIN_083_DEBUG
      debugRawSignal(currentPulses);
#endif

      // Amplify signal length from 32 (receipt) to 42 (emission)
      RawSignal.Multiply = 42;
      sendRF(currentPulses);

      return true;

   }
   return false;
}

#endif // PLUGIN_083

