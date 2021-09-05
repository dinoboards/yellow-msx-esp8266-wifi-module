#include "at-command-parser.h"
#include "at-command-dial.h"
#include "at-command-wifi.h"
#include <SoftwareSerial.h>

const bool commandEcho = true;
String lineBuffer = "";

void processPotentialCommand() {
  String lineLower = String(lineBuffer);

  if (lineBuffer == "") {
    Serial.print("\r\n");
    goto done;
  }

  lineLower.toLowerCase();

  if (lineLower == "at") {
    Serial.print("\r\nOK\r\n");
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

  Serial.printf("\r\nUnknown Command '%s'\r\n", lineBuffer);

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
