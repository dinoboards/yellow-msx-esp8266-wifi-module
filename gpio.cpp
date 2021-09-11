#include "gpio.h"
#include <Arduino.h>


#define SERIAL_RX_LED 5
#define SERIAL_TX_LED 4
#define MAIN_LED_PIN 2


#define RX_PIN 3

// GPIO13 for CTS input
#define CTSPin 13

int rxLedHoldCounter = 0;
int txLedHoldCounter = 0;

enum MainLedState {
  Off,
  WifiConnected,
  TcpConnection
};

MainLedState mainLedState = Off;
bool mainLedOn = false;
unsigned long mainLedFlashToggleTime = 0;

void initLeds() {
  pinMode(SERIAL_RX_LED, OUTPUT);
  pinMode(SERIAL_TX_LED, OUTPUT);
  pinMode(MAIN_LED_PIN, OUTPUT);

  allLedsOff();
}

void allLedsOff() {
  mainLedState = Off;

  digitalWrite(SERIAL_RX_LED, LOW);
  digitalWrite(SERIAL_TX_LED, LOW);
  digitalWrite(MAIN_LED_PIN, HIGH);
}

void ledLoop() {
  if (rxLedHoldCounter > 0)
    rxLedHoldCounter -= 1;
  else
    digitalWrite(SERIAL_RX_LED, LOW);

  if (txLedHoldCounter > 0)
    txLedHoldCounter -= 1;
  else
    digitalWrite(SERIAL_TX_LED, LOW);

  if (mainLedState == TcpConnection && mainLedFlashToggleTime < millis()) {
    digitalWrite(MAIN_LED_PIN, mainLedOn ? HIGH : LOW);
    mainLedOn = !mainLedOn;
    mainLedFlashToggleTime = millis() + (mainLedOn ? 1700 : 150);
  }
}

void rxLedFlash() {
  digitalWrite(SERIAL_RX_LED, HIGH);
  rxLedHoldCounter = 5;
}

void txLedFlash() {
  digitalWrite(SERIAL_TX_LED, HIGH);
  txLedHoldCounter = 20;
}

void tcpConnectedLedOn() {
  mainLedState = WifiConnected;
  digitalWrite(MAIN_LED_PIN, LOW);
}

void wifiLedOff() {
  mainLedState = Off;
  digitalWrite(MAIN_LED_PIN, HIGH);
  mainLedOn = false;
}

void wifiLedOn() {
  mainLedState = TcpConnection;
  digitalWrite(MAIN_LED_PIN, LOW);
  mainLedFlashToggleTime = millis() + 1000;
  mainLedOn = true;
}

void setRXOpenDrain() {
  GPC(RX_PIN) = (GPC(RX_PIN) & (0xF << GPCI)); // SOURCE(GPIO) | DRIVER(NORMAL) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
  GPEC = (1 << RX_PIN);                        // Disable
  GPF(RX_PIN) = GPFFS(GPFFS_BUS(RX_PIN));      // Set mode to BUS (RX0, TX0, TX1, SPI, HSPI or CLK depending in the pin)
}

void setCTSFlowControlOn() {
  pinMode(CTSPin, FUNCTION_4); // make pin U0CTS
  U0C0 |= (1 << UCTXHFE);      // Set bit to activate Hardware flow control
}

void setCTSFlowControlOff() {
  pinMode(CTSPin, FUNCTION_4); // make pin U0CTS
  U0C0 &= ~(1 << UCTXHFE);      // Reset bit to deactivate hardware flow control
}
