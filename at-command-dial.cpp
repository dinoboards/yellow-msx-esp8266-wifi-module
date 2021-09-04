#include <SoftwareSerial.h>
#include "system-operation-mode.h"
#include "at-command-parser.h"
#include "at-command-dial.h"

String hostName = "";
int portNumber = 2000;

void atCommandDial() {
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
}
