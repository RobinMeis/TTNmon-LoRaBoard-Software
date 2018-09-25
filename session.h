#include <lmic.h>

#ifndef SESSION_H
  #define SESSION_H

  bool session_available(void); //Checks if there are stored keys in RTC available
  void session_store(u1_t *nwkKey, u1_t *artKey); //Stores Keys and parameters in RTC
  void session_restore(u1_t *nwkKey, u1_t *artKey); //Loads Keys from RTC
  void session_store_datarate(dr_t dr); //Stores datarate in RTC
  dr_t session_restore_datarate(void); //Loads datarate from RTC, returns false if datarate is unset, otherwise true
  void session_store_counters(u4_t up, u4_t down); //Store packet counters to RTC
  void session_restore_counters(u4_t *up, u4_t *down); //Restore packet counters from RTC
  void session_store_devaddr(u4_t devaddr); //Store device address to RTC
  u4_t session_restore_devaddr(void); //Restore device address from RTC
  void session_set_channels(void); //Sets channels for TTN
  void sesesion_next_channel(void); //Sets next channel after wake-up
#endif
