//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                 Plugin-71 Plieger York doorbell                                   ##
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
#define PLIEGER_PULSECOUNT 66
#define PLIEGER_PULSEMID 700 / RAWSIGNAL_SAMPLE_RATE
#define PLIEGER_PULSEMAX 1900 / RAWSIGNAL_SAMPLE_RATE

#ifdef PLUGIN_071
boolean Plugin_071(byte function, char *string)
{
   if (RawSignal.Number != PLIEGER_PULSECOUNT)
      return false;
   unsigned long bitstream = 0L;
   unsigned int id = 0;
   byte chime = 0;
   //==================================================================================
   // get all 32 bits
   for (byte x = 1; x <= PLIEGER_PULSECOUNT - 2; x += 2)
   {
      if (RawSignal.Pulses[x] > PLIEGER_PULSEMID)
      {
         if (RawSignal.Pulses[x] > PLIEGER_PULSEMAX)
            return false;
         if (RawSignal.Pulses[x + 1] > PLIEGER_PULSEMID)
            return false; // Valid Manchester check
         bitstream = (bitstream << 1) | 0x1;
      }
      else
      {
         if (RawSignal.Pulses[x + 1] < PLIEGER_PULSEMID)
            return false; // Valid Manchester check
         bitstream = (bitstream << 1);
      }
   }
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   if ((SignalHash != SignalHashPrevious) || (RepeatingTimer + 1000 < millis()))
   {
      // not seen the RF packet recently
      if (bitstream == 0)
         return false; // sanity check
   }
   else
   {
      // already seen the RF packet recently
      return true;
   }
   //==================================================================================
   // first perform two checks to validate the data
   if (((bitstream >> 8) & 0xff) != 0x00)
      return false; // these 8 bits are always 0
   chime = bitstream & 0xff;
   if (chime != 0x1c && chime != 0x03 && chime != 0xE0)
      return false; // the chime number can only have 3 values
   //==================================================================================
   id = (bitstream >> 16) & 0xffff; // get 16 bits unique address
   if (chime == 0xE0)
      chime = 1;
   if (chime == 0x1C)
      chime = 2;
   //==================================================================================
   // Output
   // ----------------------------------
   Serial.print("20;");
   PrintHexByte(PKSequenceNumber++);
   Serial.print(F(";Plieger;")); // Label
   // ----------------------------------
   sprintf(pbuffer, "ID=%04x;", id); // ID
   Serial.print(pbuffer);
   Serial.print("SWITCH=1;CMD=ON;");
   sprintf(pbuffer, "CHIME=%02x;", chime); // chime number
   Serial.print(pbuffer);
   Serial.println();
   //==================================================================================
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;     // do not process the packet any further
   return true;
}
#endif // PLUGIN_071
