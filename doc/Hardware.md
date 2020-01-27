# FNIRSI-5012H Hardware

There are two models of the PCB. The first revision has 4 mechanical relays.
The second revision has only one mechanical relay. The text below only applies
to the second revision.

## Main components on the board

* GD32F407VET6 - MCU (Cortex-M4, 512 KB Flash, 192 KB SRAM)
* AD9288 (most likely a clone, no markings) - ADC
* TP4056 - Li-lon Battery Charger
* TLV70033DDCR - 3.3 V LDO
* PC817 - Photocoupler
* CPC1002N - Optically coupled solid state relay
* SN74HC148 - 8-line to 3-line priority encoder
* SS8550 - PNP transistor
* HFD4 - Mechanical relay
* OPA356 - 200 MHz, CMOS OpAmp
* W25Q64JV - 64 Mbit SPI Flash
* TFT Display - Based on ST7789S

## ADC

The ADC is pin-compatible with AD9288, but there are a couple of pins in
the original that are marked NC, but bypassed to ground with capacitors
in this design. It also has a hemispherical dimple for the first pin marker.
The original has a flat bottom dimple. So it is most likely a clone,
but fully pin-compatible. It might be MXT2088.

## Power System

There are two LDOs in the system. One is the main LDO powering all the components.
The other one is dedicated to the LCD backlight. The brightness is controlled by
PWMing the enable signal.

The LDO inputs are always supplied by the battery and he power switch
controls enable pin of the main LDO.

## USB

The USB connector has programming pins (SWDIO/SWCLK) on the data pins,
so this connector is used for initial programming during the manufacturing process.

As a result there is no way to get USB working without hardware modification.
Modification would be relatively easy to do. USB FS data pins are located right
next to the programming pins. But I'm not sure there is any value to having USB on
this device.

## Buttons

SN74HC148 are used to read the keyboard. Each IC handles 8 buttons and two buttons
are connected directly to the MCU pins (for a total of 18 buttons).

Allocation of keys to priority encoders (PE):

| Channel | PE0   | PE1       |
|:-------:|:------|:----------|
| 0       | STOP  | 50%       |
| 1       | F1    | TRIG DOWN |
| 2       | EDGE  | TRIG UP   |
| 3       | AC/DC | TRIG      |
| 4       | UP    | MENU      |
| 5       | RIGHT | SAVE      |
| 6       | MODE  | LEFT      |
| 7       | AUTO  | DOWN      |

F2 and 1X10X are connected directly to the MCU pins.

It is an interesting way to reduce the number of pins required for
the keyboard, but obviously only one key from the group can be detected
at a time, even if multiple keys are pressed.

## Other Hardware

* CPC1002N is used for shorting the AC/DC capacitor.
* SS8550 are used to control PC817s and a mechanical relay.
* OPA356 is a buffer right before the ADC.
* W25Q64JV is used for waveform storage (and possibly settings, but not the firmware).

## Attenuation (Vertical Scale Control)

Attenuation is set though the 7 transistors (controlling optocouplers and a relay).
I number the transistors Q0-Q6 starting from the one closest to the BNC connector.
Each volts/div setting enables one transistor:
* 10 V/div - Q4
* 5 V/div - Q5
* 2 V/div - Q6
* 1 V/div - Q1
* 500 mV/div - Q0
* 200 mV/div - Q2
* 100 mV/div and 50 mV/div - Q3

50 and 100 mV/div are not different on the hardware level, so the amplitude
is just changed in the software. This is also the only range that is controlled
using a mechanical relay.

## Front-end

Here is a schematic of the front-end of this oscilloscope. None of the capacitor values are known.

Transistors are part of the optocouplers. The paired transistors are the same optocouplers,
but they are driven at the same time (LEDs on the other side are in series).

![FNIRSI-5012H Front-end]:(doc/frontend.jpg)

## GD32F407VET6 Pinout

And finally, the full pinout of the MCU.

ADC
| Pin  | Function   |
|:----:|:-----------|
| PD0  | ADC B D0   |
| PD1  | ADC B D1   |
| PD2  | ADC B D2   |
| PD3  | ADC B D3   |
| PD4  | ADC B D4   |
| PD5  | ADC B D5   |
| PD6  | ADC B D6   |
| PD7  | ADC B D7   |
| PD8  | ADC A D7   |
| PD9  | ADC A D6   |
| PD10 | ADC A D5   |
| PD11 | ADC A D4   |
| PD12 | ADC A D3   |
| PD13 | ADC A D2   |
| PD14 | ADC A D1   |
| PD15 | ADC A D0   |
| PA8  | ADC A CLK  |
| PA9  | ADC B CLK  |

LCD
| Pin  | Function   |
|:----:|:-----------|
| PC6  | LCD_RESET  |
| PB3  | LCD_RD     |
| PB4  | LCD_WR     |
| PB5  | LCD_RS     |
| PB6  | LCD_CS     |
| PE0  | LCD_D0     |
| PE1  | LCD_D1     |
| PE2  | LCD_D2     |
| PE3  | LCD_D3     |
| PE4  | LCD_D4     |
| PE5  | LCD_D5     |
| PE6  | LCD_D6     |
| PE7  | LCD_D7     |
| PB0  | LCD_BL_EN  |

Vertical Controls
| Pin  | Function   |
|:----:|:-----------|
| PB9  | Q0         |
| PB8  | Q1         |
| PB7  | Q2         |
| PC12 | Q3         |
| PC11 | Q4         |
| PC10 | Q5         |
| PA15 | Q6         |
| PC15 | AC/DC      |
| PA4  | Offset (DAC output) |

Keyboard
| Pin  | Function   |
|:----:|:-----------|
| PE13 | PE0.A0     |
| PB14 | PE0.A1     |
| PB13 | PE0.A2     |
| PE14 | PE0.GS     |
| PE15 | PE1.A0     |
| PB12 | PE1.A1     |
| PB11 | PE1.A2     |
| PB10 | PE1.GS     |
| PE12 | BTN_1X10X  |
| PE11 | BTN_F2     |

Flash
| Pin  | Function   |
|:----:|:-----------|
| PA3  | FLASH_CS   |
| PA5  | FLASH_CLK  |
| PA6  | FLASH_MISO |
| PA7  | FLASH_MOSI |

Miscellaneous
| Pin  | Function   |
|:----:|:-----------|
| PB15 | CHARGE     |
| PB1  | VBAT_SENSE (Vbat / 2) |
| PHx  | 20 MHz Crystal |
| PA13 | SWDIO      |
| PA14 | SWCLK      |

