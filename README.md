# Embedded Lab Firmware — STM32F407 Battery Balancer

Firmware for a two-cell Li-ion battery-balancing prototype developed with the **STM32F407G-DISC1** board. The application measures or simulates individual cell voltages, decides whether balancing is necessary, and controls PWM outputs intended to drive the external power-electronics stage.

> **Current state:** the balancing decision and PWM modules are implemented, but the controller currently uses fixed mock voltages instead of the ADC measurements. The firmware is therefore suitable for logic and PWM testing, but it is not yet ready to control a real battery pack without further validation and safety work.

## Project purpose

The firmware is part of an active cell-balancing system. Its intended function is to:

1. Measure the voltages of two series-connected cells.
2. Calculate the voltage difference between the cells.
3. Enable balancing when the difference exceeds a configured threshold.
4. Generate the PWM signals required by the balancing and converter hardware.
5. Stop the switching signals when balancing is unnecessary or the voltage readings are invalid.

The present implementation is a first two-cell prototype. It does not yet determine the charge-transfer direction or implement a complete multi-cell balancing strategy.

## Hardware and software platform

| Item | Configuration |
|---|---|
| Development board | STM32F407G-DISC1 |
| Microcontroller | STM32F407VGT6 / Cortex-M4F |
| System clock | 168 MHz |
| IDE | STM32CubeIDE |
| Configuration tool | STM32CubeMX |
| STM32 firmware package | STM32Cube FW_F4 V1.28.3 |
| Programming/debug interface | On-board ST-LINK using SWD |

## How the firmware works

The application is organized into three main modules:

```text
main.c
  └── BalancerController_Update() every 100 ms
        ├── Obtain cell voltages
        │     ├── Current mode: fixed mock values
        │     └── Available mode: ADC battery monitor
        ├── Calculate |Vcell1 - Vcell2|
        ├── Compare the result with the balancing threshold
        └── Start/stop and update the PWM outputs
```

### 1. Initialization

After the HAL and system clock are initialized, `main()` configures the generated peripherals and then initializes the two application modules:

```c
BatteryMonitor_Init(&hadc1);
BalancerController_Init();
```

`BalancerController_Init()` clears the controller status and stops the PWM outputs. The main loop then calls `BalancerController_Update()` once every 100 ms.

The generated USB Host process is also called in the loop, but USB is not currently part of the balancing algorithm.

### 2. Voltage acquisition

The `battery_monitor` module supports two ADC measurements for a two-cell series pack:

- **Node 1:** midpoint between cell 1 and cell 2
- **Node 2:** top of the complete two-cell pack

The ADC channels are:

| Measurement | MCU pin | ADC channel |
|---|---|---|
| Node 1 | PA1 | ADC1_IN1 |
| Node 2 | PA2 | ADC1_IN2 |

ADC values are converted as follows:

```text
ADC pin voltage = raw ADC value × 3.3 V / 4095
Node voltage    = ADC pin voltage × divider gain
Cell 1 voltage  = Node 1 voltage
Cell 2 voltage  = Node 2 voltage - Node 1 voltage
```

The current divider gains are placeholders:

```c
NODE1_DIVIDER_GAIN = 2.0
NODE2_DIVIDER_GAIN = 4.0
```

They must be changed to match the real resistor-divider values before connecting the sensing circuit.

A reading is marked valid only when both calculated cell voltages are between **2.5 V and 4.25 V**. The ADC is configured for 12-bit, software-triggered, single conversions. Each channel is selected at runtime and sampled with `ADC_SAMPLETIME_144CYCLES`.

### 3. Current mock-voltage mode

The controller currently executes:

```c
BalancerController_ReadMockVoltages(&cell1, &cell2);
```

with these fixed values:

```c
Cell 1 = 4.10 V
Cell 2 = 3.45 V
```

The resulting difference is **0.65 V**, which is greater than the configured **0.50 V** threshold. Consequently, the current firmware enables balancing every time the controller runs.

