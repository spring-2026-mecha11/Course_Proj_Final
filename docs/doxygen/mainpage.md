# Auto Playing Slide Whistle

## Overview

### Project Summary

Our project consists of a smart slide whistle instrument where one whistle is
played by the user and a second whistle is controlled automatically. The
second whistle is designed to either play with the user in unison or harmonize
with them.

The automated whistle is played using a blower fan. A stepper motor actuates
the slide to control pitch, while a servo motor opens or covers the sound hole
so the whistle can start and stop making sound. The internal pressure is
measured using a pressure sensor and regulated using a feedback control loop.
In harmonization mode, an outer feedback loop compares the pitches measured
by the two microphones and uses the resulting error to adjust the
harmonization whistle's pitch with the stepper motor.

### Completed Assembly

@image html completed-assembly.png "Completed Auto Playing Slide Whistle assembly" width=900px

The completed assembly includes the two slide whistles, stepper-driven
carriage, blower and airflow system, servo mute, custom electronics, and
handheld frame.

## Hardware Design

### Mechanical Assembly

**Design Overview**

The fixture is designed to be held in the user's hand. The user holds the
fixture by the handle in their non-dominant hand, holds it up to their mouth,
and actuates the slide with their dominant hand. The automated whistle and fan
are offset to the left side of the main whistle so they do not interfere with
the user's playing. All heavy components are in line with the main handle so
that the fixture is comfortable to hold and does not create a moment that
torques the user's hand sideways.

The automated slide whistle is actuated using a stepper motor. The stepper
motor drives a belt connected to a carriage that slides along an aluminum
extrusion. A 3D-printed bracket connects the slide whistle handle to the
carriage.

@image html slide-mechanism.png "CAD view of the stepper-driven slide mechanism" width=850px

A PC centrifugal blower plays the automated slide whistle. A custom
3D-printed shroud connects the blower to the whistle. The geometry transitions
are smooth to limit turbulence and reduce head losses.

@image html blower-shroud.png "CAD view of the centrifugal blower and airflow shroud" width=750px

A servo drives a 3D-printed stopper that blocks the outlet of the slide
whistle. This allows for instantaneous pauses without fully stopping the
blower fan, which is impractical because of the fan's inertia.

@image html servo-stopper.png "CAD view of the servo-operated whistle stopper" width=750px

**Manufacturing**

All fasteners, slide whistles, actuators, and sensors were purchased. Most of
the frame components were 3D printed, and key structural members were made
from sheet metal. These sheet-metal components were waterjet cut and then bent
using a press brake. The outer casing and electronics enclosure were also 3D
printed, with a laser-cut acrylic top.

**Final CAD Render**

@image html final-cad-render.png "Final CAD render of the complete assembly" width=950px

### Electronics and Custom PCB

We built two custom PCBs for this project: a main control board and an audio
breakout board.

**Main Board**

@image html main-board-render.png "Rendered main control PCB" width=850px

The main board is powered by a 12 V battery. A switching regulator steps the
voltage down to 5 V, and a low-dropout regulator further reduces it to 3.3 V.
The board contains an STM32F411 microcontroller, which runs the firmware for
the project. The board also contains a TMC5240 stepper motor driver with power
traces sized for the expected motor current. Additional circuitry provides
interfaces for the pressure sensor, blower fan, and servo motor.

**Audio Breakout Board**

@image html audio-breakout-board.png "Audio breakout board" width=850px

The audio breakout board is responsible for capturing the audio signals from
both slide whistles and converting the analog microphone outputs into digital
audio data. A microphone is positioned next to the sound hole of each slide
whistle to capture its acoustic output with minimal noise interference. The
microphone signals are first amplified using MAX4466 preamplifiers to increase
the signal amplitude and improve the digitization.

The amplified audio signals are then sampled by a PCM1802 stereo
analog-to-digital converter. The PCM1802 converts the analog microphone signals
into 24-bit digital audio and transmits the data to the main control board using
the I2S communication protocol. In addition to the audio data, the cable
connecting the audio breakout board to the main board carries the 5 V and 3.3 V
power rails, ground, and the clock signals required by the PCM1802 for
synchronized data transfer. The STM32F411 on the main board receives the
digital audio stream and processes it in real time to determine the pitch
produced by each slide whistle.

