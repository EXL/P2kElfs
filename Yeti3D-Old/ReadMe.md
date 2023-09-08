Yeti3D Old
==========

Port of the "Yeti3D Old" 3D engine demo by **Derek J. Evans** from [Nintendo Game Boy Advance](https://en.wikipedia.org/wiki/Game_Boy_Advance) (GBA) to the Motorola P2K phones platform.

## Screenshots from Motorola SLVR L6

![Screenshot of Yeti3D Old from Motorola L6 1](../images/Screenshot_Yeti3D_Old_L6_1.png) ![Screenshot of Yeti3D Old from Motorola L6 2](../images/Screenshot_Yeti3D_Old_L6_2.png) ![Screenshot of Yeti3D Old from Motorola L6 3](../images/Screenshot_Yeti3D_Old_L6_3.png) ![Screenshot of Yeti3D Old from Motorola L6 4](../images/Screenshot_Yeti3D_Old_L6_4.png)

## Screenshots from Motorola ROKR E1

![Screenshot of Yeti3D Old from Motorola E1 1](../images/Screenshot_Yeti3D_Old_E1_1.png) ![Screenshot of Yeti3D Old from Motorola E1 2](../images/Screenshot_Yeti3D_Old_E1_2.png) ![Screenshot of Yeti3D Old from Motorola E1 3](../images/Screenshot_Yeti3D_Old_E1_3.png) ![Screenshot of Yeti3D Old from Motorola E1 4](../images/Screenshot_Yeti3D_Old_E1_4.png)

## Photos of Motorola SLVR L6 and Motorola ROKR E1

![Photo of Yeti3D Old on Motorola L6 and Motorola E1 1](../images/Photo_Yeti3D_Old_L6_E1_1.jpg)

## Videos of Motorola SLVR L6 and Motorola ROKR E1

See [Yeti3D Engine (old version) port to ELF for Motorola P2K phones](https://www.youtube.com/watch?v=qHC2QYrFZlk) video on YouTube.

## Web Demo

* [Yeti3D (old version) Web](https://lab.exlmoto.ru/y3do/) page.
* [Yeti3D (old version) Web](https://exlmotodev.github.io/y3do/) mirror page.

## Key controls

* D-Pad, 2, 4, 6, 8 - Movement.
* Left Soft Key, End Key, 0 - Quit.

## ELF files

| Phone   | Screen  | Flags | Viewport           | Fixed FPS  | FPS (avg, min-max) | ELF filename        |
| ---     | ---     | ---   | ---                | ---        | ---                |---                  |
| SLVR L6 | 128x160 | ROT0  | 160x128 (original) | 30.0       | 3.18, 3.1-3.6      | Y3DO_O1_160x128.elf |
| SLVR L6 | 128x160 | ROT90 | 122x82 (zoom)      | 30.0       | 4.21, 3.9-4.6      | Y3DO_Z1_122x82.elf  |
| SLVR L6 | 128x160 | ROT90 | 96x64 (zoom)       | 30.0       | 4.49, 4.2-4.7      | Y3DO_Z1_96x64.elf   |
| SLVR L6 | 128x160 | ROT90 | 72x48 (zoom)       | 30.0       | 5.53, 4.6-6.2      | Y3DO_Z1_72x48.elf   |
| ROKR E1 | 176x220 | ROT0  | 220x176 (original) | 30.0       | 2.31, 2.1-2.7      | Y3DO_O1_220x176.elf |
| ROKR E1 | 176x220 | ROT90 | 122x82 (zoom)      | 30.0       | 4.86, 4.6-5.3      | Y3DO_Z1_122x82.elf  |
| ROKR E1 | 176x220 | ROT90 | 96x64 (zoom)       | 30.0       | 4.99, 4.8-5.9      | Y3DO_Z1_96x64.elf   |
| ROKR E1 | 176x220 | ROT90 | 72x48 (zoom)       | 30.0       | 6.14, 5.7-6.7      | Y3DO_Z1_72x48.elf   |

### Legend

* O (1st) - Original screens resolution without any transformations.
* Z (1st) - ATI hardware rootation or zoom surface e.g. 122x82 => 176x220, 96x64 => 128x160, etc.
* 1 (2nd) - Elf for ElfPack 1.0, tested with.
* 2 (2nd) - Elf for ElfPack 2.0, tested with.

### Summary

* 5 ELFs + 5 ElfPack 2.0 ELFs = 10 ELFs.

## Additional information

The ELF-application has been tested on the following phones and firmware:

* Motorola SLVR L6: R3443H1_G_0A.65.0BR
* Motorola ROKR E1: R373_G_0E.30.49R

Application type: GUI + ATI.

## Useful links

* https://www.gbadev.org/demos.php?showinfo=568
