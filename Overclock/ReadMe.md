Overclock
=========

An application for overclocking ARM7TDMI core in Neptune SoC phones from stock 52 MHz to 65 MHz.

## Screenshots from Motorola SLVR L6

![Screenshot of Overclock from Motorola L6 1](../images/Screenshot_Overclock_L6_1.png) ![Screenshot of Overclock from Motorola L6 2](../images/Screenshot_Overclock_L6_2.png) ![Screenshot of Overclock from Motorola L6 3](../images/Screenshot_Overclock_L6_3.png)

## Screenshots from Motorola V600

![Screenshot of Overclock from Motorola V600 1](../images/Screenshot_Overclock_V600_1.png) ![Screenshot of Overclock from Motorola V600 2](../images/Screenshot_Overclock_V600_2.png) ![Screenshot of Overclock from Motorola V600 3](../images/Screenshot_Overclock_V600_3.png)

## Screenshots from Motorola ROKR E1

![Screenshot of Overclock from Motorola E1 1](../images/Screenshot_Overclock_E1_1.png) ![Screenshot of Overclock from Motorola E1 2](../images/Screenshot_Overclock_E1_2.png) ![Screenshot of Overclock from Motorola E1 3](../images/Screenshot_Overclock_E1_3.png)

## Videos of benchmarking Motorola phones

