//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-003: Kaku (ARC)                                      ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of sending and receiving the ARC protocol known from Klik-Aan-Klik-Uit (KAKU) 
 * transmitter, switch, PIR, door sensor etc.
 * Devices following the ARC protocol can often be recognized by their manual switch settings for 
 * the unit and house code number. Like for example by code wheel, dip switch or mini switch.
 *
 * This plugin also works with the following devices:
 * Princeton PT2262 / MOSDESIGN M3EB / Domia Lite / Klik-Aan-Klik-Uit / Intertechno / Sartano 2606. 
 * 
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : StuntTeam & Jonas Jespersen (Sartano) 2015..2016 
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 ***********************************************************************************************
 * Address = A0..P16 according to KAKU adressing/notation
 ***********************************************************************************************
 * Het signaal bestaat drie soorten reeksen van vier pulsen, te weten: 
 * 0 = T,3T,T,3T, 1 = T,3T,3T,T, short 0 = T,3T,T,T Hierbij is iedere pulse (T) 350us PWDM
 *
 * KAKU Supports:
 *        on/off, waarbij de pulsreeks er als volgt uit ziet: 000x en x staat voor Off / On
 *    all on/off, waarbij de pulsreeks er als volgt uit ziet: 001x en x staat voor All Off / All On 
 ***********************************************************************************************
 * Brand               Model                   Chipset      Timing L/H
 * Intertechno         ITK200                  MDT10P61S    8/43
 * Profile             PN-47N                  SC5262       5/26-27
 * Elro Home Comfort   AB-600MA                PT2262       7/29-30   6/30 6/31     
 * Eurodomest          972080                  HS2303
 * Elro Home Control   AB-440 (Home Control)   HX2262
 * Elro Home Control   AB-4xx (Home Control)   HX2272       
 * Profile             PN-44N                  SC2262       9/34
 * ProMax              RSL366T                 SC5262       11/41        10/54 11/54 ?
 * Phenix              YC-4000S                HX2262
 * Flamingo            FA500R                               8/30
 * Select Remote       1728029                 HS2262       3/22 
 ***********************************************************************************************
 * POSSIBLE BIT VARIATIONS:  (Note: Bit order is reversed -compared to order in the RF signal- in the description below and in the processing!!!) 
 * ------------------------  (   real: 1111 0000 011 1  =>  reversed: 1 110 0000 1111)
 * ARC with 0 & F bits:
 * -------------------
 * KAKU               1 110 0000 0000
 * Intertechno ITK200 D CCC BBBB AAAA
 *                    A bit 0-3  = address 2^4 = 16 addresses
 *                    B bit 4-7  = device  2^4 = 16 devices
 *                    C bit 8-10 = always (110)  ! (AND with 0x700 result must be 0x600)
 *                    D bit 11   = on/off command (1=on 0=off)
 * Timing: 8-9 /33-34  (low/high)
 * Pulses: 0101 and 0110   eg.  bit state 0 and f
 * 1 110 0000 0000  A1 on
 * 0 110 0000 0000  A1 off
 * 1 110 0001 0000  A2 on
 * 0 110 0001 0000  A2 off
 * 1 110 0010 0000  A3 on
 * 0 110 0010 0000  A3 off
 * 1 110 0011 0000  A4 on
 * 0 110 0011 0000  A4 off
 * 1 110 0000 0000  itk200 on
 * 0 110 0000 0000  itk200 off 
 * -------------------
 * Perel              1 1111 1111 11 1  
 *                    A BBBB CCCC DD E
 *                    A bit 0    = on/off command 
 *                    B bit 1-4  = system code         1111 each bit corresponds to a button number 
 *                    C bit 5-8  = device / Unitcode   1111 each bit corresponds to a button number 
 *                    D bit 9-10 = always '11'
 *                    E bit 11   = on/off command (1=on 0=off)
 * Timing: 8-9 /33-34 
 * Analyser: 362 / 1086
 * Pulses: 0101 and 0110  eg.  bit state 0 and f 
 * 1 110 100 1110 1  1 on
 * 0 110 100 1110 0  1 off
 * 1 110 101 1110 1  2 on 
 * 0 110 101 1110 0  2 off
 * 1 110 110 1110 1  3 on
 * 0 110 110 1110 0  3 off
 * -------------------
 * Elro Home Easy     0 1 00001 10001
 * Elro Home Control  D C BBBBB AAAAA
 * Brennenstuhl       A bit 0-4 = address 2^5 = 32 addresses
 *    Comfort         B bit 5-9 = device  
 *                    C bit 10  = unknown (always 1?)
 *                    D bit 11  = on/off command (1=on 0=off)
 * Timing: 7-8-9 /29-30 
 * Pulses: 0101 and 0110  eg.  bit state 0 and f 
 * 1 1 10101 11000   I12 on
 * 0 1 10101 11000   I12 off
 * 1 1 10011 00000   A7 on
 * 0 1 10011 00000   A7 off
 * -------------------
 * AB400R             0 1 11111 11111  
 *                    D C BBBBB AAAAA
 *                    A bit 0-4  = system code - each bit corresponds to a dip switch number (order 54321)
 *                    B bit 5-9  = device/unit code
 *                    C bit 10   = on/off command (reverse of D)
 *                    D bit 11   = on/off command
 * Timing: 8 /30
 * Analyser: 330/990     
 * Pulses: 0101 and 0110  eg.  bit state 0 and f 
 * 10 11110 11110  A ON
 * 10 11101 11110  B ON
 * 10 11011 11110  C ON
 * 10 10111 11110  D ON
 * 01 11110 11110  A OFF
 * 01 11101 11110  B OFF
 * 01 11011 11110  C OFF
 * 01 10111 11110  D OFF
 * -------------------
 * Sartano:           0 1 00001 10001
 * Phenix YC-4000B    D C BBBBB AAAAA
 *                    A bit 0-4  = address 2^5 = 32 addresses / Housecode  
 *                    B bit 5-9  = device / Unitcode   each bit corresponds to a button number   
 *                    C bit 10   = off command   (inverse command)
 *                    D bit 11   = on command    
 * Timing: 11-12 /31-37 
 * Pulses: 0101 and 0110  eg.  bit state 0 and f 
 * 01 01000 00000

 * -------------------
 * ProMAX:            0 111 0000 0001
 *                    D CCC BBBB AAAA
 *                    A bit 0-3  = address 2^5 = 32 addresses / Housecode
 *                    B bit 4-7  = device / Unitcode   11111 each bit corresponds to a button number 
 *                    C bit 8-10 = Always 111
 *                    D bit 11   = on command    
 * Timing: 10-11 / 40-41
 * Pulses: 0101 and 0110  eg.  bit state 0 and f 
 * 111111101110 1 ON
 * 111111011110 2 ON
 * 111110111110 3 ON
 * 111101111110 4 ON
 
 * 011111101110 1 OFF
 * 011111011110 2 OFF
 * 011110111110 3 OFF
 * 011101111110 4 OFF
 * -------------------
 * ProFile:           0 110 0001 0001
 *                    D CCC BBBB AAAA
 *                    A bit 0-3  = address 2^4 = 16 addresses / Housecode
 *                    B bit 4-7  = device / Unitcode   11111 each bit corresponds to a button number 
 *                    C bit 8-10 = Always 110
 *                    D bit 11   = on command    
 * Timing: 9/34 
 * Pulses: 0101 and 0110  eg.  bit state 0 and f 
 * 111000000000 1 ON
 * 111000010000 2 ON
 * 111000100000 3 ON
 * 111000110000 4 ON
 * 011000000000 1 OFF
 * 011000010000 2 OFF
 * 011000100000 3 OFF
 * 011000110000 4 OFF
 * -------------------
 * ProFile:           
 * Everflourish       0 110 0001 0001
 * EMW203             D CCC BBBB AAAA
 *                    A bit 0-3  = Button A/B/C/D
 *                    B bit 4-6  = Button 1/2/3
 *                    C bit 7-10 = Always 1111
 *                    D bit 11   = on/off command (on = tri-state)
 * Timing: 14/55
 * Analyser: 
 * Pulses: 0101, 1010 and 0110 eg.  bit state 0, 1 and f 
 * 211111101110 A1 on
 * 211111011110 A2 on
 * 211110111110 A3 on
 * 211111101101 B1 on
 * 211111011101 B2 on
 * 211110111101 B3 on
 * 011111101110 A1 off
 * 011111011110 A2 off
 * 011110111110 A3 off
 * 011111101101 B1 off
 * 011111011101 B2 off
 * 011110111101 B3 off
 * -------------------


 * -------------------
 *                    0 111 0000 1 000    reverse: 000 1 0000 111 0    and EF1 = 0?
 *                    A BBB CCCC D EEE
 * SelectRemote       A/C/E      = always 0 0000 000 !
 * Blokker:           B bit 1-3  = device number 
 *                    D bit 8    = on/off command
 * Timing: 0-1-2 /33-34 
 * Analyser: 232 / 692
 * Pulses: 0101 and 1010   eg.  bit state 0 and 1 
 
 * -------------------
 *                    1111 111111 11
 *                    AAAA BBBBBB CC  
 * Action:            A bit 0-3  = address 2^4 = 16 addresses 
 * Impuls             B bit 4-9  = device           
 *                    C bit 10-11= on/off command   10=on 01=off
 * -------------------
 *                    10100 00010 0 0 
 *                    AAAAA BBBBB C D
 * InterTechno        A bit 0-4  = address 2^5 = 32 addresses
 * Düwi Terminal      B bit 5-9  = device  
 * Cogex              C bit 10   = always 0 
 *                    D bit 11   = on/off command 
 * -------------------
 * 20;DB;DEBUG;Pulses=50;Pulses(uSec)=425,1050,250,1025,250,1025,250,1025,250,1025,250,1025,250,1025,250,1025,250,1050,250,1025,250,1025,250,1025,250,1025,250,1025,250,1025,250,1025,250,1050,250,1025,250,1025,950,300,250,1050,950,300,250,1025,950,300,250; 
 *
 * KAKU Doorbell          010111101111
 * 20;07;DEBUG;Pulses=50;Pulses(uSec)=300,950,250,950,250,950,950,275,250,950,250,950,250,950,950,275,250,950,950,275,250,950,950,250,250,950,950,275,250,950,250,950,250,950,950,275,250,950,950,250,250,950,950,275,250,950,950,250,250;
 * 20;09;DEBUG;Pulses=50;Pulses(uSec)=3675,950,250,950,250,950,950,250,250,950,250,950,250,950,950,275,250,950,950,250,250,950,950,275,250,950,950,250,250,950,250,950,250,950,950,250,250,950,950,275,250,950,950,275,250,950,950,275,250;
 * HE842
 * 20;03;DEBUG;Pulses=50;Pulses(uSec)=270,870,840,240,210,870,840,240,210,870,210,870,210,870,840,240,210,870,210,870,210,870,210,870,210,870,210,870,210,870,840,240,210,870,210,870,210,870,840,240,210,870,840,240,210,870,210,870,210,6990;
 * 360/1380  12/46
 * 20;54;DEBUG;Pulses=50;Pulses(uSec)=1410,390,1350,360,1350,360,1380,360,1350,360,1380,360,1380,360,1380,360,1350,360,1350,360,1350,360,1380,360,1380,360,1380,360,1350,360,1380,360,1350,360,1350,360,390,1350,390,1350,390,1320,390,1320,420,1320,420,1320,390,6990;
 \*********************************************************************************************/
