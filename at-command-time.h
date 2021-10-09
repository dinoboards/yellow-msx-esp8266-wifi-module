#ifndef __AT_COMMAND_TIME
#define __AT_COMMAND_TIME

#include <ESP8266WiFi.h>
#include <ezTime.h>

extern Timezone myTimeZone;

extern void atCommandSetLocale();
extern void atCommandGetTime();

#endif
