#include "at-command-parser.h"
#include "at-command-time.h"
#include "client-connection.h"
#include "gpio.h"
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ezTime.h>

Timezone myTimeZone;

void timezoneSetup() {
  waitForSync();
  myTimeZone.setLocation("Australia/Melbourne");
}

void atCommandSetLocale() {
  if (myTimeZone.setLocation(lineBuffer.substring(10)))
    Serial.print("OK\r\n");
  else
    Serial.print("ERROR\r\n");
}

void atCommandGetTime() {
  waitForSync();
  Serial.print(myTimeZone.dateTime(ISO8601));
  Serial.print("\r\nOK\r\n");
}
