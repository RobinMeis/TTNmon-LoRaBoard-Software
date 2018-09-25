#include "user.h"
#include <HardwareSerial.h>

void user_init(void) { //Hey user, do you want to do some init? Called after VBat check, but before os_init()
  Serial.println("user_init called");
  touch_pad_init();
}

void user_loop(void) { //Hey user, do you want to do something? Has to be non-blocking!
}

bool user_uplink(uint8_t *payload, int *length, int *FPort) { //Hey user, do you want to send anything? Otherwise I will only send VBat
  Serial.println("user_uplink requested");
  uint16_t touch_value;
  touch_pad_read(TOUCH_PAD_NUM8, &touch_value);
  Serial.println(touch_value);
  payload[0] = touch_value & 0x00FF;
  payload[1] = (touch_value & 0xFF00) >> 8;
  *FPort = 2;
  *length = 2;
}

void user_downlink(int FPort, int dataBeg, int length, uint8_t *payload) { //Hey user, we received downlink. Do you want to do anything?
  Serial.println("Hey user, we received downlink. Do you want to do anything?");
  Serial.println(FPort);
}

void user_sleep(int minutes) { //Do your own stuff to put controller into sleep if you want
  //If you don't need this, just leave it empty
}

