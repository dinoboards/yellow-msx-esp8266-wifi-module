/*********
  Yellow MSX for RC2014
  RC2014 Wifi Module
  Serial to (telnet) TCP bridge
*********/

const int LED_PIN = 5;

#include <ESP8266WiFi.h>
#include "gpio.h"
#include "parse-string.h"
#include "at-command-parser.h"
#include "passthrough-escaping.h"
#include "system-operation-mode.h"

WiFiClient client;

void setup() {
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(19200);
  setRXOpenDrain();

  delay(100);

  Serial.println("Wifi Module for Yellow MSX\r\n");

  // Serial.println();
  // Serial.println();
  // Serial.print("Connecting to ");
  // Serial.println(ssid);

  // WiFi.begin(ssid, password);

  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }

  // Serial.println("");
  // Serial.println("WiFi connected");
  // Serial.print("IP address: ");
  // Serial.println(WiFi.localIP());

  // if (client.connect("192.168.86.146", 2000)) {
  //   Serial.println("connected to telnet session");
  // }
}

int incomingByte = 0;
int counter = 0;
int timeOfLastIncomingByte = 0;


void loop() {
  const int timeSinceLastByte = millis() - timeOfLastIncomingByte;

  testForEscapeSequence(timeSinceLastByte);

  if (Serial.available() > 0) {
    digitalWrite(LED_PIN, HIGH);   // turn the LED on
    counter = 100;

    incomingByte = Serial.read();

    if (systemState == commandMode)
      processCommandByte(incomingByte);
    else if (incomingByte == '+') {
      processPotentialEscape(timeSinceLastByte);

    } else {
      abortEscapeSquence();
      Serial.print((char)incomingByte);
    }

    timeOfLastIncomingByte = millis();
  } else {
   if (counter > 0)
    counter -= 1;
    if (counter == 0)
      digitalWrite(LED_PIN, LOW);    // turn the LED off
  }

  // if( client.available() > 0 ) {
  //   char c = client.read();
  //   Serial.print(c);
  // }
}
