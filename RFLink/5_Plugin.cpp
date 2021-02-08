// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#include <Arduino.h>
#include "RFLink.h"
#include "2_Signal.h"
#include "5_Plugin.h"

#ifdef RFLINK_ASYNC_RECEIVER_ENABLED
using namespace AsyncSignalScanner;
#endif //RFLINK_ASYNC_RECEIVER_ENABLED

boolean (*Plugin_ptr[PLUGIN_MAX])(byte, char *); // Receive plugins
byte Plugin_id[PLUGIN_MAX];
byte Plugin_State[PLUGIN_MAX];
#ifndef ARDUINO_AVR_UNO // Optimize memory limite to 2048 bytes on arduino uno
String Plugin_Description[PLUGIN_MAX];
#endif

boolean (*PluginTX_ptr[PLUGIN_TX_MAX])(byte, char *); // Trasmit plugins
byte PluginTX_id[PLUGIN_TX_MAX];
byte PluginTX_State[PLUGIN_TX_MAX];

boolean RFDebug = RFDebug_0;     // debug RF signals with plugin 001 (no decode)
boolean QRFDebug = QRFDebug_0;   // debug RF signals with plugin 001 but no multiplication (faster?, compact)
boolean RFUDebug = RFUDebug_0;   // debug RF signals with plugin 254 (decode 1st)
boolean QRFUDebug = QRFUDebug_0; // debug RF signals with plugin 254 but no multiplication (faster?, compact)

/**********************************************************************************************\
 * Load plugins
\*********************************************************************************************/
#include "./Plugins/_Plugin_Config_01.h"

#ifdef PLUGIN_001
#include "./Plugins/Plugin_001.c"
#endif

#ifdef PLUGIN_002
#include "./Plugins/Plugin_002.c"
#endif

#ifdef PLUGIN_003
#include "./Plugins/Plugin_003.c"
#endif

#ifdef PLUGIN_004
#include "./Plugins/Plugin_004.c"
#endif

#ifdef PLUGIN_005
#include "./Plugins/Plugin_005.c"
#endif

#ifdef PLUGIN_006
#include "./Plugins/Plugin_006.c"
#endif

#ifdef PLUGIN_007
#include "./Plugins/Plugin_007.c"
#endif

#ifdef PLUGIN_008
#include "./Plugins/Plugin_008.c"
#endif

#ifdef PLUGIN_009
#include "./Plugins/Plugin_009.c"
#endif

#ifdef PLUGIN_010
#include "./Plugins/Plugin_010.c"
#endif

#ifdef PLUGIN_011
#include "./Plugins/Plugin_011.c"
#endif

#ifdef PLUGIN_012
#include "./Plugins/Plugin_012.c"
#endif

#ifdef PLUGIN_013
#include "./Plugins/Plugin_013.c"
#endif

#ifdef PLUGIN_014
#include "./Plugins/Plugin_014.c"
#endif

#ifdef PLUGIN_015
#include "./Plugins/Plugin_015.c"
#endif

#ifdef PLUGIN_016
#include "./Plugins/Plugin_016.c"
#endif

#ifdef PLUGIN_017
#include "./Plugins/Plugin_017.c"
#endif

#ifdef PLUGIN_018
#include "./Plugins/Plugin_018.c"
#endif

#ifdef PLUGIN_019
#include "./Plugins/Plugin_019.c"
#endif

#ifdef PLUGIN_020
#include "./Plugins/Plugin_020.c"
#endif

#ifdef PLUGIN_021
#include "./Plugins/Plugin_021.c"
#endif

#ifdef PLUGIN_022
#include "./Plugins/Plugin_022.c"
#endif

#ifdef PLUGIN_023
#include "./Plugins/Plugin_023.c"
#endif

#ifdef PLUGIN_024
#include "./Plugins/Plugin_024.c"
#endif

#ifdef PLUGIN_025
#include "./Plugins/Plugin_025.c"
#endif

#ifdef PLUGIN_026
#include "./Plugins/Plugin_026.c"
#endif

#ifdef PLUGIN_027
#include "./Plugins/Plugin_027.c"
#endif

#ifdef PLUGIN_028
#include "./Plugins/Plugin_028.c"
#endif

#ifdef PLUGIN_029
#include "./Plugins/Plugin_029.c"
#endif

#ifdef PLUGIN_030
#include "./Plugins/Plugin_030.c"
#endif

#ifdef PLUGIN_031
#include "./Plugins/Plugin_031.c"
#endif

#ifdef PLUGIN_032
#include "./Plugins/Plugin_032.c"
#endif

#ifdef PLUGIN_033
#include "./Plugins/Plugin_033.c"
#endif

#ifdef PLUGIN_034
#include "./Plugins/Plugin_034.c"
#endif

#ifdef PLUGIN_035
#include "./Plugins/Plugin_035.c"
#endif

#ifdef PLUGIN_036
#include "./Plugins/Plugin_036.c"
#endif

#ifdef PLUGIN_037
#include "./Plugins/Plugin_037.c"
#endif

#ifdef PLUGIN_038
#include "./Plugins/Plugin_038.c"
#endif

#ifdef PLUGIN_039
#include "./Plugins/Plugin_039.c"
#endif

#ifdef PLUGIN_040
#include "./Plugins/Plugin_040.c"
#endif

#ifdef PLUGIN_041
#include "./Plugins/Plugin_041.c"
#endif

#ifdef PLUGIN_042
#include "./Plugins/Plugin_042.c"
#endif

#ifdef PLUGIN_043
#include "./Plugins/Plugin_043.c"
#endif

#ifdef PLUGIN_044
#include "./Plugins/Plugin_044.c"
#endif

