# yellow-msx-esp8266-wifi-module
Firmware for the RC2014 WIFI module for operating with Yellow MSX modules

### Arduino Libraries

* [Arduino Core](https://github.com/esp8266/Arduino)
* [ezTime](https://github.com/ropg/ezTime)

### Support Commands

`AT+CWJAP=<ssid>,<password>`

eg:
AT+CWJAP=home,password

Connect to wifi network.  ESP8266's on board led should flash if connected to wifi

`ATD<domain:port>`

eg: ATDsotanomsxbbs.org:23

Create a serial to TCP bridge connection for typically connecting to BBS.  Switch from command mode to passthrough.

`ATH`

Disconnect any established TCP connections

`+++`

Return to command mode from passthrough mode.  Must have 1 second prior to sequence and after sequence where no characters are sent.

> NB: any connection establish are not disconnected.


## Uploading script to RC2014 Wifi Module.

Uploading this firmware to the module requires a few things.  The following describes the key steps using the Arduino IDE.

Recommended you remove the Module from the backplane.

1. You need to have the [Arduino IDE](https://www.arduino.cc/en/software) installed.

2. Connect your PC to the Wifi Module via the FTDI interface.

3. Ensure the 3 jumpers on the module are on the right side (programming).

4. Hit the reset button

5. Open this Arduino Project and under Tools/Manage Libraries - add the required libraries

    ezTime

6. Under **File/Preferences**, in the field *Additional Board Urls*, add to the comma separated list the ESP8266 board index:

    http://arduino.esp8266.com/stable/package_esp8266com_index.json

7. Under *Tools* select or ensure the following settings:

| Configuration | Recommended/required |
| -----------| -----------|
| Board Type: | ESP8266 Generic |
| Builtin Leds: | 2 |
| Upload Speed: | 115200 |
| CPU Frequency: | 160Mhz |
| Crystal Frequence: | 26Mhz |
| Flash Size: | 4MB: (FS: 1MB, OTA: ~1019KB) |
| Flash Frequency: | 40Mhz |
| Port: | <Your PC Port>* |


8. Click the upload button to compile and upload the script

9. Remove the FTDI connection

10. Move the 3 jumpers back to the run configuration

\* Check windows' Device Manager to find your COM port if you dont know which one the FTDI selected.
