//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                 Plugin-071 Plieger York doorbell                                   ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding the Plieger York Doorbell protocol
 * 
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : StuntTeam 2015..2016
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical Information:
 * Decodes signals from a Plieger York Doorbell, (66 pulses, 32 bits, 433 MHz).
 * Plieger Message Format: 
 * 0000000001010101 00000000 00011100    c2  0x1c
 *                           00000011    c3  0x03
 *                           11100000    c1  0xE0
 *                           --------   8 bits chime number (3 chimes, can be changed with a jumped on the transmitter) 
 *                  -------- 8 bits always 0 
 * ---------------- 16 bits code which can be changed with a button on the inside of the transmitter 
 *
 * Note: The transmitter sends two times the same packet when the bell button is pressed
 * the retransmit is killed to prevent reporting the same press twice
 *
 * Sample packet: (Nodo Pulse timing)
 * Pulses=66, Pulses(uSec)=700,250,275,725,750,250,275,725,750,250,275,725,750,250,275,725,750,250,
 * 275,725,750,250,275,725,750,250,275,725,750,250,275,725,275,725,275,725,275,725,275,725,275,725,
 * 275,725,275,725,275,725,275,725,275,725,275,725,750,250,750,250,750,250,275,725,275,725,225,
 * 20;8C;DEBUG;Pulses=66;Pulses(uSec)=1800,550,600,1500,1600,550,600,1500,1600,550,600,1500,1600,550,600,1500,1600,550,600,1500,1600,500,600,1500,1600,550,600,1550,1600,550,600,1500,600,1500,600,1500,600,1500,600,1500,600,1500,600,1500,600,1500,600,1500,600,1500,600,1500,600,1500,1600,550,1600,500,1600,550,600,1500,600,1500,450;
 * 20;2D;DEBUG;Pulses=66;Pulses(uSec)=875,275,300,750,800,275,300,750,800,275,300,750,800,275,300,750,800,275,300,750,800,250,300,750,800,275,275,750,800,275,300,750,300,750,300,750,300,750,300,750,300,750,300,750,300,750,300,750,300,750,300,750,300,750,800,275,800,275,800,250,300,750,300,750,225;
 * 20;2E;Plieger York;ID=aaaa;SWITCH=1;CMD=ON;CHIME=02;
 \*********************************************************************************************/
#define PLIEGER_PLUGIN_ID 071
#define PLUGIN_DESC_071 "Plieger"

#define PLIEGER_PULSECOUNT 66

#define PLIEGER_PULSEMID 700 / RAWSIGNAL_SAMPLE_RATE
#define PLIEGER_PULSEMAX 1900 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_071
#include "../4_Display.h"

boolean Plugin_071(byte function, char *string)
{
   if (RawSignal.Number != PLIEGER_PULSECOUNT)
      return false;

   unsigned long bitstream = 0L;
   unsigned int id = 0;
   byte chime = 0;
   //==================================================================================
   // Get all 32 bits
   //==================================================================================
   for (byte x = 1; x <= PLIEGER_PULSECOUNT - 2; x += 2)
   {
      bitstream <<= 1; // Always shift
      if (RawSignal.Pulses[x] > PLIEGER_PULSEMID)
      {
         if (RawSignal.Pulses[x] > PLIEGER_PULSEMAX)
            return false;
         if (RawSignal.Pulses[x + 1] > PLIEGER_PULSEMID)
            return false; // Valid Manchester check
         bitstream |= 0x1;
      }
      else
      {
         if (RawSignal.Pulses[x + 1] < PLIEGER_PULSEMID)
            return false; // Valid Manchester check
         // bitstream |= 0x0;
      }
   }
   //==================================================================================
   // Perform a quick sanity check
   //==================================================================================
   if (bitstream == 0)
      return false;
   if (((bitstream >> 8) & 0xFF) != 0x00)
      return false; // these 8 bits are always 0
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   if ((SignalHash != SignalHashPrevious) || ((RepeatingTimer + 1000) < millis()) || (SignalCRC != bitstream))
      SignalCRC = bitstream; // not seen the RF packet recently
   else
      return true; // already seen the RF packet recently
   //==================================================================================
   id = ((bitstream >> 16) & 0xFFFF); // get 16 bits unique address
   chime = bitstream & 0xFF;
   switch (chime)
   {
   case 0x03:
      chime = 0;
      break;
   case 0xE0:
      chime = 1;
      break;
   case 0x1C:
      chime = 2;
      break;
   default:
      return false; // the chime number can only have 3 values
   }
   //==================================================================================
   // Output
   //==================================================================================
   display_Header();
   display_Name(PSTR("Plieger"));
   display_IDn(id, 4);
   display_SWITCH(1);
   display_CMD(CMD_Single, CMD_On); // #ALL #ON
   display_CHIME(chime);
   display_Footer();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;     // do not process the packet any further
   return true;
}
#endif // PLUGIN_071
