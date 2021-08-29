#ifndef __SYSTEM_OPERATION_MODE
#define __SYSTEM_OPERATION_MODE

enum SystemState {
  commandMode,
  passthroughMode,
};

extern SystemState systemState;

#endif
