# VGRS Burglar Alarm Realtime

## Overview

This project implements a realtime burglar alarm system on the STM32H750B-DK development board.
It integrates:
- MFRC522 RFID reader for authorized tag detection
- Motion sensor for intrusion detection
- Built-in capacitive touchscreen for user interaction and status display

The firmware is built for the STM32H750B-DK board, which uses an STM32H750xB device based on a high-performance Arm Cortex-M7 core running up to 480 MHz. The MCU includes a double-precision/single-precision FPU, DSP instruction set, MPU, and high-speed memories with 128 Kbytes of flash and up to 1 Mbyte of RAM (including TCM RAM and user SRAM).

STM32H750xB devices also provide advanced peripherals for this project, including:
- LTDC graphics and DMA2D for the built-in touchscreen
- Flexible external memory support for SDRAM and Quad-SPI
- Multiple SPI/I2C/UART interfaces for RFID and sensor connections
- Integrated ADCs, DACs, RNG, and crypto acceleration

## Hardware

Target board:
- STM32H750B-DK

Connected peripherals:
- MFRC522 RFID module
- Motion sensor (PIR / presence detector)
- On-board touchscreen display

Project structure indicates support for:
- STM32H7 HAL drivers
- FreeRTOS
- SDRAM, touchscreen, LCD and external SPI/NFC devices

## Key Features

- RFID-based access control using MFRC522
- Motion-triggered alarm detection
- Touchscreen user interface for system status and control
- Real-time behavior using FreeRTOS and STM32H7 peripherals

## Repository Layout

## Repository Layout

- `Core/Inc` and `Core/Src` - main application code, HAL wrappers, board support and peripheral drivers
  - `main.c` / `main.h` – Core application entry point, initialization, and primary task orchestrator.
  - `touchscreen.c` / `touchscreen.h` – UI rendering engine (keypads, fields, status overlays, and countdowns).
  - `stm32h7xx_it.c` – Interrupt Service Routines (including EXTI motion sensor callbacks).
  - `stm32h7xx_hal_msp.c` – MCU Support Package peripheral clock and GPIO pin setups (e.g., UART3).
- `Core/Components` - driver implementations for MFRC522, touch controller, SDRAM, and flash devices
  - `mfrc522.c` – MFRC522 RFID reader driver and card-tracking logic.
  - `stm32_lcd.c` – Platform-agnostic 2D graphics primitive drawing utilities.
  - `stm32h750b_discovery_lcd.c` – LTDC frame buffer and screen backlight PWM control.
  - `stm32h750b_discovery_ts.c` – FT5336 capacitive touch controller interface.
  - `stm32h750b_discovery_bus.c` – Low-level I2C4 bus communication layers.
  - `stm32h750b_discovery_sdram.c` – FMC external SDRAM initialization and MDMA framing code.
- `Drivers/STM32H7xx_HAL_Driver` - MCU HAL sources and headers
- `Middlewares/Third_Party/FreeRTOS` - realtime operating system support
- `Startup` - MCU startup code and linker configuration
- `Fonts` - display font assets for touchscreen UI
## Notes

- The project is configured for the STM32H750XBHX device variant.
- Build and flash tooling are managed through the generated STM32CubeIDE/Makefile environment.
- The touchscreen and LCD drivers are implemented in `stm32_lcd.c` and `touchscreen.c`.
- RFID logic is handled in `mfrc522.c`.

## Usage

1. Connect the STM32H750B-DK board to the host PC via USB.
2. Ensure the MFRC522 reader and motion sensor are wired to the appropriate board interfaces.
3. Build and flash the firmware using the provided project files.
4. Use the touchscreen UI to monitor alarm state and manage access.

## References

- ST STM32H750 value line product page: https://www.st.com/en/microcontrollers-microprocessors/stm32h750xb.html
- MFRC522 RFID reader documentation
- STM32H7 HAL and FreeRTOS documentation
