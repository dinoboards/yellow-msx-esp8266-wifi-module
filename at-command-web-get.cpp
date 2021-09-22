#include "at-command-web-get.h"
#include "at-command-parser.h"
#include "client-connection.h"
#include "gpio.h"
#include "system-operation-mode.h"
#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClientSecureBearSSL.h>

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

char packetBuffer[128];

XModemState xmodemState = XMODEMSTATE_NONE;
unsigned char packetNo = 0;
int tryNo = 0;
int crcBuf;
unsigned char checksumBuf;
int packetLen = 128;
int timeout;
bool oldChecksum;
WiFiClient *wifiClient;
HTTPClient *httpClient;
int fileSize;
bool lastPacketSent;
unsigned char nextPacketHeader;

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

void prepareNextPacket() {
  memset(packetBuffer, EOF, 128);

  if (fileSize == 0 || lastPacketSent || !httpClient->connected()) {
    nextPacketHeader = EOT;
    return;
  }

  if (httpClient->connected() && fileSize == -1) {
    int c = wifiClient->readBytes(packetBuffer, sizeof(packetBuffer));
    if (c == 0) {
      nextPacketHeader = EOT;
      return;
    }

    lastPacketSent = c != sizeof(packetBuffer);
    nextPacketHeader = SOH;

    calculateChecksums();
    return;
  }

  int c = wifiClient->readBytes(packetBuffer, std::min((size_t)fileSize, sizeof(packetBuffer)));
  if (!c) {
    nextPacketHeader = CAN;
    return;
  }

  fileSize -= c;
  nextPacketHeader = SOH;
  calculateChecksums();
}

XModemState sendPacket() {
  switch (nextPacketHeader) {
  case SOH:
    Serial.write(SOH);
    Serial.write(packetNo);
    Serial.write(~packetNo);
    Serial.write(packetBuffer, 128);
    if (oldChecksum)
      Serial.write((char)checksumBuf);
    else {
      Serial.write((char)(crcBuf >> 8));
      Serial.write((char)(crcBuf & 0xFF));
    }
    return XMODEMSTATE_ACK;

  case EOT:
    httpClient->end();
    Serial.write(EOT);
    return XMODEMSTATE_FINAL_ACK;

  default:
    Serial.write(CAN);
    Serial.write(CAN);
    Serial.write(CAN);
    operationMode = CommandMode;
    return XMODEMSTATE_NONE;
  }
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
      prepareNextPacket();
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
  const String url = lineBuffer.substring(7);

  if (wifiClient)
    delete wifiClient;

  //https://
  if (url[4] == 's') {
    wifiClient = new BearSSL::WiFiClientSecure();
    ((BearSSL::WiFiClientSecure*)wifiClient)->setInsecure();
  }
  else
    wifiClient = new WiFiClient();

  if (httpClient)
    delete httpClient;
  httpClient = new HTTPClient();

  const bool r = httpClient->begin(*wifiClient, url);
  const bool connected = httpClient->connected();

  int httpCode = httpClient->GET();
  if (httpCode <= 0) {
    Serial.printf("ERROR: GET '%s' returned error code: %d. (%d, %d)\r\n", url.c_str(), httpCode, (int)r, (int)connected);
    return;
  }

  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("ERROR: Http returned error status: %d, %s\r\n", httpCode, httpClient->errorToString(httpCode).c_str());
    return;
  }

  Serial.write("OK\r\n");

  fileSize = httpClient->getSize();

  packetNo = 1;
  tryNo = 0;
  timeout = millis() + 1000; // Timeout in one second if we dont get 'C' or 'NAK'
  operationMode = XmodemSending;
  lastPacketSent = false;
  prepareNextPacket();
  xmodemState = XMODEMSTATE_WAIT_FOR_START;
}
