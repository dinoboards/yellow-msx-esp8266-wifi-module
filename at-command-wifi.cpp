#include <SoftwareSerial.h>
#include "system-operation-mode.h"
#include "at-command-parser.h"
#include "at-command-wifi.h"
#include "parse-string.h"
#include <ESP8266WiFi.h>

void atCommandWifi() {
  char ssid[MAX_COMMAND_ARG_SIZE+1];
  char password[MAX_COMMAND_ARG_SIZE+1];
  const int next = parseString(&lineBuffer[9], ssid) + 9;
  parseString(&lineBuffer[next], password);

  Serial.println("\r\nAttempting to connect to wifi with ");
  Serial.print(ssid);
  Serial.print(" ");
  delay(100);
  Serial.print(password);
  Serial.print("\r\n\r\n");

  WiFi.begin(ssid, password);

  int count = 20;
  while (WiFi.status() != WL_CONNECTED && count >= 0) {
    delay(500);
    Serial.print(".");
    count--;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("\r\nERROR\r\n");
    return;
  }

  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  Serial.print("IP address: ");
  delay(200);
  Serial.print(WiFi.localIP());
  Serial.print("\r\n");

  Serial.print("\r\nOK\r\n");
}