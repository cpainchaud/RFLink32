// Custom RFLink R33 for Nodo Small boards

// Additional commands provided by plugin 250:
//  10;FREEMEM;
//  10;SOUND;<frequency>;<duration msec>;
//  10;GPIO;<pin>;<0/1>;
//  10;STATUS;<pin>;

// R33 custom V1.0
//  - Removed all unneeded plugins to reduce RAM resource load, only KAKU loaded
//  - Raw pulses max 256 to reduce RAM resource load
//  - RFLinkHW() function changed to stub file (not in use on Nodo Small units)
//  - Added #define PIN_SPEAKER 6 (used in plugin 250)
//  - Removed BSF pins (not in use on Nodo Small units)
//  - Added Plugin_250_Scan() to scanevent
//  - Added IR receive
//  - Autoreset for special two pin RX/TX upload
//  - Changed KAKU PULSEMID from 510->600 (plugin 003)
//  - Changed many sprintf/strcasecmp to sprintf_P/strcasecmp_P to save RAM

// *********************************************************************************************************************************
// * Arduino project "Nodo RadioFrequencyLink aka Nodo RFLink Version 1.1" 
// * © Copyright 2015 StuntTeam - NodoRFLink 
// * Portions © Copyright 2010..2015 Paul Tonkes (original Nodo 3.7 code)
// *
// *                                       Nodo RadioFrequencyLink aka Nodo RFLink Version 1.1
// *                                                      
// ********************************************************************************************************************************
// * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License 
// * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty 
// * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// * You received a copy of the GNU General Public License along with this program in file 'COPYING.TXT'.
// * For more information on GPL licensing: http://www.gnu.org/licenses
// ********************************************************************************************************************************

// ================================================================================================================================
// Supply the full path to the RFLink files in the define below 
//
// Geef in onderstaande "define" regel het volledige pad op waar de .ino bestanden zich bevinden die je nu geopend hebt.
// ================================================================================================================================

#define SKETCH_PATH S:\Software\RFLink\source\RFLink R33 Small2\RFLink

// ================================================================================================================================
// IMPORTANT NOTE: This code can run on an Arduino Uno / Nano / Pro Mini / Nodo Small / Arduino Nodo 1.6,1.7
//                 But this is officially unsupported!
//                 It should work as long as only a few small plugins are loaded
// ================================================================================================================================

