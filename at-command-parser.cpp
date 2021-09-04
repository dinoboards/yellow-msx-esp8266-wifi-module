#include <SoftwareSerial.h>
#include "at-command-parser.h"
#include "at-command-dial.h"
#include "at-command-wifi.h"

const bool commandEcho = true;
String lineBuffer = "";

void processPotentialCommand() {
  if (lineBuffer == "AT") {
    Serial.print("\r\nOK\r\n");
    goto done;
  }

  if(lineBuffer == "") {
    Serial.print("\r\n");
    goto done;
  }

  if (lineBuffer.startsWith("ATD")) {
    atCommandDial();
    goto done;
  }
  if(lineBuffer.startsWith("AT+CWJAP=")) {
    atCommandWifi();
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
