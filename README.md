# TTNmon-LoRaBoard-Software
Basic software for TTNmon LoRa Node

TODO: Much more description and documentation! This project is very alpha!

# LoRa Commands
These commands can be used for remote configuration using downlink messages using the according FPort.

## Port 1
### PROVISIONING
Send the provisioning instructions

Send payload (1 + 8 + 16 = 25 Byte) as followed:
'P' <APPEUI> <APPKEY>

```
50 11 22 33 44 55 66 77 88 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16
|  |                       |
|  |                       --- APPKEY
|  ---------------------------- APPEUI
------------------------------- Command indicator for provisioning 'P'
```
### Reboot
Send 00

### Reprovisioning
Send 01
Sets EEPROM address 0 to 0xFF and reboots. A new provisioning proccess is invoked.

### Rejoin
Send 02
Tries to rejoin network. If it fails, it will continue to use the previous sesion

# Serial commands
Some options can only be configured using a serial connection to your board

## Voltage correction:
Serial: vx.xx while x.xx is current voltage of battery. Correction for the voltage divider and ADC is automatically calculated.

# EEPROM layout
0       Provisioned? 0xFF if not, else 0x0
1 - 8   APPEUI
9 - 23  APPKEY
25      Stored JOIN? 0xFF if not, else 0x0
