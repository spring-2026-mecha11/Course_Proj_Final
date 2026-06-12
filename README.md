# Auto Playing Slide Whistle

**A smart slide whistle system that listens to a user-played whistle and drives a second whistle to play along in unison or harmony.**

**Authors:** Daniel Norris, Jonah Prichard, and Max Schecter

## Overview

This project combines a handheld mechanical slide-whistle assembly, custom electronics, and STM32 firmware to create an automatically played companion whistle. The user plays one slide whistle manually while a second whistle is driven by a blower fan, stepper-actuated slide, and servo mute. The firmware can track microphone pitch input and adjust the automated whistle in real time.

Full project documentation, CAD images, hardware details, firmware module references, and Doxygen-generated source documentation are available at the project site:

**[https://spring-2026-mecha11.github.io/Course_Proj_Final/](https://spring-2026-mecha11.github.io/Course_Proj_Final/)**

## Hardware

- **Controller:** STM32F411 microcontroller configured with STM32CubeMX
- **Slide actuation:** SY42STH38-1684A stepper motor driven by a TMC5240 motor driver
- **Airflow:** PWM-controlled PC centrifugal blower and custom 3D-printed shroud
- **Mute mechanism:** FS90 servo driving a stopper over the whistle outlet
- **Pressure feedback:** MP3V5004G differential pressure sensor for blower control
- **Audio input:** Two microphones with MAX4466 preamplifiers and a PCM1802 stereo ADC over I2S
- **Mechanical system:** 3D-printed brackets, sheet-metal frame parts, aluminum extrusion, belt-driven carriage, and handheld enclosure

## Firmware

The firmware is organized as a CubeMX-generated STM32 project with team-written application modules under `Core/Inc` and `Core/Src`. CubeMX initializes peripherals such as GPIO, timers, ADC, SPI, I2S, DMA, USB, and clocks. After hardware setup, `App_Init()` initializes the custom modules and `App_Task()` runs the cooperative application loop.

Major software components include:

- **State machine:** Coordinates boot, hardware initialization, stepper homing, mode selection, live harmonizer mode, song-player placeholder mode, and safe error handling.
- **Stepper motion:** Homes the slide with a limit switch, enforces calibrated travel limits, and exposes absolute, relative, and percent-based motion commands.
- **Pressure system:** Calibrates the pressure sensor, filters pressure readings, and runs a PI loop to control blower fan duty.
- **Servo system:** Converts mute and position requests into PWM commands for the outlet stopper.
- **Audio system:** Receives stereo I2S audio from the PCM1802 and routes sample blocks to pitch detection.
- **Pitch processing:** Uses a YIN-style detector over rolling sample windows to estimate whistle pitch.
- **Live harmonizer:** Compares user and automated whistle pitch and adjusts the stepper target using an outer PI pitch-control loop.

## Operating Modes

The system starts in a safe muted state, initializes hardware modules, and homes the stepper before normal use. After homing, a button-based mode selector chooses between:

- **Live harmonizer:** The active implementation. The system listens to both whistles, controls the servo mute from audio activity, and moves the automated slide based on pitch error.
- **Song player:** A planned pathway for future autonomous note sequencing. The state-machine structure exists, but final note timing, slide motion, and servo sequencing are not implemented.

## Repository Structure

```text
Course_Proj_Final/
├── Core/
│   ├── Inc/                  # Application headers and STM32 HAL configuration
│   └── Src/                  # Application modules and CubeMX source files
├── Drivers/                  # STM32 HAL and CMSIS support files
├── Middlewares/              # STM32 USB device middleware
├── USB_DEVICE/               # USB CDC device configuration
├── docs/
│   └── doxygen/              # Doxygen main page, images, theme files, and layout
├── Course_Proj_Final.ioc     # STM32CubeMX project configuration
├── Doxyfile                  # Doxygen configuration
└── README.md
```

## Documentation

The published documentation site is hosted with GitHub Pages:

[https://spring-2026-mecha11.github.io/Course_Proj_Final/](https://spring-2026-mecha11.github.io/Course_Proj_Final/)

To regenerate the local Doxygen output from the repository root:

```sh
doxygen Doxyfile
```

The Doxygen configuration uses `docs/doxygen/mainpage.md` as the main page and documents the team-written firmware in `Core/Inc` and `Core/Src`.
