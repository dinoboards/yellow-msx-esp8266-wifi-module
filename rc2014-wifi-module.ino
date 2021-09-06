

/*********
  Yellow MSX for RC2014
  RC2014 Wifi Module
  Serial to (telnet) TCP bridge
*********/

const int SERIAL_RX_LED = 5;
const int SERIAL_TX_LED = 4;

#include "at-command-parser.h"
#include "gpio.h"
#include "parse-string.h"
#include "passthrough-escaping.h"
#include "system-operation-mode.h"
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ezTime.h>

WiFiClient client;
int updateProgressFilter = 0;

void setup() {
#ifdef WIFI_IS_OFF_AT_BOOT
  enableWiFiAtBootTime(); // can be called from anywhere with the same effect
#endif

  pinMode(SERIAL_RX_LED, OUTPUT);
  pinMode(SERIAL_TX_LED, OUTPUT);

  Serial.begin(19200);
  setRXOpenDrain();
  setCTSFlowControl();

  Serial.println("\r\n\033[2JWifi Module for Yellow MSX.\r\n");

  WiFi.begin();

  int count = 20;
  while (WiFi.status() != WL_CONNECTED && count >= 0) {
    delay(500);
    Serial.print(".");
    count--;
  }

  if (WiFi.status() != WL_CONNECTED)
    Serial.print("\r\nWiFi not connected\r\n");
  else
    Serial.printf("\r\nWiFi connected to %s\r\n", WiFi.SSID());

  ArduinoOTA.onStart([]() {
    String type;

    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "application";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.print("Start updating " + type + "\r\n");
  });

  ArduinoOTA.onEnd([]() { Serial.print("\r\nEnd"); });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if ((updateProgressFilter & 7) == 0 || progress == total)
      Serial.printf("\r\033[2KProgress: %u%%", (progress / (total / 100)));

    updateProgressFilter++;
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.print("Auth Failed\r\n");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.print("Begin Failed\r\n");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.print("Connect Failed\r\n");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.print("Receive Failed\r\n");
    } else if (error == OTA_END_ERROR) {
      Serial.print("End Failed\r\n");
    }
  });
  ArduinoOTA.begin();

  waitForSync();

  Serial.println("UTC: " + UTC.dateTime());

  Timezone myTimeZone;
  myTimeZone.setLocation("Australia/Melbourne");
  Serial.print("Local Time is: " + myTimeZone.dateTime());
  Serial.print("\r\n");

  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.print("\r\n");
  Serial.print("READY\r\n");
}

int incomingByte = 0;
int rxLedHoldCounter = 0;
int txLedHoldCounter = 0;
int timeOfLastIncomingByte = 0;

bool wasPassthroughMode = false;

void loop() {
  const int timeSinceLastByte = millis() - timeOfLastIncomingByte;

  ArduinoOTA.handle();

  testForEscapeSequence(timeSinceLastByte);

  if (wasPassthroughMode && !isPassthroughMode()) {
    Serial.print("\r\nREADY\r\n");
  }

  wasPassthroughMode = isPassthroughMode();

  if (Serial.available() > 0) {
    digitalWrite(SERIAL_RX_LED, HIGH); // turn the LED on
    rxLedHoldCounter = 5;

    incomingByte = Serial.read();

    if (isCommandMode())
      processCommandByte(incomingByte);

    else if (incomingByte == '+') {
      processPotentialEscape(timeSinceLastByte);

    } else {
      abortEscapeSquence();
      client.write((char)incomingByte);
    }

    timeOfLastIncomingByte = millis();
  } else {
    if (rxLedHoldCounter > 0)
      rxLedHoldCounter -= 1;
    if (rxLedHoldCounter == 0)
      digitalWrite(SERIAL_RX_LED, LOW); // turn the LED off
  }

  if (isPassthroughMode())
    if (client.available() > 0) {

      digitalWrite(SERIAL_TX_LED, HIGH); // turn the LED on
      txLedHoldCounter = 20;

      const char c = client.read();
      Serial.print(c);
    }

  if (txLedHoldCounter > 0)
    txLedHoldCounter -= 1;
  if (txLedHoldCounter == 0)
    digitalWrite(SERIAL_TX_LED, LOW); // turn the LED off
}
