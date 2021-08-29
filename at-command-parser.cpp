#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include "system-operation-mode.h"
#include "at-command-parser.h"
#include "parse-string.h"

const bool commandEcho = true;
String lineBuffer = "";

String hostName = "";
int portNumber = 2000;

void processPotentialCommand() {
  if (lineBuffer.startsWith("ATD")) {
    hostName = lineBuffer.substring(3);

    int hostPortSeperatorPosition = lineBuffer.indexOf(':');

    if (hostPortSeperatorPosition != -1) {
      hostName = hostName.substring(0, hostPortSeperatorPosition);
      portNumber = hostName.substring(hostPortSeperatorPosition).toInt();
    }

    Serial.print("\r\nAttempting to connect to ");
    Serial.print(hostName);
    Serial.print(":");
    Serial.print(portNumber);
    Serial.print(" ....\r\n");
    systemState = passthroughMode;
    goto done;
  }

  if (lineBuffer == "AT") {
    Serial.print("\r\nOK\r\n");
    goto done;
  }

  if(lineBuffer.startsWith("AT+CWJAP=")) {
    char ssid[MAX_COMMAND_ARG_SIZE+1];
    char password[MAX_COMMAND_ARG_SIZE+1];
    const int next = parseString(&lineBuffer[9], ssid) + 9;
    parseString(&lineBuffer[next], password);

    WiFi.begin(ssid, password);

    int count = 10;
    while (WiFi.status() != WL_CONNECTED && count >= 0) {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\r\nERROR ");
    }

    Serial.println("\r\nOK\r\n");
    goto done;
  }

  Serial.print("\r\nUnknown Command '");
  Serial.print(lineBuffer);
  Serial.println();

done:
  lineBuffer = "";
}


void processCommandByte(char incomingByte) {
  if (incomingByte == 13) {
    processPotentialCommand();
  } else {
    if (incomingByte >= 32 && incomingByte < 127) {
      lineBuffer += ((char)incomingByte);

      if (commandEcho)
        Serial.print((char)incomingByte);
    }
  }
}
