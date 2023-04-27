Yeti3D Old
==========

Port of the "Yeti3D Old" 3D engine demo by **Derek J. Evans** from [Nintendo Game Boy Advance](https://en.wikipedia.org/wiki/Game_Boy_Advance) (GBA) to the Motorola P2K phones platform.

## Screenshots from Motorola SLVR L6

![Screenshot of Yeti3D Old from Motorola L6 1](../images/Screenshot_Yeti3D_Old_L6_1.png) ![Screenshot of Yeti3D Old from Motorola L6 2](../images/Screenshot_Yeti3D_Old_L6_2.png) ![Screenshot of Yeti3D Old from Motorola L6 3](../images/Screenshot_Yeti3D_Old_L6_3.png) ![Screenshot of Yeti3D Old from Motorola L6 4](../images/Screenshot_Yeti3D_Old_L6_4.png)

## Screenshots from Motorola ROKR E1

![Screenshot of Yeti3D Old from Motorola E1 1](../images/Screenshot_Yeti3D_Old_E1_1.png) ![Screenshot of Yeti3D Old from Motorola E1 2](../images/Screenshot_Yeti3D_Old_E1_2.png) ![Screenshot of Yeti3D Old from Motorola E1 3](../images/Screenshot_Yeti3D_Old_E1_3.png) ![Screenshot of Yeti3D Old from Motorola E1 4](../images/Screenshot_Yeti3D_Old_E1_4.png)

## Key controls

* D-Pad, 2, 4, 6, 8 - Movement.
* Left Soft Key, End Key, 0 - Quit.

## Screen resolutions and viewports

| Phone   | Screen  | Flags | Viewport           | FPS  | ELF filename        |
| ---     | ---     | ---   | ---                | ---  |---                  |
| SLVR L6 | 128x160 | ROT0  | 160x128 (original) | 0.0  | Y3DO_O1_128x160.elf |
| SLVR L6 | 128x160 | ROT90 | 122x82 (zoom)      | 0.0  | Y3DO_Z1_122x82.elf  |
| SLVR L6 | 128x160 | ROT90 | 96x64 (zoom)       | 0.0  | Y3DO_Z1_96x64.elf   |
| SLVR L6 | 128x160 | ROT90 | 72x48 (zoom)       | 0.0  | Y3DO_Z1_72x48.elf   |
| ROKR E1 | 176x220 | ROT0  | 220x176 (original) | 0.0  | Y3DO_O1_176x220.elf |
| ROKR E1 | 176x220 | ROT90 | 122x82 (zoom)      | 0.0  | Y3DO_Z1_122x82.elf  |
| ROKR E1 | 176x220 | ROT90 | 96x64 (zoom)       | 0.0  | Y3DO_Z1_96x64.elf   |
| ROKR E1 | 176x220 | ROT90 | 72x48 (zoom)       | 0.0  | Y3DO_Z1_72x48.elf   |

### Legend

* O (1st) - Original screens resolution without any transformations.
* Z (1st) - ATI hardware rootation or zoom surface e.g. 122x82 => 176x220, 96x64 => 128x160, etc.
* 1 (3rd) - Elf for ElfPack 1.0, tested with.
* 2 (3rd) - Elf for ElfPack 2.0, tested with.

### Summary

* 5 ELFs + ElfPack 2.0 ELFs = 10 ELFs.

## Additional information

The ELF-application has been tested on the following phones and firmware:

* Motorola SLVR L6: R3443H1_G_0A.65.0BR
* Motorola ROKR E1: R373_G_0E.30.49R

Application type: GUI + ATI.

## Useful links

* https://www.gbadev.org/demos.php?showinfo=568