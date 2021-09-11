

/*********
  Yellow MSX for RC2014
  RC2014 Wifi Module
  Serial to (telnet) TCP bridge
*********/

#include "at-command-parser.h"
#include "at-command-time.h"
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

  Serial.println("\r\n\033[2JWifi Module for Yellow MSX.\r\n");

  if ((WiFi.SSID() == "") || (WiFi.SSID() == NULL))
    return;

  WiFi.begin();

  int count = 20;
  while (WiFi.status() != WL_CONNECTED && count >= 0) {
    delay(500);
    Serial.print(".");
    count--;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("\r\nWiFi not connected\r\n");
    return;
  } else
    Serial.printf("\r\nWiFi connected to %s\r\n", WiFi.SSID());

  wifiLedOn();

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
    if ((updateProgressFilter & 7) == 0 || progress == total) {
      setCTSFlowControlOff(); // Dont wont to blocked, if serial not able to send
      Serial.printf("\r\033[2KProgress: %u%%", (progress / (total / 100)));
      setCTSFlowControlOn(); // Dont wont to blocked, if serial not able to send
    }
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

  Serial.print("Syncing time ...");
  timezoneSetup();
  Serial.print(" DONE\r\n");

  delay(250); //as we dont have flow control - give the RC2014 time to consume serial data
  Serial.print("Local Time is: ");
  Serial.print(myTimeZone.dateTime());

  delay(250); //as we dont have flow control - give the RC2014 time to consume serial data
  Serial.print("\r\nIP address: ");
  Serial.print(WiFi.localIP());

  delay(250); //as we dont have flow control - give the RC2014 time to consume serial data
  Serial.print("\r\nCPU Speed: ");
  Serial.print(ESP.getCpuFreqMHz());

  delay(250); //as we dont have flow control - give the RC2014 time to consume serial data
  Serial.print("MHz\r\nREADY\r\n");

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
    Serial.print("\r\nREADY\r\n");
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

    if (isCommandMode()) {
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
  }

  if (isPassthroughMode())
    if (client.available() > 0) {
      txLedFlash();
      const char c = client.read();
      Serial.print(c);
    }
}
