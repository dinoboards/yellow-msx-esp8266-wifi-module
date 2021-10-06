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

  if (lineLower == F("ate0")) {
    if (commandEcho)
      Serial.print(F("\r\n"));
    commandEcho = false;
    Serial.print(F("OK\r\n"));
    goto done;
  }

  if (commandEcho)
    Serial.print(F("\r\n"));

  if (lineBuffer == F(""))
    goto done;

  if (lineLower == F("at")) {
    Serial.print(F("OK\r\n"));
    goto done;
  }

  if (lineLower.startsWith(F("atd"))) {
    atCommandDial();
    goto done;
  }

  if (lineLower.startsWith(F("at+cwjap="))) {
    atCommandWifi();
    goto done;
  }

  if (lineLower == F("ath")) {
    atCommandHangup();
    goto done;
  }

  if (lineLower.startsWith(F("at+locale="))) {
    atCommandSetLocale();
    goto done;
  }

  if (lineLower == F("at+time?")) {
    atCommandGetTime();
    goto done;
  }

  if (lineLower.startsWith(F("at+wget"))) {
    atCommandWebGet();
    goto done;
  }

  if (lineLower == F("ate1")) {
    commandEcho = true;
    Serial.print(F("OK\r\n"));
    goto done;
  }

  if (lineLower == F("+++")) {
    Serial.print("OK\r\n");
    goto done;
  }

  Serial.printf(PSTR("Unknown Command '%s'\r\n"), lineBuffer);

done:
  lineBuffer = F("");
}

void processCommandByte(char incomingByte) {
  if (incomingByte == 13) {
    processPotentialCommand();
  } else {
    if (incomingByte == BACKSPACE) {
      lineBuffer = lineBuffer.substring(0, lineBuffer.length() - 1);

      if (commandEcho)
        Serial.print(F("\b \b"));
    }

    if (incomingByte >= 32 && incomingByte < 127) {
      lineBuffer += ((char)incomingByte);

      if (commandEcho)
        Serial.print((char)incomingByte);
    }
  }
}
