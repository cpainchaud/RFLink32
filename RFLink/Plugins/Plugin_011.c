//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                  Plugin-011: Home Confort Smart Home                              ##
//#######################################################################################################
/*********************************************************************************************\
 *
 * IMPORTANT WARNING : Does not works correctly on ESP8266 if SIGNAL_MIN_PREAMBLE_US is too short (< 768 us for)
 * 
 * Decodes signals from a Home Confort Smart Home - TEL-010 
 * http://idk.home-confort.net/divers/t%C3%A9l%C3%A9commande-rf-pour-produits-smart-home
 *
 * Author  (present)  : StormTeam 2018..2020 - Marc RIVES (aka Couin3)
 * Support (present)  : https://github.com/couin3/RFLink 
 * Author  (original) : StuntTeam & François Pasteau 2015..2016
 * Support (original) : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *
 *********************************************************************************************
 * Technical information: Home Confort Smart Home - TEL-010 devices
 * --------------------------------------------------------------------------------------------
 * _Byte 0_ _Byte 1_ _Byte 2_ _Byte 3_ _Byte 4_ _Byte 5_ _Bits_
 * AAAAAAAA BBBBBBBB CCCDDDDD EEEEEEEE FFFFFFFF GGHHHHHH II
 * 11111111 11111111 11100010 00000000 00000000 10000001 00  Type 1 (no address, using A1-D4)
 * 00110110 10100100 01111010 00000000 00000000 10000001 00  Type 2 (using 19 bit? address and button number 1-?
 *
 * A = Address? (19 bits?)
 * B = Address? (19 bits?)
 * C = Address? (19 bits?)
 * D = Command bits (see below)
 * E = verification bytes?, Always 0x00? => possibly reserved for future use or other device types
 * F = verification bytes?, Always 0x00? => possibly reserved for future use or other device types
 * G = Command bits (see below)
 * H = always '000001' ?
 * I = Stop bits?, Always '00'
 * 
 * D & G explanation:
 * DDDDD GG
 * 11233 45
 * 00000 00
   11010 10 D3
 * 1=switch setting 00=A  10=B  01=C  11=D
 * 2=group indicator 1=group command
 * 3=button number  00=1  01=2  10=3  11=4
 * 4=command 0=off, 1=on
 * 5=group indicator 1=group command
 * ------------
 * 101010101010101010101010101010101010100101011001010101010101010101010101010101011001010101010110 00
 * 111111111111111111100010000000000000000010000001 00 A3 on
 * 20;17;DEBUG;Pulses=100;Pulses(uSec)=
 * 2675,200,600,200,600,700,100,700,100,200,600,700,100,700,100,200,600,700,100,225,600,725,75,225,600,225,575,725,75,225,
 * 575,225,575,225,575,725,75,725,75,725,75,725,75,225,575,725,75,225,575,225,575,225,575,225,575,225,575,250,575,250,
 * 575,250,575,250,575,250,550,250,550,250,550,250,550,250,550,250,550,250,550,250,575,725,75,250,550,250,550,250,550,250,
 * 550,250,550,250,550,750,50,250,50; =99
 *
 * 2675,200,600,200,600,700,100,700,100,200,600,700,100,700,100,200,600,700,100,225,600,725,75,225,600,225,575,725,75,225,575,225,575,225,575,725,75,725,75,725,75,725,75,225,575,725,75,225,575,225,575,225,575,225,575,225,575,250,575,250,575,250,575,250,575,250,550,250,550,250,550,250,550,250,550,250,550,250,550,250,575,725,75,250,550,250,550,250,550,250,550,250,550,250,550,750,50,250,50
 * --------------------------------------------------------------------------------------------
 * Technical information: Home Confort Smart Home - STHI-100 devices
 * --------------------------------------------------------------------------------------------
 * https://uploads.entrepot-du-bricolage.fr/PDF/609492_sthi-100___sthi-100x2.pdf
 * First 24 bits (bitstream1) : Random ID
 * 
 * Last 24 bits (bitstream2) :
 * Temperature and humidity are sent separetely, in different frames. 
 * Last 7 bits are function :
 *    0x41 : Temperature
 *    0x42 : Humidity
 * 
 *    Temperature :
 *    1100 1000 0100 0100 0001
 *    TTTT Tttt tttt tFFF FFFF
 *    T : Temperature ones, in Celsius
 *    t : Temperature digists
 *    F : Function
 * Samples : 
 * 20;XX;DEBUG;Pulses=100;Pulses(uSec)=2752,300,640,640,300,640,300,640,300,300,640,640,300,640,300,640,300,640,300,640,300,300,640,300,640,300,640,640,300,640,300,640,300,300,640,300,640,300,640,640,300,300,640,640,300,640,300,640,300,300,640,300,640,300,640,640,300,640,300,300,640,640,300,300,640,300,640,300,640,300,640,300,640,300,640,300,640,300,640,300,640,640,300,300,640,300,640,300,640,300,640,300,640,640,300,640,300,640,300,448;
 * 20;XX;DEBUG;Pulses=100;Pulses(uSec)=2752,300,640,640,300,640,300,640,300,300,640,640,300,640,300,640,300,640,300,640,300,300,640,300,640,300,640,640,300,640,300,640,300,300,640,300,640,300,640,640,300,300,640,640,300,640,300,640,300,300,640,300,640,300,640,640,300,640,300,300,640,640,300,300,640,300,640,300,640,300,640,300,640,300,640,300,640,300,640,300,640,640,300,300,640,300,640,300,640,300,640,300,640,640,300,640,300,640,300,448;
 
 \*********************************************************************************************/

