# RFLink-ESP CLI Reference Guide


All commands start with "10;" and end with ";" , example : "10;rfdebug=on;"

- 10;rfdebug=<on|off>;


Changing configuration from the CLI:

## Print current configuration to Serial

- 10;config;dump;

## Reset to factory defaults

- 10;config;reset;

Resets all the configurationd to factory defaults. Beware that you will need to reboot.

## Edit configuration
`10;config;set;<json code here>`


### Enabled Wifi Station/Client mode:

`10;config;set;{"wifi":{"client_enabled":true,"client_dhcp_enabled":true,"client_ssid":"my_home_wifi"}}`

## Send RF pulses manually

`10;signal;sendRF;{"repeat":3,"delay":10,"pulses":[400,20,400,30,60,20,400,30,6020,400,30,6020,400,30,6020,400,30,6020,400,30,6020,400,30,6020,400,30,6020,400,30]}`