#include "at-command-msx-rc2014.h"
#include "at-command-parser.h"
#include "at-command-time.h"
#include "at-command-wifi.h"
#include "gpio.h"
#include "version.h"
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ezTime.h>

void firmwareInit(const bool assumeNoFlowControl) {
  int count;
  Serial.print(F("\r\nESP8266 Firmware for MSX on RC2014\r\n"));

  if (assumeNoFlowControl)
    delay(250);

  Serial.print(F("Version: " VERSION "\r\n"));

  if (assumeNoFlowControl)
    delay(250);

  Serial.print(F("Built: " __DATE__ " at " __TIME__ "\r\n"));

  if (assumeNoFlowControl)
    delay(250);

  const bool haveWifiCredentials = !((WiFi.SSID() == F("")) || (WiFi.SSID() == NULL));

  if (!haveWifiCredentials) {
    Serial.print(F("Wifi Status: No Credentials Stored\r\nREADY\r\n"));
    return;
  }

  if (assumeNoFlowControl)
    delay(250);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(F("Wifi Status: Connecting ."));
    count = 20;
    while (WiFi.status() != WL_CONNECTED && count >= 0) {
      delay(500);
      Serial.print(F("."));
      count--;
    }
  } else {
    Serial.print(F("Wifi Status:"));
  }

  if (assumeNoFlowControl)
    delay(250);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(F(" Not Connected "));
    Serial.print(WiFi.status());

    if (assumeNoFlowControl)
      delay(250);

    Serial.print(F("\r\nSSID: "));
    Serial.print(WiFi.SSID());
  } else {
    wifiLedOn();
    Serial.print(F(" Connected"));

    Serial.print(F("\r\nSSID: "));
    Serial.print(WiFi.SSID());

    if (assumeNoFlowControl)
      delay(250);

    Serial.print(F("\r\nIP: "));
    Serial.print(WiFi.localIP());

    events();
    Serial.print(F("\r\nNTP: "));
    if (timeStatus() != timeSet) {
      count = 10;
      while (timeStatus() != timeSet && count > 0) {
        delay(250);
        events();
        Serial.print(F("."));
      }
      Serial.print(F(" "));
    }

    if (timeStatus() == timeSet)
      Serial.print(F("Synced"));
    else
      Serial.print(F("Not synced"));

    if (assumeNoFlowControl)
      delay(250);

    Serial.print(F("\r\nTime: "));
    if (timeStatus() == timeSet) {
      myTimeZone.setCache(0);
      Serial.print(myTimeZone.dateTime(ISO8601));
    } else
      Serial.print(F("Not Set or synced."));

    Serial.print("\r\nTimezone: ");
    Serial.print(myTimeZone.getTimezoneName());

    if (assumeNoFlowControl)
      delay(250);
  }

  if (assumeNoFlowControl)
    delay(250);

  Serial.print(F("\r\n"));
  Serial.print(F("READY\r\n"));
}

void atCommandMsxRc2014() {
  Serial.print(F("OK\r\n"));
  firmwareInit(false);
}
