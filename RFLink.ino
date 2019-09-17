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

#include <avr/power.h>

#define BUILDNR                         0x07                                    // shown in version
#define REVNR                           0x33                                    // shown in version and startup string
#define MIN_RAW_PULSES                    74 // 20                              // =8 bits. Minimal number of bits*2 that need to have been received before we spend CPU time on decoding the signal.
#define RAWSIGNAL_SAMPLE_RATE             30                                    // Sample width / resolution in uSec for raw RF pulses.
#define MIN_PULSE_LENGTH                  60 // 25                              // Pulses shorter than this value in uSec. will be seen as garbage and not taken as actual pulses.
#define SIGNAL_TIMEOUT                     3 // 7                               // Timeout, after this time in mSec. the RF signal will be considered to have stopped.
#define SIGNAL_REPEAT_TIME               250 // 500                             // Time in mSec. in which the same RF signal should not be accepted again. Filters out retransmits.
#define BAUD                           57600                                    // Baudrate for serial communication.
#define TRANSMITTER_STABLE_DELAY         500                                    // delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms).
#define RAW_BUFFER_SIZE                  148 // 512                             // Maximum number of pulses that is received in one go.
#define PLUGIN_MAX                         5 // 55                              // Maximum number of Receive plugins
#define PLUGIN_TX_MAX                      0 // 26                              // Maximum number of Transmit plugins
#define SCAN_HIGH_TIME                    50                                    // time interval in ms. fast processing for background tasks
#define FOCUS_TIME                        50                                    // Duration in mSec. that, after receiving serial data from USB only the serial port is checked. 
#define INPUT_COMMAND_SIZE                60                                    // Maximum number of characters that a command via serial can be.
#define PRINT_BUFFER_SIZE                 60                                    // Maximum number of characters that a command should print in one go via the print buffer.

#define VALUE_PAIR                      44
#define VALUE_ALLOFF                    55
#define VALUE_OFF                       74
#define VALUE_ON                        75
#define VALUE_DIM                       76
#define VALUE_BRIGHT                    77
#define VALUE_UP                        78
#define VALUE_DOWN                      79
#define VALUE_STOP                      80
#define VALUE_CONFIRM                   81
#define VALUE_LIMIT                     82
#define VALUE_ALLON                     141

// PIN Definition

// Uno
#define PIN_RF_TX_VCC               NOT_A_PIN                                  // +5 volt / Vcc power to the transmitter on this pin
#define PIN_RF_TX_GND               NOT_A_PIN                                  // Ground power to the transmitter on this pin
#define PIN_RF_TX_DATA              NOT_A_PIN                                  // Data to the 433Mhz transmitter on this pin
#define PIN_RF_RX_VCC               4                                          // Power to the receiver on this pin
#define PIN_RF_RX_GND               NOT_A_PIN                                  // Ground to the receiver on this pin
#define PIN_RF_RX_DATA              2                                          // On this input, the 433Mhz-RF signal is received. LOW when no signal.

/*
  // Mega
  #define PIN_BSF_0                   22                                          // Board Specific Function lijn-0
  #define PIN_BSF_1                   23                                          // Board Specific Function lijn-1
  #define PIN_BSF_2                   24                                          // Board Specific Function lijn-2
  #define PIN_BSF_3                   25                                          // Board Specific Function lijn-3
  #define PIN_RF_TX_VCC               15                                          // +5 volt / Vcc power to the transmitter on this pin
  #define PIN_RF_TX_DATA              14                                          // Data to the 433Mhz transmitter on this pin
  #define PIN_RF_RX_VCC               16                                          // Power to the receiver on this pin
  #define PIN_RF_RX_DATA              19                                          // On this input, the 433Mhz-RF signal is received. LOW when no signal.
*/
//****************************************************************************************************************************************