#define HC_PLUGIN_ID 011
#define PLUGIN_DESC_011 "HomeConfort"
#define HC_PULSECOUNT 100
#define HC_PULSE_PREAMBLE_D 2400
#define HC_PULSE_MID_D 500
#define HC_PULSE_MAX_D 800

#ifdef PLUGIN_011
#include "../4_Display.h"

boolean Plugin_011(byte function, const char *string)
{
   const long HC_PULSE_PREAMBLE = HC_PULSE_PREAMBLE_D / RawSignal.Multiply;
   const long HC_PULSE_MID = HC_PULSE_MID_D / RawSignal.Multiply;
   const long HC_PULSE_MAX = HC_PULSE_MAX_D / RawSignal.Multiply;

  if (RawSignal.Number != HC_PULSECOUNT) // Incorrect pulse count
      return false;
  if (RawSignal.Pulses[1] < HC_PULSE_PREAMBLE )
      return false; // First (start) pulse needs to be long

   unsigned long bitstream1 = 0; // holds first 24 bits
   unsigned long bitstream2 = 0; // holds last 26 bits
   byte bitcounter = 0;          // counts number of received bits (converted from pulses)
   byte command = 0;
   byte channel = 0;
   byte subchan = 0;
   byte group = 0;

   //==================================================================================
   // Get all 48 bits
   //==================================================================================
   for (int x = 2; x < 96 ; x += 2) // Go through pulses
   { // get bytes
      if (RawSignal.Pulses[x] > HC_PULSE_MID )
      { // long pulse
         if (RawSignal.Pulses[x] > HC_PULSE_MAX )
            return false; // Pulse range check
         if (RawSignal.Pulses[x + 1] > HC_PULSE_MID )
            return false; // Manchester check
         if (bitcounter < 24)
         {
            bitstream1 <<= 1;
            bitstream1 |= 0x1;
         }
         else
         {
            bitstream2 <<= 1;
            bitstream2 |= 0x1;
         }
      }
      else
      { // short pulse
         if (RawSignal.Pulses[x] > HC_PULSE_MID)
            return false; // pulse range check
         if (RawSignal.Pulses[x + 1] < HC_PULSE_MID )
            return false; // Manchester check
         if (bitcounter < 24)
            bitstream1 <<= 1;
         else
            bitstream2 <<= 1;
      }
      bitcounter++;
      if (bitcounter == 47)
         break;
   }

   //==================================================================================
   // Prevent repeating signals from showing up
   //==================================================================================
   if (SignalHash != SignalHashPrevious || ((RepeatingTimer + 500) < millis()) || SignalCRC != bitstream2)
      SignalCRC = bitstream2; // not seen the RF packet recently
   else
      return true; // already seen the RF packet recently

   //==================================================================================
   // Find function in bistream 2 :
   // 0x40 or 0x00 (all bits) : Switch
   // 0x42 (7 bits)           : Humidity
   // 0x41 (7 bits)           : Temperature
   //==================================================================================
   if ( (bitstream2 & 0x7F) == 0x41 ) 
      { // Temperature
         int tempunits = bitstream2 >> 15 & 0x1F;
         int tempdecs = bitstream2 >> 7 & 0xF;
         int temperature = tempunits * 10 + tempdecs;

         display_Header();
         display_Name(PSTR("HomeConfort"));
         display_IDn((bitstream1 & 0xFFFFFF), 6); //"%S%06lx"
         display_TEMP(temperature);
         display_Footer();
      }
   else if ( (bitstream2 & 0x7F) == 0x42 )  
      { // Humidity
         byte hygro = bitstream2 >>15;

         display_Header();
         display_Name(PSTR("HomeConfort"));
         display_IDn((bitstream1 & 0xFFFFFF), 6); //"%S%06lx"
         display_HUM(hygro);
         display_Footer();
      }
   else if ( (bitstream2 & 0xFFFFBF) == 0x0 ) 
      {  // Switch code - Taken from Original RFLink R29 code
         //==================================================================================
         // now process the command / switch settings
         //==================================================================================
         byte tempbyte = (bitstream1 >> 3) & 0x03; // determine switch setting (a/b/c/d)
         channel = 0x41;
         if (tempbyte == 2)
            channel = 0x42;
         if (tempbyte == 1)
            channel = 0x43;
         if (tempbyte == 3)
            channel = 0x44;

         subchan = (bitstream1 & 0x03) + 1; // determine button number
         command = (bitstream2 >> 7) & 0x01; // on/off command
         group = (bitstream2 >> 6) & 0x01;   // group setting
         bitstream1 = bitstream1 >> 5;
         //==================================================================================
         // Output
         //==================================================================================
         display_Header();
         display_Name(PSTR("HomeConfort"));
         display_IDn((bitstream1 & 0xFFFFFF), 6); //"%S%06lx"
         char c_SWITCH[5];
         sprintf(c_SWITCH, "%c%d", channel, subchan);
         display_SWITCHc(c_SWITCH);
         display_CMD((group & B01), (command & B01)); // #ALL , #ON
         display_Footer();
         //==================================================================================
      }
   else // Unknow protocol or function
      return false;
       
   RawSignal.Repeats = true; // suppress repeats of the same RF packet
   RawSignal.Number = 0;
   return true;
}
#endif // PLUGIN_011

