Ambilight
=========

The "Ambilight" ELF daemon utility with GUI settings for some funlights and bias lighting functionality.

## Screenshots from Motorola V600

![Screenshot 1 of Ambilight from Motorola V600](../images/Screenshot_Ambilight_V600_1.png) ![Screenshot 2 of Ambilight from Motorola V600](../images/Screenshot_Ambilight_V600_2.png) ![Screenshot 3 of Ambilight from Motorola V600](../images/Screenshot_Ambilight_V600_3.png) ![Screenshot 4 of Ambilight from Motorola V600](../images/Screenshot_Ambilight_V600_4.png)

## Screenshots from Motorola ROKR E1

![Screenshot 1 of Ambilight from Motorola ROKR E1](../images/Screenshot_Ambilight_E1_1.png) ![Screenshot 2 of Ambilight from Motorola ROKR E1](../images/Screenshot_Ambilight_E1_2.png) ![Screenshot 3 of Ambilight from Motorola ROKR E1](../images/Screenshot_Ambilight_E1_3.png) ![Screenshot 4 of Ambilight from Motorola ROKR E1](../images/Screenshot_Ambilight_E1_4.png)

## Videos of Motorola ROKR E1

See [Ambilight](https://TODO) video on YouTube.

## Modes

* Ambilight - The side LEDs use the most common color on the screen as source of light.
* Color - The side LEDs use the specified HEX string with (0xRGB) format as source of light.
* Color Blink - The side LEDs blink with the specified HEX string with (0xRGB) format as source of light.
* Flash 25% - The flash LED lights up at 25% brightness of maximum.
* Flash 50% - The flash LED lights up at 50% brightness of maximum.
* Flash 100% - The flash LED lights up at maximum brightness. Danger! This mode may damage the flash LED!
* Network - The side LEDs blink on network signal strength: 0..30% - red, 30..60% - yellow, 60..100% - green.
* Battery - The side LEDs blink depending on battery: 0..30% - red, 30..60% - yellow, 60..100% - green.
* Rainbow - Smooth rainbow effect on the side LEDs.
* Random - Smooth color transitions to random colors.
* Stroboscope - Strobe mode with side LEDs (white color) and flash LED.
* Strobo Color - Strobe mode with side LEDs (random color) and flash LED.

## ELF files

* Ambilight.elf (ELF for ElfPack 1.0)
* Ambilight_V600.elf (ELF for ElfPack 1.0, Motorola V600 version)

## Additional information

The ELF-application has been tested on the following phones and firmware:

* Motorola ROKR E1: R373_G_0E.30.49R
* Motorola V600: TRIPLETS_G_0B.09.72R

Application type: Deamon + GUI + ATI + Funlights.
