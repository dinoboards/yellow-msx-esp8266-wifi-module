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
