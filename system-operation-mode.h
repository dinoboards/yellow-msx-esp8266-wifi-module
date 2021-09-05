#ifndef __SYSTEM_OPERATION_MODE
#define __SYSTEM_OPERATION_MODE

#define isCommandMode() (!client.connected())
#define isPassthroughMode() (client.connected())

#endif
