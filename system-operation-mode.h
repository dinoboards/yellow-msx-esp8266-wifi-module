#ifndef __SYSTEM_OPERATION_MODE
#define __SYSTEM_OPERATION_MODE

enum OperationMode {
  CommandMode,
  PassthroughMode
};

extern OperationMode operationMode;

#define isCommandMode() (operationMode == CommandMode)
#define isPassthroughMode() (operationMode == PassthroughMode)

#endif
