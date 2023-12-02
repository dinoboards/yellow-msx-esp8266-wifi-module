#include "at-command-parser.h"
#include "at-command-uart.h"
#include "flash_store.h"
#include "gpio.h"

void atCommandSetBaud() {
  const int baud_rate = lineBuffer.substring(8).toInt();

  Serial.flush();
  Serial.begin(baud_rate);
  setRXOpenDrain();
  setCTSFlowControlOn();

  if(lineBuffer.endsWith(",store"))
    eeprom_set_baud(baud_rate);

  Serial.print(F("OK\r\n"));
}

