#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#include <rom/rtc.h>

#include <algorithm>
#include <iterator>

#include "config.h"
#include "provisioning.h"
#include "session.h"
#include "power.h"
#include "user/user.h"

static u1_t DEVEUI[8];
static u1_t APPEUI[8] = PROVISIONING_APPEUI; //Initialize for provisioning
static u1_t APPKEY[16] = PROVISIONING_APPKEY;

static u1_t NWKSKEY[16], APPSKEY[16]; //ABP Definitions

void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}
void os_getDevKey (u1_t* buf) { memcpy_P(buf, APPKEY, 16);}

static uint8_t payload[TOTAL_PAYLOAD_SIZE], user_payload[USER_PAYLOAD_SIZE];
static osjob_t sendjob;

static float battery_voltage;
static char voltage[4], serialInput, voltageRead;

unsigned TX_INTERVAL = 10;
static enum state {NONE, PROVISIONING, PROVISIONING_COMPLETE, PROVISIONING_FAILED, REBOOT, SHUTDOWN, DELAYED_SLEEP} state = NONE;
static int target_millis;

// Pin mapping
const lmic_pinmap lmic_pins = {
  .mosi = MOSI,
  .miso = MISO,
  .sck = SCK,
  .nss = NSS,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = RST,
  .dio = {DIO0, DIO1, DIO2},
};