#ifdef PLUGIN_045
#include "./Plugins/Plugin_045.c"
#endif

#ifdef PLUGIN_046
#include "./Plugins/Plugin_046.c"
#endif

#ifdef PLUGIN_047
#include "./Plugins/Plugin_047.c"
#endif

#ifdef PLUGIN_048
#include "./Plugins/Plugin_048.c"
#endif

#ifdef PLUGIN_049
#include "./Plugins/Plugin_049.c"
#endif

#ifdef PLUGIN_050
#include "./Plugins/Plugin_050.c"
#endif

#ifdef PLUGIN_051
#include "./Plugins/Plugin_051.c"
#endif

#ifdef PLUGIN_052
#include "./Plugins/Plugin_052.c"
#endif

#ifdef PLUGIN_053
#include "./Plugins/Plugin_053.c"
#endif

#ifdef PLUGIN_054
#include "./Plugins/Plugin_054.c"
#endif

#ifdef PLUGIN_055
#include "./Plugins/Plugin_055.c"
#endif

#ifdef PLUGIN_056
#include "./Plugins/Plugin_056.c"
#endif

#ifdef PLUGIN_057
#include "./Plugins/Plugin_057.c"
#endif

#ifdef PLUGIN_058
#include "./Plugins/Plugin_058.c"
#endif

#ifdef PLUGIN_059
#include "./Plugins/Plugin_059.c"
#endif

#ifdef PLUGIN_060
#include "./Plugins/Plugin_060.c"
#endif

#ifdef PLUGIN_061
#include "./Plugins/Plugin_061.c"
#endif

#ifdef PLUGIN_062
#include "./Plugins/Plugin_062.c"
#endif

#ifdef PLUGIN_063
#include "./Plugins/Plugin_063.c"
#endif

#ifdef PLUGIN_064
#include "./Plugins/Plugin_064.c"
#endif

#ifdef PLUGIN_065
#include "./Plugins/Plugin_065.c"
#endif

#ifdef PLUGIN_066
#include "./Plugins/Plugin_066.c"
#endif

#ifdef PLUGIN_067
#include "./Plugins/Plugin_067.c"
#endif

#ifdef PLUGIN_068
#include "./Plugins/Plugin_068.c"
#endif

#ifdef PLUGIN_069
#include "./Plugins/Plugin_069.c"
#endif

#ifdef PLUGIN_070
#include "./Plugins/Plugin_070.c"
#endif

#ifdef PLUGIN_071
#include "./Plugins/Plugin_071.c"
#endif

#ifdef PLUGIN_072
#include "./Plugins/Plugin_072.c"
#endif

#ifdef PLUGIN_073
#include "./Plugins/Plugin_073.c"
#endif

#ifdef PLUGIN_074
#include "./Plugins/Plugin_074.c"
#endif

#ifdef PLUGIN_075
#include "./Plugins/Plugin_075.c"
#endif

#ifdef PLUGIN_076
#include "./Plugins/Plugin_076.c"
#endif

#ifdef PLUGIN_077
#include "./Plugins/Plugin_077.c"
#endif

#ifdef PLUGIN_078
#include "./Plugins/Plugin_078.c"
#endif

#ifdef PLUGIN_079
#include "./Plugins/Plugin_079.c"
#endif

#ifdef PLUGIN_080
#include "./Plugins/Plugin_080.c"
#endif

#ifdef PLUGIN_081
#include "./Plugins/Plugin_081.c"
#endif

#ifdef PLUGIN_082
#include "./Plugins/Plugin_082.c"
#endif

#ifdef PLUGIN_083
//#define PLUGIN_083_DEBUG
#include "./Plugins/Plugin_083.c"
#endif

#ifdef PLUGIN_084
#include "./Plugins/Plugin_084.c"
#endif

#ifdef PLUGIN_085
#include "./Plugins/Plugin_085.c"
#endif

#ifdef PLUGIN_086
#include "./Plugins/Plugin_086.c"
#endif

#ifdef PLUGIN_087
#include "./Plugins/Plugin_087.c"
#endif

#ifdef PLUGIN_088
#include "./Plugins/Plugin_088.c"
#endif

#ifdef PLUGIN_089
#include "./Plugins/Plugin_089.c"
#endif

#ifdef PLUGIN_090
#include "./Plugins/Plugin_090.c"
#endif

#ifdef PLUGIN_091
#include "./Plugins/Plugin_091.c"
#endif

#ifdef PLUGIN_092
#include "./Plugins/Plugin_092.c"
#endif

#ifdef PLUGIN_093
#include "./Plugins/Plugin_093.c"
#endif

#ifdef PLUGIN_094
#include "./Plugins/Plugin_094.c"
#endif

#ifdef PLUGIN_095
#include "./Plugins/Plugin_095.c"
#endif

#ifdef PLUGIN_096
#include "./Plugins/Plugin_096.c"
#endif

#ifdef PLUGIN_097
#include "./Plugins/Plugin_097.c"
#endif

#ifdef PLUGIN_098
#include "./Plugins/Plugin_098.c"
#endif

#ifdef PLUGIN_099
#include "./Plugins/Plugin_099.c"
#endif

#ifdef PLUGIN_100
#include "./Plugins/Plugin_100.c"
#endif

#ifdef PLUGIN_101
#include "./Plugins/Plugin_101.c"
#endif

#ifdef PLUGIN_102
#include "./Plugins/Plugin_102.c"
#endif

#ifdef PLUGIN_103
#include "./Plugins/Plugin_103.c"
#endif

#ifdef PLUGIN_104
#include "./Plugins/Plugin_104.c"
#endif

#ifdef PLUGIN_105
#include "./Plugins/Plugin_105.c"
#endif

#ifdef PLUGIN_106
#include "./Plugins/Plugin_106.c"
#endif

