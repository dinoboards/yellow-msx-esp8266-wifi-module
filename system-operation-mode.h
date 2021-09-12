#ifndef __SYSTEM_OPERATION_MODE
#define __SYSTEM_OPERATION_MODE

enum OperationMode {
  CommandMode,
  PassthroughMode,
  XmodemSending
};

extern OperationMode operationMode;

#define isXModemSendingMode() (operationMode == XmodemSending)
#define isCommandMode() (operationMode == CommandMode)
#define isPassthroughMode() (operationMode == PassthroughMode)

#endif
