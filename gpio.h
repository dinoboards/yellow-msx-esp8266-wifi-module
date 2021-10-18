
extern void setRXOpenDrain();
extern void setCTSFlowControlOn();
extern void setCTSFlowControlOff();

extern void initLeds();
extern void ledLoop();
extern void allLedsOff();
extern void rxLedFlash();
extern void txLedFlash();
extern void wifiLedOn();
extern void wifiLedOff();
extern void tcpConnectedLedOn();

#define SERIAL_RX_LED 5
#define SERIAL_TX_LED 4
#define MAIN_LED_PIN 2

#define RX_PIN 3

// GPIO13 for CTS input
#define CTSPin 13