#ifdef PLUGIN_107
#include "./Plugins/Plugin_107.c"
#endif

#ifdef PLUGIN_108
#include "./Plugins/Plugin_108.c"
#endif

#ifdef PLUGIN_109
#include "./Plugins/Plugin_109.c"
#endif

#ifdef PLUGIN_110
#include "./Plugins/Plugin_110.c"
#endif

#ifdef PLUGIN_111
#include "./Plugins/Plugin_111.c"
#endif

#ifdef PLUGIN_112
#include "./Plugins/Plugin_112.c"
#endif

#ifdef PLUGIN_113
#include "./Plugins/Plugin_113.c"
#endif

#ifdef PLUGIN_114
#include "./Plugins/Plugin_114.c"
#endif

#ifdef PLUGIN_115
#include "./Plugins/Plugin_115.c"
#endif

#ifdef PLUGIN_116
#include "./Plugins/Plugin_116.c"
#endif

#ifdef PLUGIN_117
#include "./Plugins/Plugin_117.c"
#endif

#ifdef PLUGIN_118
#include "./Plugins/Plugin_118.c"
#endif

#ifdef PLUGIN_119
#include "./Plugins/Plugin_119.c"
#endif

#ifdef PLUGIN_120
#include "./Plugins/Plugin_120.c"
#endif

#ifdef PLUGIN_250
#include "./Plugins/Plugin_250.c"
#endif

#ifdef PLUGIN_251
#include "./Plugins/Plugin_251.c"
#endif

#ifdef PLUGIN_252
#include "./Plugins/Plugin_252.c"
#endif

#ifdef PLUGIN_253
#include "./Plugins/Plugin_253.c"
#endif

#ifdef PLUGIN_254
#include "./Plugins/Plugin_254.c"
#endif