To use the real ADC measurements, change the voltage-source selection in `Src/balancer_controller.c`:

```c
// BalancerController_ReadMockVoltages(&cell1, &cell2);
BalancerController_ReadBatteryVoltages(&cell1, &cell2);
```

The voltage dividers and ADC readings must be validated before doing this on real cells.

### 4. Balancing decision

The controller calculates:

```text
voltage_difference = |cell1_voltage - cell2_voltage|
```

Balancing is enabled only when:

```text
voltage_difference > 0.50 V
```

When balancing is enabled, the PWM module is started and the red on-board LED **LD5 (PD14)** is turned on. When balancing is disabled, both configured duties are set to zero, PWM is stopped, and LD5 is turned off.

The current algorithm uses only the absolute difference. It does **not** yet identify which cell is higher or select a charge-transfer direction.

### 5. PWM generation

The current `pwm_control` module uses TIM1 and TIM4.

| Signal | Timer output | Pin | Frequency | Duty while active |
|---|---|---|---:|---:|
| Main balancing PWM | TIM1_CH1 | PE9 | approximately 22.5 kHz | 60% |
| Complementary PWM | TIM1_CH1N | PE8 | approximately 22.5 kHz | complementary hardware output |
| Secondary/boost PWM | TIM4_CH1 | PD12 | 50 kHz | 40% |

The frequencies are obtained from the timer configuration:

```text
TIM1: 168 MHz / (7466 + 1) ≈ 22.5 kHz
TIM4:  84 MHz / (1679 + 1) = 50 kHz
```

TIM1 is configured as an advanced-control timer and provides both CH1 and CH1N. The configured dead time is currently **zero**, so external gate-driver requirements and shoot-through protection must be addressed before driving a MOSFET half bridge.

TIM3 is also generated by CubeMX on PC6, but it is not used by the current `pwm_control` module.

## Main application status

The controller exposes its latest state through `BalancerController_GetStatus()`:

```c
typedef struct
{
    float cell1_voltage;
    float cell2_voltage;
    float voltage_difference;
    uint8_t balancing_active;
} BalancerStatus_t;
```

This structure can be watched in the STM32CubeIDE debugger to verify the input voltages, calculated difference, and balancing state.

The battery monitor similarly exposes raw ADC values, ADC-pin voltages, reconstructed node voltages, calculated cell voltages, and a validity flag through `BatteryMonitor_GetReadings()`.

## Project structure

```text
EmbeddedLabFirmware/
├── Inc/
│   ├── balancer_controller.h     # Balancing state and controller interface
│   ├── battery_monitor.h         # ADC measurement data and interface
│   ├── pwm/
│   │   └── pwm_control.h         # PWM start, stop, and duty interface
│   ├── main.h                    # Generated pin definitions and HAL declarations
│   └── ...                       # STM32 HAL and USB headers
├── Src/
│   ├── main.c                    # Initialization and 100 ms application loop
│   ├── balancer_controller.c     # Threshold decision and balancing state
│   ├── battery_monitor.c         # ADC acquisition and voltage reconstruction
│   ├── pwm/
│   │   └── pwm_control.c         # TIM1/TIM4 PWM control
│   └── ...                       # Generated STM32 HAL and USB source files
├── Drivers/                      # CMSIS and STM32F4 HAL drivers
├── Middlewares/                  # STM32 USB Host middleware
├── Startup/                      # STM32F407 startup assembly
├── EmbeddedLabFirmware.ioc       # STM32CubeMX hardware configuration
├── STM32F407VGTX_FLASH.ld        # Flash linker script
├── STM32F407VGTX_RAM.ld          # RAM linker script
└── README.md
```

## Build and flash

### STM32CubeIDE

1. Open STM32CubeIDE.
2. Select **File → Import → Existing Projects into Workspace**.
3. Select the `EmbeddedLabFirmware` directory.
4. Open `EmbeddedLabFirmware.ioc` and confirm the configured board/peripherals.
5. Select **Project → Clean** and then **Project → Build Project**.
6. Connect the STM32F407G-DISC1 through its ST-LINK USB connector.
7. Start a Debug or Run configuration to flash the firmware.

