# STM32 Environmental Data Logger

An embedded environmental monitoring and data logging system built around the STM32F429 microcontroller using Embedded C, STM32 HAL, and STM32CubeIDE.

The project is being developed incrementally, starting with sensor acquisition and live visualization, followed by persistent storage and FreeRTOS-based task management.

**Current status: Milestone 1 complete, Environmental Monitoring System (October 2026)**

The current firmware acquires environmental measurements from a Bosch BME280 sensor and displays temperature, relative humidity, and atmospheric pressure on a 128×64 SSD1315 OLED. Both devices share a single I2C bus.

**Persistent data logging and FreeRTOS integration are not yet implemented.**

<!-- Add a photo of the running system here, e.g.: ![Live readings on the OLED](docs/milestone1.jpg) -->
<img width="3024" height="4032" alt="IMG_1041" src="https://github.com/user-attachments/assets/df7e1fd9-2f81-4e93-a5fc-93a2ddab7e83" />


## Hardware

| Component | Function |
|---|---|
| STM32F429I-DISC1 | Main microcontroller development board |
| Bosch BME280 | Temperature, humidity, and pressure sensor |
| 128×64 SSD1315 OLED | Live environmental measurements |
| microSD module | Planned persistent storage |

### Wiring

| Signal | STM32F429 pin | BME280 | OLED |
|---|---|---|---|
| I2C3 SCL | PA8 | SCL | SCL |
| I2C3 SDA | PC9 | SDA | SDA |
| Power | 5V / GND | VCC / GND | VCC / GND |
| 7-bit I2C address | | 0x77 | 0x3C |

The I2C bus runs at 100 kHz.

## Milestone 1: Sensor Acquisition and OLED Integration

**Completed functionality:**

- Custom BME280 driver using STM32 HAL I2C transactions.
- BME280 sensor configuration, factory calibration extraction, and measurement compensation.
- Temperature, humidity, and atmospheric pressure acquisition.
- Custom SSD1315 OLED driver: controller initialization and text rendering using a 5×7 bitmap font.
- Live display of environmental measurements.
- Sensor communication and measurement-validity checks.
- Runtime sensor fault indication and recovery following disconnection and reconnection.

### Engineering Highlights

**Sensor calibration and compensation:** Implemented datasheet-defined calibration decoding and fixed-point compensation, including handling packed signed humidity coefficients.

**Display interface:** Implemented OLED text rendering directly through controller GDDRAM page addressing, with no display library.

**Fault handling:** Added raw-measurement validity checks, runtime fault indication, and restoration of required BME280 configuration after sensor reconnection.

## Verification

**Sensor power-cycle test:** With the firmware running, BME280 power was removed for approximately 2 seconds. Invalid measurements were rejected and the OLED displayed `ERR`. After reconnection, the driver restored the required sensor configuration and temperature, humidity, and pressure returned to valid readings without resetting the STM32.

**Physical-response test:** Temperature and humidity response were verified using physical stimulus. After sensor reconnection, warm breath caused humidity to rise again, confirming that humidity acquisition had been successfully restored.

## Development Roadmap

| Milestone | Status |
|---|---|
| BME280 acquisition and compensation | Complete |
| OLED integration and visualization | Complete |
| Runtime sensor fault handling and reconnection | Complete |
| microSD storage using SPI and FatFs | Planned |
| FreeRTOS scheduling and inter-task communication | Planned |
| Integrated system testing and documentation | Planned |

## Current Limitations

- Persistent microSD logging has not yet been implemented.
- FreeRTOS task scheduling, queues, and I2C synchronization have not yet been integrated; the current milestone uses a simple polling loop.
- Runtime BME280 disconnection/reconnection has been verified, but sensor failure during system startup has not yet been fully integrated and tested.

## Project Goal

The final system will periodically acquire environmental measurements, display the latest readings, and persist structured records to a microSD card. The firmware will incorporate error detection, recovery behavior, and verification of system operation.

This README will be expanded as additional milestones are completed.

## Repository Notes

Generated STM32 HAL/CMSIS driver files are excluded from this repository.

## Acknowledgments

5×7 font data adapted from the Adafruit GFX Library (BSD license). See `Core/Src/font5x7.c` for the full license notice.