#ifdef PLUGIN_255
#include "./Plugins/Plugin_255.c"
#endif
/*********************************************************************************************/
void PluginInit(void)
{
  byte x;

  // Wis de pointertabel voor de plugins.
  for (x = 0; x < PLUGIN_MAX; x++)
  {
    Plugin_ptr[x] = 0;
    Plugin_id[x] = 0;
    Plugin_State[x] = P_Disabled;
  }

  x = 0;

#ifdef PLUGIN_001
  Plugin_id[x] = 1;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_001;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_001;
#endif

#ifdef PLUGIN_002
  Plugin_id[x] = 2;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_002;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_002;
#endif

#ifdef PLUGIN_003
  Plugin_id[x] = 3;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_003;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_003;
#endif

#ifdef PLUGIN_004
  Plugin_id[x] = 4;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_004;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_004;
#endif

#ifdef PLUGIN_005
  Plugin_id[x] = 5;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_005;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_005;
#endif

#ifdef PLUGIN_006
  Plugin_id[x] = 6;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_006;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_006;
#endif

#ifdef PLUGIN_007
  Plugin_id[x] = 7;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_007;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_007;
#endif

#ifdef PLUGIN_008
  Plugin_id[x] = 8;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_008;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_008;
#endif

#ifdef PLUGIN_009
  Plugin_id[x] = 9;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_009;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_009;
#endif

#ifdef PLUGIN_010
  Plugin_id[x] = 10;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_010;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_010;
#endif

#ifdef PLUGIN_011
  Plugin_id[x] = 11;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_011;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_011;
#endif

#ifdef PLUGIN_012
  Plugin_id[x] = 12;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_012;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_012;
#endif

#ifdef PLUGIN_013
  Plugin_id[x] = 13;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_013;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_013;
#endif

#ifdef PLUGIN_014
  Plugin_id[x] = 14;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_014;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_014;
#endif

#ifdef PLUGIN_015
  Plugin_id[x] = 15;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_015;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_015;
#endif

#ifdef PLUGIN_016
  Plugin_id[x] = 16;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_016;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_016;
#endif

#ifdef PLUGIN_017
  Plugin_id[x] = 17;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_017;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_017;
#endif

#ifdef PLUGIN_018
  Plugin_id[x] = 18;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_018;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_018;
#endif

#ifdef PLUGIN_019
  Plugin_id[x] = 19;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_019;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_019;
#endif

#ifdef PLUGIN_020
  Plugin_id[x] = 20;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_020;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_020;
#endif

#ifdef PLUGIN_021
  Plugin_id[x] = 21;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_021;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_021;
#endif

#ifdef PLUGIN_022
  Plugin_id[x] = 22;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_022;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_022;
#endif

#ifdef PLUGIN_023
  Plugin_id[x] = 23;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_023;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_023;
#endif

#ifdef PLUGIN_024
  Plugin_id[x] = 24;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_024;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_024;
#endif

#ifdef PLUGIN_025
  Plugin_id[x] = 25;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_025;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_025;
#endif

#ifdef PLUGIN_026
  Plugin_id[x] = 26;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_026;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_026;
#endif

#ifdef PLUGIN_027
  Plugin_id[x] = 27;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_027;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_027;
#endif

#ifdef PLUGIN_028
  Plugin_id[x] = 28;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_028;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_028;
#endif

#ifdef PLUGIN_029
  Plugin_id[x] = 29;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_029;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_029;
#endif

#ifdef PLUGIN_030
  Plugin_id[x] = 30;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_030;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_030;
#endif

#ifdef PLUGIN_031
  Plugin_id[x] = 31;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_031;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_031;
#endif

#ifdef PLUGIN_032
  Plugin_id[x] = 32;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_032;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_032;
#endif

#ifdef PLUGIN_033
  Plugin_id[x] = 33;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_033;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_033;
#endif

#ifdef PLUGIN_034
  Plugin_id[x] = 34;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_034;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_034;
#endif

#ifdef PLUGIN_035
  Plugin_id[x] = 35;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_035;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_035;
#endif

#ifdef PLUGIN_036
  Plugin_id[x] = 36;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_036;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_036;
#endif

#ifdef PLUGIN_037
  Plugin_id[x] = 37;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_037;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_037;
#endif

#ifdef PLUGIN_038
  Plugin_id[x] = 38;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_038;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_038;
#endif

#ifdef PLUGIN_039
  Plugin_id[x] = 39;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_039;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_039;
#endif

#ifdef PLUGIN_040
  Plugin_id[x] = 40;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_040;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_040;
#endif

#ifdef PLUGIN_041
  Plugin_id[x] = 41;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_041;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_041;
#endif

#ifdef PLUGIN_042
  Plugin_id[x] = 42;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_042;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_042;
#endif

#ifdef PLUGIN_043
  Plugin_id[x] = 43;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_043;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_043;
#endif

#ifdef PLUGIN_044
  Plugin_id[x] = 44;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_044;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_044;
#endif

#ifdef PLUGIN_045
  Plugin_id[x] = 45;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_045;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_045;
#endif

#ifdef PLUGIN_046
  Plugin_id[x] = 46;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_046;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_046;
#endif

#ifdef PLUGIN_047
  Plugin_id[x] = 47;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_047;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_047;
#endif

#ifdef PLUGIN_048
  Plugin_id[x] = 48;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_048;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_048;
#endif

#ifdef PLUGIN_049
  Plugin_id[x] = 49;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_049;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_049;
#endif

#ifdef PLUGIN_050
  Plugin_id[x] = 50;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_050;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_050;
#endif

#ifdef PLUGIN_051
  Plugin_id[x] = 51;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_051;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_051;
#endif

#ifdef PLUGIN_052
  Plugin_id[x] = 52;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_052;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_052;
#endif

#ifdef PLUGIN_053
  Plugin_id[x] = 53;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_053;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_053;
#endif

#ifdef PLUGIN_054
  Plugin_id[x] = 54;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_054;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_054;
#endif

#ifdef PLUGIN_055
  Plugin_id[x] = 55;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_055;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_055;
#endif

#ifdef PLUGIN_056
  Plugin_id[x] = 56;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_056;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_056;
#endif

#ifdef PLUGIN_057
  Plugin_id[x] = 57;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_057;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_057;
#endif

#ifdef PLUGIN_058
  Plugin_id[x] = 58;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_058;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_058;
#endif

#ifdef PLUGIN_059
  Plugin_id[x] = 59;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_059;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_059;
#endif

#ifdef PLUGIN_060
  Plugin_id[x] = 60;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_060;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_060;
#endif

#ifdef PLUGIN_061
  Plugin_id[x] = 61;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_061;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_061;
#endif

#ifdef PLUGIN_062
  Plugin_id[x] = 62;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_062;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_062;
#endif

#ifdef PLUGIN_063
  Plugin_id[x] = 63;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_063;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_063;
#endif

#ifdef PLUGIN_064
  Plugin_id[x] = 64;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_064;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_064;
#endif

#ifdef PLUGIN_065
  Plugin_id[x] = 65;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_065;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_065;
#endif

#ifdef PLUGIN_066
  Plugin_id[x] = 66;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_066;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_066;
#endif

#ifdef PLUGIN_067
  Plugin_id[x] = 67;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_067;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_067;
#endif

#ifdef PLUGIN_068
  Plugin_id[x] = 68;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_068;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_068;
#endif

#ifdef PLUGIN_069
  Plugin_id[x] = 69;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_069;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_069;
#endif

#ifdef PLUGIN_070
  Plugin_id[x] = 70;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_070;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_070;
#endif

#ifdef PLUGIN_071
  Plugin_id[x] = 71;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_071;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_071;
#endif

#ifdef PLUGIN_072
  Plugin_id[x] = 72;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_072;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_072;
#endif

#ifdef PLUGIN_073
  Plugin_id[x] = 73;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_073;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_073;
#endif

#ifdef PLUGIN_074
  Plugin_id[x] = 74;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_074;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_074;
#endif

#ifdef PLUGIN_075
  Plugin_id[x] = 75;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_075;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_075;
#endif

#ifdef PLUGIN_076
  Plugin_id[x] = 76;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_076;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_076;
#endif

#ifdef PLUGIN_077
  Plugin_id[x] = 77;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_077;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_077;
#endif

#ifdef PLUGIN_078
  Plugin_id[x] = 78;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_078;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_078;
#endif

#ifdef PLUGIN_079
  Plugin_id[x] = 79;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_079;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_079;
#endif

#ifdef PLUGIN_080
  Plugin_id[x] = 80;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_080;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_080;
#endif

#ifdef PLUGIN_081
  Plugin_id[x] = 81;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_081;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_081;
#endif

#ifdef PLUGIN_082
  Plugin_id[x] = 82;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_082;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_082;
#endif

#ifdef PLUGIN_083
  Plugin_id[x] = 83;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_083;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_083;
#endif

#ifdef PLUGIN_084
  Plugin_id[x] = 84;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_084;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_084;
#endif

#ifdef PLUGIN_085
  Plugin_id[x] = 85;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_085;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_085;
#endif

#ifdef PLUGIN_086
  Plugin_id[x] = 86;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_086;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_086;
#endif

#ifdef PLUGIN_087
  Plugin_id[x] = 87;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_087;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_087;
#endif

#ifdef PLUGIN_088
  Plugin_id[x] = 88;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_088;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_088;
#endif

#ifdef PLUGIN_089
  Plugin_id[x] = 89;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_089;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_089;
#endif

#ifdef PLUGIN_090
  Plugin_id[x] = 90;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_090;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_090;
#endif

#ifdef PLUGIN_091
  Plugin_id[x] = 91;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_091;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_091;
#endif

#ifdef PLUGIN_092
  Plugin_id[x] = 92;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_092;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_092;
#endif

#ifdef PLUGIN_093
  Plugin_id[x] = 93;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_093;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_093;
#endif

#ifdef PLUGIN_094
  Plugin_id[x] = 94;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_094;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_094;
#endif

#ifdef PLUGIN_095
  Plugin_id[x] = 95;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_095;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_095;
#endif

#ifdef PLUGIN_096
  Plugin_id[x] = 96;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_096;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_096;
#endif

#ifdef PLUGIN_097
  Plugin_id[x] = 97;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_097;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_097;
#endif

#ifdef PLUGIN_098
  Plugin_id[x] = 98;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_098;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_098;
#endif

#ifdef PLUGIN_099
  Plugin_id[x] = 99;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_099;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_099;
#endif

#ifdef PLUGIN_100
  Plugin_id[x] = 100;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_100;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_100;
#endif

#ifdef PLUGIN_101
  Plugin_id[x] = 101;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_101;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_101;
#endif

#ifdef PLUGIN_102
  Plugin_id[x] = 102;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_102;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_102;
#endif

#ifdef PLUGIN_103
  Plugin_id[x] = 103;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_103;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_103;
#endif

#ifdef PLUGIN_104
  Plugin_id[x] = 104;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_104;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_104;
#endif

#ifdef PLUGIN_105
  Plugin_id[x] = 105;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_105;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_105;
#endif

#ifdef PLUGIN_106
  Plugin_id[x] = 106;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_106;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_106;
#endif

#ifdef PLUGIN_107
  Plugin_id[x] = 107;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_107;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_107;
#endif

#ifdef PLUGIN_108
  Plugin_id[x] = 108;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_108;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_108;
#endif

#ifdef PLUGIN_109
  Plugin_id[x] = 109;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_109;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_109;
#endif

#ifdef PLUGIN_110
  Plugin_id[x] = 110;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_110;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_110;
#endif

#ifdef PLUGIN_111
  Plugin_id[x] = 111;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_111;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_111;
#endif

#ifdef PLUGIN_112
  Plugin_id[x] = 112;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_112;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_112;
#endif

#ifdef PLUGIN_113
  Plugin_id[x] = 113;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_113;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_113;
#endif

#ifdef PLUGIN_114
  Plugin_id[x] = 114;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_114;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_114;
#endif

#ifdef PLUGIN_115
  Plugin_id[x] = 115;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_115;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_115;
#endif

#ifdef PLUGIN_116
  Plugin_id[x] = 116;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_116;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_116;
#endif

#ifdef PLUGIN_117
  Plugin_id[x] = 117;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_117;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_117;
#endif

#ifdef PLUGIN_118
  Plugin_id[x] = 118;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_118;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_118;
#endif

#ifdef PLUGIN_119
  Plugin_id[x] = 119;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_119;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_119;
#endif

#ifdef PLUGIN_120
  Plugin_id[x] = 120;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_120;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_120;
#endif

#ifdef PLUGIN_250
  Plugin_id[x] = 250;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_250;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_250;
#endif

#ifdef PLUGIN_251
  Plugin_id[x] = 251;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_251;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_251;
#endif

#ifdef PLUGIN_252
  Plugin_id[x] = 252;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_252;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_252;
#endif

#ifdef PLUGIN_253
  Plugin_id[x] = 253;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_253;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_253;
#endif

#ifdef PLUGIN_254
  Plugin_id[x] = 254;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_254;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_254;
#endif

#ifdef PLUGIN_255
  Plugin_id[x] = 255;
#ifndef ARDUINO_AVR_UNO
  Plugin_Description[x] = PLUGIN_DESC_255;
#endif
  Plugin_State[x] = P_Enabled;
  Plugin_ptr[x++] = &Plugin_255;
#endif

  // Initialiseer alle plugins door aanroep met verwerkingsparameter PLUGIN_INIT
  PluginInitCall(0, 0);
}
/*********************************************************************************************/
void PluginTXInit(void)
{
  byte x;

  // Wis de pointertabel voor de plugins.
  for (x = 0; x < PLUGIN_TX_MAX; x++)
  {
    PluginTX_ptr[x] = 0;
    PluginTX_id[x] = 0;
  }

  x = 0;

#ifdef PLUGIN_TX_001
  PluginTX_id[x] = 1;
  PluginTX_ptr[x++] = &PluginTX_001;
#endif

#ifdef PLUGIN_TX_002
  PluginTX_id[x] = 2;
  PluginTX_ptr[x++] = &PluginTX_002;
#endif

#ifdef PLUGIN_TX_003
  PluginTX_id[x] = 3;
  PluginTX_ptr[x++] = &PluginTX_003;
#endif

#ifdef PLUGIN_TX_004
  PluginTX_id[x] = 4;
  PluginTX_ptr[x++] = &PluginTX_004;
#endif

#ifdef PLUGIN_TX_005
  PluginTX_id[x] = 5;
  PluginTX_ptr[x++] = &PluginTX_005;
#endif

#ifdef PLUGIN_TX_006
  PluginTX_id[x] = 6;
  PluginTX_ptr[x++] = &PluginTX_006;
#endif

#ifdef PLUGIN_TX_007
  PluginTX_id[x] = 7;
  PluginTX_ptr[x++] = &PluginTX_007;
#endif

#ifdef PLUGIN_TX_008
  PluginTX_id[x] = 8;
  PluginTX_ptr[x++] = &PluginTX_008;
#endif

#ifdef PLUGIN_TX_009
  PluginTX_id[x] = 9;
  PluginTX_ptr[x++] = &PluginTX_009;
#endif

#ifdef PLUGIN_TX_010
  PluginTX_id[x] = 10;
  PluginTX_ptr[x++] = &PluginTX_010;
#endif

#ifdef PLUGIN_TX_011
  PluginTX_id[x] = 11;
  PluginTX_ptr[x++] = &PluginTX_011;
#endif

#ifdef PLUGIN_TX_012
  PluginTX_id[x] = 12;
  PluginTX_ptr[x++] = &PluginTX_012;
#endif

#ifdef PLUGIN_TX_013
  PluginTX_id[x] = 13;
  PluginTX_ptr[x++] = &PluginTX_013;
#endif

#ifdef PLUGIN_TX_014
  PluginTX_id[x] = 14;
  PluginTX_ptr[x++] = &PluginTX_014;
#endif

#ifdef PLUGIN_TX_015
  PluginTX_id[x] = 15;
  PluginTX_ptr[x++] = &PluginTX_015;
#endif

#ifdef PLUGIN_TX_016
  PluginTX_id[x] = 16;
  PluginTX_ptr[x++] = &PluginTX_016;
#endif

#ifdef PLUGIN_TX_017
  PluginTX_id[x] = 17;
  PluginTX_ptr[x++] = &PluginTX_017;
#endif

#ifdef PLUGIN_TX_018
  PluginTX_id[x] = 18;
  PluginTX_ptr[x++] = &PluginTX_018;
#endif

#ifdef PLUGIN_TX_019
  PluginTX_id[x] = 19;
  PluginTX_ptr[x++] = &PluginTX_019;
#endif

#ifdef PLUGIN_TX_020
  PluginTX_id[x] = 20;
  PluginTX_ptr[x++] = &PluginTX_020;
#endif

#ifdef PLUGIN_TX_021
  PluginTX_id[x] = 21;
  PluginTX_ptr[x++] = &PluginTX_021;
#endif

#ifdef PLUGIN_TX_022
  PluginTX_id[x] = 22;
  PluginTX_ptr[x++] = &PluginTX_022;
#endif

#ifdef PLUGIN_TX_023
  PluginTX_id[x] = 23;
  PluginTX_ptr[x++] = &PluginTX_023;
#endif

#ifdef PLUGIN_TX_024
  PluginTX_id[x] = 24;
  PluginTX_ptr[x++] = &PluginTX_024;
#endif

#ifdef PLUGIN_TX_025
  PluginTX_id[x] = 25;
  PluginTX_ptr[x++] = &PluginTX_025;
#endif

#ifdef PLUGIN_TX_026
  PluginTX_id[x] = 26;
  PluginTX_ptr[x++] = &PluginTX_026;
#endif

#ifdef PLUGIN_TX_027
  PluginTX_id[x] = 27;
  PluginTX_ptr[x++] = &PluginTX_027;
#endif

#ifdef PLUGIN_TX_028
  PluginTX_id[x] = 28;
  PluginTX_ptr[x++] = &PluginTX_028;
#endif

#ifdef PLUGIN_TX_029
  PluginTX_id[x] = 29;
  PluginTX_ptr[x++] = &PluginTX_029;
#endif

#ifdef PLUGIN_TX_030
  PluginTX_id[x] = 30;
  PluginTX_ptr[x++] = &PluginTX_030;
#endif

#ifdef PLUGIN_TX_031
  PluginTX_id[x] = 31;
  PluginTX_ptr[x++] = &PluginTX_031;
#endif

#ifdef PLUGIN_TX_032
  PluginTX_id[x] = 32;
  PluginTX_ptr[x++] = &PluginTX_032;
#endif

#ifdef PLUGIN_TX_033
  PluginTX_id[x] = 33;
  PluginTX_ptr[x++] = &PluginTX_033;
#endif

#ifdef PLUGIN_TX_034
  PluginTX_id[x] = 34;
  PluginTX_ptr[x++] = &PluginTX_034;
#endif

#ifdef PLUGIN_TX_035
  PluginTX_id[x] = 35;
  PluginTX_ptr[x++] = &PluginTX_035;
#endif

#ifdef PLUGIN_TX_036
  PluginTX_id[x] = 36;
  PluginTX_ptr[x++] = &PluginTX_036;
#endif

#ifdef PLUGIN_TX_037
  PluginTX_id[x] = 37;
  PluginTX_ptr[x++] = &PluginTX_037;
#endif

#ifdef PLUGIN_TX_038
  PluginTX_id[x] = 38;
  PluginTX_ptr[x++] = &PluginTX_038;
#endif

#ifdef PLUGIN_TX_039
  PluginTX_id[x] = 39;
  PluginTX_ptr[x++] = &PluginTX_039;
#endif

#ifdef PLUGIN_TX_040
  PluginTX_id[x] = 40;
  PluginTX_ptr[x++] = &PluginTX_040;
#endif

#ifdef PLUGIN_TX_041
  PluginTX_id[x] = 41;
  PluginTX_ptr[x++] = &PluginTX_041;
#endif

#ifdef PLUGIN_TX_042
  PluginTX_id[x] = 42;
  PluginTX_ptr[x++] = &PluginTX_042;
#endif

#ifdef PLUGIN_TX_043
  PluginTX_id[x] = 43;
  PluginTX_ptr[x++] = &PluginTX_043;
#endif

#ifdef PLUGIN_TX_044
  PluginTX_id[x] = 44;
  PluginTX_ptr[x++] = &PluginTX_044;
#endif

#ifdef PLUGIN_TX_045
  PluginTX_id[x] = 45;
  PluginTX_ptr[x++] = &PluginTX_045;
#endif

#ifdef PLUGIN_TX_046
  PluginTX_id[x] = 46;
  PluginTX_ptr[x++] = &PluginTX_046;
#endif

#ifdef PLUGIN_TX_047
  PluginTX_id[x] = 47;
  PluginTX_ptr[x++] = &PluginTX_047;
#endif

#ifdef PLUGIN_TX_048
  PluginTX_id[x] = 48;
  PluginTX_ptr[x++] = &PluginTX_048;
#endif

#ifdef PLUGIN_TX_049
  PluginTX_id[x] = 49;
  PluginTX_ptr[x++] = &PluginTX_049;
#endif

#ifdef PLUGIN_TX_050
  PluginTX_id[x] = 50;
  PluginTX_ptr[x++] = &PluginTX_050;
#endif

#ifdef PLUGIN_TX_051
  PluginTX_id[x] = 51;
  PluginTX_ptr[x++] = &PluginTX_051;
#endif

#ifdef PLUGIN_TX_052
  PluginTX_id[x] = 52;
  PluginTX_ptr[x++] = &PluginTX_052;
#endif

#ifdef PLUGIN_TX_053
  PluginTX_id[x] = 53;
  PluginTX_ptr[x++] = &PluginTX_053;
#endif

#ifdef PLUGIN_TX_054
  PluginTX_id[x] = 54;
  PluginTX_ptr[x++] = &PluginTX_054;
#endif

#ifdef PLUGIN_TX_055
  PluginTX_id[x] = 55;
  PluginTX_ptr[x++] = &PluginTX_055;
#endif

#ifdef PLUGIN_TX_056
  PluginTX_id[x] = 56;
  PluginTX_ptr[x++] = &PluginTX_056;
#endif

#ifdef PLUGIN_TX_057
  PluginTX_id[x] = 57;
  PluginTX_ptr[x++] = &PluginTX_057;
#endif

#ifdef PLUGIN_TX_058
  PluginTX_id[x] = 58;
  PluginTX_ptr[x++] = &PluginTX_058;
#endif

#ifdef PLUGIN_TX_059
  PluginTX_id[x] = 59;
  PluginTX_ptr[x++] = &PluginTX_059;
#endif

#ifdef PLUGIN_TX_060
  PluginTX_id[x] = 60;
  PluginTX_ptr[x++] = &PluginTX_060;
#endif

#ifdef PLUGIN_TX_061
  PluginTX_id[x] = 61;
  PluginTX_ptr[x++] = &PluginTX_061;
#endif

#ifdef PLUGIN_TX_062
  PluginTX_id[x] = 62;
  PluginTX_ptr[x++] = &PluginTX_062;
#endif

#ifdef PLUGIN_TX_063
  PluginTX_id[x] = 63;
  PluginTX_ptr[x++] = &PluginTX_063;
#endif

#ifdef PLUGIN_TX_064
  PluginTX_id[x] = 64;
  PluginTX_ptr[x++] = &PluginTX_064;
#endif

#ifdef PLUGIN_TX_065
  PluginTX_id[x] = 65;
  PluginTX_ptr[x++] = &PluginTX_065;
#endif

#ifdef PLUGIN_TX_066
  PluginTX_id[x] = 66;
  PluginTX_ptr[x++] = &PluginTX_066;
#endif

#ifdef PLUGIN_TX_067
  PluginTX_id[x] = 67;
  PluginTX_ptr[x++] = &PluginTX_067;
#endif

#ifdef PLUGIN_TX_068
  PluginTX_id[x] = 68;
  PluginTX_ptr[x++] = &PluginTX_068;
#endif

#ifdef PLUGIN_TX_069
  PluginTX_id[x] = 69;
  PluginTX_ptr[x++] = &PluginTX_069;
#endif

#ifdef PLUGIN_TX_070
  PluginTX_id[x] = 70;
  PluginTX_ptr[x++] = &PluginTX_070;
#endif

#ifdef PLUGIN_TX_071
  PluginTX_id[x] = 71;
  PluginTX_ptr[x++] = &PluginTX_071;
#endif

#ifdef PLUGIN_TX_072
  PluginTX_id[x] = 72;
  PluginTX_ptr[x++] = &PluginTX_072;
#endif

#ifdef PLUGIN_TX_073
  PluginTX_id[x] = 73;
  PluginTX_ptr[x++] = &PluginTX_073;
#endif

#ifdef PLUGIN_TX_074
  PluginTX_id[x] = 74;
  PluginTX_ptr[x++] = &PluginTX_074;
#endif

#ifdef PLUGIN_TX_075
  PluginTX_id[x] = 75;
  PluginTX_ptr[x++] = &PluginTX_075;
#endif

#ifdef PLUGIN_TX_076
  PluginTX_id[x] = 76;
  PluginTX_ptr[x++] = &PluginTX_076;
#endif

#ifdef PLUGIN_TX_077
  PluginTX_id[x] = 77;
  PluginTX_ptr[x++] = &PluginTX_077;
#endif

#ifdef PLUGIN_TX_078
  PluginTX_id[x] = 78;
  PluginTX_ptr[x++] = &PluginTX_078;
#endif

#ifdef PLUGIN_TX_079
  PluginTX_id[x] = 79;
  PluginTX_ptr[x++] = &PluginTX_079;
#endif

#ifdef PLUGIN_TX_080
  PluginTX_id[x] = 80;
  PluginTX_ptr[x++] = &PluginTX_080;
#endif

#ifdef PLUGIN_TX_081
  PluginTX_id[x] = 81;
  PluginTX_ptr[x++] = &PluginTX_081;
#endif

#ifdef PLUGIN_TX_082
  PluginTX_id[x] = 82;
  PluginTX_ptr[x++] = &PluginTX_082;
#endif

#ifdef PLUGIN_TX_083
  PluginTX_id[x] = 83;
  PluginTX_ptr[x++] = &PluginTX_083;
#endif

#ifdef PLUGIN_TX_084
  PluginTX_id[x] = 84;
  PluginTX_ptr[x++] = &PluginTX_084;
#endif

#ifdef PLUGIN_TX_085
  PluginTX_id[x] = 85;
  PluginTX_ptr[x++] = &PluginTX_085;
#endif

#ifdef PLUGIN_TX_086
  PluginTX_id[x] = 86;
  PluginTX_ptr[x++] = &PluginTX_086;
#endif

#ifdef PLUGIN_TX_087
  PluginTX_id[x] = 87;
  PluginTX_ptr[x++] = &PluginTX_087;
#endif

#ifdef PLUGIN_TX_088
  PluginTX_id[x] = 88;
  PluginTX_ptr[x++] = &PluginTX_088;
#endif

#ifdef PLUGIN_TX_089
  PluginTX_id[x] = 89;
  PluginTX_ptr[x++] = &PluginTX_089;
#endif

#ifdef PLUGIN_TX_090
  PluginTX_id[x] = 90;
  PluginTX_ptr[x++] = &PluginTX_090;
#endif

#ifdef PLUGIN_TX_091
  PluginTX_id[x] = 91;
  PluginTX_ptr[x++] = &PluginTX_091;
#endif

#ifdef PLUGIN_TX_092
  PluginTX_id[x] = 92;
  PluginTX_ptr[x++] = &PluginTX_092;
#endif

#ifdef PLUGIN_TX_093
  PluginTX_id[x] = 93;
  PluginTX_ptr[x++] = &PluginTX_093;
#endif

#ifdef PLUGIN_TX_094
  PluginTX_id[x] = 94;
  PluginTX_ptr[x++] = &PluginTX_094;
#endif

#ifdef PLUGIN_TX_095
  PluginTX_id[x] = 95;
  PluginTX_ptr[x++] = &PluginTX_095;
#endif

#ifdef PLUGIN_TX_096
  PluginTX_id[x] = 96;
  PluginTX_ptr[x++] = &PluginTX_096;
#endif

#ifdef PLUGIN_TX_097
  PluginTX_id[x] = 97;
  PluginTX_ptr[x++] = &PluginTX_097;
#endif

#ifdef PLUGIN_TX_098
  PluginTX_id[x] = 98;
  PluginTX_ptr[x++] = &PluginTX_098;
#endif

#ifdef PLUGIN_TX_099
  PluginTX_id[x] = 99;
  PluginTX_ptr[x++] = &PluginTX_099;
#endif

#ifdef PLUGIN_TX_100
  PluginTX_id[x] = 100;
  PluginTX_ptr[x++] = &PluginTX_100;
#endif

#ifdef PLUGIN_TX_250
  PluginTX_id[x] = 250;
  PluginTX_ptr[x++] = &PluginTX_250;
#endif

#ifdef PLUGIN_TX_251
  PluginTX_id[x] = 251;
  PluginTX_ptr[x++] = &PluginTX_251;
#endif

#ifdef PLUGIN_TX_252
  PluginTX_id[x] = 252;
  PluginTX_ptr[x++] = &PluginTX_252;
#endif

#ifdef PLUGIN_TX_253
  PluginTX_id[x] = 253;
  PluginTX_ptr[x++] = &PluginTX_253;
#endif

#ifdef PLUGIN_TX_254
  PluginTX_id[x] = 254;
  PluginTX_ptr[x++] = &PluginTX_254;
#endif

#ifdef PLUGIN_TX_255
  PluginTX_id[x] = 255;
  PluginTX_ptr[x++] = &PluginTX_255;
#endif

  // Initialiseer alle plugins door aanroep met verwerkingsparameter PLUGINTX_INIT
  PluginTXInitCall(0, 0);
}
/*********************************************************************************************\
 * This function initializes the Receive plugin function table
 \*********************************************************************************************/
