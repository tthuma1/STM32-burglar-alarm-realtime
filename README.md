# VGRS Burglar Alarm — STM32H750B-DK

Realtime burglar alarm firmware for the [STM32H750B-DK](https://www.st.com/en/microcontrollers-microprocessors/stm32h750xb.html) discovery kit. The system arms and disarms via PIN + RFID authentication, detects intrusion with a PIR sensor, and reports all state changes to a companion webserver over Ethernet.

## Hardware

### Board

**STM32H750B-DK** — Cortex-M7 @ 480 MHz, 128 KB flash, up to 1 MB RAM, on-board 4" capacitive touchscreen (480×272), Ethernet (LAN8742A PHY, MII interface).

### Wiring

| Peripheral | Interface | Pins |
|---|---|---|
| MFRC522 RFID | SPI2 | CS → PB4, RESET → PI8 |
| PIR motion sensor | GPIO EXTI | Signal → PG3 (Arduino D2) |
| Debug UART | USART3 | TX → PB10, RX → PB11 (VCP, 115200 baud) |
| Touchscreen | I2C4 + LTDC | On-board (FT5336 controller) |

All components use 3.3V power.

#### MFRC522 RFID

| Signal | STM32 Pin | Arduino Header |
|---|---|---|
| SDA (SS) | PB4 | D10 |
| SCK | PD3 | D13 |
| MOSI | PB15 | D11 |
| MISO | PI2 | D12 |
| RST | PI8 | D7 |

#### LED

| Signal | STM32 Pin | Arduino Header |
|---|---|---|
| LED+ | PA6 | D3 |

#### PIR Sensor

| Signal | STM32 Pin | Arduino Header |
|---|---|---|
| MOTION | PG3 | D2 |

## System Design

### Alarm State Machine

```
ALARM_OFF ──(PIN + RFID)──► WAITING_FOR_MOTION
                                    │
                              PIR triggers
                                    │
                                    ▼
                           MOTION_DETECTED
                           (10 s countdown)
                                    │
                         countdown expires / not cancelled
                                    │
                                    ▼
                            ALARM_TRIGGERED
                           (LED breathing on)
                                    │
                              PIN + RFID
                                    │
                                    ▼
                               ALARM_OFF
```

To arm or disarm, the user must first enter the correct 4-digit PIN on the touchscreen, then scan an authorized RFID tag. Both steps are required; either alone is rejected.

When the alarm is armed and the PIR fires, a 10-second countdown starts on the touchscreen. If the alarm is not disarmed in time, `ALARM_TRIGGERED` is set and the breathing LED activates.

### FreeRTOS Tasks

| Task | Stack | Role |
|---|---|---|
| `defaultTask` | 4 KB | Runs LwIP, polls and renders the touchscreen |
| `taskRFID` | 4 KB | Reads MFRC522; validates card UID against hardcoded authorized ID |
| `taskLED` | 2 KB | PWM breathing effect (TIM3 CH1) when alarm is triggered |
| `taskHTTP` | 2 KB | Dequeues `HttpEvent_t` values and sends HTTP POST to the server |

Thread flags and `g_http_event_queue` (capacity 4) are used for inter-task signalling.

### RTC

The on-chip RTC is driven by the LSE (32.768 kHz crystal). The touchscreen renders a live date/time widget in the sidebar.

### HTTP Event Reporting

`taskHTTP` opens a raw TCP connection to `192.168.1.1:5000` and sends an HTTP/1.1 POST for each state change. The device has a static IP of `192.168.1.10`.

```
POST /event HTTP/1.1
Host: 192.168.1.1
Content-Type: application/json
Connection: close

{"type": "alarm_on"}
```

| Event type | Trigger |
|---|---|
| `alarm_on` | Alarm successfully armed |
| `alarm_off` | Alarm successfully disarmed |
| `motion_detected` | PIR EXTI fires while armed |
| `alarm_triggered` | TIM6 countdown expires |

See [`webserver/README.md`](webserver/README.md) for the server-side API and setup.

## Building and Flashing

The project is configured for **STM32CubeIDE**. Open `VGRS-burglar-alarm-realtime.ioc` or import the project directory.

1. Build with **Project → Build All** (or `Ctrl+B`).
2. Flash with **Run → Debug** or **Run → Run** via the ST-LINK on the USB connector. If this doesn't work (due to flash too small issues), try flashing with the command in [`flash-command.txt`](flash-command.txt)

> The MPU is configured to mark Ethernet DMA buffers at `0x30040000` as non-cacheable before D-Cache is enabled. Do not move this call relative to `SCB_EnableDCache()`.

## Repository Structure

```
├── Core/
│   ├── Inc/            # Application headers (main.h, touchscreen.h, mfrc522.h, ...)
│   ├── Src/            # Application sources (main.c, touchscreen.c, mfrc522.c, ...)
│   └── Components/     # Board-level drivers (LCD, SDRAM, touch controller, ...)
├── Drivers/
│   └── STM32H7xx_HAL_Driver/
├── Middlewares/
│   └── Third_Party/FreeRTOS/
├── Fonts/              # Display font assets
├── Startup/            # MCU startup code and linker scripts
├── webserver/          # Companion Flask event log server
└── VGRS-burglar-alarm-realtime.ioc
```