### Actuators and Sensors

The system uses an SY42STH38-1684A stepper motor controlled by the TMC5240
motor driver. The stepper motor positions the slide of the automated whistle.
It homes to one side using a limit switch, and the upper end of travel is set
by counting the number of steps.

An FS90 servo from Pololu is controlled by a PWM output from the
microcontroller. The servo opens and closes the sound hole to control when the
whistle produces sound.

The blower fan is also controlled using PWM. Its control line is normally
pulled high and is switched to ground through an N-channel MOSFET. The MOSFET
gate is connected to a PWM output on the microcontroller.

An MP3V5004G differential pressure sensor measures the system pressure. The
microcontroller uses this measurement as feedback in a control loop to adjust
the blower fan and maintain a constant pressure setpoint. During calibration,
20 pressure-sensor readings are taken over a short period and averaged. The
pressure-sensor readings are somewhat noisy, so the firmware filters the
measurements.

## Software and Control

## 1. System-Level Software Architecture

The software for this project is organized around a task-state architecture. The STM32 hardware peripherals are configured using STM32CubeMX, which generates the initialization code for GPIO, timers, ADC, SPI, I2S, DMA, USB, and system clocks. After the CubeMX-generated hardware initialization is complete, the project transitions into the custom application layer.

The custom application layer begins with `App_Init()`, which initializes the high-level software modules used by the system. These include mode selection, pressure control, servo control, stepper control, audio input processing, the state machine, and the live harmonizer logic. After initialization, the program repeatedly calls `App_Task()` from the main loop. `App_Task()` acts as the main scheduler for the application-level tasks, allowing each subsystem to update without placing low-level hardware details directly inside `main.c`.

This structure keeps the project modular and easier to debug. CubeMX is responsible for configuring the microcontroller hardware, while the custom application code is responsible for system behavior and coordination between subsystems.

## 2. Task-State Main Architecture

The central control structure of the project is the state machine. The state machine determines what the overall system should be doing at any given time and coordinates the different software modules accordingly.

The major system states are:

```text
BOOT
INIT_HARDWARE
HOME_STEPPER
MODE_SELECT
LIVE_HARMONIZER
SONG_PLAYER
ERROR
```

At startup, the system enters a safe initialization sequence. The servo mute is closed, the fan is disabled, and the live harmonizer and song player paths are disabled. After initialization, the stepper motor is homed so the system has a known mechanical zero position.

Once homing is complete, the system enters mode selection. A button-based interface is used to choose the operating mode. A single click selects live harmonizer mode, while a double click selects song player mode.

In live harmonizer mode, the system listens to the audio input, controls the servo mute based on the reference audio signal, and adjusts the stepper position based on pitch error. The state machine also handles mode exit behavior by muting the servo, stopping the stepper, disabling the fan, and returning to mode selection.

The song player mode has a pathway created for future implementation. The state machine can enter song player mode, set the appropriate system flags, and provide a structure for enabling song playback logic in the future. However, as the software currently stands, the song player is essentially an empty placeholder module. It does not yet contain the final note sequencing, stepper movement, or servo timing logic needed for autonomous song playback.

The error state provides a safe fallback condition. If a fault occurs, the system disables active modules, mutes the servo, stops the stepper, turns off the fan, and prevents continued operation until the issue is addressed.

## 3. Stepper Motor Driver Implementation

The stepper motor is controlled through a TMC5240 stepper driver. The low-level TMC driver handles SPI communication with the driver chip, while the `Stepper_Motion` module provides higher-level movement commands for the rest of the system.

The stepper software supports homing, absolute movement, percent-based movement, stopping, and position feedback. The most important functions include:

```c
Stepper_Init();
Stepper_Home();
Stepper_MoveToPercent(float percent);
Stepper_MoveToAbsSteps(int32_t target_steps);
Stepper_Stop();
Stepper_GetPositionPercent();
Stepper_GetPositionSteps();
```

The stepper must be homed before normal operation. Homing establishes a known zero position using the mechanical limit switch. Once homed, the software treats the full slide travel range as a 0 to 100 percent scale. This allows the live harmonizer and future song player code to command slide position using percent travel instead of raw step counts.

