# Open-5012h - Open Source Firmware for the FNIRSI-5012H Oscilloscope

This is an open source firmware for the FNIRSI-5012H oscilloscope.

The project is in the Alpha stage. The firmware is usable, but was not well tested.
Use at your own risk.

There are two models of the FNIRSI-5012H PCB. The first revision has 4 mechanical
relays. The second revision has only one mechanical relay. This project is created
for the more recent board with a single relay. I'm not sure how different the boards
actually are, I don't have the old hardware.

FNIRSI-5012H hardware is described in details [here](doc/Hardware.md).

## Installation

The MCU (GigaDevice GD32F407VET6) is locked and can't be erased or reused.
Swapping the chip is the only way to run this firmware.

New GD32F407VET6 are available for $5 from LCSC.

## Programming

Programming pins are available on the USB connector. There are no other dedicated
test points or connectors on the board, so making an adapter is the best way to go.

Details of the programmer connections are [here](doc/Programming.md).

## Keyboard

Some buttons have multiple functions assigned to them. **1X10X** button is used
as a **SHIFT** and marked as such in the text below. 1X/10X state can be selected
through the menu.

In the oscilloscope mode the buttons have the following functions:

| Button | Function |
|:---:|:---|
| **AC/DC** | Select AC or DC Coupling |
| **STOP** | Start, Stop or Retrigger Capture |
| **EDGE** | Select Trigger Edge |
| **TRIG** | Select Trigger Mode (Normal, Auto, Single) |
| **TRIG UP** / **TRIG DOWN** | Change Trigger Level |
| **SHIFT** + **TRIG UP** / **TRIG DOWN** | Change Sample Rate Limit |
| **UP** / **DOWN** | Change Vertical Position |
| **SHIFT** + **UP** / **DOWN** | Change Vertical Scale |
| **UP** + **DOWN** | Set Vertical Position to 0 |
| **LEFT** / **RIGHT** | Change Horizontal Position |
| **SHIFT** + **LEFT** / **RIGHT** | Change Horizontal Scale |
| **LEFT** + **RIGHT** | Set Horizontal Position to 0 |

## Calibration

Your hardware will require calibration. The default calibration values are
the values from my development device, so they will definitely not work for
any other device. 

The calibration mode is entered by holding **SHIFT** + **MODE** while powering on
the device,

The calibration should be performed with a fully charged battery and
the charger disconnected. It is better to have LCD backlight level set to 100%.

The calibration should be performed with a direct connection to the device or
a probe in 1X mode.

In the calibration mode trigger controls are fixed to the auto mode with
the rising edge and the level set to 0.

The trigger control buttons change their functions:

| Button | Function |
|:---:|:---|
| **TRIG UP** / **TRIG DOWN** | Change Calibration Value |
| **SHIFT** + **TRIG UP** / **TRIG DOWN** | Change Calibration Parameter |

In the calibration mode trigger parameters in the status line are replaced
with the calibration data.

There are 4 calibration parameters:
1. Zero -- Zero offset
2. Delta -- Delta between the channels in a dual channel mode
3. Scale -- Voltage Scale
4. Offset -- Voltage Offset

Zero and Delta are common to all vertical scales, but Scale and Offset
have to be adjusted for each vertical scale individually.

In the Zero and Delta modes status line shows raw ADC readings and
the screen is split into two halves. On the left trace shows raw ADC
readings and the right side shows magnified view of the same trace.

In the Scale and Offset modes status line shows minimum and maximum voltage
values. All the calibration procedures are performed with the constant (DC)
voltages, so the minimum and maximum voltages will be close, but they will
be slightly different due to noise.

The calibration procedure consists of a few steps.

### Zero

This step must be performed in a single channel mode (sample rate is 62 MSPS or below).

For this test the input must be shorted (or at least disconnected).

The goal is to adjust the trace positions so it is exactly in the middle of the screen.

Middle of the screen corresponds to 0 V, which is the raw ADC reading of 0x80.

### Delta

This step must be performed in a dual channel mode (sample rate is 125 MSPS).

For this test the input must be shorted (or at least disconnected).

The goal is to adjust the trace so that even and odd samples (coming from the ADC
channels A and B) have the same value.

### Scale

This must be performed in a single channel mode (sample rate is 62 MSPS or below).
The vertical offset must be set to 0 V.

The input must be connected to the source of the variable voltage.

The voltage must be set so that ADC readings are close to the full scale,
but not clipping.

Zero or Delta mode raw readings can be used to select appropriate voltage.
Ideally the raw ADC data should be in the range 0xd0 - 0xf0.

The exact voltage does not matter as long as it can be measured independently
(for example using multimeter).

The calibration value must be adjusted until the minimum and maximum values
in the status bar match the measured value.

The calibration value for the negative voltage may be slightly different.
It is recommended to perform this procedure for the negative voltage as well
and adjust the calibration until symmetrical results are reached.

This step must be performed for all vertical scale settings separately.

### Offset

This must be performed in a single channel mode (sample rate is 62 MSPS or below).

The input must be connected to the source of the variable voltage.

Set the vertical offset close to the edge of the grid (+4 or -4 divisions).

Adjust calibration value until the displayed minimum and maximum voltages
match the actually measures voltage.

Perform this procedure for both positive and negative vertical position settings.

This step must be performed for all vertical scale settings separately.


