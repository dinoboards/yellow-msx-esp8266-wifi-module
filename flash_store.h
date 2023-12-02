#ifndef __FLASH_STORE
#define __FLASH_STORE

#include <EEPROM_Rotate.h>

extern EEPROM_Rotate EEPROMr;

extern void eeprom_setup();
extern int eeprom_get_baud();
extern void eeprom_set_baud(const int new_baud);
extern void eeprom_ota_reset();

#endif

