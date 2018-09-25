#include "session.h"
#include <rom/rtc.h>
#include <lmic.h>
#include <HardwareSerial.h>

RTC_DATA_ATTR char channel;
RTC_DATA_ATTR u4_t framesDown, framesUp, DEVADDR;
RTC_DATA_ATTR dr_t datarate;
RTC_DATA_ATTR u1_t NWKSKEY[16], APPSKEY[16];

void session_store(u1_t *nwkKey, u1_t *artKey) {
  int n;
  channel = 3;
  for (n=0; n<16; ++n) //Store network session key to RTC
    NWKSKEY[n] = nwkKey[n];

  for (n=0; n<16; ++n) //Store app session key to RTC
    APPSKEY[n] = artKey[n];
}

void session_restore(u1_t *nwkKey, u1_t *artKey) {
  int n;

  for (n=0; n<16; ++n) //Restore network session key from RTC
    nwkKey[n] = NWKSKEY[n];

  for (n=0; n<16; ++n) //Restore app session key from RTC
    artKey[n] = APPSKEY[n];
}

void session_store_devaddr(u4_t devaddr) {
  DEVADDR = devaddr;
}

u4_t session_restore_devaddr(void) {
  return DEVADDR;
}

void session_store_datarate(dr_t dr) {
  datarate = dr;
}

dr_t session_restore_datarate(void) {
  return datarate;
}

void session_store_counters(u4_t up, u4_t down) {
  framesUp = up;
  framesDown = down;
}

void session_restore_counters(u4_t *up, u4_t *down) {
  *up = framesUp;
  *down = framesDown;
}

void session_set_channels(void) {
  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK, DR_FSK), BAND_MILLI); // g2-band
}

void sesesion_next_channel(void) {
  int n;

  if (channel < 7) channel = channel + 1;
  else channel = 0;

  for (n = 0; n<9; ++n) //Select TX channel
    if (n != channel) LMIC_disableChannel(n);
}
