// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef Plugin_h
#define Plugin_h

#include <Arduino.h>

extern boolean (*Plugin_ptr[PLUGIN_MAX])(byte, char *); // Receive plugins
extern byte Plugin_id[PLUGIN_MAX];

extern boolean RFDebug;  // debug RF signals with plugin 001
extern boolean RFUDebug; // debug RF signals with plugin 254
extern boolean QRFDebug; // debug RF signals with plugin 254 but no multiplication

// void(*Reboot)(void) = 0;
// boolean (*Plugin_ptr[PLUGIN_MAX])(byte, char*);

// Of all the devices that are compiled, the addresses are stored in a table so that you can jump to them
void PluginInit(void);
// void PluginTXInit(void);
byte PluginInitCall(byte Function, char *str);
// byte PluginTXInitCall(byte Function, char *str);
byte PluginRXCall(byte Function, char *str);
// byte PluginTXCall(byte Function, char *str);

#endif