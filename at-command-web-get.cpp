#include "at-command-web-get.h"
#include "at-command-parser.h"
// #include "cert-store.h"
#include "client-connection.h"
#include "gpio.h"
#include "system-operation-mode.h"
#include "version.h"
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

// BearSSL::CertStoreP certStore;

enum XModemState { XMODEMSTATE_NONE = 1, XMODEMSTATE_WAIT_FOR_START, XMODEMSTATE_NAK_SOH, XMODEMSTATE_NAK_EOT, XMODEMSTATE_NAK_CAN, XMODEMSTATE_ACK, XMODEMSTATE_FINAL_ACK };

char *packetBuffer;

XModemState xmodemState = XMODEMSTATE_NONE;
XModemState xmodemRetryState;
unsigned char packetNo = 0;
int crcBuf;
unsigned char checksumBuf;
unsigned long timeout;
bool oldChecksum;
WiFiClient *wifiClient;
HTTPClient *httpClient;
int fileSize;
bool lastPacketSent;
int packetLength = 128;

void calculateChecksums() {
  checksumBuf = 0x00;
  crcBuf = 0x00;

  for (int i = 0; i < packetLength; i++) {
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

XModemState xmodemCompleted() {
  xmodemState = XMODEMSTATE_NONE;
  operationMode = CommandMode;

  free(packetBuffer);
  packetBuffer = NULL;

  delete httpClient;
  httpClient = NULL;

  delete wifiClient;
  wifiClient = NULL;

  return XMODEMSTATE_NONE;
}

XModemState prepareNextPacket() {
  memset(packetBuffer, EOF, packetLength);

  if (fileSize == -1) {
    if (httpClient->connected() && !lastPacketSent) {
      int c = wifiClient->readBytes(packetBuffer, packetLength);
      if (c == 0) {
        return XMODEMSTATE_NAK_EOT;
      }

      lastPacketSent = c != sizeof(packetBuffer);

      calculateChecksums();
      return XMODEMSTATE_NAK_SOH;
    }

    return XMODEMSTATE_NAK_EOT;
  }

  if (fileSize == 0 || !httpClient->connected()) {
    return XMODEMSTATE_NAK_EOT;
  }

  int c = wifiClient->readBytes(packetBuffer, std::min(fileSize, packetLength));
  if (!c) {
    return XMODEMSTATE_NAK_CAN;
  }

  fileSize -= c;
  calculateChecksums();
  return XMODEMSTATE_NAK_SOH;
}

XModemState sendEOT() {
  httpClient->end();
  Serial.write(EOT);
  return XMODEMSTATE_FINAL_ACK;
}

XModemState sendCAN() {
  Serial.write(CAN);
  Serial.write(CAN);
  Serial.write(CAN);
  Serial.print(F("Send Packet Cancelled\r\n"));
  return xmodemCompleted();
}

XModemState sendPreparedPacket() {
  Serial.write(packetLength == 128 ? SOH : STX);
  Serial.write(packetNo);
  Serial.write(~packetNo);
  Serial.write(packetBuffer, packetLength);
  if (oldChecksum)
    Serial.write((char)checksumBuf);
  else {
    Serial.write((char)(crcBuf >> 8));
    Serial.write((char)(crcBuf & 0xFF));
  }
  return XMODEMSTATE_ACK;
}

void xmodemLoop() {
  switch (xmodemState) {
  case XMODEMSTATE_NONE:
    return;

  case XMODEMSTATE_WAIT_FOR_START:
    if (millis() > timeout)
      xmodemCompleted();
    return;

  case XMODEMSTATE_NAK_SOH:
    xmodemRetryState = xmodemState = sendPreparedPacket();
    timeout = millis() + 10000;
    return;

  case XMODEMSTATE_NAK_EOT:
    xmodemRetryState = xmodemState = sendEOT();
    timeout = millis() + 10000;
    return;

  case XMODEMSTATE_NAK_CAN:
    xmodemRetryState = xmodemState = sendCAN();
    return;

  case XMODEMSTATE_ACK:
  case XMODEMSTATE_FINAL_ACK:
    const unsigned long m = millis();
    if (m > timeout) {
      Serial.write(CAN);
      Serial.write(CAN);
      Serial.write(CAN);
      Serial.printf(PSTR("Timed out... %d, %ld, %ld\r\n"), (int)xmodemState, m, timeout);
      xmodemCompleted();
      return;
    }
  }
}

void xmodemReceiveChar(unsigned char incoming) {
  switch (xmodemState) {
  case XMODEMSTATE_WAIT_FOR_START:
    xmodemState = xmodemRetryState;
    switch (incoming) {
    case 'C':
      oldChecksum = false;
      break;

    case NAK:
      oldChecksum = true;
      break;
    }
    break;

  case XMODEMSTATE_ACK:
    switch (incoming) {
    case CAN:
      xmodemCompleted();
      return;

    case ACK:
      xmodemState = prepareNextPacket();
      packetNo++;
      return;

    case NAK:
      xmodemState = xmodemRetryState;
      return;
    }
    break;

  case XMODEMSTATE_FINAL_ACK:
    switch (incoming) {
    case CAN:
      xmodemCompleted();
      return;

    case ACK:
      xmodemCompleted();
      return;
    }
    break;
  }
}

void atCommandWebGet() {
  packetLength = lineBuffer[7] == '1' ? 1024 : 128;

  const String url = lineBuffer[7] == '1' ? lineBuffer.substring(8) : lineBuffer.substring(7);

  if (packetBuffer)
    free(packetBuffer);
  packetBuffer = (char*)malloc(packetLength);

  if (httpClient)
    delete httpClient;
  httpClient = NULL;

  if (wifiClient)
    delete wifiClient;
  wifiClient = NULL;

  if (url[4] == 's' || url[4] == 'S') {
    BearSSL::WiFiClientSecure *wifiSSLClient = new BearSSL::WiFiClientSecure();

    // if (wifiSSLClient->probeMaxFragmentLength(url, 443, 512))
    //   wifiSSLClient->setBufferSizes(512, 512);

    // wifiSSLClient->setCertStore(&certStore);
    wifiSSLClient->setInsecure();
    wifiClient = wifiSSLClient;
  } else
    wifiClient = new WiFiClient();

  httpClient = new HTTPClient();
  httpClient->setReuse(false);
  httpClient->setUserAgent(F("MSX on RC2014/" VERSION " (ESP8266HTTPClient)"));
  httpClient->useHTTP10(true);

  const bool r = httpClient->begin(*wifiClient, url);
  const bool connected = httpClient->connected();

  httpClient->setReuse(false);

  int httpCode = httpClient->GET();
  if (httpCode <= 0) {
    Serial.printf(PSTR("ERROR: GET '%s' returned error: %s. (%d, %d)\r\n"), url.c_str(), httpClient->errorToString(httpCode).c_str(), (int)r, (int)connected);
    return;
  }

  if (httpCode != HTTP_CODE_OK) {
    Serial.printf(PSTR("ERROR: Http returned error status: %d\r\n"), httpCode);
    return;
  }

  Serial.write(PSTR("OK\r\n"));

  fileSize = httpClient->getSize();

  packetNo = 1;
  timeout = millis() + 10000; // Timeout in ten second if we dont get 'C' or 'NAK'
  operationMode = XmodemSending;
  lastPacketSent = false;
  xmodemRetryState = prepareNextPacket();
  xmodemState = XMODEMSTATE_WAIT_FOR_START;
}
