#include "provisioning.h"
#include "config.h"
#include <lmic.h>
#include <EEPROM.h>

void provisioning_request(int *length, int *fport, uint8_t *payload) {
  payload[0] = 0x7;
  *length = 1;
  *fport = 1;
}

bool provisioning_available(void) {  //Returns false if device was not provisioned yet
  EEPROM.begin(EEPROM_SIZE);
  if (EEPROM.read(EEPROM_PROVISIONED)) {
    EEPROM.end();
    return false;
  } else {
    EEPROM.end();
    return true;
  }
}

bool provisioning_load(u1_t *appEui, u1_t *appKey) { //Returns false if device was not provisioned yet
  int i;
  EEPROM.begin(EEPROM_SIZE);
  if (EEPROM.read(EEPROM_PROVISIONED)) {
    EEPROM.end();
    return false;
  } else {
    for (i=0; i<8; i++) { //Read APPEUI from EEPROM (reversing)
      appEui[i] = EEPROM.read(EEPROM_APPEUI + 7 - i);
    }

    for (i=0; i<16; i++) { //Read APPKEY from EEPROM
      appKey[i] = EEPROM.read(i + EEPROM_APPKEY);
    }
    EEPROM.end();
    return true;
  }
}

bool provisioning_store(int dataBeg, int length, int fport, uint8_t *payload) {  //Returns false if data was invalid
  if (!(length == 25 && payload[LMIC.dataBeg] == 'P')) return false; //Error
  else { //Store data
    int i;
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.write(EEPROM_PROVISIONED, 0x0); //Enable AppEui and AppKey

    for (i=0; i<8; i++) { //Store AppEui
      EEPROM.write(i + EEPROM_APPEUI, LMIC.frame[LMIC.dataBeg + i  + 1]);
    }

    for (i=0; i<16; i++) { //Store AppKey
      EEPROM.write(i + EEPROM_APPKEY, LMIC.frame[LMIC.dataBeg + i  + 9]);
    }
    EEPROM.end();
  }
}

void provisioning_remove(void) {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(EEPROM_PROVISIONED, 0xFF);
  EEPROM.end();
}
