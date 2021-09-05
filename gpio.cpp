#include "gpio.h"
#include <Arduino.h>

#define RX_PIN 3

// GPIO13 for CTS input
#define CTSPin 13

void setRXOpenDrain() {
  GPC(RX_PIN) = (GPC(RX_PIN) & (0xF << GPCI)); // SOURCE(GPIO) | DRIVER(NORMAL) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
  GPEC = (1 << RX_PIN);                        // Disable
  GPF(RX_PIN) = GPFFS(GPFFS_BUS(RX_PIN));      // Set mode to BUS (RX0, TX0, TX1, SPI, HSPI or CLK depending in the pin)
}

void setCTSFlowControl() {
  pinMode(CTSPin, FUNCTION_4); // make pin U0CTS
  U0C0 |= (1 << UCTXHFE);      // add this sentense to add a tx flow control via MTCK( CTS )
}
