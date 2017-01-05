! ============================================================================
! Only for educational purposes, the source might not be synchronized with the
! latest release! 
! For normal operation, use the RFLink Loader that includes the latest release
! ============================================================================

Please note that the RFLink Gateway is a freeware project.   
Stuntteam is not making money in any way.   
This means that there are no unlimited funds to purchase test devices,   
it also means the project has to rely on you, the user, to send debug data.  
  
If you want to contribute to this project, you can send a donation which is more than welcome (see www.nemcon.nl/blog2 donation button),   
or help with sending debug data of unsupported devices (you can even write and contribute plugins and/or code fixes),  
or donate the device that you would like to see supported.  
   
Right now we are looking for some older remotes and/or switches.  
Like for example: Impuls, Ikea Koppla, Powerfix, Blyss, Home Confort, Conrad, Kambrook, Everflourish 
For the implementation of the planned 2.4Ghz support we could use some simple MySensor devices.   
For the implementation of the planned 868Mhz support we could use some devices as well.   
If you have anything that you do not use, send a mail to frankzirrone@gmail.com   
Thanks in advance!  

------------------------
Synology NAS:  
If you want to use RFLink with a Synology NAS you can use:  
- an Arduino Mega clone based on CH340 USB/serial chip  
In all other cases:  
- connect a 10 uF capacitor between reset and ground on the Arduino.   
  Simply stick the pins of the capacitor in the power connector socket.  
  When you want to update the firmware of the Arduino, remove the capacitor and reconnect it when done.   
  For details about the Domoticz Synology package check out: http://www.jadahl.com  
------------------------
RFlink via Network Connection:   
It is possible to use RFlink via a network connection using ser2net.   
------------------------   
You can now use the RFLink Gateway with the following home automation software:   
Domoticz   
Jeedom   
------------------------   
R35: (Work-In-Progress) Build 05   
- Added: Brel motor support   
- Fixed: Oregon OWL CM119, CM160, CM180  
- Fixed: Corrected WH440 temperature values    
- Fixed: Improved Philips SBC   
- Fixed: Improved Chacon EMW200   
- Fixed: Removed a slash in the UPM/Esic name   
- Tested: tested and working: Lidl / Libra TR502MSV switches   
   
R34:   
- Added: Heidemann HX Silverline 70290   
- Added: Eurochron EAS 301Z / EAS 302   
- Added: Znane-01 switch set sold at Biedronka (Impuls clone)   
- Added: HomeEasy HE800 protocol support   
- Added: Fine Offset Electronics WH2, Agimex Rosenborg 66796, ClimeMET CM9088   
- Added: Somfy Smoove Origin RTS (433mhz) (receive)   
- Tested: Eurodomest 972086 (Sold at Action in Belgium)   
- Added: Eurodomest revised protocol (full autodetection)
- Added: Prologue temperature sensor support   
- Tested: tested and working: Home Confort, Smart Home PRF-100 switch set     
- Fixed: Auto detection of "Emil Lux"/LUX-Tools remote control/switch set (Sold at Obi.de Art.Nr. 2087971) (Impuls clone)    
- Fixed: Alecto WS1100 working properly again (Adjusted pulse range and displayed humidity value)   
- Fixed: Byron SX receive and send commands   
- Fixed: Ikea Koppla Send routines  
- Fixed: Improved the Impuls remote detection   
- Fixed: Impuls transmit   
- Changed: added checks for valid temperatures in various plugins   
   
R33:
- Updated RFlink loader to version 1.03 to include a serial log option with command sending ability!   
   
- Added: Full automatic 'Flamingo FA500/SilverCrest 91210/60494 RCS AAA3680/Mumbi M-FS300/Toom 1919384' protocol support! (send & receive!)  
         Note: re-learn your FA500 Remote in Domoticz   
