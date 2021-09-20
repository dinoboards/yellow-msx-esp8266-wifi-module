#include "at-command-parser.h"
#include "at-command-time.h"
#include "client-connection.h"
#include "gpio.h"
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ezTime.h>

Timezone myTimeZone;

void timezoneSetup() {
  waitForSync(2);
  myTimeZone.setCache(0);
}

void atCommandSetLocale() {
  if (myTimeZone.setLocation(lineBuffer.substring(10)))
    Serial.print("OK\r\n");
  else
    Serial.print("ERROR: Failed to set timezone.\r\n");
}

void atCommandGetTime() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("ERROR: NO WIFI\r\n");
    return;
  }

  const bool r = waitForSync(2);
  if (r) {
    Serial.print(myTimeZone.dateTime(ISO8601));
    Serial.print("\r\nOK\r\n");
    return;
  }

  Serial.print("ERROR: Failed to sync time.\r\n");
}