void onEvent (ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  switch(ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println(F("EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      Serial.println(F("EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      Serial.println(F("EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      Serial.println(F("EV_BEACON_TRACKED"));
      break;
    case EV_JOINING:
      Serial.println(F("EV_JOINING"));
      break;
    case EV_JOINED:
      if (state != PROVISIONING and state != PROVISIONING_COMPLETE and state != PROVISIONING_FAILED and state != REBOOT) {
        Serial.println(F("EV_JOINED, Storing ABP Data"));
        session_store_devaddr(LMIC.devaddr);
        session_store(LMIC.nwkKey, LMIC.artKey);
        session_store_datarate(LMIC.datarate);
      } else Serial.println(F("EV_JOINED, Storing ABP Data"));

      digitalWrite(LED_JOIN, LOW);
      LMIC_setLinkCheckMode(0);
      break;
    case EV_RFU1:
      Serial.println(F("EV_RFU1"));
      break;
    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      break;
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
      if (LMIC.txrxFlags & TXRX_ACK)
        Serial.println(F("Received ack"));
          if (LMIC.dataLen) {
              if (LMIC.frame[LMIC.dataBeg-1] == 1) { //Port 1 (config port)
                if (state == PROVISIONING) { //Do we expect to be provisioned?
                  if (LMIC.dataLen == 25) { //Has data a valid length?
                    if (LMIC.frame[LMIC.dataBeg] == 'P') { //Is data likely a provisioning packet?
                      Serial.println("Received AppEUI and AppKey, writing to EEPROM"); //TODO: Bedingungen preufen
                      provisioning_store(LMIC.dataBeg, LMIC.dataLen, LMIC.txrxFlags, LMIC.frame);
                      Serial.println("Configuration has been written");
                      state = PROVISIONING_COMPLETE;
                    } else {
                      Serial.println("Packet has correct length for provision but seems to be invalid");
                    }
                  } else {
                    Serial.println("Invalid payload length for provisioning");
                  }
                } else {
                  state = DELAYED_SLEEP;
                  target_millis = millis() + CONFIRMATION_DELAY;
                  if (LMIC.frame[LMIC.dataBeg] == 0x00) { //Reboot
                    Serial.println("Performing reboot...");
                    state = REBOOT;
                  } else if (LMIC.frame[LMIC.dataBeg] == 0x01) { //Reprovisioning
                    Serial.println("Reprovisioning");
                    provisioning_remove();
                    state = REBOOT;
                  } else if (LMIC.frame[LMIC.dataBeg] == 0x02) { //Rejoin
                    Serial.println("Rejoin");
                    LMIC_tryRejoin();
                  } else if (LMIC.frame[LMIC.dataBeg] == 0x03) { //Shutdown __WARNING__ Shutdown can't be undone remotely!
                    state = SHUTDOWN;
                    target_millis = millis() + CONFIRMATION_DELAY;
                  } else if (LMIC.frame[LMIC.dataBeg] == 0x04 && LMIC.dataLen == 3) { //Set sleep duration: 0x04 <BYTE HOURS> <BYTE MINUTES>
                    power_set_sleep(LMIC.frame[LMIC.dataBeg + 1], LMIC.frame[LMIC.dataBeg + 2]);
                  } else if (LMIC.frame[LMIC.dataBeg] == 0x05 && LMIC.dataLen == 2) { //Let it blink for specified seconds
                    lauflicht(LMIC.frame[LMIC.dataBeg + 1]);
                    target_millis = millis() + CONFIRMATION_DELAY;
                  }
                  Serial.println(F("Received "));
                  Serial.println(LMIC.dataLen);
                  Serial.println(F(" bytes of payload"));
                }
              } else {
                user_downlink(LMIC.frame[LMIC.dataBeg - 1], LMIC.dataBeg, LMIC.dataLen, LMIC.frame);
              }
            }

            session_store_counters(LMIC.seqnoUp, LMIC.seqnoDn);
            if (state != PROVISIONING and state != PROVISIONING_COMPLETE and state != PROVISIONING_FAILED and state != REBOOT and state != SHUTDOWN and state != DELAYED_SLEEP) {
              Serial.println("NOW SLEEPING zzzZZZ");
              if (battery_voltage <= SHUTDOWN_VOLTAGE) {
                int n;
                for (n = 0; n < 20; ++n) {
                  digitalWrite(LED_JOIN, HIGH); //All LEDs on
                  digitalWrite(LED_BLUE, HIGH);
                  digitalWrite(LED_WHITE, HIGH);
                  delay(50);
                  digitalWrite(LED_JOIN, LOW);
                  digitalWrite(LED_BLUE, LOW);
                  digitalWrite(LED_WHITE, LOW);
                  delay(50);
                }
                Serial.println("Shutdown!");
                power_shutdown();
              } else {
                int seconds = power_get_sleep();
                Serial.println(seconds);
                user_sleep(seconds);
                power_sleep(seconds);
              }
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void do_send(osjob_t* j){
  if (LMIC.opmode & OP_TXRXPEND) { // Check if there is not a current TX/RX job running
    Serial.println(F("OP_TXRXPEND, not sending"));
  } else {
    int length, fport;
    if (state == PROVISIONING) {
    digitalWrite(LED_WHITE, !digitalRead(LED_WHITE));
    digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
    provisioning_request(&length, &fport, payload);
    LMIC_setTxData2(fport, payload, length, 0);
    Serial.println("Sending provisioning request");
  } else if (state != PROVISIONING_COMPLETE and state != PROVISIONING_FAILED and state != REBOOT) {
    Serial.println("VOLTAGE");
    // Prepare upstream data transmission at the next possible time.
    Serial.println(battery_voltage);
    if (battery_voltage <= SHUTDOWN_VOLTAGE) {
      Serial.println("Low power, shutdown!");
      payload[0] = 0xff;
      LMIC_setTxData2(1, payload, 1, 0);
    } else {
      payload[0] = battery_voltage * 10; //Send battery voltage
      int user_length, n, fport;
      fport = 1;
      user_length = 0;
      user_uplink(user_payload, &user_length, &fport);
      for (n=0; n < user_length; ++n)
        payload[n + 1] = user_payload[n];
      LMIC_setTxData2(fport, payload, 1 + user_length, 0);
    }
    Serial.println(F("Packet queued"));
    }
  }
  // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
  pinMode(LED_JOIN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_WHITE, OUTPUT);

  Serial.begin(115200);
  delay(500);
  battery_voltage = power_voltage();
  Serial.println(battery_voltage);
  if (battery_voltage <= CRITICAL_VOLTAGE) {
    Serial.println("Low power, shutdown!");
    int n;
    for (n = 0; n < 20; ++n) {
      digitalWrite(LED_JOIN, HIGH); //All LEDs on
      digitalWrite(LED_BLUE, HIGH);
      digitalWrite(LED_WHITE, HIGH);
      delay(50);
      digitalWrite(LED_JOIN, LOW);
      digitalWrite(LED_BLUE, LOW);
      digitalWrite(LED_WHITE, LOW);
      delay(50);
    }

    power_shutdown();
  }
  user_init();

Serial.println("OS init");
  os_init(); //LMIC Init
  Serial.println("OS inited");
  LMIC_reset();
  power_get_correction(); //TODO: remove

Serial.println(rtc_get_reset_reason(0));
  if (rtc_get_reset_reason(0) != 5 or !provisioning_available()) { //Power-on reset, rejoin
    digitalWrite(LED_JOIN, HIGH); //All LEDs on
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_WHITE, HIGH);
    delay(500);
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_WHITE, LOW);

    Serial.println("=========== TTNmon Node ===========");
    Serial.println("System MAC (different formats)");

    esp_read_mac(DEVEUI, ESP_MAC_WIFI_STA); //Use wifi MAC as LoRa MAC (DevEUI)
    DEVEUI[7] = DEVEUI[5]; //Convert EUI-48 to EUI-64 according to https://standards.ieee.org/develop/regauth/tut/eui.pdf#page=15
    DEVEUI[6] = DEVEUI[4];
    DEVEUI[5] = DEVEUI[3];
    DEVEUI[3] = 0xFF;
    DEVEUI[4] = 0xFE;

    char macStr[27];
    sprintf(macStr, "  %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", DEVEUI[0], DEVEUI[1], DEVEUI[2], DEVEUI[3], DEVEUI[4], DEVEUI[5], DEVEUI[6], DEVEUI[7]);
    Serial.println(macStr);
    sprintf(macStr, "  %02X %02X %02X %02X %02X %02X %02X %02X", DEVEUI[0], DEVEUI[1], DEVEUI[2], DEVEUI[3], DEVEUI[4], DEVEUI[5], DEVEUI[6], DEVEUI[7]);
    Serial.println(macStr);
    sprintf(macStr, "  %02X%02X%02X%02X%02X%02X%02X%02X", DEVEUI[0], DEVEUI[1], DEVEUI[2], DEVEUI[3], DEVEUI[4], DEVEUI[5], DEVEUI[6], DEVEUI[7]);
    Serial.println(macStr);
    Serial.print("\n\n");

    std::reverse(std::begin(DEVEUI), std::end(DEVEUI)); //Reverse MAC for LMIC

    if (!provisioning_available()) { //EEPROM has not been written yet => device wasn't provisioned yet
      state = PROVISIONING;
      digitalWrite(LED_WHITE, HIGH);
      Serial.println("Device hasn't been provisioned yet. Please push AppKey and AppEUI using your provisioning application");
    } else {
      TX_INTERVAL = 60;
      provisioning_load(APPEUI, APPKEY);
    }
  } else { //Something else, use stored ABP data
    int n;

    Serial.println("");
    session_restore(NWKSKEY, APPSKEY);
    LMIC_setSession (0x1, session_restore_devaddr(), NWKSKEY, APPSKEY);
    session_restore_counters(&LMIC.seqnoUp, &LMIC.seqnoDn);
    session_set_channels();
    sesesion_next_channel();



    // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(session_restore_datarate(),14);
    Serial.println("Session restored");
  }



    // Start job (sending automatically starts OTAA too)
    payload[0] = 0xff;
  do_send(&sendjob);

  voltageRead = 0;
}

void loop() {
  if (Serial.available() > 0) { //Remove provisioning
    serialInput = Serial.read();
    if (serialInput == 'r') {
      Serial.println("Remove provisioning");
      provisioning_remove();
      ESP.restart();
    } else if (serialInput == 'v' || voltageRead > 0) {
      if (serialInput == 'v')
        voltageRead = 1;
      else if (voltageRead > 4) voltageRead = 0;
      else {
        if (voltageRead == 2 && serialInput == '.') {
          voltage[1] = '.';
          voltageRead = 3;
        } else if (voltageRead != 2 && (serialInput == '0' || serialInput == '1' || serialInput == '2' || serialInput == '3' || serialInput == '4' || serialInput == '5' || serialInput == '6' || serialInput == '7' || serialInput == '8' || serialInput == '9')) {
          voltage[voltageRead - 1] = serialInput;
          ++voltageRead;
        } else {
          voltageRead = 0;
        }

        if (voltageRead == 5) {
          float float_voltage = strtof(voltage, NULL);
          Serial.print("Voltage read: ");
          Serial.println(float_voltage);
          power_set_correction(float_voltage);
          voltageRead = 0;
        }
      }
    }
  }

  if (state == PROVISIONING_COMPLETE or state == PROVISIONING_FAILED or state == REBOOT) { //Reboot
    int i;
    digitalWrite(LED_WHITE, LOW);
    digitalWrite(LED_BLUE, HIGH);

    for (i=20; i>0; --i) {
      Serial.println(i);
      digitalWrite(LED_WHITE, !digitalRead(LED_WHITE));
      digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
      delay(500);
      os_runloop_once();
      digitalWrite(LED_WHITE, !digitalRead(LED_WHITE));
      digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
      delay(500);
    }

    Serial.println("Reboot now");
    ESP.restart();
  } else if (state == DELAYED_SLEEP && target_millis < millis()) {
    if (user_sleep(power_get_sleep()) == false) //Put to sleep if user didn't do this
      power_sleep(power_get_sleep());
  } else if (state == SHUTDOWN && target_millis < millis()) {
    power_shutdown();
  }
  os_runloop_once();
  user_loop();
}
