Yeti3D
======

Port of the heavy optimized "Yeti3D" 3D engine demo by **Derek J. Evans** from [Nintendo Game Boy Advance](https://en.wikipedia.org/wiki/Game_Boy_Advance) (GBA) to the Motorola P2K phones platform.

## Screenshots from Motorola C650

![Screenshot of Yeti3D from Motorola C650 1](../images/Screenshot_Yeti3D_C650_1.png) ![Screenshot of Yeti3D from Motorola C650 2](../images/Screenshot_Yeti3D_C650_2.png) ![Screenshot of Yeti3D from Motorola C650 3](../images/Screenshot_Yeti3D_C650_3.png) ![Screenshot of Yeti3D from Motorola C650 4](../images/Screenshot_Yeti3D_C650_4.png)

## Screenshots from Motorola SLVR L6

![Screenshot of Yeti3D from Motorola L6 1](../images/Screenshot_Yeti3D_L6_1.png) ![Screenshot of Yeti3D from Motorola L6 2](../images/Screenshot_Yeti3D_L6_2.png) ![Screenshot of Yeti3D from Motorola L6 3](../images/Screenshot_Yeti3D_L6_3.png)

## Screenshots from Motorola ROKR E1

![Screenshot of Yeti3D from Motorola E1 1](../images/Screenshot_Yeti3D_E1_1.png) ![Screenshot of Yeti3D from Motorola E1 2](../images/Screenshot_Yeti3D_E1_2.png) ![Screenshot of Yeti3D from Motorola E1 3](../images/Screenshot_Yeti3D_E1_3.png)

## Screenshots from Motorola E770v

![Screenshot of Yeti3D from Motorola E770v 1](../images/Screenshot_Yeti3D_E770v_1.png) ![Screenshot of Yeti3D from Motorola E770v 2](../images/Screenshot_Yeti3D_E770v_2.png) ![Screenshot of Yeti3D from Motorola E770v 3](../images/Screenshot_Yeti3D_E770v_3.png) ![Screenshot of Yeti3D from Motorola E770v 4](../images/Screenshot_Yeti3D_E770v_4.png)

## Screenshots from Motorola RAZR V3x

![Screenshot of Yeti3D from Motorola RAZR V3x 1](../images/Screenshot_Yeti3D_V3x_1.png) ![Screenshot of Yeti3D from Motorola RAZR V3x 2](../images/Screenshot_Yeti3D_V3x_2.png) ![Screenshot of Yeti3D from Motorola RAZR V3x 3](../images/Screenshot_Yeti3D_V3x_3.png) ![Screenshot of Yeti3D from Motorola RAZR V3x 4](../images/Screenshot_Yeti3D_V3x_4.png)

## Photos of Motorola SLVR L6 and Motorola ROKR E1

![Photo of Yeti3D on Motorola L6 and Motorola E1](../images/Photo_Yeti3D_L6_E1_1.jpg)

## Photos of Motorola RAZR V3i

Thanks to @Limows

![Photo of Yeti3D on Motorola V3i 1](../images/Photo_Yeti3D_V3i_1.jpg)

## Videos of Motorola SLVR L6 and Motorola ROKR E1

* See [Yeti3D Engine port to ELF for Motorola P2K phones](https://www.youtube.com/watch?v=HqgMxK00QFg) video on YouTube.
* See [Yeti3D Engine on Motorola P2K phones, 52 MHz & 65 MHz (ARM7TDMI overclocking) comparison](https://www.youtube.com/watch?v=Cnb3YsYFWZg) video on YouTube.
* See [Yeti3D Engine on Motorola V300 and Motorola V600, thanks to Paschendale!](https://www.youtube.com/watch?v=JonuDUWraoI) video on YouTube.
* See [Yeti3D Engine on Motorola RAZR V3i and Motorola ROKR E1, thanks to Bs0Dd!](https://www.youtube.com/watch?v=YpEJS38H3yc) video on YouTube.

## Web Demo

* [Yeti3D Web](https://lab.exlmoto.ru/y3d/) page.
* [Yeti3D Web](https://exlmotodev.github.io/y3d/) mirror page.

## Key controls

* D-Pad, 2, 4, 6, 8 - Movement.
* 1 - Look up.
* 3 - Look down.
* 7, 9 - Jump.
* Center, 5 - Fire (not available in optimized version).
* Left Soft Key, End Key, 0 - Quit.

## Other versions

* Slightly optimized version without sprites, enemy logic code and with reduced draw distance. See [optimize](optimize) utility project in this directory.

## ELF files

| Phone    | Screen  | Flags | Viewport <br /> Mode       | Optimized | Tasks <br /> Semaphores | Fixed FPS  | FPS <br /> (avg, min-max) | ELF filename         |
|----------|---------|-------|----------------------------|-----------|-------------------------|------------|---------------------------|----------------------|
| SLVR L6i | 128x160 | ROT0  | 160x128 <br /> fullscreen  | No        | No                      | 30.0       | 5.58 <br /> 5.4-5.7       | Y3D_VN1_160x128.elf  |
| SLVR L6i | 128x160 | ROT0  | 160x128 <br /> fullscreen  | Yes       | No                      | 30.0       | 5.64 <br /> 5.5-5.7       | Y3D_VO1_160x128.elf  |
| SLVR L6i | 128x160 | ROT90 | 120x80 <br /> rotozoom     | No        | No                      | 30.0       | 8.20 <br /> 7.8-8.5       | Y3D_RN1_120x80.elf   |
| SLVR L6i | 128x160 | ROT90 | 120x80 <br /> rotozoom     | Yes       | No                      | 30.0       | 8.23 <br /> 7.8-8.6       | Y3D_RO1_120x80.elf   |
| SLVR L6i | 128x160 | ROT90 | 96x64 <br /> rotozoom      | No        | No                      | 30.0       | 10.15 <br /> 9.9-10.4     | Y3D_RN1_96x64.elf    |
| SLVR L6i | 128x160 | ROT90 | 96x64 <br /> rotozoom      | Yes       | No                      | 30.0       | 10.18 <br /> 9.3-10.5     | Y3D_RO1_96x64.elf    |
| ROKR E1  | 176x220 | ROT0  | 220x176 <br /> fullscreen  | No        | No                      | 30.0       | 3.74 <br /> 3.4-4.0       | Y3D_VN1_220x176.elf  |
| ROKR E1  | 176x220 | ROT0  | 220x176 <br /> fullscreen  | Yes       | No                      | 30.0       | 3.91 <br /> 3.7-4.0       | Y3D_VO1_220x176.elf  |
| ROKR E1  | 176x220 | ROT0  | 220x176 <br /> fullscreen  | No        | Yes                     | 1000.0     | 2.83 <br /> 3.0-2.7       | Y3D_VN2T_220x176.elf |
| ROKR E1  | 176x220 | ROT0  | 220x176 <br /> fullscreen  | Yes       | Yes                     | 1000.0     | 2.83 <br /> 3.0-2.8       | Y3D_VO2T_220x176.elf |
| ROKR E1  | 176x220 | ROT90 | 120x80 <br /> rotozoom     | No        | No                      | 30.0       | 8.40 <br /> 7.4-8.6       | Y3D_RN1_120x80.elf   |
| ROKR E1  | 176x220 | ROT90 | 120x80 <br /> rotozoom     | Yes       | No                      | 30.0       | 8.40 <br /> 7.0-8.6       | Y3D_RO1_120x80.elf   |
| ROKR E1  | 176x220 | ROT90 | 96x64 <br /> rotozoom      | No        | No                      | 30.0       | 9.90 <br /> 8.5-10.1      | Y3D_RN1_96x64.elf    |
| ROKR E1  | 176x220 | ROT90 | 96x64 <br /> rotozoom      | Yes       | No                      | 30.0       | 11.15 <br /> 8.6-12.0     | Y3D_RO1_96x64.elf    |
| ROKR E1  | 176x220 | ROT90 | 96x64 <br /> rotozoom      | No        | Yes                     | 1000.0     | 8.04 <br /> 7.5-8.6       | Y3D_RN2T_96x64.elf   |
| ROKR E1  | 176x220 | ROT90 | 96x64 <br /> rotozoom      | Yes       | Yes                     | 1000.0     | 8.12 <br /> 7.5-8.6       | Y3D_RO2T_96x64.elf   |
| V360     | 176x220 | ROT90 | 96x64 <br /> rotozoom      | No        | No                      | 30.0       | 9.98 <br /> 6.2-12.1      | Y3D_RN1_96x64.elf    |
| V360     | 176x220 | ROT0  | 220x176 <br /> fullscreen  | No        | No                      | 30.0       | 3.64 <br /> 2.7-5.0       | Y3D_VN1D_220x176.elf |
| E770v    | 176x220 | ROT0  | 176x220 <br /> fullscreen  | No        | No                      | 30.0       | ? <br /> ?-?              | Y3D_176x220.elf      |
| RAZR V3x | 240x320 | ROT0  | 240x176 <br /> fullscreen  | No        | No                      | 30.0       | ? <br /> ?-?              | Y3D_240x176.elf      |
| RAZR V3x | 240x320 | ROT0  | 240x320 <br /> fullscreen  | No        | No                      | 30.0       | ? <br /> ?-?              | Y3D_240x320.elf      |

See [Overclock](../Overclock) project documents for overclocking Yeti3D comparision.

### Legend

* V (1st) - ATI videomode changing e.g. 176x220 => 220x176, 128x160 => 160x128, etc.
* R (1st) - ATI hardware rootation and zoom surface e.g. 120x80 => 176x220, 96x64 => 128x160, etc.
* D (1st) - DAL videomode, software rendering for Motorola C650 and similar phones.
* N (2nd) - Non-optimized version with sprites and entities logic.
* O (2nd) - Optimized version with no sprites and entities logic.
* 1 (3rd) - ELF for ElfPack 1.0, tested with.
* 2 (3rd) - ELF for ElfPack 2.0, tested with.
* T (4th) - ELF for ElfPack 2.0, with using Tasks and Semaphores.
* D (4th) - Debug test version with some unique patches like screen buffer in Java Heap and FPS output, works without using `Class_dal` constant.
* J (4th) - A special optimized versions with creating screen buffer in Java Heap also, for weak phones like Motorola V600.
* F (4th) - A special versions with FPS meter.
* L (4th) - A special versions with long search range for Motorola RAZR V3r.
* Z (4th) - A special versions for Motorola SLVR L6 phone (R3511_G_0A.52.45R_A) with upside down display.
* U (4th) - A special optimized versions with creating screen buffer in UIS Heap, for weak phones like Motorola C650.
* S (5th) - FPS limit to prevent phone's reboot.

### Summary

* 8 ELFs + 8 ElfPack 2.0 ELFs + 4 Task and Semaphores ELFs + 1 Debug ELF + 6 V600 ELFs + 8 FPS meter ELFs + 2 V3r ELFs + 2 L6 ELFs + 4 C650 ELFs + 3 ElfPack 2.0/M-CORE ELFs + 4 V635 ELFs = 50 ELFs.

## Additional information

The ELF-application has been tested on the following phones and firmware:

* Motorola C650: R365_G_0B.D3.08R
* Motorola SLVR L6: R3511_G_0A.52.45R_A
* Motorola SLVR L6i: R3443H1_G_0A.65.0BR
* Motorola ROKR E1: R373_G_0E.30.49R
* Motorola V600: TRIPLETS_G_0B.09.72R
* Motorola V360: R4513_G_08.B7.ACR
* Motorola V235: R3512_G_0A.30.6CR
* Motorola SLVR L7: R4513_G_08.B7.E0R_RB
* Motorola RAZR V3i: R4441D_G_08.01.03R
* Motorola RAZR V3r: R4515_G_08.BD.D3R
* Motorola SLVR L7e: R452D_G_08.01.0AR (some bugs)
* Motorola KRZR K1:  R452F_G_08.03.08R (some bugs)
* Motorola E770v: R252211_U_85.9B.E1P
* Motorola RAZR V3x: R252211LD_U_85.9B.E6P
* Motorola V635: R474_G_08.48.6FR

Application type: GUI + GPU + Java Heap + Videomode + Tasks (in the Task version).

## Useful links

* https://web.archive.org/web/20031204145215/http://www.theteahouse.com.au:80/gba/index.html
* https://sourceforge.net/projects/yeti3dpro/
* https://forum.motofan.ru/index.php?s=&showtopic=170514&view=findpost&p=1459600