byte PluginInitCall(byte Function, char *str)
{
  int x;

  for (x = 0; x < PLUGIN_MAX; x++)
  {
    if (Plugin_id[x] != 0)
    {
      Plugin_ptr[x](Function, str);
    }
  }
  return true;
}
/*********************************************************************************************\
 * This function initializes the Transmit plugin function table
 \*********************************************************************************************/
byte PluginTXInitCall(byte Function, char *str)
{
  int x;

  for (x = 0; x < PLUGIN_TX_MAX; x++)
  {
    if (PluginTX_id[x] != 0)
    {
      PluginTX_ptr[x](Function, str);
    }
  }
  return true;
}
/*********************************************************************************************\
 * With this function plugins are called that have Receive functionality. 
 \*********************************************************************************************/
byte PluginRXCall(byte Function, char *str)
{
  for (byte x = 0; x < PLUGIN_MAX; x++)
  {
    if ((Plugin_id[x] != 0) && (Plugin_State[x] >= P_Enabled))
    {
      SignalHash = x; // store plugin number
      if (Plugin_ptr[x](Function, str))
      {
        SignalHashPrevious = SignalHash; // store previous plugin number after success
        return true;
      }
    }
  }
  return false;
}
/*********************************************************************************************\
 * With this function plugins are called that have Transmit functionality. 
 \*********************************************************************************************/
byte PluginTXCall(byte Function, char *str)
{
  int x;

  for (x = 0; x < PLUGIN_TX_MAX; x++)
  {
    if (PluginTX_id[x] != 0)
    {
      if (PluginTX_ptr[x](Function, str))
      {
        return true;
      }
    }
  }
  return false;
}
/*********************************************************************************************/