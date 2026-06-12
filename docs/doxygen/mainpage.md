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

### Main Program Structure

### Motor and Servo Control

### Pressure Control

### Pitch Detection and Harmonization

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

The final system worked well at the subsystem level, but the main limitation
was the blower fan. The fan could not generate enough pressure to use the full
acoustic range of the slide whistle. A human player naturally changes breath
pressure to stay in the desired harmonic range across the full range of slide travel. But our automated whistle could only maintain sound in the middle portion of the slide travel. Because of this, the pitch-matching loop worked when the target note was playable on the automated whistle, but
could not fully match a user playing notes outside that range.

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

The biggest future improvement would be a higher-pressure blower or redesigned
airflow path so the automated whistle can reach the same notes as a human
player. With a wider usable acoustic range, the pitch-matching control loop
would be much more effective. MIDI support could also be revisited once this limitation is solved.

### Demonstration and Attachments

-Does the project move and behave properly, as shown in a recorded video?
-Do you have photographs of the completed board? 