#define SILVERCREST_PLUGIN_ID 016
#define PLUGIN_DESC_016 "Silvercrest remote controlled power sockets"
#define SerialDebugActivated

#ifdef PLUGIN_016
#include "../4_Display.h"
#include "../1_Radio.h"
#include "../7_Utils.h"

#define PLUGIN_016_ID "Silvercrest"
#define PLUGIN_016_DEBUG

const int SLVCR_BitCount = 24;
const int8_t SLVCR_CodeCount = 8;
const uint16_t SLVCR_OnCodes[SLVCR_CodeCount] = {0xf756, 0x7441, 0xd9c5, 0xe3aa, 0x6af3, 0x453f, 0x0f6e, 0xc170};
const uint16_t SLVCR_OffCodes[SLVCR_CodeCount] = {0x20e7, 0x5212, 0x9d88, 0x8c0b, 0x16bc, 0x3b99, 0xb8dd, 0xae24};

boolean Plugin_016(byte function, const char *string)
{
   const int SLVCR_MinPulses = 180;
   const int SLVCR_MaxPulses = 320;

   if (RawSignal.Number >= SLVCR_MinPulses && RawSignal.Number <= SLVCR_MaxPulses) 
   {
      const int SLVCR_StartPulseDuration = 2000 / RawSignal.Multiply;
      const int SLVCR_LongPulseMinDuration = 900 / RawSignal.Multiply;
      const int SLVCR_LongPulseMaxDuration = 1400 / RawSignal.Multiply;
      const int SLVCR_ShortPulseMinDuration = 200 / RawSignal.Multiply;
      const int SLVCR_ShortPulseMaxDuration = 600 / RawSignal.Multiply;
      const int SLVCR_PulsesCount = SLVCR_BitCount * 2;

      // Look for an overly long "off" pulse, followed by SLVCR_PulsesCount then again by an overly long "off" pulse
      // Ignore the first pulses, they are too noisy to be reliable
      int pulseIndex = 30; 
      while ((pulseIndex < RawSignal.Number - SLVCR_PulsesCount - 2) && (RawSignal.Pulses[pulseIndex] < SLVCR_StartPulseDuration)  && (RawSignal.Pulses[pulseIndex + SLVCR_PulsesCount + 2] < SLVCR_StartPulseDuration))
         pulseIndex++;
      if (pulseIndex % 2 != 0) 
      {
         #ifdef PLUGIN_016_DEBUG
         Serial.println(F(PLUGIN_016_ID ": Found, but not on an off Pulse"));
         #endif
         return false;
      }

      if (pulseIndex + SLVCR_PulsesCount < RawSignal.Number)
      {
         // found start pulse followed by at least 24 bits (2 pulses per bit)
         pulseIndex++;
         uint8_t packet[SLVCR_BitCount / 8] = {0, 0, 0};
         if (!decode_pwm(packet, SLVCR_BitCount, RawSignal.Pulses, RawSignal.Number, pulseIndex, SLVCR_ShortPulseMinDuration, SLVCR_ShortPulseMaxDuration, SLVCR_LongPulseMinDuration, SLVCR_LongPulseMaxDuration))   
         {
            #ifdef PLUGIN_016_DEBUG
            Serial.println(F(PLUGIN_016_ID ": Could not decode PWM!"));
            #endif
            return false;
            return false;
         }

        SerialDebugPrint(F(PLUGIN_016_ID ": packet = "));
        SerialDebugPrint(packet[0], 16);
        SerialDebugPrint(packet[1], 16);
        SerialDebugPrintln(packet[2], 16);

        // The button Id is in the second nibble of the last byte
        uint8_t buttonId = packet[2] & 0x0F;

        SerialDebugPrint("buttonId =");
        SerialDebugPrintln(buttonId, 16);

         // If button Id has bit 1 set, then invert the command
         enum CMD_OnOff effectiveCommandOn = CMD_On;
         enum CMD_OnOff effectiveCommandOff = CMD_Off;
         if (buttonId & 2)  
         {
            effectiveCommandOn = CMD_Off;  
            effectiveCommandOff = CMD_On;  
         }

        uint8_t remoteId = packet[0] >> 4;

         // Try to find the command bits inside our truth table
         uint16_t commandBits = ((uint16_t)(packet[0] & 0x0F) << 12)| ((uint16_t)packet[1]) << 4 | (packet[2] & 0xF0) >> 4; //(packet >> 4) & 0xFFFF;
         enum CMD_OnOff command = CMD_Unknown;
         byte commandCodeIndex = 0;
         while (commandCodeIndex < SLVCR_CodeCount && command == CMD_Unknown)
         {
            if (commandBits == SLVCR_OnCodes[commandCodeIndex])
               command = effectiveCommandOn;
            else if (commandBits == SLVCR_OffCodes[commandCodeIndex])
               command = effectiveCommandOff;

            commandCodeIndex++;
         }

         if (command == CMD_Unknown)
         {
            #ifdef PLUGIN_016_DEBUG
            Serial.print(F(PLUGIN_016_ID ": Unknown command bits: "));
            Serial.println(commandBits, 16);
            #endif
            return false;
         }

         // all is good, output the received packet
         display_Header();
         display_Name(PLUGIN_016_ID);
         //display_IDn(packet, 6); // global packet, to be removed later on
         display_IDn(remoteId, 1);  // remote Id
//         display_IDn(packet & 0x0F, 6);  // remote Id
         display_SWITCH(buttonId); // button Id
         display_CMD(false, command);
         display_Footer();
         RawSignal.Repeats = true; // suppress repeats of the same RF packet
         return true;
      }
      else
      {
         #ifdef PLUGIN_016_DEBUG
         Serial.print(F(PLUGIN_016_ID ": Start found, but not enough pulses left: "));
         Serial.print(pulseIndex + 2 * SLVCR_BitCount);
         Serial.print(F(" >= ")); 
         Serial.println(RawSignal.Number);
         #endif
      }
   }

   return false;
}
#endif //PLUGIN_016

