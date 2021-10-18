#include "at-command-msx-rc2014.h"
#include "at-command-parser.h"
#include "at-command-time.h"
#include "at-command-wifi.h"
#include "gpio.h"
#include "version.h"
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ezTime.h>

void pgm_serial_slow_print(const char *p, const bool assumeNoFlowControl) {
  unsigned char c;
  while(c = pgm_read_byte(p++)) {
    Serial.write(c);
    if (assumeNoFlowControl)
      delay(10);
  }
}

void serial_slow_print(const char *p, const bool assumeNoFlowControl) {
  unsigned char c;
  while(c = *p++) {
    Serial.write(c);
    if (assumeNoFlowControl)
      delay(10);
  }
}

void serial_slow_write(const char c, const bool assumeNoFlowControl) {
  Serial.write(c);
  if (assumeNoFlowControl)
    delay(15);
}

void firmwareInit(const bool assumeNoFlowControl) {
  int count;
  pgm_serial_slow_print(PSTR("\r\nESP8266 Firmware for MSX on RC2014\r\n"), assumeNoFlowControl);

  pgm_serial_slow_print(PSTR("Version: " VERSION "\r\n"), assumeNoFlowControl);

  pgm_serial_slow_print(PSTR("Built: " __DATE__ " at " __TIME__ "\r\n"), assumeNoFlowControl);

  const bool haveWifiCredentials = !((WiFi.SSID() == F("")) || (WiFi.SSID() == NULL));

  if (!haveWifiCredentials) {
    pgm_serial_slow_print(PSTR("Wifi Status: No Credentials Stored\r\nREADY\r\n"), assumeNoFlowControl);
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    pgm_serial_slow_print(PSTR("Wifi Status: Connecting ."), assumeNoFlowControl);
    count = 20;
    while (WiFi.status() != WL_CONNECTED && count >= 0) {
      delay(500);
      Serial.write('.');
      count--;
    }
  } else {
    pgm_serial_slow_print(PSTR("Wifi Status:"), assumeNoFlowControl);
  }

  if (WiFi.status() != WL_CONNECTED) {
    pgm_serial_slow_print(PSTR(" Not Connected "), assumeNoFlowControl);
    Serial.print(WiFi.status());
    pgm_serial_slow_print(PSTR("\r\nSSID: "), assumeNoFlowControl);
    serial_slow_print(WiFi.SSID().c_str(), assumeNoFlowControl);
  } else {
    wifiLedOn();
    pgm_serial_slow_print(PSTR(" Connected\r\nSSID: "), assumeNoFlowControl);
    serial_slow_print(WiFi.SSID().c_str(), assumeNoFlowControl);

    pgm_serial_slow_print(PSTR("\r\nIP: "), assumeNoFlowControl);
    serial_slow_print(WiFi.localIP().toString().c_str(), assumeNoFlowControl);

    events();
    pgm_serial_slow_print(PSTR("\r\nNTP: "), assumeNoFlowControl);
    if (timeStatus() != timeSet) {
      count = 10;
      while (timeStatus() != timeSet && count > 0) {
        events();
        serial_slow_write('.', assumeNoFlowControl);
      }
      serial_slow_write(' ', assumeNoFlowControl);
    }

    if (timeStatus() == timeSet)
      pgm_serial_slow_print(PSTR("Synced"), assumeNoFlowControl);
    else
      pgm_serial_slow_print(PSTR("Not synced"), assumeNoFlowControl);

    pgm_serial_slow_print(PSTR("\r\nTime: "), assumeNoFlowControl);
    if (timeStatus() == timeSet) {
      myTimeZone.setCache(0);
      serial_slow_print(myTimeZone.dateTime(ISO8601).c_str(), assumeNoFlowControl);
    } else
      pgm_serial_slow_print(PSTR("Not Set or synced."), assumeNoFlowControl);

    pgm_serial_slow_print(PSTR("\r\nTimezone: "), assumeNoFlowControl);
    serial_slow_print(myTimeZone.getTimezoneName().c_str(), assumeNoFlowControl);
  }

  pgm_serial_slow_print(PSTR("\r\nREADY\r\n"), assumeNoFlowControl);
}

void atCommandMsxRc2014() {
  Serial.print(F("OK\r\n"));
  firmwareInit(false);
}