The calibrated travel range is:

```c
#define STEPPER_RANGE_STEPS 270000
```

This means the software maps percent commands to stepper position approximately as:

```text
0%   -> 0 steps
50%  -> 135000 steps
100% -> 270000 steps
```

The stepper module also uses different motion profiles depending on the type of movement. A safer, slower profile is used for homing and certain direct movement commands. A faster profile is used for percent-based movement, which is important for live harmonizer operation because the slide position must respond quickly to pitch corrections.

Separate current settings are also used for different motion types. Percent-based movement uses a stronger run-current setting to reduce missed steps during active control, while safer current settings are used during other operating conditions.

## 4. Stepper Calibration Process

Stepper calibration was required to determine the safe mechanical travel range of the slide whistle mechanism. The first step was to establish a reliable zero position through the homing routine. After the stepper could consistently home, a temporary debugger-based calibration method was added.

This calibration method allowed absolute step positions to be commanded manually from the debugger. The target step count was gradually increased while observing the physical slide mechanism. This made it possible to find the maximum safe extension without overdriving the mechanism.

After testing, the maximum safe travel was determined to be approximately 270,000 steps. This value was then written into the software as the travel range limit.

```c
#define STEPPER_RANGE_STEPS 270000
```

Once this value was established, all percent-based movement commands could be safely mapped within the calibrated range. The software clamps target positions so that commands cannot exceed the minimum or maximum step limits.

The calibration process was important because the slide whistle mechanism has physical travel limits that must be respected. Without this calibration, the stepper could be commanded beyond the safe mechanical range. By calibrating the step range and converting motion to percentages, later control code became easier to write, safer to test, and simpler to tune.

## 5. Servo Mute Driver Implementation

The servo is used as a mute mechanism for the slide whistle. It is controlled with a PWM output generated by one of the STM32 timer channels. The servo driver converts desired angular positions into PWM pulse widths.

The servo module provides simple high-level commands:

```c
ServoSystem_Up();
ServoSystem_Down();
ServoSystem_SetPosition(float angle_degrees);
```

The servo driver uses a center pulse of approximately 1500 microseconds and converts angle to pulse width using a scale factor. The commanded angle is clamped between minimum and maximum limits so the servo is not driven beyond its intended range.

During startup, the servo is commanded to the down or muted position before PWM is started. This ensures the system begins in a safe muted state.

At the state-machine level, the servo is controlled through mute requests rather than direct angle commands. The state machine uses:

```c
Servo_SetMuted_Request(true);
Servo_SetMuted_Request(false);
```

A muted request closes the servo mute, while an unmuted request opens it.

In live harmonizer mode, the servo opens only when the reference audio signal is valid. If the reference signal disappears, the servo closes after a short delay. This delay helps prevent the servo from bouncing open and closed due to momentary audio dropouts or pitch detection flicker.
## 6. Fan Pressure Control Module

The fan pressure control module regulates the airflow supplied to the slide whistle. It reads the pressure sensor, compares the measured pressure to a target pressure, and adjusts the fan PWM output using a PI control loop.

During startup, the pressure system performs a calibration sequence to establish a zero-pressure reference. The raw ADC readings from the pressure sensor are averaged during this calibration period to determine the baseline sensor offset. After calibration, new ADC readings are converted into pressure values relative to this zero reference. This helps remove sensor bias and improves consistency between runs.

The measured pressure signal is also filtered before being used by the controller. This reduces noise from the pressure sensor and prevents small measurement fluctuations from causing unnecessary fan speed changes. The filtered pressure value is then compared with the target pressure to calculate pressure error.

During normal operation, the fan is enabled only after an operating mode is selected. Live harmonizer mode uses the live-mode pressure target, while song player mode includes a pathway for a separate song-mode pressure target in the future.

The general control behavior is:

```text id="ebr4m9"
pressure error = target pressure - measured pressure
fan command = proportional response + integral response
```

