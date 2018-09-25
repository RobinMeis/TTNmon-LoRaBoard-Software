#ifndef POWER_H
#define POWER_H

float power_voltage(void); //Returns battery voltage
void power_shutdown(void); //Puts controller to final sleep
void power_sleep(int seconds); //Puts controller to sleep for specified time
void power_set_sleep(char hours, char minutes); //Writes sleeping time to EEPROM
int power_get_sleep(void); //Returns sleeping duration in Seconds (from EEPROM)
void lauflicht(int seconds); //Lauftlicht
double power_get_correction(void); //Gets corection factor for Battery ADC
void power_set_correction(float voltage); //Calculates correction factor for Battery ADC based on external measured voltage
#endif
