Spout
=====

Port of the "Spout" game by kuni to the Motorola P2K phones platform.

## Screenshots from Motorola C650

![Screenshot of Spout from Motorola C650 1](../images/Screenshot_Spout_C650_1.png) ![Screenshot of Spout from Motorola C650 2](../images/Screenshot_Spout_C650_2.png) ![Screenshot of Spout from Motorola C650 3](../images/Screenshot_Spout_C650_3.png)

## Screenshots from Motorola SLVR L6

![Screenshot of Spout from Motorola L6 1](../images/Screenshot_Spout_L6_1.png) ![Screenshot of Spout from Motorola L6 2](../images/Screenshot_Spout_L6_2.png) ![Screenshot of Spout from Motorola L6 3](../images/Screenshot_Spout_L6_3.png)

## Screenshots from Motorola ROKR E1

![Screenshot of Spout from Motorola E1 1](../images/Screenshot_Spout_E1_1.png) ![Screenshot of Spout from Motorola E1 2](../images/Screenshot_Spout_E1_2.png) ![Screenshot of Spout from Motorola E1 3](../images/Screenshot_Spout_E1_3.png)

## Photos of Motorola SLVR L6 and Motorola ROKR E1

![Photo of Spout on Motorola L6 and Motorola E1 1](../images/Photo_Spout_L6_E1_1.jpg)

## Photos of Motorola RAZR V3i

Thanks to @Limows

![Photo of Spout on Motorola V3i 1](../images/Photo_Spout_V3i_1.jpg)

## Videos of Motorola SLVR L6 and Motorola ROKR E1

See [Some ELF demos/games on Motorola P2K phones using ATI Imageon W2240/W2250 videochip](https://www.youtube.com/watch?v=DsYLWXPgmsU) video on YouTube.

## Web Demo

* [Spout Web](https://lab.exlmoto.ru/spout/) page.
* [Spout Web](https://exlmotodev.github.io/spout/) mirror page.

## Key controls

* D-Pad, 2, 4, 6, 8 - Movement.
* Center, 5 - Fire.
* 3 - Enable autofire.
* Right Soft Key, 9 - Pause.
* Left Soft Key, End Key, 0 - Quit.

## Other versions

* Version with long sight tail. See [long_tail](long_tail) utility project in this directory.

## ELF files

| Phone    | Screen  | Flags | Viewport            | Fixed FPS  | FPS (avg, min-max) | ELF filename   |
|----------|---------|-------|---------------------|------------|--------------------|----------------|
| SLVR L6i | 128x160 | ROT90 | 128x88 (rotozoom)   | 15.0       | 15.38, 14.9-16.9   | Spout_RL1.elf  |
| SLVR L6i | 128x160 | ROT0  | 128x88 (original)   | 15.0       | 15.29, 14.7-15.8   | Spout_OL1.elf  |
| ROKR E1  | 176x220 | ROT90 | 128x88 (rotozoom)   | 15.0       | 15.20, 15.1-15.3   | Spout_RL1.elf  |
| ROKR E1  | 176x220 | ROT0  | 128x88 (original)   | 15.0       | 15.18, 15.1-15.3   | Spout_OL1.elf  |
| SLVR L6i | 128x160 | ROT90 | 128x88 (rotozoom)   | 30.0       | 16.80, 16.9-17.2   | Spout_RH1.elf  |
| SLVR L6i | 128x160 | ROT0  | 128x88 (original)   | 30.0       | 21.69, 19.6-24.3   | Spout_OH1.elf  |
| ROKR E1  | 176x220 | ROT90 | 128x88 (rotozoom)   | 30.0       | 27.91, 19.2-30.3   | Spout_RH1.elf  |
| ROKR E1  | 176x220 | ROT0  | 128x88 (original)   | 30.0       | 28.56, 18.8-30.3   | Spout_OH1.elf  |
| V235     | 128x160 | ROT90 | 128x88 (rotozoom)   | 30.0       | 17.05, 16.9-17.2   | Spout_RH1J.elf |
| V360     | 176x220 | ROT90 | 128x88 (rotozoom)   | 30.0       | 26.93, 21.7-30.3   | Spout_RH1J.elf |

### Legend

* R (1st) - ATI hardware rootation and zoom surface e.g. 128x88 => 220x176, 128x88 => 160x128, etc.
* O (1st) - Original screens resolution without any transformations.
* L (2nd) - Low, 15 FPS timer ticks.
* H (2nd) - High, 30 FPS timer ticks.
* 1 (3rd) - ELF for ElfPack 1.0, tested with.
* 2 (3rd) - ELF for ElfPack 2.0, tested with.
* J (4th) - Use Java Heap instead of System Heap for phones with small amount of RAM (needs suspended Java application before ELF launch).
* U (4th) - Use UIS Heap instead of System Heap for phones with small amount of RAM.
* L (4th) - Long tail version.

### Summary

* 4 ELFs + 4 ElfPack 2.0 ELFs + 8 Java Heap ELFs + 4 Motorola V600 ELFs + 2 Motorola C650 ELFs + 1 EP1 LongTail ELF + 2 Motorola V635 ELFs = 25 ELFs.

## Additional information

The ELF-application has been tested on the following phones and firmware:

* Motorola C650: R365_G_0B.D3.08R
* Motorola SLVR L6: R3511_G_0A.52.45R_A
* Motorola SLVR L6i: R3443H1_G_0A.65.0BR
* Motorola ROKR E1: R373_G_0E.30.49R
* Motorola V600: TRIPLETS_G_0B.09.72R
* Motorola V635: R474_G_08.48.6FR
* Motorola V360: R4513_G_08.B7.ACR
* Motorola V235: R3512_G_0A.30.6CR
* Motorola SLVR L7: R4513_G_08.B7.E0R_RB
* Motorola RAZR V3i: R4441D_G_08.01.03R
* Motorola SLVR L7e: R452D_G_08.01.0AR (memory bugs)
* Motorola KRZR K1:  R452F_G_08.03.08R (memory bugs)

Application type: GUI + GPU.

## Useful links

* https://exlmoto.ru/spout-droid/
* https://github.com/EXL/Spout
* http://www.susami.co.jp/kuni/junk/junk.htm
* https://njw.me.uk/spout/
