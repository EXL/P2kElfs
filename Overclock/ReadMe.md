Overclock
=========

An application for overclocking ARM7TDMI core in Neptune SoC phones from stock 52 MHz to 65 MHz.

## Screenshots from Motorola SLVR L6

![Screenshot of Overclock from Motorola L6 1](../images/Screenshot_Overclock_L6_1.png) ![Screenshot of Overclock from Motorola L6 2](../images/Screenshot_Overclock_L6_2.png) ![Screenshot of Overclock from Motorola L6 3](../images/Screenshot_Overclock_L6_3.png)

## Screenshots from Motorola ROKR E1

![Screenshot of Overclock from Motorola E1 1](../images/Screenshot_Overclock_E1_1.png) ![Screenshot of Overclock from Motorola E1 2](../images/Screenshot_Overclock_E1_2.png) ![Screenshot of Overclock from Motorola E1 3](../images/Screenshot_Overclock_E1_3.png)

## Videos of benchmarking Motorola phones

See [Overclocking and Benchmakring old Motorola P2K phones (52 MHz to 65 MHz)](https://youtu.be/IO8aktssBo8) video on YouTube.

## Overclocking results

### 52 MHz

![Screenshot of Overclock results on Benchmark from Motorola E1 1](../images/Screenshot_Overclock_Benchmark_E1_1.png) ![Screenshot of Overclock results on JBenchmark from Motorola E1 1](../images/Screenshot_Overclock_JBenchmark_E1_1.png) ![Screenshot of Overclock results on Benchmark from Motorola L6 1](../images/Screenshot_Overclock_Benchmark_L6_1.png) ![Screenshot of Overclock results on Benchmark from Motorola L6 2](../images/Screenshot_Overclock_Benchmark_L6_2.png)  ![Screenshot of Overclock results on JBenchmark from Motorola L6 1](../images/Screenshot_Overclock_JBenchmark_L6_1.png) ![Screenshot of Overclock results on JBenchmark from Motorola L6 1](../images/Screenshot_Overclock_JBenchmark_L6_2.png)

### 65 MHz

![Screenshot of Overclock results on Benchmark from Motorola E1 2](../images/Screenshot_Overclock_Benchmark_E1_2.png) ![Screenshot of Overclock results on JBenchmark from Motorola E1 2](../images/Screenshot_Overclock_JBenchmark_E1_2.png) ![Screenshot of Overclock results on Benchmark from Motorola L6 3](../images/Screenshot_Overclock_Benchmark_L6_3.png) ![Screenshot of Overclock results on Benchmark from Motorola L6 4](../images/Screenshot_Overclock_Benchmark_L6_4.png)  ![Screenshot of Overclock results on JBenchmark from Motorola L6 3](../images/Screenshot_Overclock_JBenchmark_L6_3.png) ![Screenshot of Overclock results on JBenchmark from Motorola L6 4](../images/Screenshot_Overclock_JBenchmark_L6_4.png)

### Benchmark table

| Phone    | Freq   | CPU (MCU)                                                  | GPU (IPU)                                                                              | RAM (SRAM)                                           | HEAP (J2ME)                |
| ---      | ---    | ---                                                        | ---                                                                                    | ---                                                  | ---                        |
| SLVR L6i | 52 MHz | 1305 ms <br /> 3.21 BMIPS <br /> 2101 ms <br /> 5.41 DMIPS | 3919 ms <br /> 52.6 FPS <br /> 7301 ms <br /> 33.5 FPS <br /> 14380 ms <br /> 16.4 FPS | 159 ms <br /> 1227520 B <br /> 13 ms <br /> 117504 B | 4.11 sec <br /> 1112.0 KiB |
| SLVR L6i | 65 MHz | 1039 ms <br /> 4.03 BMIPS <br /> 1677 ms <br /> 6.78 DMIPS | 3900 ms <br /> 52.6 FPS <br /> 7015 ms <br /> 33.8 FPS <br /> 11442 ms <br /> 20.2 FPS | 125 ms <br /> 1222912 B <br /> 10 ms <br /> 117504 B | 2.34 sec <br /> 1164.0 KiB |
| ROKR E1  | 52 MHz | 1304 ms <br /> 3.21 BMIPS <br /> 2080 ms <br /> 5.47 DMIPS | 3425 ms <br /> 62.5 FPS <br /> 6279 ms <br /> 39.5 FPS <br /> 13635 ms <br /> 17.7 FPS | 22 ms <br /> 524800 B <br /> 2 ms <br /> 77312 B     | 7.39 sec <br /> 1060.0 KiB |
| ROKR E1  | 65 MHz | 1039 ms <br /> 4.03 BMIPS <br /> 1653 ms <br /> 6.88 DMIPS | 3454 ms <br /> 62.5 FPS <br /> 5986 ms <br /> 39.5 FPS <br /> 11079 ms <br /> 21.6 FPS | 18 ms <br /> 526336 B <br /> 2 ms <br /> 77312 B     | 5.44 sec <br /> 1060.0 KiB |
| SLVR L7  | 52 MHz | 1304 ms <br /> 3.21 BMIPS <br /> 1909 ms <br /> 5.96 DMIPS | ? <br /> ? <br /> ? <br /> ? <br /> ? <br /> ?                                         | ? <br /> ? <br /> ? <br /> ?                         | ? <br /> ?                 |
| SLVR L7  | 65 MHz | 1043 ms <br /> 4.01 BMIPS <br /> 1528 ms <br /> 7.44 DMIPS | ? <br /> ? <br /> ? <br /> ? <br /> ? <br /> ?                                         | ? <br /> ? <br /> ? <br /> ?                         | ? <br /> ?                 |
| V360     | 52 MHz | 1304 ms <br /> 3.21 BMIPS <br /> 1922 ms <br /> 5.92 DMIPS | ? <br /> ? <br /> ? <br /> ? <br /> ? <br /> ?                                         | ? <br /> ? <br /> ? <br /> ?                         | ? <br /> ?                 |
| V360     | 65 MHz | 1035 ms <br /> 4.04 BMIPS <br /> 1524 ms <br /> 7.46 DMIPS | ? <br /> ? <br /> ? <br /> ? <br /> ? <br /> ?                                         | ? <br /> ? <br /> ? <br /> ?                         | ? <br /> ?                 |
| RAZR V3i | 52 MHz | 1313 ms <br /> 3.17 BMIPS <br /> 1929 ms <br /> 5.90 DMIPS | 3947 ms <br /> 55.5 FPS <br /> 6888 ms <br /> 35.1 FPS <br /> 13768 ms <br /> 15.9 FPS | 71 ms <br /> 1091584 B <br /> 5 ms <br /> 112384 B   | 5.44 sec <br /> 1192.0 KiB |
| RAZR V3i | 65 MHz | 1067 ms <br /> 3.91 BMIPS <br /> 1563 ms <br /> 7.28 DMIPS | 3905 ms <br /> 55.5 FPS <br /> 6688 ms <br /> 35.1 FPS <br /> 10763 ms <br /> 22.7 FPS | 46 ms <br /> 960512 B <br /> 4 ms <br /> 112384 B    | ? <br /> ?                 |

See [Benchmark](../Benchmark) project documents for more stock results.

### Yeti3D table

| Phone    | Freq   | Screen  | Flags | Viewport <br /> Mode       | Optimized | FPS <br /> (avg, min-max) | ELF filename          |
| ---      | ---    | ---     | ---   | ---                        | ---       | ---                       | ---                   |
| SLVR L6i | 52 MHz | 128x160 | ROT0  | 160x128 <br /> fullscreen  | No        | 5.58 <br /> 5.4-5.7       | Y3D_VN1F_160x128.elf  |
| SLVR L6i | 65 MHz | 128x160 | ROT0  | 160x128 <br /> fullscreen  | No        | 7.38 <br /> 5.1-8.5       | Y3D_VN1F_160x128.elf  |
| SLVR L6i | 52 MHz | 128x160 | ROT0  | 160x128 <br /> fullscreen  | Yes       | 5.64 <br /> 5.5-5.7       | Y3D_VO1F_160x128.elf  |
| SLVR L6i | 65 MHz | 128x160 | ROT0  | 160x128 <br /> fullscreen  | Yes       | 7.30 <br /> 5.3-8.3       | Y3D_VO1F_160x128.elf  |
| SLVR L6i | 52 MHz | 128x160 | ROT90 | 120x80 <br /> rotozoom     | No        | 8.20 <br /> 7.8-8.5       | Y3D_RN1F_120x80.elf   |
| SLVR L6i | 65 MHz | 128x160 | ROT90 | 120x80 <br /> rotozoom     | No        | 10.38 <br /> 9.3-11.1     | Y3D_RN1F_120x80.elf   |
| SLVR L6i | 52 MHz | 128x160 | ROT90 | 120x80 <br /> rotozoom     | Yes       | 8.23 <br /> 7.8-8.6       | Y3D_RO1F_120x80.elf   |
| SLVR L6i | 65 MHz | 128x160 | ROT90 | 120x80 <br /> rotozoom     | Yes       | 10.47 <br /> 9.4-12.9     | Y3D_RO1F_120x80.elf   |
| SLVR L6i | 52 MHz | 128x160 | ROT90 | 96x64 <br /> rotozoom      | No        | 10.15 <br /> 9.9-10.4     | Y3D_RN1F_96x64.elf    |
| SLVR L6i | 65 MHz | 128x160 | ROT90 | 96x64 <br /> rotozoom      | No        | 11.74 <br /> 10.5-14.2    | Y3D_RN1F_96x64.elf    |
| SLVR L6i | 52 MHz | 128x160 | ROT90 | 96x64 <br /> rotozoom      | Yes       | 10.18 <br /> 9.3-10.5     | Y3D_RO1F_96x64.elf    |
| SLVR L6i | 65 MHz | 128x160 | ROT90 | 96x64 <br /> rotozoom      | Yes       | 12.90 <br /> 10.3-15.8    | Y3D_RO1F_96x64.elf    |
| ROKR E1  | 52 MHz | 176x220 | ROT0  | 220x176 <br /> fullscreen  | No        | 3.74 <br /> 3.4-4.0       | Y3D_VN1F_220x176.elf  |
| ROKR E1  | 65 MHz | 176x220 | ROT0  | 220x176 <br /> fullscreen  | No        | 5.24 <br /> 5.0-5.4       | Y3D_VN1F_220x176.elf  |
| ROKR E1  | 52 MHz | 176x220 | ROT0  | 220x176 <br /> fullscreen  | Yes       | 3.91 <br /> 3.7-4.0       | Y3D_VO1F_220x176.elf  |
| ROKR E1  | 65 MHz | 176x220 | ROT0  | 220x176 <br /> fullscreen  | Yes       | 5.34 <br /> 5.0-5.4       | Y3D_VO1F_220x176.elf  |
| ROKR E1  | 52 MHz | 176x220 | ROT90 | 120x80 <br /> rotozoom     | No        | 8.40 <br /> 7.4-8.6       | Y3D_RN1F_120x80.elf   |
| ROKR E1  | 65 MHz | 176x220 | ROT90 | 120x80 <br /> rotozoom     | No        | 12.00 <br /> 11.9-12.1    | Y3D_RN1F_120x80.elf   |
| ROKR E1  | 52 MHz | 176x220 | ROT90 | 120x80 <br /> rotozoom     | Yes       | 8.40 <br /> 7.0-8.6       | Y3D_RO1F_120x80.elf   |
| ROKR E1  | 65 MHz | 176x220 | ROT90 | 120x80 <br /> rotozoom     | Yes       | 12.09 <br /> 9.8-15.1     | Y3D_RO1F_120x80.elf   |
| ROKR E1  | 52 MHz | 176x220 | ROT90 | 96x64 <br /> rotozoom      | No        | 9.90 <br /> 8.5-10.1      | Y3D_RN1F_96x64.elf    |
| ROKR E1  | 65 MHz | 176x220 | ROT90 | 96x64 <br /> rotozoom      | No        | 14.75 <br /> 12.9-15.1    | Y3D_RN1F_96x64.elf    |
| ROKR E1  | 52 MHz | 176x220 | ROT90 | 96x64 <br /> rotozoom      | Yes       | 11.15 <br /> 8.6-12.0     | Y3D_RO1F_96x64.elf    |
| ROKR E1  | 65 MHz | 176x220 | ROT90 | 96x64 <br /> rotozoom      | Yes       | 19.92 <br /> 15.3-20.4    | Y3D_RO1F_96x64.elf    |

See [Yeti3D](../Yeti3D) project documents for more stock results.

## ELF files

* Overclock.elf (ELF for ElfPack 1.0)
* Overclock_L7e.elf (ELF for ElfPack 1.0, Motorola SLVR L7e and Motorola V360 version)
* Overclock_V600.elf (ELF for ElfPack 1.0, Motorola V600 version)

## Additional information

The ELF-application has been tested on the following phones and firmware:

* Motorola SLVR L6i: R3443H1_G_0A.65.0BR
* Motorola ROKR E1: R373_G_0E.30.49R
* Motorola V360: R4513_G_08.B7.ACR
* Motorola SLVR L7: R4513_G_08.B7.E0R_RB
* Motorola SLVR L7e: R452D_G_08.01.0AR (not working)
* Motorola KRZR K1:  R452F_G_08.03.08R (not working)

Application type: GUI.

Please read the [Overclocking MCU in Motorola E398 and similar phones](https://forum.motofan.ru/index.php?showtopic=1742268) topic on MotoFan.Ru forum for additional information (in Russian).
