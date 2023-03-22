# GC RFID Card Reader Emulation

## Hardware

- Esp32-S3 (others are probably also possible, but haven't test) or Arduino Pro Micro
- PN532
- USB to Serial adapter supporting hardware flow control (has CTS line)

## Wiring

### Esp32 and PN532

Communicating through SPI

| Esp32   | PN532   |
| ------- | ------- |
| 7       | NSS     |
| 15      | MOSI    |
| 16      | SCK     |
| 17      | MISO    |
| 3.3v/5v | 3.3v/5v |
| GND     | GND     |

### Esp32 and Serial

| Esp32 | Serial |
| ----- | ------ |
| 1     | CTS    |
| 2     | TX     |
| 42    | RX     |
| GND   | GND    |

### Arduino Pro Micro and PN532
Communicating through SPI

| Arduino | PN532   |
| ------- | ------- |
| 7       | NSS     |
| 16      | MOSI    |
| 15      | SCK     |
| 14      | MISO    |
| 3.3v/5v | 3.3v/5v |
| GND     | GND     |

### Esp32 and Serial

|Arduino| Serial |
| ----- | ------ |
| 2     | CTS    |
| RX    | TX     |
| TX    | RX     |
| GND   | GND    |

## Game

Patch the game to use the COM port you specify, do not use COM2 (COM2 is hooked by jconfig)
