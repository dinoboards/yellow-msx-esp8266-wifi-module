#include "at-command-hangup.h"
#include "at-command-parser.h"
#include "client-connection.h"
#include "gpio.h"
#include <SoftwareSerial.h>

void atCommandHangup() {
  client.stop();
  wifiLedOn();

  Serial.print("\r\nOK\r\n");
}
