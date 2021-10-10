#include "at-command-free-memory.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>

void atCommandFreeMemory() {
  Serial.print(F("OK\r\nFreeHeap: "));
  Serial.print(ESP.getFreeHeap());
  Serial.print(F("\r\nMaxFreeBlockSize: "));
  Serial.print(ESP.getMaxFreeBlockSize());
  Serial.print(F("\r\nFreeContStack: "));
  Serial.print(ESP.getFreeContStack());
  Serial.print(F("\r\n"));
}