See [Overclocking and Benchmakring old Motorola P2K phones (52 MHz to 65 MHz)](https://youtu.be/IO8aktssBo8) video on YouTube.

## Overclocking results

### 52 MHz

![Screenshot of Overclock results on Benchmark from Motorola E1 1](../images/Screenshot_Overclock_Benchmark_E1_1.png) ![Screenshot of Overclock results on JBenchmark from Motorola E1 1](../images/Screenshot_Overclock_JBenchmark_E1_1.png) ![Screenshot of Overclock results on Benchmark from Motorola L6 1](../images/Screenshot_Overclock_Benchmark_L6_1.png) ![Screenshot of Overclock results on Benchmark from Motorola L6 2](../images/Screenshot_Overclock_Benchmark_L6_2.png)  ![Screenshot of Overclock results on JBenchmark from Motorola L6 1](../images/Screenshot_Overclock_JBenchmark_L6_1.png) ![Screenshot of Overclock results on JBenchmark from Motorola L6 1](../images/Screenshot_Overclock_JBenchmark_L6_2.png)

### 65 MHz

![Screenshot of Overclock results on Benchmark from Motorola E1 2](../images/Screenshot_Overclock_Benchmark_E1_2.png) ![Screenshot of Overclock results on JBenchmark from Motorola E1 2](../images/Screenshot_Overclock_JBenchmark_E1_2.png) ![Screenshot of Overclock results on Benchmark from Motorola L6 3](../images/Screenshot_Overclock_Benchmark_L6_3.png) ![Screenshot of Overclock results on Benchmark from Motorola L6 4](../images/Screenshot_Overclock_Benchmark_L6_4.png)  ![Screenshot of Overclock results on JBenchmark from Motorola L6 3](../images/Screenshot_Overclock_JBenchmark_L6_3.png) ![Screenshot of Overclock results on JBenchmark from Motorola L6 4](../images/Screenshot_Overclock_JBenchmark_L6_4.png)

### JBenchmark table

| Phone    | Freq   | Total | Text  | 2D Shapes | 3D Shapes | Fill Rate | Animation |
|----------|--------|-------|-------|-----------|-----------|-----------|-----------|
| C650     | 52 MHz | 880   | 212   | ?         | ?         | ?         | ?         |
| C650     | 65 MHz | 1078  | 265   | 259       | 196       | 113       | 245       |
| SLVR L6i | 52 MHz | 1051  | 255   | 255       | 194       | 89        | 258       |
| SLVR L6i | 65 MHz | 1108  | 257   | 258       | 223       | 112       | 258       |
| ROKR E1  | 52 MHz | 1197  | 368   | 303       | 142       | 52        | 332       |
| ROKR E1  | 65 MHz | 1493  | 463   | 377       | 179       | 65        | 409       |
| SLVR L7  | 52 MHz | 1486  | 494   | 358       | 153       | 51        | 430       |
| SLVR L7  | 65 MHz | 1822  | 595   | 450       | 192       | 64        | 521       |
| V360     | 52 MHz | 1485  | 486   | 365       | 161       | 55        | 418       |
| V360     | 65 MHz | 1891  | 612   | 473       | 197       | 69        | 540       |
| RAZR V3i | 52 MHz | 1208  | 391   | 303       | 125       | 40        | 349       |
| RAZR V3i | 65 MHz | 1493  | 495   | 358       | 156       | 51        | 433       |
| RAZR V3r | 52 MHz | 1092  | 340   | 269       | 124       | 44        | 315       |
| RAZR V3r | 65 MHz | 1352  | 420   | 340       | 150       | 55        | 387       |

### Benchmark table

| Phone    | Freq   | CPU (MCU)                                                  | GPU (IPU)                                                                              | RAM (SUAPI)                                          | RAM (UIS)                  | HEAP (J2ME)                |
|----------|--------|------------------------------------------------------------|----------------------------------------------------------------------------------------|------------------------------------------------------|----------------------------|----------------------------|
| С650     | 52 MHz | 1317 ms <br /> 3.17 BMIPS <br /> 2029 ms <br /> 5.61 DMIPS | ?                                                                                      | 46 ms <br /> 279552 B <br /> 1 ms <br /> 18176 B     | 147 ms <br /> 415232 B     | ? sec <br /> ? KiB         |
| С650     | 65 MHz | 1235 ms <br /> 3.39 BMIPS <br /> 1632 ms <br /> 6.97 DMIPS | ?                                                                                      | 36 ms <br /> 279552 B <br /> 1 ms <br /> 18176 B     | 112 ms <br /> 415232 B     | 1.57 sec <br /> 672.0 KiB  |
| SLVR L6i | 52 MHz | 1305 ms <br /> 3.21 BMIPS <br /> 2101 ms <br /> 5.41 DMIPS | 3919 ms <br /> 52.6 FPS <br /> 7301 ms <br /> 33.5 FPS <br /> 14380 ms <br /> 16.4 FPS | 159 ms <br /> 1227520 B <br /> 13 ms <br /> 117504 B | ? ms <br /> ? B            | 4.11 sec <br /> 1112.0 KiB |
| SLVR L6i | 65 MHz | 1039 ms <br /> 4.03 BMIPS <br /> 1677 ms <br /> 6.78 DMIPS | 3900 ms <br /> 52.6 FPS <br /> 7015 ms <br /> 33.8 FPS <br /> 11442 ms <br /> 20.2 FPS | 125 ms <br /> 1222912 B <br /> 10 ms <br /> 117504 B | ? ms <br /> ? B            | 2.34 sec <br /> 1164.0 KiB |
| ROKR E1  | 52 MHz | 1304 ms <br /> 3.21 BMIPS <br /> 2080 ms <br /> 5.47 DMIPS | 3425 ms <br /> 62.5 FPS <br /> 6279 ms <br /> 39.5 FPS <br /> 13635 ms <br /> 17.7 FPS | 22 ms <br /> 524800 B <br /> 2 ms <br /> 77312 B     | ? ms <br /> ? B            | 7.39 sec <br /> 1060.0 KiB |
| ROKR E1  | 65 MHz | 1039 ms <br /> 4.03 BMIPS <br /> 1653 ms <br /> 6.88 DMIPS | 3454 ms <br /> 62.5 FPS <br /> 5986 ms <br /> 39.5 FPS <br /> 11079 ms <br /> 21.6 FPS | 18 ms <br /> 526336 B <br /> 2 ms <br /> 77312 B     | ? ms <br /> ? B            | 5.44 sec <br /> 1060.0 KiB |
| SLVR L7  | 52 MHz | 1304 ms <br /> 3.21 BMIPS <br /> 1909 ms <br /> 5.96 DMIPS | 3316 ms <br /> 62.5 FPS <br /> 6365 ms <br /> 39.6 FPS <br /> 13239 ms <br /> 17.1 FPS | 38 ms <br /> 514048 B <br /> 4 ms <br /> 77312 B     | ? ms <br /> ? B            | 1.56 sec <br /> 296.0 KiB  |
| SLVR L7  | 65 MHz | 1043 ms <br /> 4.01 BMIPS <br /> 1528 ms <br /> 7.44 DMIPS | ? ms <br /> ? FPS <br /> ? ms <br /> ? FPS <br /> ? ms <br /> ? FPS                    | ? ms <br /> ? B <br /> ? ms <br /> ? B               | ? ms <br /> ? B            | ? sec <br /> ? KiB         |
| V360     | 52 MHz | 1304 ms <br /> 3.21 BMIPS <br /> 1915 ms <br /> 5.94 DMIPS | 3506 ms <br /> 57.3 FPS <br /> 6364 ms <br /> 39.5 FPS <br /> 13704 ms <br /> 17.2 FPS | 36 ms <br /> 474624 B <br /> 4 ms <br /> 77312 B     | ? ms <br /> ? B            | 1.83 sec <br /> 656.0 KiB  |
| V360     | 65 MHz | 1035 ms <br /> 4.04 BMIPS <br /> 1524 ms <br /> 7.46 DMIPS | ? ms <br /> ? FPS <br /> ? ms <br /> ? FPS <br /> ? ms <br /> ? FPS                    | ? ms <br /> ? B <br /> ? ms <br /> ? B               | ? ms <br /> ? B            | ? sec <br /> ? KiB         |
| RAZR V3i | 52 MHz | 1313 ms <br /> 3.17 BMIPS <br /> 1929 ms <br /> 5.90 DMIPS | 3947 ms <br /> 55.5 FPS <br /> 6888 ms <br /> 35.1 FPS <br /> 13768 ms <br /> 15.9 FPS | 71 ms <br /> 1091584 B <br /> 5 ms <br /> 112384 B   | ? ms <br /> ? B            | 5.44 sec <br /> 1192.0 KiB |
| RAZR V3i | 65 MHz | 1067 ms <br /> 3.91 BMIPS <br /> 1563 ms <br /> 7.28 DMIPS | 3905 ms <br /> 55.5 FPS <br /> 6688 ms <br /> 35.1 FPS <br /> 10763 ms <br /> 22.7 FPS | 46 ms <br /> 960512 B <br /> 4 ms <br /> 112384 B    | ? ms <br /> ? B            | ? sec <br /> ? KiB         |
| RAZR V3r | 52 MHz | 1304 ms <br /> 3.21 BMIPS <br /> 1914 ms <br /> 5.94 DMIPS | 4461 ms <br /> 51.5 FPS <br /> 6054 ms <br /> 42.5 FPS <br /> 13234 ms <br /> 19.0 FPS | 28 ms <br /> 385280 B <br /> 4 ms <br /> 77312 B     | ? ms <br /> ? B            | 5.21 sec <br /> 1280.0 KiB |
| RAZR V3r | 65 MHz | 1042 ms <br /> 4.01 BMIPS <br /> 1534 ms <br /> 7.42 DMIPS | 2641 ms <br /> 83.3 FPS <br /> 4814 ms <br /> 53.5 FPS <br /> 11046 ms <br /> 23.7 FPS | 22 ms <br /> 385280 B <br /> 3 ms <br /> 77312 B     | ? ms <br /> ? B            | 4.15 sec <br /> 1280.0 KiB |
| V600     | 52 MHz | 1334 ms <br /> 3.12 BMIPS <br /> 1965 ms <br /> 5.79 DMIPS | 2640 ms <br /> 82.3 FPS <br /> 4922 ms <br /> 49.9 FPS <br /> 12073 ms <br /> 20.1 FPS | 43 ms <br /> 224000 B <br /> 2 ms <br /> 18176 B     | ? ms <br /> ? B            | 1.28 sec <br /> 524.0 KiB  |
| V600     | 65 MHz | 1070 ms <br /> 3.91 BMIPS <br /> 1559 ms <br /> 7.30 DMIPS | 2589 ms <br /> 83.6 FPS <br /> 3994 ms <br /> 62.5 FPS <br /> 9913 ms <br /> 25.5 FPS  | 32 ms <br /> 202752 B <br /> 1 ms <br /> 18176 B     | ? ms <br /> ? B            | 0.86 sec <br /> 544.0 KiB  |

See [Benchmark](../Benchmark) project documents for more stock results.

### Yeti3D table

| Phone    | Freq   | Screen  | Flags | Viewport <br /> Mode       | Optimized | FPS <br /> (avg, min-max) | ELF filename          |
|----------|--------|---------|-------|----------------------------|-----------|---------------------------|-----------------------|
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

* Motorola C650: R365_G_0B.D3.08R
* Motorola SLVR L6: R3511_G_0A.52.45R_A
* Motorola SLVR L6i: R3443H1_G_0A.65.0BR
* Motorola ROKR E1: R373_G_0E.30.49R
* Motorola V360: R4513_G_08.B7.ACR
* Motorola SLVR L7: R4513_G_08.B7.E0R_RB
* Motorola SLVR L7e: R452D_G_08.01.0AR (not working)
* Motorola KRZR K1:  R452F_G_08.03.08R (not working)
* Motorola RAZR V3i: R4441D_G_08.01.03R
* Motorola RAZR V3r: R4515_G_08.BD.D3R

Application type: GUI.

Please read the [Overclocking MCU in Motorola E398 and similar phones](https://forum.motofan.ru/index.php?showtopic=1742268) topic on MotoFan.Ru forum for additional information (in Russian).
