#include <lmic.h>

#ifndef PROVISIONING_H
  #define PROVISIONING_H

  bool provisioning_available(void); //Checks if there are stored keys
  void provisioning_request(int *length, int *fport, uint8_t *payload); //Generates the required payload for provisioning requests TODO: de-pointer!
  bool provisioning_store(int dataBeg, int length, int fport, uint8_t *payload); //Stores received Keys in EEPROM
  bool provisioning_load(u1_t *appEui, u1_t *appKey); //Loads stored Keys from EEPROM
  void provisioning_remove(void); //Disables provisioning Keys
#endif
