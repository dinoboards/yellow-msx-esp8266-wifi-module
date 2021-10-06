#include "at-command-dial.h"
#include "at-command-parser.h"
#include "client-connection.h"
#include "system-operation-mode.h"
#include "gpio.h"
#include <SoftwareSerial.h>

String hostName;
int portNumber = 23;

void atCommandDial() {
  hostName = lineBuffer.substring(3);

  int hostPortSeperatorPosition = hostName.indexOf(':');
  portNumber = 23;

  if (hostPortSeperatorPosition != -1) {
    portNumber = hostName.substring(hostPortSeperatorPosition + 1).toInt();
    hostName = hostName.substring(0, hostPortSeperatorPosition);
  }

  Serial.print(F("Attempting to connect to "));
  Serial.print(hostName);
  Serial.print(':');
  Serial.print(portNumber);
  Serial.print(F(" ....\r\n"));

  if (client.connect(hostName, portNumber)) {
    Serial.print(F("connected\r\n"));
    tcpConnectedLedOn();
    operationMode = PassthroughMode;
  } else
    Serial.print(F("failed to connect\r\n"));
}
