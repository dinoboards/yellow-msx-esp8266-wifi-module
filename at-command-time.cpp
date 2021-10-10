#include "at-command-time.h"
#include "at-command-parser.h"
#include "client-connection.h"
#include "gpio.h"
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ezTime.h>

Timezone myTimeZone;

void atCommandSetLocale() {
  if (myTimeZone.setLocation(lineBuffer.substring(10)))
    Serial.print(F("OK\r\n"));
  else
    Serial.print(F("ERROR: Failed to set timezone.\r\n"));
}

void atCommandGetTime() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(F("ERROR: NO WIFI\r\n"));
    return;
  }

  const bool r = waitForSync(2);
  if (r) {
    Serial.print(myTimeZone.dateTime(ISO8601));
    Serial.print(F("\r\nOK\r\n"));
    return;
  }

  Serial.print(F("ERROR: Failed to sync time.\r\n"));
}
