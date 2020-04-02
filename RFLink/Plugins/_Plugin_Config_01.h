// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

// ****************************************************************************************************************************************
// RFLink List of Plugins
// ****************************************************************************************************************************************
// Here are all plugins listed that are supported and used after compilation.
// When needed additional plugins can be added or selected plugins can be enabled/disabled.
//
// BEWARE OF THE PLUGIN_MAX setting!!       TX: 51      RX: 24
// ****************************************************************************************************************************************
// Translation Plugin for oversized packets due to their breaks/pause being too short between packets
// Used for Flamingo FA500R and various others, do NOT exclude this plugin.
#define PLUGIN_001 // DO NOT CHANGE OR EXCLUDE!!
// ------------------------------------------------------------------------
// -- Any of the following protocols can be excluded whenever not needed --
// ------------------------------------------------------------------------
// #define PLUGIN_002              // Lacrosse v2 2300/3600
#define PLUGIN_003 // Kaku : Klik-Aan-Klik-Uit (with code wheel) aka ARC
#define PLUGIN_004 // NewKAKU : Klik-Aan-Klik-Uit with automatic coding aka Intertechno.
#define PLUGIN_005 // Eurodomest
#define PLUGIN_006 // Blyss
#define PLUGIN_007 // Conrad RSL2
#define PLUGIN_008 // Kambrook
#define PLUGIN_009 // X10 RF
#define PLUGIN_010 // TRC02 RGB Switch
#define PLUGIN_011 // Home Confort
#define PLUGIN_012 // Flamingo FA500R
#define PLUGIN_013 // Powerfix/Quigg
#define PLUGIN_014 // Ikea Koppla
#define PLUGIN_015 // Home Easy EU
// -------------------
// Weather sensors
// -------------------
#define PLUGIN_030 // Alecto V1 (WS3500) 434 MHz.
// #define PLUGIN_031              // Alecto V3 (WS1100/WS1200/WSD-19) 433.9 MHz.
// #define PLUGIN_032              // Alecto V4
// #define PLUGIN_033              // Conrad Pool Thermometer
// #define PLUGIN_034              // Cresta
// #define PLUGIN_035              // Imagintronix
// #define PLUGIN_040              // Mebus
// #define PLUGIN_041              // LaCrosse v3 ws7000
// #define PLUGIN_042              // UPM/Esic
#define PLUGIN_043 // LaCrosse v1
// #define PLUGIN_044              // Auriol v3
// #define PLUGIN_045              // Auriol
#define PLUGIN_046 // Auriol v2 / Xiron
// #define PLUGIN_048              // Oregon V1/2/3
// -------------------
// Motion Sensors, include when needed
// -------------------
// #define PLUGIN_060              // Ajax Chubb Varel 433 MHz. motion sensors
#define PLUGIN_061 // Chinese PIR motion door and window sensors
#define PLUGIN_062 // Chuango Alarm Devices
// #define PLUGIN_063              // Oregon PIR/ALARM/LIGHT
// -------------------
// Doorbells
// -------------------
// #define PLUGIN_070              // Select Plus (Action - Quhwa)
// #define PLUGIN_071              // Plieger York
// #define PLUGIN_072              // Byron SX doorbell
// #define PLUGIN_073              // Deltronic doorbell
// #define PLUGIN_074              // RL02
// #define PLUGIN_075              // Silvercrest
// -------------------
// Smoke detectors / Fire Places
// -------------------
// #define PLUGIN_080              // Flamingo FA20 / KD101 smoke detector
// #define PLUGIN_082              // Mertik Maxitrol / Dru fireplace
// #define PLUGIN_083              // Alecto SA33
// -------------------
// Misc
// -------------------
// #define PLUGIN_090              // Nodo Slave
// -------------------
// 868 MHZ
// -------------------
// #define PLUGIN_100              // Alecto V2 (DKW2012/ACH2010) 868 MHz.  => PLANNED
// -------------------
// Housekeeping
// -------------------
// #define PLUGIN_090              // Nodo Slave conversion plugin
#define PLUGIN_254 // Debug to show unsupported packets
// ****************************************************************************************************************************************
// RFLink List of Plugins that have TRANSMIT functionality
// ****************************************************************************************************************************************
// Here are all plugins listed that are supported and used after compilation.
// When needed additional plugins can be added or selected plugins can be enabled/disabled.
// ****************************************************************************************************************************************
// ------------------------------------------------------------------------
// -- Any of the following protocols can be excluded whenever not needed --
// ------------------------------------------------------------------------
// #define PLUGIN_TX_003           // Kaku : Klik-Aan-Klik-Uit (with code wheel) aka ARC
// #define PLUGIN_TX_004           // NewKAKU : Klik-Aan-Klik-Uit with automatic coding aka Intertechno.
// #define PLUGIN_TX_005           // Eurodomest
// #define PLUGIN_TX_006           // Blyss
// #define PLUGIN_TX_007           // Conrad RSL2
// #define PLUGIN_TX_008           // Kambrook
// #define PLUGIN_TX_009           // X10 RF
// #define PLUGIN_TX_010		   // TRC02 RGB switch
// #define PLUGIN_TX_011           // Home Confort
// #define PLUGIN_TX_012           // Flamingo FA500R  (11)
// #define PLUGIN_TX_013           // Powerfix/Quigg
// #define PLUGIN_TX_015           // Home Easy EU     (14)
// -------------------
// Doorbells
// -------------------
// #define PLUGIN_TX_070           // Select Plus (Action - Quhwa)
// #define PLUGIN_TX_072           // Byron SX doorbell
// #define PLUGIN_TX_073           // Deltronic doorbell
// #define PLUGIN_TX_074           // RL02
// -------------------
// Smoke detectors
// -------------------
// #define PLUGIN_TX_080           // Flamingo FA20 / KD101 smoke detector
// #define PLUGIN_TX_082           // Mertik Maxitrol / Dru fireplace
// -------------------
//       -=#=-
// -------------------
