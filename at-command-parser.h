#ifndef __AT_COMMAND_PARSER
#define __AT_COMMAND_PARSER

#include <Arduino.h>

extern String lineBuffer;
extern void processPotentialCommand();
extern void processCommandByte(char incomingByte);

#endif
