# RFLink-ESP CLI Reference Guide

All commands start with `10;` and end with `;` , example: `10;rfdebug=on;`

```text
10;rfdebug=<on|off>
10;reboot;
```

## Print current configuration to Serial

```text
10;config;dump;
```

## Reset to factory defaults

```text
10;config;reset;
```

Resets all the configuration to factory defaults.
Beware that you will need to reboot.

## Send RF pulses

Send manual RF pulses to the transmitting module.

Example:

```text
10;signal;sendRF;{"repeat":3,"delay":10,"pulses":[400,20,400,30,60,20,400,30,6020,400,30,6020,400,30,6020,400,30,6020,400,30,6020,400,30,6020,400,30,6020,400,30]}
```

Parameters:

| Parameter | Type         | Range     | Description                                             |
| --------- | ------------ | --------- | ------------------------------------------------------- |
| delay     | number       | 0 - 65000 | The time between repeats of your signal in milliseconds |
| repeat    | number       | 0 - 65000 | How many times to repeat the signal                     |
| pulses    | number array |           | Pulses in microseconds                                  |

## Test sample signal against plugins

Example:

```text
10;signal;testRF;{"pulses":[400,20,400,30,60,20,400,30,600]}
```

Parameters:

| Parameter | Type         | Range     | Description                                             |
| --------- | ------------ | --------- | ------------------------------------------------------- |
| pulses    | number array |           | Pulses in microseconds                                  |

## Edit configuration

```text
10;config;set;<json code here>
```

Enabled Wifi Station/Client mode example:

```text
10;config;set;{"wifi":{"client_enabled":true,"client_dhcp_enabled":true,"client_ssid":"my_home_wifi","client_password":"_PASSWORD_"}}
```

Reference:

| Parent  | Key                 | Type         | Description                                                                                         |
| ------- | ------------------- | ------------ | --------------------------------------------------------------------------------------------------- |
| portal  | enabled             | Boolean      | Use the Config Portal                                                                               |
| portal  | auth_enabled        | Boolean      | Config Portal login authentication                                                                  |
| portal  | auth_user           | String       | The Config Portal user name                                                                         |
| portal  | auth_password       | String       | The Config Portal password                                                                          |
| mqtt    | enabled             | Boolean      | Use MQTT (MQ Telemetry Transport) protocol                                                          |
| mqtt    | server              | IP Address   | The IP Address of the MQTT server                                                                   |
| mqtt    | port                | Number       | The port number of the MQTT server *[default 1883]*                                                 |
| mqtt    | id                  | String       | The client id to report to the MQTT server                                                          |
| mqtt    | user                | String       | The user name to authenticate with the MQTT server                                                  |
| mqtt    | password            | String       | The password to authenticate with the MQTT server                                                   |
| mqtt    | topic_in            | String       | The MQTT topic for received transmissions __*Will be the root path if using Separate Topics*__      |
| mqtt    | topic_out           | String       | The MQTT topic for sending a transmission                                                           |
| mqtt    | topic_lwt           | String       | The Last Will and Testament MQTT topic                                                              |
| mqtt    | lwt_enabled         | Boolean      | Enables the Last Will and Testament feature                                                         |
| mqtt    | ssl_enabled         | Boolean      | Use TLS encryption for the MQTT connection                                                          |
| mqtt    | ssl_insecure        | Boolean      | Ignore validating the MQTT server certificate                                                       |
| mqtt    | ca_cert             | String       | The PEM/DER Certificate Authority Certificate file contents __*When SSL Insecure is disabled*__     |
| wifi    | client_enabled      | Boolean      | Connect to a wireless network                                                                       |
| wifi    | client_dhcp_enabled | Boolean      | Use DHCP on the wireless network to configure the network interface                                 |
| wifi    | client_ssid         | String       | The wireless network SSID                                                                           |
| wifi    | client_password     | String       | The wireless network password                                                                       |
| wifi    | client_hostname     | String       | The host name of the device on the network                                                          |
| wifi    | client_ip           | IP Address   | The device address __*When DHCP disabled*__                                                         |
| wifi    | client_mask         | Network Mask | The network mask  __*When DHCP disabled*__                                                          |
| wifi    | client_gateway      | IP Address   | The gateway address  __*When DHCP disabled*__                                                       |
| wifi    | client_dns          | IP Address   | The DNS server address  __*When DHCP disabled*__                                                    |
| wifi    | ap_enabled          | Boolean      | Device wireless AP for direct connection                                                            |
| wifi    | ap_ssid             | String       | The SSID of the AP network                                                                          |
| wifi    | ap_password         | String       | The password of the AP network                                                                      |
| wifi    | ap_ip               | IP Address   | The address of the device on the AP network                                                         |
| wifi    | ap_network          | IP Address   | The AP network ID                                                                                   |
| wifi    | ap_mask             | Network Mask | The AP network mask                                                                                 |
| ser2net | enabled             | Boolean      | Use Serial2Net protocol                                                                             |
| ser2net | port                | Number       | Set the Serial2Net port number *[default 1900]*                                                     |
| signal  | sample_rate         | Number       |                                                                                                     |
| signal  | min_raw_pulses      | Number       | The minimum number of bits needed to be received before spending CPU time on decoding the signal    |
| signal  | seek_timeout        | Number       | After this time (in milliseconds) the signal will be considered absent                              |
| signal  | min_preamble        | Number       | After this time (in microseconds) the signal will be considered to have started                     |
| signal  | min_pulse_len       | Number       | Pulses shorter than this time (in microseconds) will be discarded                                   |
| signal  | signal_end_timeout  | Number       | After this time (in microseconds) the signal will be interpreted as completed                       |
| signal  | signal_repeat_time  | Number       | Time in milliseconds in which the same signal should not be accepted, for filtering out retransmits |
| signal  | scan_high_time      | Number       | RF listen time in milliseconds                                                                      |
| signal  | async_mode_enabled  | Boolean      |                                                                                                     |
| radio   | hardware            | Enumeration  | Possible Values: `generic` `RFM69CW` `RFM69HCW` `SX1276` `SX1278` `CC1101`                          |
| radio   | rx_data             | Number       | Receiver Data Pin                                                                                   |
| radio   | rx_vcc              | Number       | Receiver Power Pin                                                                                  |
| radio   | rx_nmos             | Number       | Receiver N-MOSFET Pin                                                                               |
| radio   | rx_pmos             | Number       | Receiver P-MOSFET Pin                                                                               |
| radio   | rx_gnd              | Number       | Receiver Ground Pin                                                                                 |
| radio   | rx_na               | Number       |                                                                                                     |
| radio   | tx_data             | Number       | Transmitter Data Pin                                                                                |
| radio   | tx_vcc              | Number       | Transmitter Power Pin                                                                               |
| radio   | tx_nmos             | Number       | Transmitter N-MOSFET Pin                                                                            |
| radio   | tx_pmos             | Number       | Transmitter P-MOSFET Pin                                                                            |

JSON Output:

```json
{
  "portal": {
    "enabled": true,
    "auth_enabled": true,
    "auth_user": "rflink32",
    "auth_password": "433mhz"
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
    "client_ssid": "RFLink-AP",
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
    "min_preamble": 100,
    "min_pulse_len": 50,
    "signal_end_timeout": 5000,
    "signal_repeat_time": 250,
    "scan_high_time": 50,
    "async_mode_enabled": false
  },
  "radio": {
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
```