#define KAKU_PLUGIN_ID 003
#define PLUGIN_DESC_003 "Kaku / AB400D / Impuls / PT2262 / Sartano / Tristate"
#define KAKU_CodeLength 12                        // number of data bits
#define KAKU_R 300 / RAWSIGNAL_SAMPLE_RATE        //360 // 300          // 370? 350 us
#define KAKU_PULSEMID 600 / RAWSIGNAL_SAMPLE_RATE // (17)  510 = KAKU_R*2 not sufficient!

#ifdef PLUGIN_003
#include "../4_Display.h"

boolean Plugin_003(byte function, char *string)
{
   if (RawSignal.Number != (KAKU_CodeLength * 4) + 2)
      return false; // conventionele KAKU bestaat altijd uit 12 data bits plus stop. Ongelijk, dan geen KAKU!
   if (RawSignal.Pulses[0] == 15)
      return true; // Home Easy, skip KAKU
   if (RawSignal.Pulses[0] == 63)
      return false; // No need to test, packet for plugin 63
   if (RawSignal.Pulses[0] == 19)
      return false; // No need to test, packet for plugin 19
   // -------------------------------------------
   int i, j;
   boolean error = false;
   unsigned long bitstream = 0L;  // to store a 12 bit code (ARC type)
   unsigned long bitstream2 = 0L; // to store a 24 bit code (Extended ARC type)
   byte tricount = 0;
   // -------------------------------------------
   byte command = 0;   // ON/OFF/DIM/BRIGHT
   byte group = 0;     // flags group command
   byte housecode = 0; // 0x40 + 1 to 16?  (41-5a?)
   byte unitcode = 0;  // 1 to 16
   // -------------------------------------------
   int PTLow = 22;      // Pulse Time - lowest found value (22 = a pulse duration of 550)
   int PTHigh = 22;     // Pulse Time - highest found value
   byte signaltype = 0; // bit map:  bit 0 = 0   bit 1 = f   bit 2 = 0/1 (PT2262)
                        // meaning: byte value    3 = kaku (bit 0/f)  5=PT2262  7=tristate 0/1/f
   byte devicetype = 0; // 0=Kaku   5=Impuls  7=Perel
   // -------------------------------------------

   // -------------------------------------------
   // ==========================================================================
   j = KAKU_PULSEMID; // set MID value
   j--;
   if (RawSignal.Pulses[0] == 33)
   {                           // If device is "Impuls"
      RawSignal.Pulses[0] = 0; // Unset Impuls conversion indicator
      j = KAKU_R;              // Set new (LOWER!) MID value
      devicetype = 5;          // Indicate Impuls device
   }
   // -------------------------------------------
   if (RawSignal.Pulses[49] > j)
      return false; // Last pulse has to be low! Otherwise we are not dealing with an ARC signal
   // ==========================================================================
   // TIMING MEASUREMENT, this will find the shortest and longest pulse within the RF packet
   // ==========================================================================
   for (i = 2; i < RawSignal.Number; i++) // skip first pulse as it is often affected by the start bit pulse duration
   {
      if (RawSignal.Pulses[i] < PTLow)       // shortest pulse?
         PTLow = RawSignal.Pulses[i];        // new value
      else if (RawSignal.Pulses[i] > PTHigh) // longest pulse?
         PTHigh = RawSignal.Pulses[i];       // new value
   }
   // -------------------------------------------
   // TIMING MEASUREMENT to devicetype
   if (devicetype != 5)
   { // Dont do the timing check for Impuls, it is already identified at this point
      if (((PTLow == 7) || (PTLow == 8)) && ((PTHigh == 30) || (PTHigh == 31)))
         devicetype = 4; // ELRO AB400
      if (((PTLow == 9) || (PTLow == 10)) && ((PTHigh == 36) || (PTHigh == 37) || ((PTHigh >= 40) && (PTHigh <= 42))))
         devicetype = 1; // ELRO AB600
      else if (((PTLow == 10) || (PTLow == 11)) && ((PTHigh >= 40) && (PTHigh <= 42)))
         devicetype = 2; // Profile PR44N  / Promax rsl366t
      else if ((PTLow == 13) && ((PTHigh >= 32) && (PTHigh <= 34)))
         devicetype = 3; // Profile PR47N
      else if (((PTLow >= 11) && (PTLow <= 12)) && ((PTHigh >= 31) && (PTHigh <= 37)))
         devicetype = 4; // Sartano
      else if (((PTLow == 12) || (PTLow == 13)) && ((PTHigh == 45) || (PTHigh == 46)))
         devicetype = 4; // Philips SBC
      else if ((PTLow <= 3) && ((PTHigh == 22) || (PTHigh == 23)))
         devicetype = 5; // Philips SBC
      //else
      //if( (PTLow == 8 ||  PTLow == 9) && (PTHigh == 33 ||  PTHigh == 34) ) devicetype=7; // Perel st=0,dt=7
   }
   //sprintf(pbuffer, "ST=%d DT=%d %d/%d",signaltype,devicetype,PTLow,PTHigh);
   //Serial.println( pbuffer );
   // ==========================================================================
   // Turn pulses into bits
   // -------------------------------------------
   for (i = 0; i < KAKU_CodeLength; i++)
   {
      if (RawSignal.Pulses[4 * i + 1] < j && RawSignal.Pulses[4 * i + 2] > j && RawSignal.Pulses[4 * i + 3] < j && RawSignal.Pulses[4 * i + 4] > j)
      {                                  // 0101
         bitstream = (bitstream >> 1);   // bit '0'
         bitstream2 = (bitstream2 << 2); // bit '0' written as '00'
         signaltype = signaltype | 1;    // bit '0' present in signal '0001'
      }
      else if (RawSignal.Pulses[4 * i + 1] < j && RawSignal.Pulses[4 * i + 2] > j && RawSignal.Pulses[4 * i + 3] > j && RawSignal.Pulses[4 * i + 4] < j)
      {              // 0110
                     //!! untested !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
         tricount++; // tri-state bit counter
         if ((i == 11) && (tricount == 1))
         {                                                               // only the last bit, "on/off command" is in tristate mode? then it must be EMW200
            bitstream = (bitstream >> 1 | (1 << (KAKU_CodeLength - 1))); // bit f (1)
            bitstream2 = (bitstream2 << 2) | 2;                          // bit 'f' written as '10'
            // DONT CHANGE signal type to tri-state to keep EMW200 in KAKU mode
         }
         else
         {
            bitstream = (bitstream >> 1 | (1 << (KAKU_CodeLength - 1))); // bit f (1)
            bitstream2 = (bitstream2 << 2) | 2;                          // bit 'f' written as '10'
            signaltype = signaltype | 2;                                 // bit 'f' present in signal '0010'
         }
      }
      else if (RawSignal.Pulses[4 * i + 1] < j && RawSignal.Pulses[4 * i + 2] > j && RawSignal.Pulses[4 * i + 3] < j && RawSignal.Pulses[4 * i + 4] < j)
      {                                      // 0100
         bitstream = (bitstream >> 1);       // Short 0, Group command on 2nd bit.  (NOT USED?!)
         bitstream2 = (bitstream2 << 2) | 3; // bit 'short' written as '11'
         group = 1;
      }
      else if (RawSignal.Pulses[4 * i + 1] > j && RawSignal.Pulses[4 * i + 2] < j && RawSignal.Pulses[4 * i + 3] > j && RawSignal.Pulses[4 * i + 4] < j)
      {                                      // 1010
         bitstream2 = (bitstream2 << 2) | 1; // bit '1' written as '01'
         signaltype = signaltype | 4;        // bit '1' present in signal '0100'
         if (devicetype == 5)
         {                                // in case of 'impuls remote' store
            bitstream = (bitstream >> 1); // bit 1 (stored as 0) (IMPULS REMOTE)
         }
         else
         {
            if (i == 11)
            {                                // last bit seems to cause trouble every now and then..?!
               bitstream = (bitstream >> 1); // bit 1 (stored as 0) (IMPULS REMOTE)
            }
            else
            {
               devicetype = 6;
               bitstream = (bitstream >> 1 | (1 << (KAKU_CodeLength - 1))); // bit f (1)
            }
         }
      }
      else
      {
         // -------------------------------------------
         // following are signal patches to fix bad transmission/receptions
         // -------------------------------------------
         if (i == 0)
         {                                  // are we dealing with a RTK/AB600 device? then the first bit is sometimes mistakenly seen as 1101
            bitstream2 = (bitstream2 << 2); // bit 0
            if (RawSignal.Pulses[4 * i + 1] > j && RawSignal.Pulses[4 * i + 2] > j && RawSignal.Pulses[4 * i + 3] < j && RawSignal.Pulses[4 * i + 4] > j)
            {                                // 1101
               bitstream = (bitstream >> 1); // 0, treat as 0101 eg 0 bit
            }
            else
            {
               error = true;
               signaltype = signaltype | 8;
            }
         }
         else
         {
            error = true;
            signaltype = signaltype | 8;
         }
      } // bad signal
   }
   //==================================================================================
   // Sort out devices based on signal type and timing measurements
   // -------------------------------------------
   //sprintf(pbuffer, "ST=%d DT=%d [%d]",signaltype,devicetype, error);
   //Serial.println( pbuffer );
   //Serial.println(bitstream,BIN);
   //Serial.println(bitstream2,BIN);
   // -------------------------------------------
   // END OF TIMING MEASUREMENTS
   // ==========================================================================
   if (error == true)
   {                // Error means that a pattern other than 0101/0110 was found
      return false; // This usually means we are dealing with a semi-compatible device
   }                // that might have more states than used by ARC
   if ((signaltype != 0x03) && (signaltype != 0x05) && (signaltype != 0x07))
      return false;
   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   //if( (SignalHash!=SignalHashPrevious) || ((RepeatingTimer+500)<millis()) ) {
   if ((SignalHash != SignalHashPrevious) || ((RepeatingTimer + 500) < millis()) || (((RepeatingTimer + 1000) > millis()) && (SignalCRC != bitstream2)))
   {
      // not seen the RF packet recently
      if (signaltype == 0x07)
      {
         if (((RepeatingTimer + 1000) > millis()) && (SignalCRC != bitstream2))
         {
            return true; // skip tristate after normal arc
         }
      }
      //Serial.print("KAKU PREV:");
      //Serial.println(SignalHashPrevious);
      if ((SignalHashPrevious == 14) && ((RepeatingTimer + 2000) > millis()))
      {
         SignalHash = 14;
         return true; // SignalHash 14 = HomeEasy, eg. cant switch KAKU after HE for 2 seconds
      }
      if ((SignalHashPrevious == 11) && ((RepeatingTimer + 2000) > millis()))
      {
         SignalHash = 11;
         return true; // SignalHash 11 = FA500, eg. cant switch KAKU after FA500 for 2 seconds
      }
      SignalCRC = bitstream2; // store RF packet identifier
   }
   else
   {
      // already seen the RF packet recently
      return true;
   }
   //==================================================================================
   // Determine signal type to sort out the various houdecode/unitcode/button bits and on/off command bits
   //==================================================================================
   if (signaltype != 0x07)
   {
      if ((bitstream & 0x700) != 0x600)
      { // valid but not real KAKU
         devicetype = 4;
      }
      // -------------------------------------------
      if (devicetype == 4)
      { // Sartano
         // ----------------------------------      // Sartano
         housecode = ((bitstream)&0x0000001FL);       // .......11111b
         unitcode = (((bitstream)&0x000003E0L) >> 5); // ..1111100000b
         housecode = ~housecode;                      // Sartano housecode is 5 bit ('A' - '`')
         housecode &= 0x0000001FL;                    // Translate housecode so that all jumpers off = 'A' and all jumpers on = '`'
         housecode += 0x41;
         switch (unitcode)
         {          // Translate unit code into button number 1 - 5
         case 0x1E: // E=1110
            unitcode = 1;
            break;
         case 0x1D: // D=1101
            unitcode = 2;
            break;
         case 0x1B: // B=1011
            unitcode = 3;
            break;
         case 0x17: // 7=0111
            unitcode = 4;
            break;
         case 0x0F: // f=1111
            unitcode = 5;
            break;
         default:
            //Serial.print("Sartano:");
            devicetype = 3; // invalid for Sartano, fall back
            break;
         }
         if (devicetype == 4)
         { // Sartano
            if (((bitstream >> 10) & 0x03) == 2)
            {
               command = 1; // On
            }
            else if (((bitstream >> 10) & 0x03) == 1)
            {
               command = 0; // Off
            }
         }
      }
      else
          // -------------------------------------------
          if (devicetype == 5)
      {                                               // IMPULS
         housecode = ((bitstream)&0x0000000FL);       // ........1111b
         unitcode = (((bitstream)&0x000003F0L) >> 4); // ..1111110000b
         housecode = ~housecode;                      // Impuls housecode is 4 bit ('A' - 'P')
         housecode &= 0x0000000FL;                    // Translate housecode so that all jumpers off = 'A' and all jumpers on = 'P'
         housecode += 0x41;
         unitcode = ~unitcode;    // Impuls unitcode is 5 bit
         unitcode &= 0x0000001FL; // Translate unitcode so that all jumpers off = '1' and all jumpers on = '64'
         if (((bitstream >> 10) & 0x03) == 2)
         {
            command = 0; // Off
         }
         else if (((bitstream >> 10) & 0x03) == 1)
         {
            command = 1; // On
         }
      }
      else
          // -------------------------------------------
          if (devicetype == 6)
      { // Blokker/SelectRemote
         // ----------------------------------
         if (((bitstream)&0xef1) != 0)
            return false; // Not a valid bitstream

         housecode = ((bitstream)&0x0000000EL); // Isolate housecode
         housecode = housecode >> 1;            // shift right 1 bit
         housecode = (~housecode) & 0x07;       // invert bits
         housecode += 0x41;                     // add 'A'
         unitcode = 0;

         if (((bitstream >> 8) & 0x01) == 1)
         {
            command = 1; // On
         }
         else
         {
            command = 0; // Off
         }
         // ----------------------------------      // Sartano
      }
      else
          // -------------------------------------------
          if (devicetype != 4)
      { // KAKU (and some compatibles for now)
         // ----------------------------------
         if ((bitstream & 0x700) != 0x600)
         { // valid but not real KAKU
            housecode = (((bitstream)&0x0f) + 0x41);
            unitcode = ((((bitstream)&0xf0) >> 4) + 1);
            devicetype = 1;
         }
         else
         {
            if ((bitstream & 0x600) != 0x600)
            {
               //Serial.println("Kaku 0/1 error");
               return false; // use two static bits as checksum
            }
            //bitstream=(bitstream)&0xffe;               // kill bit to ensure Perel works -> actually kills code wheel letter B
            housecode = (((bitstream)&0x0f) + 0x41);
            unitcode = ((((bitstream)&0xf0) >> 4) + 1);
            // ----------------------------------
         }
         if (((bitstream >> 11) & 0x01) == 1)
         {
            command = 1; // ON
         }
         else
         {
            command = 0; // OFF
         }
      }
   }
   // ==========================================================================
   // Output
   // ----------------------------------
   display_Header();

   // ----------------------------------
   if (signaltype == 0x03)
   { // '0011' bits indicate bits 0 and f are used in the signal
      if (devicetype < 4)
         display_Name(PSTR("Kaku")); // KAKU (and some compatibles for now) label
      else
      {
         if (devicetype == 4)
            display_Name(PSTR("AB400D")); // AB440R and Sartano label
         else if (devicetype == 5)
            display_Name(PSTR("Impuls")); // Impuls label
         else
            display_Name(PSTR("Sartano")); // Others
      }
   }
   else if (signaltype == 0x05)
   { // '0101' bits indicate bits 0 and 1 are used in the signal
      if (devicetype == 5)
         display_Name(PSTR("Impuls")); // Impuls label
      else
         display_Name(PSTR("PT2262")); // Others
   }
   else if (signaltype == 0x07)
      display_Name(PSTR("TriState")); // '0111' bits indicate bits 0, f and 1 are used in the signal (tri-state)
   // ----------------------------------

   if (signaltype == 0x07)
   {                                                  // '0111' bits indicate bits 0, f and 1 are used in the signal (tri-state)
      display_IDn(((bitstream2 >> 4) & 0xFFFFFF), 6); // "%S%06lx

      housecode = (bitstream2 & 0x03);
      unitcode = ((((bitstream2 >> 2) & 0x03) ^ 0x03) ^ housecode);

      //command=((bitstream2 & 0x01)^ 0x01);   << ^1 reverses lidl light
      command = (bitstream2 & 0x03); // 00 01 10  0 1 f
      if (command > 1)
         command = 1; // 0 stays 0 (OFF), 1 and f become 1 (ON)
   }
   else
      display_IDn(housecode, 2);

   display_SWITCH(unitcode);
   display_CMD(group, command); // #ALL, #ON
   display_Footer();

   // ----------------------------------
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif //PLUGIN_003

#ifdef PLUGIN_TX_003
void Arc_Send(unsigned long address);        // sends 0 and float
void NArc_Send(unsigned long bitstream);     // sends 0 and 1
void TriState_Send(unsigned long bitstream); // sends 0, 1 and float

boolean PluginTX_003(byte function, char *string)
{
   boolean success = false;
   unsigned long bitstream = 0L;
   byte command = 0;
   uint32_t housecode = 0;
   uint32_t unitcode = 0;
   byte Home = 0;    // KAKU home A..P
   byte Address = 0; // KAKU Address 1..16
   byte c = 0;
   byte x = 0;
   // ==========================================================================
   //10;Kaku;00004d;1;OFF;
   //10;Kaku;00004f;e;ON;
   //10;Kaku;000050;10;ON;
   //10;Kaku;000049;b;ON;
   //012345678901234567890
   // ==========================================================================
   if (strncasecmp(InputBuffer_Serial + 3, "KAKU;", 5) == 0)
   { // KAKU Command eg. Kaku;A1;On
      if (InputBuffer_Serial[14] != ';')
         return false;

      x = 15; // character pointer
      InputBuffer_Serial[10] = 0x30;
      InputBuffer_Serial[11] = 0x78;           // Get home from hexadecimal value
      InputBuffer_Serial[14] = 0x00;           // Get home from hexadecimal value
      Home = str2int(InputBuffer_Serial + 10); // KAKU home A is intern 0
      if (Home < 0x51)                         // take care of upper/lower case
         Home = Home - 'A';
      else if (Home < 0x71) // take care of upper/lower case
         Home = Home - 'a';
      else
      {
         return false; // invalid value
      }

      while ((c = InputBuffer_Serial[x++]) != ';')
      { // Address: 1 to 16/32
         if (c >= '0' && c <= '9')
         {
            Address = Address * 10;
            Address = Address + c - '0';
         }
         if (c >= 'a' && c <= 'f')
         {
            Address = Address + (c - 'a' + 10);
         } // 31?
         if (c >= 'A' && c <= 'F')
         {
            Address = Address + (c - 'A' + 10);
         } // 51?
      }
      //if (Address==0) {                        // group command is given: 0=all
      //   command=2;                            // Set 2nd bit for group.
      //   bitstream=Home;
      //} else {
      //   bitstream= Home | ((Address-1)<<4);
      //}

      bitstream = Home | ((Address - 1) << 4);
      command |= str2cmd(InputBuffer_Serial + x) == VALUE_ON;  // ON/OFF command
      bitstream = bitstream | (0x600 | ((command & 1) << 11)); // create the bitstream
      //Serial.println(bitstream);
      Arc_Send(bitstream);
      success = true;
      // --------------- END KAKU SEND ------------
   }
   else
       // ==========================================================================
       //10;AB400D;00004d;1;OFF;
       //012345678901234567890
       // ==========================================================================
       if (strncasecmp(InputBuffer_Serial + 3, "AB400D;", 7) == 0)
   { // KAKU Command eg. Kaku;A1;On
      if (InputBuffer_Serial[16] != ';')
         return false;
      x = 17; // character pointer
      InputBuffer_Serial[12] = 0x30;
      InputBuffer_Serial[13] = 0x78;           // Get home from hexadecimal value
      InputBuffer_Serial[16] = 0x00;           // Get home from hexadecimal value
      Home = str2int(InputBuffer_Serial + 12); // KAKU home A is intern 0
      if (Home < 0x61)                         // take care of upper/lower case
         Home = Home - 'A';
      else if (Home < 0x81) // take care of upper/lower case
         Home = Home - 'a';
      else
      {
         return false; // invalid value
      }
      while ((c = InputBuffer_Serial[x++]) != ';')
      { // Address: 1 to 16/32
         if (c >= '0' && c <= '9')
         {
            Address = Address * 10;
            Address = Address + c - '0';
         }
      }
      command = str2cmd(InputBuffer_Serial + x) == VALUE_ON; // ON/OFF command
      housecode = ~Home;
      housecode &= 0x0000001FL;
      unitcode = Address;
      if ((unitcode >= 1) && (unitcode <= 5))
      {
         bitstream = housecode & 0x0000001FL;
         if (unitcode == 1)
            bitstream |= 0x000003C0L;
         else if (unitcode == 2)
            bitstream |= 0x000003A0L;
         else if (unitcode == 3)
            bitstream |= 0x00000360L;
         else if (unitcode == 4)
            bitstream |= 0x000002E0L;
         else if (unitcode == 5)
            bitstream |= 0x000001E0L;

         if (command)
            bitstream |= 0x00000800L;
         else
            bitstream |= 0x00000400L;
      }
      //Serial.println(bitstream);
      Arc_Send(bitstream);
      success = true;
   }
   else
       // --------------- END SARTANO SEND ------------
       // ==========================================================================
       //10;PT2262;000041;1;OFF;
       //012345678901234567890
       // ==========================================================================
       if (strncasecmp(InputBuffer_Serial + 3, "PT2262;", 7) == 0)
   { // KAKU Command eg. Kaku;A1;On
      if (InputBuffer_Serial[16] != ';')
         return false;
      x = 17; // character pointer
      InputBuffer_Serial[12] = 0x30;
      InputBuffer_Serial[13] = 0x78;           // Get home from hexadecimal value
      InputBuffer_Serial[16] = 0x00;           // Get home from hexadecimal value
      Home = str2int(InputBuffer_Serial + 12); // KAKU home A is intern 0
      if (Home < 0x61)                         // take care of upper/lower case
         Home = Home - 'A';
      else if (Home < 0x81) // take care of upper/lower case
         Home = Home - 'a';
      else
      {
         return false; // invalid value
      }
      while ((c = InputBuffer_Serial[x++]) != ';')
      { // Address: 1 to 16/32
         if (c >= '0' && c <= '9')
         {
            Address = Address * 10;
            Address = Address + c - '0';
         }
      }
      // reconstruct bitstream reversed order so that most right bit can be send first
      command = str2cmd(InputBuffer_Serial + x) == VALUE_ON; // ON/OFF command
      housecode = ~Home;
      housecode &= 0x00000007L;
      housecode = (housecode) << 1;
      if (command)
         bitstream |= 0x00000100L;
      NArc_Send(bitstream); // send 24 bits tristate signal (0/1/f)
      success = true;
   }
   else
       // --------------- END Select Remote SEND ------------
       // ==========================================================================
       //10;TriState;00004d;1;OFF;
       //10;TriState;08000a;2;OFF;       20;1B;TriState;ID=08000a;SWITCH=2;CMD=OFF;
       //10;TriState;0a6980;2;OFF;
       //01234567890123456789012
       // ==========================================================================
       if (strncasecmp(InputBuffer_Serial + 3, "TriState;", 9) == 0)
   { // KAKU Command eg. Kaku;A1;On
      if (InputBuffer_Serial[18] != ';')
         return false;
      x = 19; // character pointer
      InputBuffer_Serial[10] = 0x30;
      InputBuffer_Serial[11] = 0x78;                // Get home from hexadecimal value
      InputBuffer_Serial[18] = 0x00;                // Get home from hexadecimal value
      bitstream = str2int(InputBuffer_Serial + 10); // KAKU home A is intern 0
      bitstream = (bitstream << 4);

      // 11^00^01=10   11^10^11=01   11^11^00=00
      while ((c = InputBuffer_Serial[x++]) != ';')
      { // Address: 0/1/2
         if (c >= '0' && c <= '9')
         {
            Address = Address * 10;
            Address = Address + c - '0';
         }
      }
      Address = (Address)&0x03;                  // only use 3 bits
      command = str2cmd(InputBuffer_Serial + x); // ON/OFF command
      if (command == VALUE_ON)
      { // on
         if (Address == 0x0)
            bitstream |= 0x0000000bL; // 0011
         if (Address == 0x1)
            bitstream |= 0x0000000cL; // 1011
         if (Address == 0x2)
            bitstream |= 0x00000001L; // 0001
      }
      else
      { // off
         if (Address == 0x0)
            bitstream |= 0x0000000cL; // 1100
         if (Address == 0x1)
            bitstream |= 0x0000000eL; // 1110
         if (Address == 0x2)
            bitstream |= 0x00000004L; // 0100
      }
      TriState_Send(bitstream);
      success = true;
   }
   else
       // --------------- END TRISTATE SEND ------------
       // ==========================================================================
       //10;Impuls;00004d;1;OFF;
       //012345678901234567890
       // ==========================================================================
       if (strncasecmp(InputBuffer_Serial + 3, "Impuls;", 7) == 0)
   { // KAKU Command eg. Kaku;A1;On
      if (InputBuffer_Serial[16] != ';')
         return false;
      x = 17; // character pointer
      InputBuffer_Serial[12] = 0x30;
      InputBuffer_Serial[13] = 0x78;           // Get home from hexadecimal value
      InputBuffer_Serial[16] = 0x00;           // Get home from hexadecimal value
      Home = str2int(InputBuffer_Serial + 12); // KAKU home A is intern 0
      if (Home < 0x61)                         // take care of upper/lower case
         Home = Home - 'A';
      else if (Home < 0x81) // take care of upper/lower case
         Home = Home - 'a';
      else
      {
         return false; // invalid value
      }
      while ((c = InputBuffer_Serial[x++]) != ';')
      { // Address: 1 to 16/32
         if (c >= '0' && c <= '9')
         {
            Address = Address * 10;
            Address = Address + c - '0';
         }
      }
      command = str2cmd(InputBuffer_Serial + x) == VALUE_ON; // ON/OFF command
      housecode = ~Home;
      housecode &= 0x0000001FL;
      unitcode = Address;
      if ((unitcode >= 1) && (unitcode <= 5))
      {
         bitstream = housecode & 0x0000001FL;
         if (unitcode == 1)
            bitstream |= 0x000003C0L;
         else if (unitcode == 2)
            bitstream |= 0x000003A0L;
         else if (unitcode == 3)
            bitstream |= 0x00000360L;
         else if (unitcode == 4)
            bitstream |= 0x000002E0L;
         else if (unitcode == 5)
            bitstream |= 0x000001E0L;

         if (command)
            bitstream |= 0x00000800L;
         else
            bitstream |= 0x00000400L;
      }
      TriState_Send(bitstream);
      success = true;
   }
   return success;
}

//#define KAKU_T                     390 //420 // 370              // 370? 350 us
//#define Sartano_T                  300 //360 // 300              // 300 uS

void Arc_Send(unsigned long bitstream)
{
   int fpulse = 360; // Pulse width in microseconds
   int fretrans = 8; // Number of code retransmissions
   uint32_t fdatabit;
   uint32_t fdatamask = 0x00000001;
   uint32_t fsendbuff;

   for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++)
   {
      fsendbuff = bitstream;
      // Send command

      for (int i = 0; i < 12; i++)
      { // Arc packet is 12 bits
         // read data bit
         fdatabit = fsendbuff & fdatamask; // Get most right bit
         fsendbuff = (fsendbuff >> 1);     // Shift right

         // PT2262 data can be 0, 1 or float. Only 0 and float is used by regular ARC
         if (fdatabit != fdatamask)
         { // Write 0
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 1);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 3);
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 1);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 3);
         }
         else
         { // Write float
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 1);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 3);
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 3);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 1);
         }
      }
      // Send sync bit
      digitalWrite(PIN_RF_TX_DATA, HIGH);
      delayMicroseconds(fpulse * 1);
      digitalWrite(PIN_RF_TX_DATA, LOW); // and lower the signal
      delayMicroseconds(fpulse * 31);
   }
}

