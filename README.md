# Embedded Lab Firmware - STM32F407 Battery Balancer

This project contains the firmware for an Embedded Lab battery balancing system using the **STM32F407G-DISC1** development board.

The final goal of the project is to implement a **Single Switched Capacitor battery balancing circuit**, where the STM32 monitors battery cell voltages and controls the switching sequence needed to transfer charge between cells.

At this stage, the project is focused on the basic firmware bring-up:

- STM32F407G-DISC1 project generation using STM32CubeMX / STM32CubeIDE
- GPIO testing using the onboard LEDs
- ADC initialization
- First ADC reading module for battery voltage monitoring

---

## Hardware Platform

- Development board: **STM32F407G-DISC1**
- Microcontroller: **STM32F407VGTx**
- IDE: **STM32CubeIDE**
- Configuration tool: **STM32CubeMX**
- Firmware package: **STM32Cube FW_F4**

---

## Project Goal

The objective is to develop firmware for a battery balancing system based on a **C2C circuit**.

The system will eventually:

1. Measure battery cell voltages using ADC inputs.
2. Compare the voltage difference between cells.
3. Detect when balancing is required.
4. Control switches or relays to connect a capacitor between cells.
5. Transfer charge from a higher-voltage cell to a lower-voltage cell.

---

## Current Development Status

Current implemented features:

- STM32CubeMX project generation completed.
- STM32CubeIDE project compiling correctly.
- ADC1 initialized.
- Initial `battery_monitor` module planned/created for ADC reading.

Current test objective:

```text
ADC input → raw ADC value → converted voltage → simple firmware decision




---

## Project Structure
EmbeddedLabFirmware
├── Inc
│   ├── main.h
│   ├── stm32f4xx_hal_conf.h
│   ├── stm32f4xx_it.h
│   ├── battery_monitor.h
│   ├── balancer.h
│   └── app_config.h
│
├── Src
│   ├── main.c
│   ├── stm32f4xx_hal_msp.c
│   ├── stm32f4xx_it.c
│   ├── system_stm32f4xx.c
│   ├── battery_monitor.c
│   ├── balancer.c
│
├── Drivers
│   ├── CMSIS
│   └── STM32F4xx_HAL_Driver
│
├── Middlewares
│
├── USB_HOST
│
├── Startup
│
├── EmbeddedLabFirmware.ioc
└── README.md
