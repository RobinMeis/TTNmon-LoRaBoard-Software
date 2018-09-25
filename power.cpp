#include "power.h"
#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"
#include "esp_deep_sleep.h"
#include <HardwareSerial.h>

float power_voltage(void) {
  Serial.println(analogRead(35));
  return analogRead(35) * power_get_correction();
}

void power_shutdown(void) {
  esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_deep_sleep_start();
}

void power_sleep(int seconds) {
  esp_deep_sleep_enable_timer_wakeup(seconds * 1000000);
  //esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  //esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  //esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  //   esp_deep_sleep_enable_timer_wakeup(((uint64_t) sec) * 1000000);
  esp_deep_sleep_start();
}

void power_set_sleep(char hours, char minutes) {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(EEPROM_SLEEP_HOURS, hours);
  EEPROM.write(EEPROM_SLEEP_MINUTES, minutes);
  EEPROM.end();
}

int power_get_sleep(void) {
  int minutes, hours;
  EEPROM.begin(EEPROM_SIZE);
  hours = EEPROM.read(EEPROM_SLEEP_HOURS);
  minutes = EEPROM.read(EEPROM_SLEEP_MINUTES);
  EEPROM.end();

  if (hours == 0xFF || minutes == 0xFF)
    return 60;
  else
    minutes += hours * 60;
  return minutes * 60;
}

void lauflicht(int seconds) {
  digitalWrite(LED_JOIN, LOW);
  digitalWrite(LED_WHITE, LOW);
  digitalWrite(LED_BLUE, LOW);

  int n;
  seconds *= 2;
  for (n=0; n<seconds; ++n) {
    digitalWrite(LED_JOIN, HIGH);
    digitalWrite(LED_WHITE, LOW);
    delay(125);
    digitalWrite(LED_WHITE, HIGH);
    digitalWrite(LED_JOIN, LOW);
    delay(125);
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_WHITE, LOW);
    delay(125);
    digitalWrite(LED_WHITE, HIGH);
    digitalWrite(LED_BLUE, LOW);
    delay(125);
  }
  digitalWrite(LED_WHITE, LOW);
}

void power_set_correction(float voltage) {
  EEPROM.begin(EEPROM_SIZE);
  double correction_factor = voltage / analogRead(35);
  Serial.println(correction_factor, 10);
  EEPROM.put(EEPROM_ADC_CORRECTION, correction_factor);
  EEPROM.end();
}

double power_get_correction(void) {
  EEPROM.begin(EEPROM_SIZE);
  double correction_factor;
  EEPROM.get(EEPROM_ADC_CORRECTION, correction_factor);
  Serial.println(correction_factor, 10);
  EEPROM.end();
  return correction_factor;
}