void NArc_Send(unsigned long bitstream)
{
   int fpulse = 190; // Pulse width in microseconds
   int fretrans = 7; // Number of code retransmissions
   uint32_t fdatabit;
   uint32_t fdatamask = 0x00000001;
   uint32_t fsendbuff;

   for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++)
   {
      fsendbuff = bitstream;
      // Send command

      for (int i = 0; i < 12; i++)
      { // Arc packet is 12 bits
         // read data bit
         fdatabit = fsendbuff & fdatamask; // Get most right bit
         fsendbuff = (fsendbuff >> 1);     // Shift right

         // PT2262 data can be 0, 1 or float. Only 0 and float is used by regular ARC
         if (fdatabit != fdatamask)
         { // Write 0
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 1);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 3);
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 1);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 3);
         }
         else
         { // Write 1
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 3);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 1);
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 3);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 1);
         }
      }
      // Send sync bit
      digitalWrite(PIN_RF_TX_DATA, HIGH);
      delayMicroseconds(fpulse * 1);
      digitalWrite(PIN_RF_TX_DATA, LOW); // and lower the signal
      delayMicroseconds(fpulse * 31);
   }
}

void TriState_Send(unsigned long bitstream)
{
   int fpulse = 360; // Pulse width in microseconds
   int fretrans = 8; // Number of code retransmissions
   uint32_t fdatabit;
   uint32_t fdatamask = 0x00000003;
   uint32_t fsendbuff;

   // reverse data bits (2 by 2)
   for (unsigned short i = 0; i < 12; i++)
   { // reverse data bits (12 times 2 bits = 24 bits in total)
      fsendbuff <<= 2;
      fsendbuff |= (bitstream & B11);
      bitstream >>= 2;
   }
   bitstream = fsendbuff; // store result

   for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++)
   {
      fsendbuff = bitstream;
      // Send command
      for (int i = 0; i < 12; i++)
      { // 12 times 2 bits = 24 bits in total
         // read data bit
         fdatabit = fsendbuff & fdatamask; // Get most right 2 bits
         fsendbuff = (fsendbuff >> 2);     // Shift right
                                           // data can be 0, 1 or float.
         if (fdatabit == 0)
         { // Write 0
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 3);
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 3);
         }
         else if (fdatabit == 1)
         { // Write 1
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 3);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 1);
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 3);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 1);
         }
         else
         { // Write float
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 1);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 3);
            digitalWrite(PIN_RF_TX_DATA, HIGH);
            delayMicroseconds(fpulse * 3);
            digitalWrite(PIN_RF_TX_DATA, LOW);
            delayMicroseconds(fpulse * 1);
         }
      }
      // Send sync bit
      digitalWrite(PIN_RF_TX_DATA, HIGH);
      delayMicroseconds(fpulse * 1);
      digitalWrite(PIN_RF_TX_DATA, LOW); // and lower the signal
      delayMicroseconds(fpulse * 31);
   }
}
#endif //PLUGIN_TX_003
