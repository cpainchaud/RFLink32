# RFLink-ESP Firmware Flashing Guide


## Downloading firmwares

Releases can be find here https://github.com/cpainchaud/RFLink/releases

Nightly/Development released are the latest firmware compiled from latest commits,
they are the only ones available today at this early stage of the project.

The following files are at your disposal:
- esp32-firmware-OTA.bin: to upgrade existing RFLink-ESP devices 
- esp32-full.img: to flash a new device or erase a previous installation

## First time Flash or full reset

If you are already installed RFLink on your device, beware the following will overwrite
all your existing configurations. Please have a look at the OTA Update if you just looking 
to upgrade an existing RFLink-ESP device!

Using Esptool:
`python.exe esptool.py --chip esp32 --baud 460800 write_flash -z 0x1000 esp32-full.bin`

## OTA updates

While this procedure is not supposed to overwrite your existing configurations, we strongly
advise you to make a backup of your configurations first!

#### Using WebUI
...TODO...

#### Using RFLink serial commands
...TODO...

#### Using Esptool

`python.exe esptool.py --chip esp32 --baud 460800 write_flash -z 0x10000 esp32-firmware-OTA.bin`

#### Using Espressif's official tool "Flash Download Tools (ESP8266 & ESP32 & ESP32-S2)" at https://www.espressif.com/en/support/download/other-tools:

At first menu, select Developper Mode:

![esp_tool1](https://github.com/cpainchaud/RFLink/blob/master/pictures/espressif_tool_dev_mode.png)

Then pick your model:

![esp_tool2](https://github.com/cpainchaud/RFLink/blob/master/pictures/espressif_tool_pick_model.png)

Select your firmware file and make sure you input address=0x10000, then select your COM PORT
and click start!

![esp_tool3](https://github.com/cpainchaud/RFLink/blob/master/pictures/espressif_tool_fill_fields.png)

A progress bar will show up (you may have to keep pressing your BOOT button
until the process starts)

![esp_tool4](https://github.com/cpainchaud/RFLink/blob/master/pictures/espressif_tool_progress.png)

Reboot and enjoy!