#ifdef PLUGIN_TX_016

// 10;Silvercrest;ID=RemoteId;SWITCH=ButtonId;CMD=State
// 10;Silvercrest;ID=b;SWITCH=e;CMD=ON;   // Button C
// 10;Silvercrest;ID=b;SWITCH=7;CMD=OFF;  // Button D
// 10;Silvercrest;ID=b;SWITCH=7;CMD=ON;  // Button D
boolean PluginTX_016(byte function, const char *string)
{
   unsigned long remoteId = 0;
   byte buttonId = 0;
   byte buttonCommand = 0;

   retrieve_Init();

   if (!retrieve_Name("10"))
      return false;
   if (!retrieve_Name(PLUGIN_016_ID))
      return false;
   if (!retrieve_ID(remoteId))
      return false;
   if (!retrieve_Switch(buttonId))
      return false;
   if (!retrieve_Command(buttonCommand))
      return false;
   if (!retrieve_End())
      return false;

   #ifdef PLUGIN_016_DEBUG
   Serial.println(F(PLUGIN_016_ID ": End found, working")); 
   Serial.println(buttonId); 
   #endif

   const int PreambleHighTime = 320;
   const int PreambleLowTime = 2340;
   const int ZeroBitHighTime = 320;
   const int ZeroBitLowTime = 1180;
   const int OneBitHighTime = 1120;
   const int OneBitLowTime = 416;

   const byte RepeatCount = 4;

   uint16_t commandCode = 0;  

   byte codeIndex = 1 << ((buttonId & 0x1) << 1); // one of the first 4, or one of the last four, depending on the first bit of the buttonId
   const uint16_t* OnCommandArray = SLVCR_OnCodes;
   const uint16_t* OffCommandArray = SLVCR_OffCodes;
   if (buttonId & 2)  // invert if second bit is set
   {
      OnCommandArray = SLVCR_OffCodes;
      OffCommandArray = SLVCR_OnCodes;
   }

   switch(buttonCommand)
   {
      case VALUE_ON:
         commandCode = OnCommandArray[codeIndex];
         break;
      case VALUE_OFF:
         commandCode = OffCommandArray[codeIndex];
         break;
      default:
         return false;
   }

   int command = 0;
   command |= (remoteId & 0xF) << 20; 
   command |= commandCode << 4;
   command |= buttonId & 0xF;
   
   #ifdef PLUGIN_016_DEBUG
   Serial.print(F(PLUGIN_016_ID ": Sending command ")); 
   Serial.println(command, 16); 
   #endif

   for (byte repeatIndex = 0; repeatIndex < RepeatCount; repeatIndex++)
   {
      // Send preamble
      digitalWrite(Radio::pins::TX_DATA, HIGH);
      delayMicroseconds(PreambleHighTime);
      digitalWrite(Radio::pins::TX_DATA, LOW);
      delayMicroseconds(PreambleLowTime);

      // Send bits
      int bitMask = 1 << (SLVCR_BitCount - 1);
      for(int8_t bitIndex = 0; bitIndex < SLVCR_BitCount; bitIndex++)
      {
         int HighTime = ZeroBitHighTime;
         int LowTime = ZeroBitLowTime;
         if (command & bitMask)
         {
            HighTime = OneBitHighTime;
            LowTime = OneBitLowTime;
         }

         digitalWrite(Radio::pins::TX_DATA, HIGH);
         delayMicroseconds(HighTime);
         digitalWrite(Radio::pins::TX_DATA, LOW);
         delayMicroseconds(LowTime);

         bitMask >>= 1;
      }
   }

   return true;
}

#endif //PLUGIN_TX_016

