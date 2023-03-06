# How to write a plugin

This documentation explains how to add support for a RF device, including how to decode the RF signal.

## Reverse engineering the device protocol

Start by identifying which frequency is used by your device.
Usually, this is indicated in the accompanying user manual or on the device itself.
You may also be able to lookup the devices FCC ID (if applicable) to find radio frequency specifications.

For this guide, we will use the Silvercrest set of 4 radio-controlled power plugs.

The Silvercrest remote has 433.92MHz written on it which is one of the most common frequency bands used by RF devices.

After starting the RFLink board, place it in *debug mode* by issuing this command on the serial port:

```text
10;RFUDEBUG=ON;
```

This will ensure that anytime a set of RF pulses is received and not recognized by a plugin that it will be output.
Below is an example of this format:

```text
20;XX;DEBUG;Pulses=196;Pulses(uSec)=1152,352,1152,352,1152,384,1120,384,1120,384,352,1152,352,1184,320,1184,1120,416,1088,416,1088,416,320,1152,1120,416,320,1184,1088,416,320,1184,1088,416,320,1184,1088,416,1088,416,1088,416,320,1184,320,2336,1120,416,320,1184,1088,416,1088,416,1088,416,1088,448,1088,416,320,1216,320,1184,320,1184,1088,416,1088,416,1088,448,320,1184,1088,416,320,1216,1088,448,320,1216,1088,416,320,1184,1088,416,1088,448,1088,448,320,1184,320,2336,1088,416,320,1216,1088,416,1088,448,1056,448,1056,448,1088,448,288,1216,320,1184,320,1216,1088,448,1056,448,1088,448,288,1216,1056,448,320,1216,1088,448,288,1216,1088,448,288,1184,1088,448,1056,448,1056,448,288,1216,320,2368,1056,448,288,1216,1056,448,1056,448,1056,448,1056,448,1056,448,288,1216,288,1216,288,1216,1088,448,1056,448,1056,448,288,1216,1056,448,288,1216,1056,480,288,1216,1056,480,288,1248,1056,448,1056,480,1024,480,288,1248,2976,448
```

