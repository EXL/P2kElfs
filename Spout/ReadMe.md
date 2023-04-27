Spout
=====

Port of the "Spout" game by kuni to the Motorola P2K phones platform.

## Screenshots from Motorola SLVR L6

![Screenshot of Spout from Motorola L6 1](../images/Screenshot_Spout_L6_1.png) ![Screenshot of Spout from Motorola L6 2](../images/Screenshot_Spout_L6_2.png) ![Screenshot of Spout from Motorola L6 3](../images/Screenshot_Spout_L6_3.png)

## Screenshots from Motorola ROKR E1

![Screenshot of Spout from Motorola E1 1](../images/Screenshot_Spout_E1_1.png) ![Screenshot of Spout from Motorola E1 2](../images/Screenshot_Spout_E1_2.png) ![Screenshot of Spout from Motorola E1 3](../images/Screenshot_Spout_E1_3.png)

## Key controls

* D-Pad, 2, 4, 6, 8 - Movement.
* Center, 5 - Fire.
* 3 - Enable autofire.
* Right Soft Key, 9 - Pause.
* Left Soft Key, End Key, 0 - Quit.

## ELF files

| Phone   | Screen  | Flags | Viewport            | Fixed FPS  | ELF filename  |
| ---     | ---     | ---   | ---                 | ---        |---            |
| SLVR L6 | 128x160 | ROT90 | 128x88 (rotozoom)   | 15.0       | Spout_RL1.elf |
| SLVR L6 | 128x160 | ROT0  | 128x88 (original)   | 15.0       | Spout_OL1.elf |
| ROKR E1 | 176x220 | ROT90 | 128x88 (rotozoom)   | 30.0       | Spout_RH1.elf |
| ROKR E1 | 176x220 | ROT0  | 128x88 (original)   | 30.0       | Spout_OH1.elf |

### Legend

* R (1st) - ATI hardware rootation and zoom surface e.g. 128x88 => 176x220, 128x88 => 128x160, etc.
* O (1st) - Original screens resolution without any transformations.
* L (2nd) - Low, 15 FPS timer ticks.
* H (2nd) - High, 30 FPS timer ticks.
* 1 (3rd) - Elf for ElfPack 1.0, tested with.
* 2 (3rd) - Elf for ElfPack 2.0, tested with.

### Summary

* 4 ELFs + ElfPack 2.0 ELFs = 8 ELFs.

## Additional information

The ELF-application has been tested on the following phones and firmware:

* Motorola SLVR L6: R3443H1_G_0A.65.0BR
* Motorola ROKR E1: R373_G_0E.30.49R

Application type: GUI + ATI.

## Useful links

* https://exlmoto.ru/spout-droid/
* https://github.com/EXL/Spout
* http://www.susami.co.jp/kuni/junk/junk.htm
* https://njw.me.uk/spout/
