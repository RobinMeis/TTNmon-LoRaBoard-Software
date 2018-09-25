#include <lmic.h>

#ifndef USER_H
#define USER_H

void user_init(void); //Is called on system startup __before__ LoRaWAN is initialized
void user_loop(void); //Is called regularly
bool user_uplink(uint8_t *payload, int *length, int *FPort); //Asks the user to specify any custom uplink payload for sending. Has to return false if no data was specified, otherwise true
void user_downlink(int FPort, int dataBeg, int length, uint8_t *payload); //Informs the user about incoming payload
void user_sleep(int minutes); //Do your own stuff to put controller into sleep if you want
#endif
