#include <SoftwareSerial.h>
#include "system-operation-mode.h"
#include "at-command-parser.h"
#include "at-command-dial.h"
#include "client-connection.h"

String hostName = "";
int portNumber = 2000;

void atCommandDial() {
  hostName = lineBuffer.substring(3);

  int hostPortSeperatorPosition = hostName.indexOf(':');

  if (hostPortSeperatorPosition != -1) {
    Serial.print("\r\n'" + hostName.substring(hostPortSeperatorPosition+1) + "'\r\n");
    portNumber = hostName.substring(hostPortSeperatorPosition+1).toInt();
    hostName = hostName.substring(0, hostPortSeperatorPosition);
  }

  Serial.print("\r\nAttempting to connect to ");
  Serial.print(hostName);
  Serial.print(":");
  Serial.print(portNumber);
  Serial.print(" ....\r\n");

  if (client.connect(hostName, portNumber)) {
    Serial.print("connected\r\n");
    systemState = passthroughMode;
  }
  else
    Serial.print("failed to connect\r\n");
}
