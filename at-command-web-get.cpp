#include "at-command-web-get.h"
#include "at-command-parser.h"
#include "client-connection.h"
#include "gpio.h"
#include "system-operation-mode.h"
#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#define SOH 0x01
#define STX 0x02
#define EOT 0x04
#define ENQ 0x05
#define ACK 0x06
#define LF 0x0a
#define CR 0x0d
#define DLE 0x10
#define XON 0x11
#define XOFF 0x13
#define NAK 0x15
#define CAN 0x18
#define EOF 0x1a

enum XModemState { XMODEMSTATE_NONE, XMODEMSTATE_WAIT_FOR_START, XMODEMSTATE_NACK, XMODEMSTATE_ACK, XMODEMSTATE_FINAL_ACK };

String data = "";
const char *sample = "this is a sample file to send over xmodem\r\n";
char packetBuffer[128];

XModemState xmodemState = XMODEMSTATE_NONE;
unsigned char packetNo = 0;
int tryNo = 0;
int crcBuf;
unsigned char checksumBuf;
int packetLen = 128;
int timeout;
bool oldChecksum;
WiFiClient wifiClient;
HTTPClient httpClient;
WiFiClient *stream;
int fileSize;

void calculateChecksums() {
  checksumBuf = 0x00;
  crcBuf = 0x00;

  for (int i = 0; i < 128; i++) {
    char j;
    const unsigned char inChar = packetBuffer[i];
    checksumBuf += inChar;

    crcBuf = crcBuf ^ (int)inChar << 8;
    j = 8;
    do {
      if (crcBuf & 0x8000)
        crcBuf = crcBuf << 1 ^ 0x1021;
      else
        crcBuf = crcBuf << 1;
    } while (--j);
  }
}

XModemState sendPacket() {

  memset(packetBuffer, EOF, 128);

  if (httpClient.connected() && (fileSize > 0 || fileSize == -1)) {
    int c = stream->readBytes(packetBuffer, std::min((size_t)fileSize, sizeof(packetBuffer)));
    if (!c) {
      Serial.write(CAN);
      Serial.write(CAN);
      Serial.write(CAN);
      operationMode = CommandMode;
      return XMODEMSTATE_NONE;
    }

    if (fileSize > 0)
      fileSize -= c;

    Serial.write(SOH);
    Serial.write(packetNo);
    Serial.write(~packetNo);
    Serial.write(packetBuffer, 128);

    calculateChecksums();

    if (oldChecksum)
      Serial.write((char)checksumBuf);
    else {
      Serial.write((char)(crcBuf >> 8));
      Serial.write((char)(crcBuf & 0xFF));
    }
    return XMODEMSTATE_ACK;
  }

  httpClient.end();
  Serial.write(EOT);
  return XMODEMSTATE_FINAL_ACK;
}

void xmodemLoop() {
  switch (xmodemState) {
  case XMODEMSTATE_NONE:
    return;

  case XMODEMSTATE_WAIT_FOR_START:
    if (millis() > timeout) {
      xmodemState = XMODEMSTATE_NONE;
      operationMode = CommandMode;
    }
    return;

  case XMODEMSTATE_NACK:
    xmodemState = sendPacket();
    timeout = millis() + 3000;
    return;

  case XMODEMSTATE_ACK:
  case XMODEMSTATE_FINAL_ACK:
    // TODO implement resend packet???
    if (millis() > timeout) {
      xmodemState = XMODEMSTATE_NONE;
      operationMode = CommandMode;
      Serial.write(CAN);
      Serial.write(CAN);
      Serial.write(CAN);
    }
    return;
  }
}

void xmodemReceiveChar(unsigned char incoming) {
  switch (xmodemState) {
  case XMODEMSTATE_WAIT_FOR_START:
    switch (incoming) {
    case 'C':
      oldChecksum = false;
      xmodemState = XMODEMSTATE_NACK;
      break;

    case NAK:
      oldChecksum = true;
      xmodemState = XMODEMSTATE_NACK;
      break;
    }
    break;

  case XMODEMSTATE_ACK:
    switch (incoming) {
    case CAN:
      xmodemState = XMODEMSTATE_NONE;
      operationMode = CommandMode;
      return;

    case ACK:
      xmodemState = XMODEMSTATE_NACK;
      packetNo++;
      return;
    }
    break;

  case XMODEMSTATE_FINAL_ACK:
    switch (incoming) {
    case CAN:
      xmodemState = XMODEMSTATE_NONE;
      operationMode = CommandMode;
      return;

    case ACK:
      xmodemState = XMODEMSTATE_NONE;
      operationMode = CommandMode;
      return;
    }
    break;
  }
}

void atCommandWebGet() {
  data = lineBuffer.substring(7);

  httpClient.begin(wifiClient, "http://jigsaw.w3.org/HTTP/connection.html");

  int httpCode = httpClient.GET();
  if (httpCode <= 0) {
    Serial.print("ERROR\r\n");
    return;
  }

  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("[HTTP] GET... failed, error: %s\r\n", httpClient.errorToString(httpCode).c_str());
    Serial.print("ERROR\r\n");
    return;
  }

  fileSize = httpClient.getSize();
  stream = &wifiClient;

  packetNo = 1;
  tryNo = 0;
  timeout = millis() + 30000; // Timeout in one second if we dont get 'C' or 'NAK'
  operationMode = XmodemSending;
  xmodemState = XMODEMSTATE_WAIT_FOR_START;
}
