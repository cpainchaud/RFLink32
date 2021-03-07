# RFLink-ESP CLI Reference Guide


All commands start with "10;" and end with ";" , example : "10;rfdebug=on;"


`10;rfdebug=<on|off>`
`10;reboot;`


Changing configuration from the CLI:

## Print current configuration to Serial

`10;config;dump;`

## Reset to factory defaults

`10;config;reset;`

Resets all the configurationd to factory defaults. Beware that you will need to reboot.

## Send RF pulses manually

`10;signal;sendRF;{"repeat":3,"delay":10,"pulses":[400,20,400,30,60,20,400,30,6020,400,30,6020,400,30,6020,400,30,6020,400,30,6020,400,30,6020,400,30,6020,400,30]}`


## Edit configuration
`10;config;set;<json code here>`

### Enabled Wifi Station/Client mode:

`10;config;set;{"wifi":{"client_enabtled":true,"client_dehcp_enabled":true,"client_ssid":"my_home_wifi"}}`

### Full JSON for Reference
````json
"portal": {
		"enabled": true,
		"auth_enabled": false,
		"auth_user": "",
		"auth_password": ""
	},
	"mqtt": {
		"enabled": false,
		"server": "192.168.1.74",
		"port": 1883,
		"id": "ESP8266-RFLink_xxx",
		"user": "xxx",
		"password": "xxx",
		"topic_in": "/ESP00/cmd",
		"topic_out": "/ESP00/msg",
		"topic_lwt": "/ESP00/lwt",
		"lwt_enabled": true
	},
	"wifi": {
		"client_enabled": false,
		"client_dhcp_enabled": true,
		"client_ssid": "ESPLink-AP",
		"client_password": "inputyourown",
		"client_ip": "192.168.0.200",
		"client_mask": "255.255.255.0",
		"client_gateway": "192.168.0.1",
		"client_dns": "192.168.0.1",
		"ap_enabled": true,
		"ap_ssid": "ESPLink-AP",
		"ap_password": "",
		"ap_ip": "192.168.4.1",
		"ap_network": "192.168.4.0",
		"ap_mask": "255.255.255.0"
	},
	"signal": {
		"sample_rate": 1,
		"min_raw_pulses": 24,
		"seek_timeout": 25,
		"_comment_seek_timeout": "millisecond",
		"min_preamble": 100,
		"_comment_min_preamble": "microsecond",
		"min_pulse_len": 50,
		"_comment_min_pulse_len": "microsecond",
		"signal_end_timeout": 5000,
		"_comment_signal_end_timeout": "microsecond",
		"signal_repeat_time": 250,
		"_comment_signal_repeat_time": "millisecond",
		"scan_high_time": 50,
		"_comment_scan_high_time": "millisecond",
		"async_mode_enabled": false
	},
	"radio": {
		"hardware":"generic",
		"_comment_hardware_": "enum generic, rfm69 (more to come). should be in a SELECT box",
		"hardware": "generic",
		"rx_data": 21,
		"rx_vcc": -1,
		"rx_nmos": -1,
		"rx_pmos": -1,
		"rx_gnd": -1,
		"rx_na": -1,
		"tx_data": 2,
		"tx_vcc": 4,
		"tx_nmos": -1,
		"tx_pmos": -1
	}
}
````