#ifdef PLUGIN_TX_011
void HomeConfort_Send(unsigned long bitstream1, unsigned long bitstream2);

boolean  PluginTX_011(byte function, const char *string)
{
   boolean success = false;
   //10;HomeConfort;01b523;D3;ON;
   //0123456789012345678901234567
   if (strncasecmp(InputBuffer_Serial + 3, "HomeConfort;", 12) == 0)
   { // KAKU Command eg.
      if (InputBuffer_Serial[21] != ';')
         return success;

      InputBuffer_Serial[13] = 0x30;
      InputBuffer_Serial[14] = 0x78; // Get address from hexadecimal value
      InputBuffer_Serial[21] = 0x00; // Get address from hexadecimal value

      unsigned long bitstream1 = 0L; // First Main placeholder
      unsigned long bitstream2 = 0L; // Second Main placeholder
      byte Home = 0;                 // channel A..D
      byte Address = 0;              // subchannel 1..5
      byte c;
      byte x = 22; // pointer
      // -------------------------------
      bitstream1 = str2int(InputBuffer_Serial + 13); // Address (first 19 bits)
      // -------------------------------
      while ((c = tolower(InputBuffer_Serial[x++])) != ';')
      {
         if (c >= '0' && c <= '9')
         {
            Address = Address + c - '0';
         } // Home 0..9
         if (c >= 'a' && c <= 'd')
         {
            Home = c - 'a';
         } // Address a..d
      }
      // -------------------------------
      // prepare bitstream1
      // -------------------------------
      bitstream1 = bitstream1 << 5; // make space for first 5 command bits
      Address--;                    // 1..4 to 0..3
      Address = Address & 0x03;     // only accept 2 bits for the button number part
      bitstream1 = bitstream1 + Address;

      if (Home == 0)
         c = 0; // A
      if (Home == 1)
         c = 2; // B
      if (Home == 2)
         c = 1; // C
      if (Home == 3)
         c = 3;      // D
      Home = c << 3; // shift left for the right position
      bitstream1 = bitstream1 + Home;
      // -------------------------------
      // prepare bitsrteam2
      c = 0;
      c = str2cmd(InputBuffer_Serial + x); // ON/OFF command
      bitstream2 = 1;                      // value off
      if (c == VALUE_ON)
      {
         bitstream2 = 0x81; // value on
      }
      else
      {
         if (c == VALUE_ALLOFF)
         {
            bitstream2 = 0x41;
            bitstream1 = bitstream1 + 4; // set group
         }
         else if (c == VALUE_ALLON)
         {
            bitstream2 = 0xc1;
            bitstream1 = bitstream1 + 4; // set group
         }
      }
      // -------------------------------
      HomeConfort_Send(bitstream1, bitstream2); // bitstream to send
      success = true;
   }
   return success;
}
/*
void HomeConfort_Send(unsigned long data1, unsigned long data2) { 
    int fpulse  = 270;                              // Pulse width in microseconds
    int fpulse2 = 710;                              // Pulse width in microseconds
    int fretrans = 10;                              // Number of code retransmissions

	unsigned long bitstream1 = 0L;
	unsigned long bitstream2 = 0L;
    // prepare data to send	
	for (unsigned short i=0; i<24; i++) {           // reverse data bits
		bitstream1<<=1;
		bitstream1|=(data1 & B1);
		data1>>=1;
	}
	for (unsigned short i=0; i<24; i++) {           // reverse data bits
		bitstream2<<=1;
		bitstream2|=(data2 & B1);
		data2>>=1;
	}
     // -------------------------------
     // bitstream1 holds first 24 bits of the RF data, bitstream2 holds last 24 bits of the RF data
     // -------------------------------
    // Prepare transmit
    // send bits
    for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++) {
        data1=bitstream1; 
        data2=bitstream2; 
		digitalWrite(TX_DATA, HIGH);
		delayMicroseconds(fpulse*10 + (fpulse >> 1));  //335*9=3015 //270*10=2700
		for (unsigned short i=0; i<24; i++) {
			switch (data1 & B1) {
				case 0:
					digitalWrite(TX_DATA, LOW);
					delayMicroseconds(fpulse * 1);
					digitalWrite(TX_DATA, HIGH);
					delayMicroseconds(fpulse2 * 1);  
					break;
				case 1:
					digitalWrite(TX_DATA, LOW);
					delayMicroseconds(fpulse2 * 1);
					digitalWrite(TX_DATA, HIGH);
					delayMicroseconds(fpulse * 1);
					break;
			}
			//Next bit
			data1>>=1;
		}
		for (unsigned short i=0; i<24; i++) {
			switch (data2 & B1) {
				case 0:
					digitalWrite(TX_DATA, LOW);
					delayMicroseconds(fpulse * 1);
					digitalWrite(TX_DATA, HIGH);
					delayMicroseconds(fpulse2 * 1); // 335*3=1005 260*5=1300  260*4=1040
					break;
				case 1:
					digitalWrite(TX_DATA, LOW);
					delayMicroseconds(fpulse2 * 1);
					digitalWrite(TX_DATA, HIGH);
					delayMicroseconds(fpulse * 1);
					break;
			}
			//Next bit
			data2>>=1;
		}
		//Send termination/synchronisation-signal. 
		//digitalWrite(TX_DATA, HIGH);
		//delayMicroseconds(fpulse);
		digitalWrite(TX_DATA, LOW);
		delayMicroseconds(fpulse * 27); 

        //RawSignal.Pulses[98]=300/RawSignal.Multiply;
        //RawSignal.Pulses[99]=175/RawSignal.Multiply;
        //RawSignal.Delay=125;                             // Delay between RF packets

	}
    // End transmit
}

void HomeConfort_Send(unsigned long data1, unsigned long data2) { 
    int fpulse  = 270;                              // Pulse width in microseconds
    int fpulse2 = 710;                              // Pulse width in microseconds
    int fretrans = 10;                              // Number of code retransmissions

	unsigned long bitstream1 = 0L;
	unsigned long bitstream2 = 0L;
    // prepare data to send	
	for (unsigned short i=0; i<24; i++) {           // reverse data bits
		bitstream1<<=1;
		bitstream1|=(data1 & B1);
		data1>>=1;
	}
	for (unsigned short i=0; i<24; i++) {           // reverse data bits
		bitstream2<<=1;
		bitstream2|=(data2 & B1);
		data2>>=1;
	}
     // -------------------------------
     // bitstream1 holds first 24 bits of the RF data, bitstream2 holds last 24 bits of the RF data
     // -------------------------------
    // Prepare transmit
    // send bits
    for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++) {
        data1=bitstream1; 
        data2=bitstream2; 
		digitalWrite(TX_DATA, HIGH);
		//delayMicroseconds(fpulse);  //335
		delayMicroseconds(335);       
		digitalWrite(TX_DATA, LOW);
		delayMicroseconds(fpulse*10 + (fpulse >> 1));  //335*9=3015 //270*10=2700
		for (unsigned short i=0; i<24; i++) {
			switch (data1 & B1) {
				case 0:
					digitalWrite(TX_DATA, HIGH);
					delayMicroseconds(fpulse * 1);
					digitalWrite(TX_DATA, LOW);
					delayMicroseconds(fpulse2 * 1);  
					break;
				case 1:
					digitalWrite(TX_DATA, HIGH);
					delayMicroseconds(fpulse2 * 1);
					digitalWrite(TX_DATA, LOW);
					delayMicroseconds(fpulse * 1);
					break;
			}
			//Next bit
			data1>>=1;
		}
		for (unsigned short i=0; i<24; i++) {
			switch (data2 & B1) {
				case 0:
					digitalWrite(TX_DATA, HIGH);
					delayMicroseconds(fpulse * 1);
					digitalWrite(TX_DATA, LOW);
					delayMicroseconds(fpulse2 * 1); // 335*3=1005 260*5=1300  260*4=1040
					break;
				case 1:
					digitalWrite(TX_DATA, HIGH);
					delayMicroseconds(fpulse2 * 1);
					digitalWrite(TX_DATA, LOW);
					delayMicroseconds(fpulse * 1);
					break;
			}
			//Next bit
			data2>>=1;
		}
		//Send termination/synchronisation-signal. 
		digitalWrite(TX_DATA, HIGH);
		delayMicroseconds(fpulse);
		digitalWrite(TX_DATA, LOW);
		delayMicroseconds(fpulse * 27); 

        //RawSignal.Pulses[98]=300/RawSignal.Multiply;
        //RawSignal.Pulses[99]=175/RawSignal.Multiply;
        //RawSignal.Delay=125;                             // Delay between RF packets

	}
    // End transmit
}
*/
/* Switch sample :
20;E4;DEBUG;Pulses=511;Pulses(uSec)=
330,2760,180,600,180,600,630,180,630,180,180,600,630,180,630,150,210,600,630,150,210,600,630,180,180,600,180,600,630,180,180,600,180,600,180,600,630,180,630,180,630,180,630,180,180,600,630,180,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,630,180,180,600,180,600,180,600,180,600,210,600,210,600,630,150,210,180,
270,2760,180,600,180,600,630,180,630,180,180,600,630,180,630,180,180,600,630,180,180,600,630,180,180,600,180,600,630,180,180,600,180,600,180,600,630,180,630,180,630,180,630,180,180,600,630,150,210,600,210,600,210,600,210,600,180,600,180,600,180,600,180,600,180,600,180,600,180,630,180,630,180,630,180,630,180,630,180,600,180,600,630,180,180,600,180,600,180,600,180,600,180,600,180,600,630,180,180,150,     
270,2760,210,600,210,600,630,150,630,150,210,600,630,180,630,180,180,600,630,180,180,600,630,180,180,630,180,630,630,180,180,630,180,630,180,630,630,180,630,180,630,180,630,180,180,600,630,180,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,210,600,630,150,210,600,210,600,210,600,210,600,180,600,180,600,630,180,180,180,270,2760,180,600,180,600,630,180,630,180,180,600,630,180,630,180,180,600,630,180,180,600,630,180,180,600,180,600,630,180,180,600,180,600,210,600,630,150,630,150,630,150,630,150,210,600,630,180,180,600,180,600,180,600,180,600,180,630,180,630,180,630,180,630,180,630,180,630,180,630,180,600,180,600,180,600,180,600,180,600,180,600,630,180,180,600,180,600,180,600,180,600,180,600,180,600,630,180,180,180,270,2760,210,600,180,600,630,180,630,180,180,630,630,180,630,180,180,630,630,180,180,630,630,180,180,630,180,600,630,180,180,600,180,600,180,600,630,180,630,180,630,180,630,180,180,600,630,180,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,180,600,210,600,210,600,210,600,210,600,210,600,210,600,210,600,210,600,630,180,180,600,180,630,180,630,180,630,180,630,180,630,630,180,180,180,270,2760,180,600,180,600,630,180,630,180,180;


20;CA;DEBUG;Pulses=100;Pulses(uSec)=
    2490,180,630,150,630,600,180,600,180,150,630,600,180,600,180,150,630,630,180,150,630,630,180,150,630,150,630,630,180,150,630,150,630,150,630,630,180,600,180,600,180,600,180,150,630,600,180,150,630,150,630,150,630,150,630,150,630,150,630,150,630,150,630,150,630,150,630,150,630,150,630,150,630,150,630,150,630,150,630,150,630,150,630,150,630,150,630,150,630,150,630,150,630,150,630,630,180,180,60,6990;
*/