- Added: Unitec 48111/48112 (receive)   
- Added: Avidsen   
- Added: Somfy Telis (433mhz) (receive)   
- Fixed: Extreme temperature value rejection for various sensor types (TFA/LaCrosse)   
- Fixed: Improved Blyss send routines   
- Added: Support for old Xiron temperature sensor in Cresta plugin (Temperature only sensor was not handled)   
- Added: Biowin meteo sensor   
- Fixed: Imagintronix humidity and temperature values were sometimes incorrect   
- Fixed: AB4400/Sartano/Phenix detection corrected   
- Fixed: Modification to allow EMW200/203 to work better   
- Changed: ARC (and compatible) remote and switch handling improved   
- Fixed: Improved Impuls handling   
- Fixed: Auriol V3 minus temperature handling    
- Fixed: TRC02RGB send   
- Fixed: Oregon OWL180 data   
- Changed: Aster signal detection so that L^Home model 32311T is recognized as well   
- Changed: ID for Nodo Slave 'Wind direction/Wind gust' combined so that Domoticz can handle the data   
- Changed: Protocol handling order for 'multi-protocol' transmitting devices   
   
R32:
- Added: Europe RS-200, Conrad TR-200 
- Added: Bofu motor transmit
- Added: ARC group command support
- Added: support for ARC based tri-state protocol
- Tested and working: Hormann 868mhz receive
- Changed: Bofu motor signal repetition detection improved 
- Fixed: Aster transmit routines
- Fixed: plugin 003 output was not processed correctly by Domoticz
- Fixed: ARC higher address numbers did not work correctly in combination with Domoticz
- Changed: Chacon/Powerfix/Mandolyn/Quigg transmit routine and optimized timing
- Changed: Increased the number of re-transmits in the Home Easy protocol to improve signal reception and distance
- Changed: Sensor plugins now suppressing ARC derived protocols a bit better

R31:
- New Device: Forrinx Wireless Doorbell
- New Device: TRC02 RGB controller
- New Device: OWL CM180
- New Device: ELMES CTX3H and CTX4H contact sensor
- New Device: Bofu Motor (receive)
- New Device: Aster / GEMINI   EMC/99/STI/037 
- Changed: EV1527 based sensors were reported as X10, now they are reported as EV1527. Note that it might be needed to re-add the devices to Domoticz
- Changed: increased number of retransmits for ARC and AC protocols
- Fixed: Koppla switch number was incorrect
- Fixed: Powerfix/Mandolyn/Chacon Parity calculation in send routines
- Fixed: Powerfix/Mandolyn/Chacon timing
- Fixed Windspeed value for WS2300
- Fixed: Home Easy HE300 ON/OFF signal was reversed
- Changed: HomeEasy suppressing additional protocol data to avoid reporting the same event multiple times under different protocols
- Fixed: More fixes to avoid duplicate reporting of the same event (various protocols)

R30: 
- New Device: Conrad 9771 Pool Thermometer
- New Device: SilverCrest Z31370-TX Doorbell
- New Device: Smartwares remote controls (among others: SH5-TDR-K 10.037.17) 
- New Device: Chuango Alarm devices Motion/Door/Window etc. (among others: CG-105S)
- New Device: Oregon Scientific NR868 PIR/night light
- New Device: Oregon Scientific MSR939 PIR
- New Device: Imagintronix Temperature/Soil humidity sensor
- New Device: Ikea Koppla (receive)
- New Device: Chacon (TR-502MSV, NR.RC402)
- Fixed: Arc protocol send
- Fixed: Impuls. Note: pair devices with the KAKU protocol, the remote is recognized separately. (Needs more tests!)
- Changed: Plugin 3 send method, combined routines
- Changed: HomeConfort was recognized as Impuls, now using GDR2 name
- Changed: HomeEasy remotes can deliver various signals, now skipping KAKU compatible signals and just reporting the HomeEasy code when both codes are transmitted
- Fixed: HomeEasy group on/off command was reversed for HE8xx devices, now correctly detects differences between HE3xx and HE8xx
- Fixed: HomeEasy was not able to control HE87x switches, changed the entire transmit routine
- Changed: stretched Xiron timing checks
- Changed: Various timing modifications (NewKaku/AC, Blyss) due to the new timing introduced at version R26
- Changed: Plugin 61, Chinese Alarm devices, reversed bits as it seemed to correspond better to bit settings, increased address range
- Fixed: Flamingo Smokedetector packet detection tightened up to prevent false positives
- Fixed: Corrected Conrad RSL command interpretation
- Added: Extended Nodo Slave support to support separate and combined sensors
- Added: Extended Nodo Slave support to support pulse meters

R29: 
- Fixed: AC/NewKaku high unit numbers were incorrect. 
         If you already have devices with high unit numbers in Domoticz, just throw them away and let them be recognized again

