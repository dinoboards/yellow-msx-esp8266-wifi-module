#include "at-command-wifi.h"
#include "at-command-parser.h"
#include "gpio.h"
#include "parse-string.h"
#include "system-operation-mode.h"
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

void atCommandWifi() {
  allLedsOff();

  char ssid[MAX_COMMAND_ARG_SIZE + 1];
  char password[MAX_COMMAND_ARG_SIZE + 1];
  const int next = parseString(&lineBuffer[9], ssid) + 9;
  parseString(&lineBuffer[next], password);

  WiFi.begin(ssid, password);

  int count = 20;
  while (WiFi.status() != WL_CONNECTED && count >= 0) {
    delay(500);
    Serial.print(F("."));
    count--;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(F("\r\nERROR\r\n"));
    return;
  }

  wifiLedOn();
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  Serial.print(F("\r\nIP address: "));
  Serial.print(WiFi.localIP());
  Serial.print(F("\r\nOK\r\n"));
}
