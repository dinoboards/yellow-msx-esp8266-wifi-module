#include <Arduino.h>
#include "gpio.h"

void setRXOpenDrain() {
  GPC(RX_PIN) = (GPC(RX_PIN) & (0xF << GPCI)); //SOURCE(GPIO) | DRIVER(NORMAL) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
  GPEC = (1 << RX_PIN); //Disable
  GPF(RX_PIN) = GPFFS(GPFFS_BUS(RX_PIN));//Set mode to BUS (RX0, TX0, TX1, SPI, HSPI or CLK depending in the pin)
}