The module also reports whether the pressure is stable, allowing the system to determine whether airflow has reached the desired operating condition. This module helps provide consistent airflow so the whistle can produce a more stable tone during operation.
## 7. Audio Signal Processing
The audio processing code is written in C and runs on the STM32F411 microcontroller. The STM32 receives digital audio from the PCM1802 using the I2S peripheral with DMA. Using DMA allows audio samples to be collected continuously in the background while the main loop handles pitch detection and control logic. The DMA buffer is split into half complete and full complete sections. When either callback occurs, the corresponding flag is set, and the main loop processes that half of the buffer.
The PCM1802 outputs 24-bit I2S audio, but the pitch detection code only uses the upper 16 bits of each sample. The stereo audio stream is separated into left and right sample arrays. The left microphone (next to the user slide whistle) is used as the target pitch input, and the right microphone (next to the automated slide whistle) is used as the measured slide whistle pitch.
Pitch detection is performed using the YIN algorithm. Before pitch detection, the code removes the DC offset from the sample window and calculates the signal energy. This energy value is used to reject quiet or noisy signals so that the system does not respond when no valid tone is present. Separate energy thresholds are used for the left and right microphones because each microphone can have different gain and background noise levels.
The main loop waits for DMA flags, reconstructs the audio samples, updates the pitch detectors, and calculates the pitch error. This error tells the controller whether the whistle pitch is too high or too low compared to the target. The pitch data is then converted into a desired stepper motor position. The motor is commanded as a percentage of its total travel range, where 0% corresponds to the slide position that produces the highest note and 100% corresponds to the lowest note.
Audio processing is separated into functions for sample reconstruction, energy calculation, pitch detection, and block processing. The DMA callbacks are kept short and only set flags, while the heavier processing is done in the main loop.


## 8. Audio-Based Stepper Motion

The live harmonizer is the part of the software that connects the audio input system to the stepper motion system. Its purpose is to adjust the slide position until the measured whistle pitch matches the reference pitch.

The reference microphone determines whether the system should be actively unmuted. If the reference pitch is valid, the servo mute opens. If the reference pitch is not valid, the servo closes and the stepper is stopped or held safely.

When both the reference pitch and measured pitch are valid, the software calculates pitch error:

```text
pitch error = target pitch - measured pitch
```

The live harmonizer then uses a PI-style control loop to convert this pitch error into a stepper position correction. The proportional term responds to the current pitch error, while the integral term accumulates error over time to help reduce steady-state offset.

The controller uses debugger-adjustable gains:

```c
live_harmonizer_kp
live_harmonizer_ki
```

The calculated control output is applied as a percentage-based stepper target. The target is clamped to the calibrated travel range so the stepper cannot command the slide outside its safe mechanical limits.

Important live harmonizer debug variables include:

```c
debug_live_pitch_error_hz
debug_live_pitch_integral
debug_live_nudge_percent
debug_live_stepper_target_percent
debug_live_control_active
debug_live_stepper_status
```

These variables make it possible to tune the controller while watching the actual pitch error, commanded correction, and resulting stepper target.

## 9. Song Player Pathway

A song player pathway was included in the state machine to support future expansion of the project. The system can enter a song player state, set song player flags, and call placeholder song player request functions.

The intended future purpose of song player mode is to automatically command the stepper and servo through a predefined sequence of notes and rests. In that final version, the song player would likely use calibrated stepper percent positions for note pitches and servo mute commands for rests or note separation.

However, in the current implementation, the song player does not yet contain real playback logic. It is essentially an empty module with the surrounding state-machine framework already prepared. This makes it easier to add song playback later without restructuring the rest of the system.

## 10. Debugging and Testing Support

Debugger-accessible variables were added throughout the software to make hardware bring-up and tuning easier. This allows system behavior to be observed directly in STM32CubeIDE without relying on UART print statements.

Examples include:

```c
debug_system_state
debug_active_mode
debug_stepper_homed
stepper_debug_position
stepper_debug_percent
debug_audio_target_pitch_hz
debug_audio_measured_pitch_hz
debug_audio_pitch_error_hz
debug_live_stepper_target_percent
```

These variables were used to verify mode selection, pressure/fan behavior, servo mute response, I2S audio input, pitch detection, stepper homing, percent-based stepper motion, and live harmonizer control behavior.