The checked-in `Debug/makefile` was generated with a machine-specific Windows linker-script path. A command-line build on a different computer may therefore fail until STM32CubeIDE regenerates the Debug build files or the linker-script path is corrected.

## Testing the current implementation

### Logic test with mock values

1. Keep `BalancerController_ReadMockVoltages()` enabled.
2. Build and flash the project.
3. Confirm that LD5 turns on.
4. Measure the timer outputs with an oscilloscope:
   - PE9: TIM1_CH1, approximately 22.5 kHz
   - PE8: TIM1_CH1N, complementary output
   - PD12: TIM4_CH1, 50 kHz
5. Watch `balancer_status` in the debugger. The expected values are approximately:

```text
cell1_voltage       = 4.10 V
cell2_voltage       = 3.45 V
voltage_difference  = 0.65 V
balancing_active    = 1
```

### Threshold-off test

Change the mock voltages so that their difference is at most 0.50 V. After flashing, LD5 should remain off and the PWM duties should be zero.

### ADC test

Before connecting a real series battery pack, validate each ADC channel with safe laboratory voltage sources and confirm:

- The voltage at PA1 and PA2 never exceeds the MCU ADC range.
- The divider gains match the installed resistors.
- Node 1 and Node 2 share the correct measurement reference.
- `cell2_voltage = node2_voltage - node1_voltage` produces the expected result.
- Invalid readings stop balancing.

## Configuration points

The main behavior can currently be adjusted in these files:

| Setting | File | Current value |
|---|---|---:|
| Mock cell voltages | `Src/balancer_controller.c` | 4.10 V / 3.45 V |
| Balancing threshold | `Src/balancer_controller.c` | 0.50 V |
| TIM1 duty | `Src/balancer_controller.c` | 60% |
| TIM4 duty | `Src/balancer_controller.c` | 40% |
| Node 1 divider gain | `Src/battery_monitor.c` | 2.0 |
| Node 2 divider gain | `Src/battery_monitor.c` | 4.0 |
| Valid cell range | `Src/battery_monitor.c` | 2.5–4.25 V |
| Controller period | `Src/main.c` | 100 ms |

## Current limitations and next steps

The following work remains before the firmware can be considered hardware-ready:

- Replace the manually selected mock/ADC source with a clear build-time or runtime configuration.
- Calibrate and verify the real voltage-divider gains and ADC reference.
- Add ADC filtering, averaging, and measurement-fault diagnostics.
- Determine which cell is higher and implement the correct transfer direction.
- Add hysteresis around the balancing threshold to prevent rapid on/off switching.
- Add per-cell overvoltage, undervoltage, temperature, and current protection.
- Configure and validate suitable dead time for the actual MOSFETs and gate driver.
- Verify the synchronization and phase relationship required by the power stage.
- Explicitly verify or add stopping of the TIM1 complementary output; `PWM_Start()` starts CH1N with `HAL_TIMEx_PWMN_Start()`, while the current `PWM_Stop()` does not call `HAL_TIMEx_PWMN_Stop()`.
- Check all HAL return values in the PWM module and enter a safe state on errors.
- Extend the two-cell controller to the intended multi-cell topology.
- Remove or disable unused generated peripherals if USB, I2C, I2S, SPI, or TIM3 are not needed.

## Safety notice

Li-ion cells can deliver high fault currents and may be damaged by overvoltage, undervoltage, excessive current, incorrect switching, or wiring errors. Test the ADC and PWM signals separately before connecting the power stage. Use current-limited laboratory supplies during bring-up, verify gate-driver grounding and isolation, and do not connect a real battery pack until the sensing, shutdown behavior, dead time, and protection circuits have been validated.

## Developers

- Roger Castro
- Gautam Gupta
- Ankita Kamat
