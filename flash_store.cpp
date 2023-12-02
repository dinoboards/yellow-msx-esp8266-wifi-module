#include "flash_store.h"
#include <EEPROM_Rotate.h>

EEPROM_Rotate EEPROMr;

const int default_baud_rate = 19200;

void eeprom_setup() {
  EEPROMr.size(4);
  EEPROMr.begin(128);
}

int eeprom_get_baud() {
  int value;
  int value_complement;
  
  EEPROMr.get(10, value);
  EEPROMr.get(10 + sizeof(int), value_complement);

  if (value != ~value_complement) {
    EEPROMr.put(10, default_baud_rate);
    EEPROMr.put(10 + sizeof(int), ~default_baud_rate);
    EEPROMr.commit();

    return default_baud_rate;
  }

  return value;
}

void eeprom_set_baud(const int new_baud) {
  EEPROMr.put(10, new_baud);
  EEPROMr.put(10 + sizeof(int), ~new_baud);
  EEPROMr.commit();
}

void eeprom_ota_reset() {
  EEPROMr.rotate(false);
  eeprom_set_baud(default_baud_rate);
}
