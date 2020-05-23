// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef Plugin_h
#define Plugin_h

#include <Arduino.h>

#define PLUGIN_MAX 55   // 55         // Maximum number of Receive plugins
#define PLUGIN_TX_MAX 5 // 26         // Maximum number of Transmit plugins

enum PState
{
    P_Forbidden,
    P_Disabled,
    P_Enabled,
    P_Mandatory
};

extern boolean (*Plugin_ptr[PLUGIN_MAX])(byte, char *); // Receive plugins
extern byte Plugin_id[PLUGIN_MAX];
extern byte Plugin_State[PLUGIN_MAX];
extern String Plugin_Description[PLUGIN_MAX];

extern boolean (*PluginTX_ptr[PLUGIN_TX_MAX])(byte, char *); // Transmit plugins
extern byte PluginTX_id[PLUGIN_TX_MAX];
extern byte PluginTX_State[PLUGIN_TX_MAX];

extern boolean RFDebug;   // debug RF signals with plugin 001 (no decode)
extern boolean QRFDebug;  // debug RF signals with plugin 001 but no multiplication (faster?, compact)
extern boolean RFUDebug;  // debug RF signals with plugin 254 (decode 1st)
extern boolean QRFUDebug; // debug RF signals with plugin 254 but no multiplication (faster?, compact)

// Of all the devices that are compiled, the addresses are stored in a table so that you can jump to them
void PluginInit(void);
void PluginTXInit(void);
byte PluginInitCall(byte Function, char *str);
byte PluginTXInitCall(byte Function, char *str);
byte PluginRXCall(byte Function, char *str);
byte PluginTXCall(byte Function, char *str);

#endif