This debugging approach was especially useful because multiple hardware systems needed to interact correctly. Being able to watch the system state, audio validity, pitch values, and stepper target position helped isolate problems during development.

## 11. Overall Implementation Summary

Overall, the software is structured as a modular embedded control system. STM32CubeMX handles the low-level microcontroller hardware setup, while the custom application layer uses `App_Init()` and `App_Task()` to initialize and repeatedly update the project’s software modules.

The state machine coordinates the high-level behavior of the system, including startup, homing, mode selection, live harmonizer operation, future song player operation, and error handling.

The stepper driver provides calibrated position control of the slide mechanism. The servo driver provides a simple mute mechanism. The audio system provides real-time pitch detection from two microphone channels. The live harmonizer combines these systems by using audio pitch error to command stepper motion while using the reference audio signal to control the servo mute.

The song player mode is currently only a prepared pathway for future development. The structure is present, but the playback logic has not yet been implemented.

The final result is a software framework that supports safe startup, calibrated stepper motion, servo mute control, real-time audio processing, and future expansion into automated song playback.





## Conclusion

This project successfully integrated custom electronics, mechanical actuation,
sensing, and feedback control into a final functional prototype. This section
evaluates the system's performance, discusses the major challenges encountered
during development, and identifies opportunities for future improvement.

### Performance and Functionality

We completed most of the core functionality that we set out to build. The
stepper motor reliably homes and actuates the slide whistle across its full
mechanical range, so the system can repeatably control the slide position. The
servo blocks and unblocks the whistle outlet, which lets the automated whistle
start and stop making sound without fully stopping the blower. The microphones
can detect pitch, and the pressure sensor gives the firmware usable feedback
from inside the airflow path.

The pressure PI loop also worked. Using pressure-sensor feedback, the firmware
can adjust the blower fan to maintain a more consistent pressure than open-loop
fan control.

The original idea included MIDI support with note to location mapping, but we
cut that scope so we could focus on the mimicry feature. MIDI streaming would
have required computer-side software, MIDI parsing, and a mapping from notes to
stepper positions. For a three-person team, the remaining scope was still
appropriately ambitious because the project included two custom PCBs, three
actuators, three primary sensors, two control loops, and mechanical design work.

The final system worked well at the subsystem level, but the main limitations
where the blower fan and noise isolation between the two microphones. The fan could not generate enough pressure to use the full acoustic range of the slide whistle. A human player naturally changes breath pressure to stay in the desired harmonic range across the full range of slide travel. But our automated whistle could only maintain sound in the middle portion of the slide travel. Because of this, the pitch-matching loop worked when the target note was playable on the automated whistle, but could not fully match a user playing notes outside that range. In addition, because the two microphones were so close together, the sound they heard was not isolated to their individual slide whistle. This cross talk between microphones made the error values we were reading unreliable, furtur reducing the efficacy of our control loop.

### Challenges and Workarounds

One major electrical issue happened during early board testing. We plugged in
the fan while the board was powered, and the STM32 failed. The board started
drawing excessive current, but the cause was not obvious at first. We diagnosed
the problem by applying current-limited power and checking which components
heated up. The STM32 became very hot, and after we removed it from the board,
the short disappeared. This showed that the microcontroller had failed rather
than there being a permanent short elsewhere on the PCB.

The most likely cause was the fan being hot-plugged, which created a large
inrush current that damaged the STM32. We replaced the microcontroller, plugged
in the fan before the board was powered, and the board worked correctly
afterward.

Another debugging challenge came from the microphone data. The microphones were
working, but the UART transmission was clipping the values, which made the
problem look like a sensor issue at first. After isolating the issue to the
communication path, we fixed the data transport so the pitch readings could be
used correctly.

The biggest two future improvements would be a higher-pressure blower or redesigned
airflow path so the automated whistle can reach the same notes as a human
player in addition to proper noise isolation between the two microphones. With a wider usable acoustic range and an accurate pitch error, the pitch-matching control loop would be much more effective. MIDI support could also be revisited once the note range limitation is solved.

### Demonstration and Attachments

-Does the project move and behave properly, as shown in a recorded video?
-Do you have photographs of the completed board? 
