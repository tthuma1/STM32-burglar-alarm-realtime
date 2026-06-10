# VGRS Burglar Alarm Realtime

## Overview

Realtime burglar alarm firmware for the STM32H750B-DK. The alarm is armed/disarmed via PIN entry and an authorized RFID tag scan. A PIR sensor on PG3 triggers a TIM6 countdown; if not cancelled in time, an alarm event fires. All state changes are reported over Ethernet to a companion Flask webserver.

Target MCU: **STM32H750xB** (Cortex-M7, 480 MHz, 128 KB flash, up to 1 MB RAM).

## FreeRTOS Tasks

| Task | Purpose |
|---|---|
| `defaultTask` | LwIP init, touchscreen poll/render |
| `taskLED` | PWM breathing LED when alarm is armed |
| `taskRFID` | Card detection and PIN validation |
| `taskHTTP` | Dequeues `HttpEvent_t` and sends HTTP POST |

Tasks communicate via thread flags and `g_http_event_queue`.

## HTTP Event Reporting

`taskHTTP` sends `POST /event HTTP/1.1` to `192.168.1.1:5000` (device IP: `192.168.1.10`) with body `{"type": "<event_type>"}`.

| `event_type` | Trigger |
|---|---|
| `alarm_on` | PIN + RFID validated, alarm armed |
| `alarm_off` | PIN + RFID validated, alarm disarmed |
| `motion_detected` | EXTI interrupt on PG3 |
| `alarm_triggered` | TIM6 countdown expired |

The companion webserver lives in `webserver/` — see `webserver/CLAUDE.md`.

## Repository Layout

- `Core/Inc`, `Core/Src` — application code and peripheral drivers
  - `main.c` / `main.h` — entry point, peripheral init, task definitions, HTTP client
  - `touchscreen.c` / `touchscreen.h` — PIN keypad, status overlays, countdown UI
  - `stm32h7xx_it.c` — ISRs: EXTI motion callback, TIM6 alarm trigger
  - `stm32h7xx_hal_msp.c` — peripheral clock and GPIO setup
- `Core/Components` — MFRC522, FT5336 touch, SDRAM, Quad-SPI flash drivers
- `Drivers/STM32H7xx_HAL_Driver` — STM32H7 HAL
- `Middlewares/Third_Party/FreeRTOS` — FreeRTOS kernel
- `webserver/` — Flask event log server
- `Startup`, `Fonts` — MCU startup/linker scripts, display font assets

## Notes

- The MPU marks Ethernet DMA buffers at `0x30040000` as non-cacheable before enabling D-Cache to prevent HardFaults.
- Debug output: USART3 at 115200 baud (VCP pins PB10/PB11).
- Build and flash via STM32CubeIDE or the generated Makefile.

## References

- [ST STM32H750 product page](https://www.st.com/en/microcontrollers-microprocessors/stm32h750xb.html)
- MFRC522 RFID reader datasheet
- STM32H7 HAL and FreeRTOS documentation
