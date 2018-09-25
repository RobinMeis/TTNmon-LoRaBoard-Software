#ifndef CONFIG_H
  #define CONFIG_H
  /* Provisioning Keys
  Application to JOIN for remote provisioning */
  #define PROVISIONING_APPEUI { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 } //Replace with you own APPEUI. This one is only for demo purposes!
  #define PROVISIONING_APPKEY { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 }; //Replace with your own APPKEY. This one is only for demo purposes!

  /* Shutdown voltages */
  #define SHUTDOWN_VOLTAGE 3.75 //Message will be sent before shutdown
  #define CRITICAL_VOLTAGE 3.7 //Shutdown will be performed immeadiately
  #define BATTERY_ADC_CORRECTION 0.003141559 //Default correction factor for battery ADC. You might want to adjust it

  /* Confirmation delay
  Time to wait before shutdown if downlink was received */
  #define CONFIRMATION_DELAY 20000 //in milliseconds

  /* Hardware
  Select hardware version (1-0 or 1-1)
  (If you don't have an early prototype, you will likely use 1-1 as prototype board files where never released!)
  */
  #define HW_VERSION 1-1

  #if HW_VERSION == 1-0
    /* RFM**W (SPI) Config */
    #define MOSI 27
    #define MISO 19
    #define SCK 5
    #define NSS 2
    #define RST 18
    #define DIO0 14
    #define DIO1 23
    #define DIO2 25
  #elif HW_VERSION == 1-1
    /* RFM**W (SPI) Config */
    #define MOSI 32
    #define MISO 34
    #define SCK 5
    #define NSS 23
    #define RST 22
    #define DIO0 25
    #define DIO1 27
    #define DIO2 14

    #define USER1 33
    #define USER2 4
    #define USER3 2
    #define USER4 15

    #define DHT_PIN 17
  #else
    #error Invalid Hardware Version
  #endif

  /* Payload config */
  #define USER_PAYLOAD_SIZE 20 //Max payload size in Bytes
  #define TOTAL_PAYLOAD_SIZE 21 //At least one Byte more than USER_PAYLOAD_SIZE

  /* EEPROM Layout
  Start positions*/
  #define EEPROM_SIZE 64
  #define EEPROM_PROVISIONED 0 //1 Byte
  #define EEPROM_APPEUI 1 //8 Byte
  #define EEPROM_APPKEY 9 //16 Byte
  #define EEPROM_SLEEP_HOURS 26 //1 Byte
  #define EEPROM_SLEEP_MINUTES 27 //1 Byte
  #define EEPROM_ADC_CORRECTION 28 //Double
#endif
