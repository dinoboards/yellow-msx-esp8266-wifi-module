#include "at-command-parser.h"
#include "at-command-dial.h"
#include "at-command-hangup.h"
#include "at-command-time.h"
#include "at-command-wifi.h"
#include "at-command-web-get.h"
#include <SoftwareSerial.h>

bool commandEcho = true;
String lineBuffer = "";

#define BACKSPACE 8


void processPotentialCommand() {
  String lineLower = String(lineBuffer);
  lineLower.toLowerCase();

  if (lineLower == "ate0") {
    if (commandEcho)
      Serial.print("\r\n");
    commandEcho = false;
    Serial.print("OK\r\n");
    goto done;
  }

  if (commandEcho)
    Serial.print("\r\n");

  if (lineBuffer == "")
    goto done;

  if (lineLower == "at") {
    Serial.print("\OK\r\n");
    goto done;
  }

  if (lineLower.startsWith("atd")) {
    atCommandDial();
    goto done;
  }

  if (lineLower.startsWith("at+cwjap=")) {
    atCommandWifi();
    goto done;
  }

  if (lineLower == "ath") {
    atCommandHangup();
    goto done;
  }

  if (lineLower.startsWith("at+locale=")) {
    atCommandSetLocale();
    goto done;
  }

  if (lineLower == "at+time?") {
    atCommandGetTime();
    goto done;
  }

  if (lineLower.startsWith("at+wget")) {
    atCommandWebGet();
    goto done;
  }

  if (lineLower == "ate1") {
    commandEcho = true;
    Serial.print("OK\r\n");
    goto done;
  }

  if (lineLower == "+++") {
    Serial.print("OK\r\n");
    goto done;
  }

  Serial.printf("Unknown Command '%s'\r\n", lineBuffer);

done:
  lineBuffer = "";
}

void processCommandByte(char incomingByte) {
  if (incomingByte == 13) {
    processPotentialCommand();
  } else {
    if (incomingByte == BACKSPACE) {
      lineBuffer = lineBuffer.substring(0, lineBuffer.length() - 1);

      if (commandEcho)
        Serial.print("\b \b");
    }

    if (incomingByte >= 32 && incomingByte < 127) {
      lineBuffer += ((char)incomingByte);

      if (commandEcho)
        Serial.print((char)incomingByte);
    }
  }
}
