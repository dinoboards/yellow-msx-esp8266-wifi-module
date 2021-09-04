#include <SoftwareSerial.h>
#include "passthrough-escaping.h"
#include "system-operation-mode.h"
#include "client-connection.h"

int passthroughEscapeSequence = 0;
bool potentialEscapeSequenceForming = false;

void abortEscapeSquence() {
  if (!potentialEscapeSequenceForming)
    return;

  for(int i = 0; i < passthroughEscapeSequence; i++)
    client.write('+');
  passthroughEscapeSequence = 0;
  potentialEscapeSequenceForming = false;
}

void processPotentialEscape(const int timeSinceLastByte) {
  switch(passthroughEscapeSequence) {
    case 0:
      if (timeSinceLastByte > 1000) {
        potentialEscapeSequenceForming = true;
        passthroughEscapeSequence++;
        return;
      }
      client.write('+');
      return;

    case 1:
    case 2:
      if (timeSinceLastByte < 200) {
        passthroughEscapeSequence++;
        return;
      }
      break;
  }

  passthroughEscapeSequence++;
  abortEscapeSquence();
}

void testForEscapeSequence(const int timeSinceLastByte) {
  if (!potentialEscapeSequenceForming)
    return;

  if ((passthroughEscapeSequence == 1 || passthroughEscapeSequence == 2) && timeSinceLastByte > 210) {
    abortEscapeSquence();
  }

  if (passthroughEscapeSequence == 3 && timeSinceLastByte > 1000) {
    Serial.print("\r\nREADY\r\n");
    systemState = commandMode;
    passthroughEscapeSequence = 0;
  }
}
