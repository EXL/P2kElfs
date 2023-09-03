Yeti3D
======

Port of the heavy optimized "Yeti3D" 3D engine demo by **Derek J. Evans** from [Nintendo Game Boy Advance](https://en.wikipedia.org/wiki/Game_Boy_Advance) (GBA) to the Motorola P2K phones platform.

## Screenshots from Motorola SLVR L6

![Screenshot of Yeti3D from Motorola L6 1](../images/Screenshot_Yeti3D_L6_1.png) ![Screenshot of Yeti3D from Motorola L6 2](../images/Screenshot_Yeti3D_L6_2.png) ![Screenshot of Yeti3D from Motorola L6 3](../images/Screenshot_Yeti3D_L6_3.png)

## Screenshots from Motorola ROKR E1

![Screenshot of Yeti3D from Motorola E1 1](../images/Screenshot_Yeti3D_E1_1.png) ![Screenshot of Yeti3D from Motorola E1 2](../images/Screenshot_Yeti3D_E1_2.png) ![Screenshot of Yeti3D from Motorola E1 3](../images/Screenshot_Yeti3D_E1_3.png)

## Photos of Motorola SLVR L6 and Motorola ROKR E1

![Photo of Yeti3D on Motorola L6 and Motorola E1](../images/Photo_Yeti3D_L6_E1_1.jpg)

## Photos of Motorola RAZR V3i

Thanks to @Limows

![Photo of Yeti3D on Motorola V3i 1](../images/Photo_Yeti3D_V3i_1.jpg) ![Photo of Yeti3D on Motorola V3i 2](../images/Photo_Yeti3D_V3i_2.jpg)

## Photos of Motorola V235

Thanks to @Indepth6

![Photo of Yeti3D on Motorola V235 1](../images/Photo_Yeti3D_V235_1.jpg)

## Photos of Motorola V360

Thanks to @baat

![Photo of Yeti3D on Motorola V360 1](../images/Photo_Yeti3D_V360_1.jpg)

## Videos of Motorola SLVR L6 and Motorola ROKR E1

See [Yeti3D Engine port to ELF for Motorola P2K phones](https://www.youtube.com/watch?v=HqgMxK00QFg) video on YouTube.

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

* Slightly optimized version without sprites, enemy logic code and with reduced draw distance. See [yeti3d_memory_optimize](https://github.com/EXL/P2kElfs/tree/yeti3d_memory_optimize) branch in this repository.

## ELF files

| Phone   | Screen  | Flags | Viewport <br /> Mode       | Optimized | Tasks <br /> Semaphores | Fixed FPS  | FPS <br /> (avg, min-max) | ELF filename         |
| ---     | ---     | ---   | ---                        | ---       | ---                     | ---        | ---                       | ---                  |
| SLVR L6 | 128x160 | ROT0  | 160x128 <br /> fullscreen  | No        | No                      | 30.0       | 5.58 <br /> 5.4-5.7       | Y3D_VN1_128x160.elf  |
| SLVR L6 | 128x160 | ROT0  | 160x128 <br /> fullscreen  | Yes       | No                      | 30.0       | 5.64 <br /> 5.5-5.7       | Y3D_VO1_128x160.elf  |
| SLVR L6 | 128x160 | ROT90 | 120x80 <br /> rotozoom     | No        | No                      | 30.0       | 8.20 <br /> 7.8-8.5       | Y3D_RN1_120x80.elf   |
| SLVR L6 | 128x160 | ROT90 | 120x80 <br /> rotozoom     | Yes       | No                      | 30.0       | 8.23 <br /> 7.8-8.6       | Y3D_RO1_120x80.elf   |
| SLVR L6 | 128x160 | ROT90 | 96x64 <br /> rotozoom      | No        | No                      | 30.0       | 10.15 <br /> 9.9-10.4     | Y3D_RN1_96x64.elf    |
| SLVR L6 | 128x160 | ROT90 | 96x64 <br /> rotozoom      | Yes       | No                      | 30.0       | 10.18 <br /> 9.3-10.5     | Y3D_RO1_96x64.elf    |
| ROKR E1 | 176x220 | ROT0  | 220x176 <br /> fullscreen  | No        | No                      | 30.0       | 3.74 <br /> 3.4-4.0       | Y3D_VN1_176x220.elf  |
| ROKR E1 | 176x220 | ROT0  | 220x176 <br /> fullscreen  | Yes       | No                      | 30.0       | 3.91 <br /> 3.7-4.0       | Y3D_VO1_176x220.elf  |
| ROKR E1 | 176x220 | ROT0  | 220x176 <br /> fullscreen  | No        | Yes                     | 1000.0     | 2.83 <br /> 3.0-2.7       | Y3D_VN2T_176x220.elf |
| ROKR E1 | 176x220 | ROT0  | 220x176 <br /> fullscreen  | Yes       | Yes                     | 1000.0     | 2.83 <br /> 3.0-2.8       | Y3D_VO2T_176x220.elf |
| ROKR E1 | 176x220 | ROT90 | 120x80 <br /> rotozoom     | No        | No                      | 30.0       | 8.40 <br /> 7.4-8.6       | Y3D_RN1_120x80.elf   |
| ROKR E1 | 176x220 | ROT90 | 120x80 <br /> rotozoom     | Yes       | No                      | 30.0       | 8.40 <br /> 7.0-8.6       | Y3D_RO1_120x80.elf   |
| ROKR E1 | 176x220 | ROT90 | 96x64 <br /> rotozoom      | No        | No                      | 30.0       | 9.90 <br /> 8.5-10.1      | Y3D_RN1_96x64.elf    |
| ROKR E1 | 176x220 | ROT90 | 96x64 <br /> rotozoom      | Yes       | No                      | 30.0       | 11.15 <br /> 8.6-12.0     | Y3D_RO1_96x64.elf    |
| ROKR E1 | 176x220 | ROT90 | 96x64 <br /> rotozoom      | No        | Yes                     | 1000.0     | 8.04 <br /> 7.5-8.6       | Y3D_RN2T_96x64.elf   |
| ROKR E1 | 176x220 | ROT90 | 96x64 <br /> rotozoom      | Yes       | Yes                     | 1000.0     | 8.12 <br /> 7.5-8.6       | Y3D_RO2T_96x64.elf   |
| V360    | 176x220 | ROT90 | 96x64 <br /> rotozoom      | No        | No                      | 30.0       | 9.98 <br /> 6.2-12.1      | Y3D_RN1_96x64.elf    |
| V360    | 176x220 | ROT0  | 220x176 <br /> fullscreen  | No        | No                      | 30.0       | 3.64 <br /> 2.7-5.0       | Y3D_VN1D_176x220.elf |

### Legend

* V (1st) - ATI videomode changing e.g. 176x220 => 220x176, 128x160 => 160x128, etc.
* R (1st) - ATI hardware rootation and zoom surface e.g. 120x80 => 176x220, 96x64 => 128x160, etc.
* N (2nd) - Non-optimized version with sprites and entities logic.
* O (2nd) - Optimized version with no sprites and entities logic.
* 1 (3rd) - Elf for ElfPack 1.0, tested with.
* 2 (3rd) - Elf for ElfPack 2.0, tested with.
* T (4th) - Elf for ElfPack 2.0, with using Tasks and Semaphores.
* D (4th) - Debug test version with some unique patches like screen buffer in Java Heap and FPS output, works without using `Class_dal` constant.
* J (4th) - A special optimized versions with creating screen buffer in Java Heap also, for weak phones like Motorola V600.

### Summary

* 8 ELFs + ElfPack 2.0 ELFs + 4 Task and Semaphores ELFs + 1 Debug ELF + 3 J-weak V600 ELFs = 24 ELFs.

## Additional information

The ELF-application has been tested on the following phones and firmware:

* Motorola SLVR L6: R3443H1_G_0A.65.0BR
* Motorola ROKR E1: R373_G_0E.30.49R
* Motorola V600: TRIPLETS_G_0B.09.72R
* Motorola V360: R4513_G_08.B7.ACR
* Motorola V235: R3512_G_0A.30.6CR
* Motorola SLVR L7: R4513_G_08.B7.E0R_RB
* Motorola RAZR V3i: R4441D_G_08.01.03R

Application type: GUI + ATI + Java Heap + Videomode + Tasks (in the Task version).

## Useful links

* https://web.archive.org/web/20031204145215/http://www.theteahouse.com.au:80/gba/index.html
* https://sourceforge.net/projects/yeti3dpro/
* https://forum.motofan.ru/index.php?s=&showtopic=170514&view=findpost&p=1459600