#define PLUGIN_011_RFLOW 270  // 300
#define PLUGIN_011_RFHIGH 720 // 800

void HomeConfort_Send(unsigned long bitstream1, unsigned long bitstream2)
{
   RawSignal.Repeats = 8;  // Number of RF packet retransmits
   RawSignal.Delay = 125;  // Delay between RF packets
   RawSignal.Number = 100; // Length

   uint32_t fdatabit;
   uint32_t fdatamask = 0x800000;
   // -------------------------------
   // bitstream1 holds first 24 bits of the RF data, bitstream2 holds last 24 bits of the RF data
   // -------------------------------
   RawSignal.Pulses[1] = 2600 / RawSignal.Multiply;

   for (byte i = 2; i < 103; i = i + 2)
   {
      if (i < 50)
      {                                     // first 24 bits
         fdatabit = bitstream1 & fdatamask; // Get most left bit
         bitstream1 = (bitstream1 << 1);    // Shift left

         if (fdatabit != fdatamask)
         { // Write 0
            RawSignal.Pulses[i] = PLUGIN_011_RFLOW / RawSignal.Multiply;
            RawSignal.Pulses[i + 1] = PLUGIN_011_RFHIGH / RawSignal.Multiply;
         }
         else
         { // Write 1
            RawSignal.Pulses[i] = PLUGIN_011_RFHIGH / RawSignal.Multiply;
            RawSignal.Pulses[i + 1] = PLUGIN_011_RFLOW / RawSignal.Multiply;
         }
      }
      else
      {
         fdatabit = bitstream2 & fdatamask; // Get most left bit
         bitstream2 = (bitstream2 << 1);    // Shift left

         if (fdatabit != fdatamask)
         { // Write 0
            RawSignal.Pulses[i] = PLUGIN_011_RFLOW / RawSignal.Multiply;
            RawSignal.Pulses[i + 1] = PLUGIN_011_RFHIGH / RawSignal.Multiply;
         }
         else
         { // Write 1
            RawSignal.Pulses[i] = PLUGIN_011_RFHIGH / RawSignal.Multiply;
            RawSignal.Pulses[i + 1] = PLUGIN_011_RFLOW / RawSignal.Multiply;
         }
      }
   }
   RawSignal.Pulses[98] = 300 / RawSignal.Multiply;
   RawSignal.Pulses[99] = 175 / RawSignal.Multiply;

   RawSendRF();
}
#endif // PLUGIN_TX_011