R28: 
- Fixed: FA20RF smoke detector transmit from Domoticz 

R27: 
- Added: OSV1 battery status 
- Fixed: OSV1 boundaries and removed some debug info 
- Fixed: Some plugins set an incorrect sampling rate divider value 
- Changed: AlectoV1 false positives filter was too agressive

R26:
- Added: QRFDEBUG command to do faster logging of undecoded data
- Added: VERSION command
- Added: Powerfix/Quigg switches
- Added: proper Lacrosse V3 WS7000 sensor support
- Changed: config file and plugin integration
- Changed: timeout and divider value
- Changed: Lacrosse V2 WS2300/WS3600 plugin number to get faster processing, changed various other parts as well
- Changed: Lacrosse V1 pulse duration checks
- Changed: various parts to improve speed
- Changed: Flamingo Smoke detector signal re-transmits from 8 to 10 times
- Added: Additional tests on Alecto V1 and Alecto V4 to filter out false positives
- Fixed: AC (NewKaku) protocol send for some device numbers
- Fixed: little bug in UPM code
- Fixed: Oregon wind speed reporting
- Fixed: Wind speed calculations
- Fixed: Wind direction reporting in all plugins
- Fixed: AlectoV3 humidity value displaying out of range values
- Fixed: OregonV1 decoding

R25:
- Fixed: Eurodomest address range check
- Fixed: Alecto V1 and V3 humidity handling
- Fixed: Lacrosse WS2300/WS3600 and labelled as LacrosseV2

R24: 
- Fixed: Flamingo Smoke Detector timings and device address usage
- Fixed: Timing for Nexa/Jula Anslut

R23: 
- Changed: Alecto V1 temperature data filtering
- Added: Alecto V1 battery status now shown for temperature sensors

R22: 
- Various additional tests and fixes after intensive tests
- Added: Home Confort send and recognition by Domoticz

R21: 
- Re-Activated PIR & Door/Window sensors (plugin 60/61) 

R20: 
- Switched to Arduino 1.6.5

R19: 
- Complete rewrite
- Added: Home Confort Smart Home - TEL-010
- Added: RGB LED Controller
- Added: RL-02 Digital Doorbell
- Added: Deltronic Doorbell
- Added: Sartano 2606 remote & switch

r18:
- Added Banggood SKU174397, Sako CH113, Homemart/Onemall FD030 and Blokker (Dake) 1730796 outdoor temperature sensor
- Tested Okay: Promax RSL366T, Profile PR-44N & PR-47N
- Fixed: LaCrosse humidity values are correctly displayed
- Fixed: Humidity values that originate from slave Nodos are correctly displayed
- Fixed: UPM/Esic insane temperature values are skipped
- Removed Xiron & Auriol debug data 
- Tightened pulse range on various protocols to prevent false positives

r17:
- Modified Oregon THGR228N code, 
- Modified Newkaku(AC) dim values, 
- Corrected support for KAKU door switches, 
- Fixed Nodo Slave sensors, 
- Improved speed and priorities so that group commands are properly transmitting

r16: 
- Fixed Aleco V1 temperature ID to match wind sensors
- Fixed HomeEasy transmit
- Added AC(NewKaku) dimmer support

r15:
- Improved large packet translation

r14: 
- Changed Motion sensors (60/61)

r13:
- Flamingo Smoke detector fix
- Added Xiron sensor support

r11/12:
- Mertik / Dru Send added

r10:
- Added Auriol Z32171A

r9:
- Fixed Kaku send with high device id's (P1 M1 etc)

r8:
- Improved descriptions

r7:
- Fixed Oregon RTGR328N ID and humidity format
- Fixed UPM/Esic humidity format
- Fixed Alecto humidity format

r6:
- Fixed Auriol V2 plugin
- Updated Auriol plugin
- Fixed Lacrosse Humidity

r1/2/3/4/5:
- Added X10 receive/transmit plugin
- Minor changes & improvements


Special thanks to:
Alex, Benoit, Bert, Christophe, Deennoo, Emmanuel, Gerrit, Goran, Graeme, Jelle, John, Jonas, Marek, Mark, Martinus, Maurice, 
Paul, Pim, Remco, Richard, Rob, Sebastien, Thibaut, William
and everyone who contributed with feedback, suggestions, debug data, tests etc.