The above example shows that the receiver has seen 196 pulses.
The first was a `high` state that lasted 1152 microseconds followed by a `low` state that lasted 352 microseconds and so on.
It is often easier to represent those pulses as a time-based graph to allow for easy visualization of the received signal.
Use the `pulses_to_csv.bat` script available in the `tools` folder to convert the above output.
The batch file will start PowerShell and execute the `pulses_to_csv.ps1` script which then creates a CSV file that can be interpreted by [PulseView](https://sigrok.org/wiki/PulseView).

This script will ask you for the following parameters:

| Parameter | Description                                                        |
| --------- | ------------------------------------------------------------------ |
| filename  | The name of the output CSV file                                    |
| pulsesStr | The pulses outputted by RFLink (content after the `Pulses(uSec)=`) |

> If PulseView is installed to the default location, it will launch automatically once the pulses have been converted.

The above example gives the following output:

![silvercrest_full](silvercrest_full.png "Silvercrest representation")

With the visualized output, it makes it easy to see that there is a repeating pattern.
A `short` high pulse, followed by a `long` down pulse, followed by 48 medium or short pulses.

By pressing the same button multiple times we notice that we always receive between 180 and 320 pulses.
The repeating pattern is always the same length and that the long, medium and short pulses durations do not vary much from one press to the other.
Also notice that the 48 medium or short pulses always occur as one of four possible patterns.

Pressing another button on the remote, notice that the pulses we receive still follow the same pattern except that the 48 medium or short pulses give a different set of four possible patterns.

When looking at those 48 pulses, there are two possible pairs:

* Medium followed by short
* Short followed by medium

This indicates a well know kind of encoding called "PWM".
The duration of a pair is always the same but the `high`/`low` ratio takes two values which define a single bit of information.

## Reception and Decoding

We can write a plugin that will decode the received data now that we have good enough knowledge of the protocol.

Open the `Plugins/_Plugin_Config_01.h` file and look inside it to find a plugin number that makes sense with respect to the existing ones.
As we are dealing with a remote, this will be placed in the first block.
The first available number is 16, so we add the following line:

```cpp
#define PLUGIN_016 // Silvercrest
```

This will make the project try to compile the `Plugins/Plugin_016.c` file which then add to the source tree.

The first thing to do is to give a description for our plugin:

```cpp
#define PLUGIN_DESC_016 "Silvercrest remote controlled power sockets"
```

We can also define a constant for our plugin number like so:

```cpp
#define SILVERCREST_PLUGIN_ID 016
```

And finally, we declare the function that will decode the pulses:

```cpp
boolean Plugin_016(byte function, const char *string)
{  
}
```

> This function must be inside a `IFDEF PLUGIN_06` section to allow it not to be compiled if the user decides to exclude your plugin.

The parameters are required by the RFLink framework but can usually be ignored. 
When booting, the plugins are told to initialize themselves by calling their decode method with `string` set to `null`.
In our case, we don't have any initialization to perform so we can ignore the parameters.

The first thing to do inside the method is to declare constants for the pulses that we are expecting to see, like so:

```cpp
const int SLVCR_MinPulses = 180;
const int SLVCR_MaxPulses = 320;
const int SLVCR_StartPulseDuration = 2000 / RawSignal.Multiply;
```

The pulse duration constants are divided by the `RawSignal.Multiply` value.
This is because the user may want to filter out shorter signals by applying a division to the raw values as read from the receiver.  

> Those divisions are made once at the start of the method and they should not be moved outside the method due to this.
The division would be done at boot time when `RawSignal.Multiply` has not yet been set and has a value of 0.
Dividing by 0 will trigger a fatal exception causing the board to be stuck in a boot loop!

The basic idea when writing the decode method is to abort as soon as possible to give back control to other plugins as quickly as possible.
The shorter the time to process a series of pulses, the less messages are missed due to the board either listening or decoding.

As we've seen above, the signal has a somewhat variable total number of pulses which means we start by writing this test:

```cpp
if (RawSignal.Number >= SLVCR_MinPulses && RawSignal.Number <= SLVCR_MaxPulses)
```

Then we look for the start of the bytes we are interested in and once we have found them we call the helper method for decoding PWM pulses from `7_Utils.h`: `decode_pwm`.

> Using the helper methods reduces bugs and makes the code easily maintainable.

This method requires a few parameters - in particular the duration of pulses, specified as a minimum and maximum duration.
This is to allow for slight variations in the signal which may occur because of interference and receiving hardware differences.

Once we have the 3 decoded bytes we can then process them.

> If anything out of place is detected - return immediately to save time on decoding.

Use the methods from `4_Display.h` to send the messages to the listening endpoints.

> Never use `Serial.WriteLn` as this will only output to the console and not to MQTT or Ser2Net.

Using the `display_XX` methods is the only way to make sure the decoded information is properly sent. And it also takes care of the message counter that appears as the second element on the output string.

The output is always started by a single `display_Header()` and finished by a single `display_Footer()` with as many `display_XX` calls as required for the protocol you have decoded.

## Transmission and Encoding

For transmission, you will also have to go into the `Plugins/_Plugin_Config_01.h` file and add a line for your plugin in the TX section.
You must reuse the same number as the reception part, which gives this line to be added:

```cpp
#define PLUGIN_TX_016 // Silvercrest
```

Inside the `Plugin_016.c` file, add a new function surrounded by the appropriate `IFDEF` like the following:

```cpp
#ifdef PLUGIN_TX_016
// 10;Silvercrest;ID=RemoteId;SWITCH=ButtonId;CMD=State
// 10;Silvercrest;ID=b;SWITCH=e;CMD=ON; // Button C
// 10;Silvercrest;ID=b;SWITCH=7;CMD=OFF; // Button D
// 10;Silvercrest;ID=b;SWITCH=7;CMD=ON; // Button D
boolean PluginTX_016(byte function, const  char *string)
{  
}
#endif  //PLUGIN_TX_016
```

It is recommended to add a few sample commands as comments above the definition of the method, this makes it easier to understand the available parameters.

As with the receiving function, the parameters can be safely ignored.

The first thing to do is to decode the command received by the board and for this you must use the `receive_XX ` helper methods from `4_Display.h`.

> Using the helper methods reduces bugs and makes the code easily maintainable.

Give control back as soon as possible to spend as little time as possible interpreting a command that is not for the device. For example:

```cpp
retrieve_Init();
if (!retrieve_Name("10"))
    return false;
if (!retrieve_Name(PLUGIN_016_ID))
    return false;
if (!retrieve_ID(remoteId))
    return false;
if (!retrieve_Switch(buttonId))
    return false;
if (!retrieve_Command(buttonCommand))
    return false;
if (!retrieve_End())
    return false;
```

As you will see by looking at those function source code, they allow for an optional `prefix` parameter to be specified on the control command. For instance, for the remote ID, you can either give `b` or `ID=b` in the command string. This is a convenient way to remember the usage of the given parameters. But please note that it does not allow to change the order of parameters at will, they are still expected in the order defined in the "prefix less" form.
If everything went well, we go on to prepare the bytes that we will send to the emitter and then manipulate the output pin for the appropriate duration:

```cpp
digitalWrite(Radio::pins::TX_DATA, HIGH);
delayMicroseconds(PreambleHighTime);
digitalWrite(Radio::pins::TX_DATA, LOW);
delayMicroseconds(PreambleLowTime);
```

> This is most likely to change in the future as it is quite error prone and not interrupt proof.

### Storing and Retrieving Values

Some protocols include a value that changes for every sent packet which follows some mathematical formula.
Often it is just an increase of the previous value with a roll-over when it reaches a given maximum value.
This is called a rolling code and can be seen in some remote-controlled garage doors as a way to avoid clashes with neighboring systems.

In these cases, the transmitting plugin needs a way to store the value to be used for the next transmission.
The `LittleFS` file system provided by RFLink32 can store this value in a file which will survive a device reboot or firmware update.

#### Storing Values  

```cpp
File file = LittleFS.open(ConfigFileName, "r+");  
file.write((uint8_t*)&rollingCode, sizeof(rollingCode));  
file.close();
```

#### Retrieving Values

```cpp
File file = LittleFS.open(ConfigFileName, "r");
if (file.read((uint8_t*)&rollingCode, sizeof(rollingCode)) != sizeof(rollingCode))  
{
    // notify error
}
file.close();
```

#### File Name

The `ConfigFileName` variable is a constant value and is declared like the following:

```cpp
const char ConfigFileName[] = "/MyProtocol.bin";
```

Replace `MyProtocol` by your protocol name.

> The leading forward slash **must** be included in the file name.
