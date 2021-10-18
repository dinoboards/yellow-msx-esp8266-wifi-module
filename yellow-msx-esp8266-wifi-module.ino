

/*********
  Yellow MSX for RC2014
  RC2014 Wifi Module
  Serial to (telnet) TCP bridge
*********/

#include "at-command-msx-rc2014.h"
#include "at-command-parser.h"
#include "at-command-time.h"
#include "at-command-web-get.h"
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

  Serial.begin(19200);
  setRXOpenDrain();
  setCTSFlowControlOff(); // Dont wont to have setup blocked, if serial not able to send

  initLeds();

  Serial.println(F("\r\n"));

  if (!(WiFi.SSID() == F("")) || (WiFi.SSID() == NULL))
    WiFi.begin();

  Serial.print(F("\033[2J"));

  firmwareInit(true);
  if (WiFi.status() != WL_CONNECTED)
    return;

  ArduinoOTA.onStart([]() { Serial.print(F("Updating ESP8266 Firmware\r\n")); });

  ArduinoOTA.onEnd([]() { Serial.print(F("\r\nCompleted Download.\r\n")); });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if ((updateProgressFilter & 7) == 0 || progress == total) {
      setCTSFlowControlOff(); // Dont wont to blocked, if serial not able to send
      Serial.printf(PSTR("\r\033[2KProgress: %u%%"), (progress / (total / 100)));
    }
    updateProgressFilter++;
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf(PSTR("Error[%u]: "), error);
    if (error == OTA_AUTH_ERROR) {
      Serial.print(F("Auth Failed\r\n"));
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.print(F("Begin Failed\r\n"));
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.print(F("Connect Failed\r\n"));
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.print(F("Receive Failed\r\n"));
    } else if (error == OTA_END_ERROR) {
      Serial.print(F("End Failed\r\n"));
    }
  });
  ArduinoOTA.begin();

  Serial.flush();
  setCTSFlowControlOn();
}

int incomingByte = 0;
int timeOfLastIncomingByte = 0;
bool wasPassthroughMode = false;

void loop() {
  const int timeSinceLastByte = millis() - timeOfLastIncomingByte;

  ledLoop();

  ArduinoOTA.handle();

  testForEscapeSequence(timeSinceLastByte);

  if (isPassthroughMode() && !client.connected()) {
    Serial.print(F("\r\nREADY\r\n"));
    operationMode = CommandMode;

    if (WiFi.status() != WL_CONNECTED)
      wifiLedOff();
    else
      wifiLedOn();
  }

  wasPassthroughMode = isPassthroughMode();

  if (Serial.available() > 0) {
    rxLedFlash();

    incomingByte = Serial.read();

    if (isXModemSendingMode()) {
      txLedFlash();
      xmodemReceiveChar(incomingByte);
    }

    else if (isCommandMode()) {
      txLedFlash();
      processCommandByte(incomingByte);
    }

    else if (incomingByte == '+') {
      processPotentialEscape(timeSinceLastByte);

    } else {
      abortEscapeSquence();
      client.write((char)incomingByte);
    }

    timeOfLastIncomingByte = millis();
  } else if (isXModemSendingMode())
    xmodemLoop();

  if (isPassthroughMode())
    if (client.available() > 0) {
      txLedFlash();
      const char c = client.read();
      Serial.print(c);
    }
}
