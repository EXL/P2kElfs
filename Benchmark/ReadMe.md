Benchmark
=========

A simple ELF benchmarking application for Motorola P2K phones.

## Screenshots from Motorola SLVR L6

![Screenshot of Benchmark from Motorola L6 1](../images/Screenshot_Benchmark_L6_1.png) ![Screenshot of Benchmark from Motorola L6 2](../images/Screenshot_Benchmark_L6_2.png) ![Screenshot of Benchmark from Motorola L6 3](../images/Screenshot_Benchmark_L6_3.png) ![Screenshot of Benchmark from Motorola L6 4](../images/Screenshot_Benchmark_L6_4.png)

![Screenshot of Benchmark from Motorola L6 5](../images/Screenshot_Benchmark_L6_5.png) ![Screenshot of Benchmark from Motorola L6 6](../images/Screenshot_Benchmark_L6_6.png) ![Screenshot of Benchmark from Motorola L6 7](../images/Screenshot_Benchmark_L6_7.png) ![Screenshot of Benchmark from Motorola L6 8](../images/Screenshot_Benchmark_L6_8.png)

## Screenshots from Motorola V600

![Screenshot of Benchmark from Motorola V600 1](../images/Screenshot_Benchmark_V600_1.png) ![Screenshot of Benchmark from Motorola V600 2](../images/Screenshot_Benchmark_V600_2.png) ![Screenshot of Benchmark from Motorola V600 3](../images/Screenshot_Benchmark_V600_3.png) ![Screenshot of Benchmark from Motorola V600 4](../images/Screenshot_Benchmark_V600_4.png)

![Screenshot of Benchmark from Motorola V600 5](../images/Screenshot_Benchmark_V600_5.png) ![Screenshot of Benchmark from Motorola V600 6](../images/Screenshot_Benchmark_V600_6.png) ![Screenshot of Benchmark from Motorola V600 7](../images/Screenshot_Benchmark_V600_7.png)

## Screenshots from Motorola ROKR E1

![Screenshot of Benchmark from Motorola E1 1](../images/Screenshot_Benchmark_E1_1.png) ![Screenshot of Benchmark from Motorola E1 2](../images/Screenshot_Benchmark_E1_2.png) ![Screenshot of Benchmark from Motorola E1 3](../images/Screenshot_Benchmark_E1_3.png) ![Screenshot of Benchmark from Motorola E1 4](../images/Screenshot_Benchmark_E1_4.png)

![Screenshot of Benchmark from Motorola E1 5](../images/Screenshot_Benchmark_E1_5.png) ![Screenshot of Benchmark from Motorola E1 6](../images/Screenshot_Benchmark_E1_6.png) ![Screenshot of Benchmark from Motorola E1 7](../images/Screenshot_Benchmark_E1_7.png)

## Videos of benchmarking Motorola phones

See [Overclocking and Benchmakring old Motorola P2K phones (52 MHz to 65 MHz)](https://youtu.be/IO8aktssBo8) video on YouTube.

## Benchmarking results

| Phone    | Set   | Elf | CPU (MCU)                                                  | GPU (IPU)                                                                              | RAM (SRAM)                                           | HEAP (J2ME)                | S/W & H/W                                                                                                                                                                                                                 |
|----------|-------|-----|------------------------------------------------------------|----------------------------------------------------------------------------------------|------------------------------------------------------|----------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| SLVR L6i | thumb | EP1 | 1318 ms <br /> 3.17 BMIPS <br /> 2124 ms <br /> 5.35 DMIPS | 3919 ms <br /> 52.6 FPS <br /> 7301 ms <br /> 33.5 FPS <br /> 14380 ms <br /> 16.4 FPS | 159 ms <br /> 1227520 B <br /> 13 ms <br /> 117504 B | 4.11 sec <br /> 1112.0 KiB | R3443H1_0BR <br /> 128x160 cSTN @ 51 Hz @ RGB565 <br /> ATI Imageon W2240 <br /> T: 196608, I: 196608, E: 0 <br /> I: 41280, E: 0, S: 0 <br /> Neptune LTE @ 52 MHz <br /> 8 MB RAM, 256 KB iRAM, 32 MB ROM               |
| SLVR L6i | arm   | EP1 | 1311 ms <br /> 3.19 BMIPS <br /> 3036 ms <br /> 3.74 DMIPS | 5185 ms <br /> 43.8 FPS <br /> 7671 ms <br /> 29.9 FPS <br /> 18552 ms <br /> 13.6 FPS | 168 ms <br /> 1227520 B <br /> 13 ms <br /> 117504 B | 4.22 sec <br /> 1112.0 KiB | R3443H1_0BR <br /> 128x160 cSTN @ 51 Hz @ RGB565 <br /> ATI Imageon W2240 <br /> T: 196608 @ I: 196608 @ E: 0 <br /> I: 41280 @ E: 0, S: 0 <br /> Neptune LTE @ 52 MHz <br /> 8 MB RAM, 256 KB iRAM, 32 MB ROM            |
| ROKR E1  | thumb | EP1 | 1317 ms <br /> 3.17 BMIPS <br /> 2094 ms <br /> 5.43 DMIPS | 3425 ms <br /> 62.5 FPS <br /> 6279 ms <br /> 39.5 FPS <br /> 13635 ms <br /> 17.7 FPS | 22 ms <br /> 524800 B <br /> 2 ms <br /> 77312 B     | 7.39 sec <br /> 1060.0 KiB | R373_49R <br /> 176x220 TFT @ 60 Hz @ RGB565 <br /> ATI Imageon W2250 <br /> T: 196608, I: 196608, E: 0 <br /> I: 3320, E: 0, S: 0 <br /> Neptune LTE @ 52 MHz <br /> 8 MB RAM, 256 KB iRAM, 32 MB ROM                    |
| ROKR E1  | arm   | EP1 | 1318 ms <br /> 3.17 BMIPS <br /> 3020 ms <br /> 3.76 DMIPS | 4895 ms <br /> 52.3 FPS <br /> 7744 ms <br /> 31.5 FPS <br /> 17113 ms <br /> 14.8 FPS | 27 ms <br /> 526336 B <br /> 3 ms <br /> 77312 B     | 7.46 sec <br /> 1060.0 KiB | R373_49R <br /> 176x220 TFT @ 60 Hz @ RGB565 <br /> ATI Imageon W2250 <br /> T: 196608, I: 196608, E: 0 <br /> I: 3320, E: 0, S: 0 <br /> Neptune LTE @ 52 MHz <br /> 8 MB RAM, 256 KB iRAM, 32 MB ROM                    |
| ROKR E1  | thumb | EP2 | 1300 ms <br /> 3.21 BMIPS <br /> 1812 ms <br /> 6.28 DMIPS | 3332 ms <br /> 62.5 FPS <br /> 6246 ms <br /> 39.5 FPS <br /> 12720 ms <br /> 18.4 FPS | 29 ms <br /> 433920 B <br /> 2 ms <br /> 77312 B     | 6.95 sec <br /> 1060.0 KiB | R373_49R <br /> 176x220 TFT @ 60 Hz @ RGB565 <br /> ATI Imageon W2250 <br /> T: 196608, I: 196608, E: 0 <br /> I: 3320, E: 0, S: 0 <br /> Neptune LTE @ 52 MHz <br /> 8 MB RAM, 256 KB iRAM, 32 MB ROM                    |
| SLVR L7  | thumb | EP1 | 1304 ms <br /> 3.21 BMIPS <br /> 1909 ms <br /> 5.96 DMIPS | 3316 ms <br /> 62.5 FPS <br /> 6365 ms <br /> 39.6 FPS <br /> 13239 ms <br /> 17.1 FPS | 38 ms <br /> 514048 B <br /> 4 ms <br /> 77312 B     | 1.56 sec <br /> 296.0 KiB  | R4513_E0R_RB <br /> 176x220 TFT @ 60 Hz @ RGB565 <br /> ATI Imageon W2240 <br /> T: 196608, I: 196608, E: 0 <br /> I: 4928, E: 0, S: 0 <br /> Neptune LTE2 @ 52 MHz <br /> 8 MB RAM, 512 KB iRAM, 32 MB ROM               |
| V360     | thumb | EP1 | 1304 ms <br /> 3.21 BMIPS <br /> 1915 ms <br /> 5.94 DMIPS | 3506 ms <br /> 57.3 FPS <br /> 6364 ms <br /> 39.5 FPS <br /> 13704 ms <br /> 17.2 FPS | 36 ms <br /> 474624 B <br /> 4 ms <br /> 77312 B     | 1.83 sec <br /> 656.0 KiB  | R4513_ACR <br /> 176x220 TFT @ 60 Hz @ RGB565 <br /> ATI Imageon W2240 <br /> T: 196608, I: 196608, E: 0 <br /> I: 4929, E: 0, SYSB: 0 <br /> Neptune LTE2 @ 52 MHz <br /> 8 MB RAM, 512 KB iRAM, 32 MB ROM               |
| SLVR L7e | thumb | EP1 | ? ms <br /> ? ms <br /> 926 ms <br /> 12.29 DMIPS          | 3309 ms <br /> 62.5 FPS <br /> 6022 ms <br /> 39.5 FPS <br /> 9554 ms <br /> 23.2 FPS  | 89 ms <br /> 3359232 B <br /> 5 ms <br /> 230144 B   | 6.95 sec <br /> 1172.0 KiB | R452D_0AR <br /> 176x220 TFT @ 60 Hz @ RGB565 <br /> ATI Imageon W2282 <br /> T: 2293760, I: 196608, E: 2097152 <br /> I: 42112, E: 1009152, S: 0 <br /> Neptune LTE2 @ 52? MHz <br /> 16 MB RAM, 512 KB iRAM, 64 MB ROM  |
| KRZR K1  | thumb | EP1 | ? ms <br /> ? ms <br /> 941 ms <br /> 12.09 DMIPS          | 2209 ms <br /> 88.5 FPS <br /> 4321 ms <br /> 51.9 FPS <br /> 9180 ms <br /> 24.5 FPS  | 111 ms <br /> 2231808 B <br /> 5 ms <br /> 127744 B  | 4.09 sec <br /> 952.0 KiB  | R452F_08R <br /> 176x220 TFT @ 90 Hz @ RGB565 <br /> ATI Imageon W2282 <br /> T: 2293760, I: 196608, E: 2097152 <br /> I: 42112, E: 1009152, S: 0 <br /> Neptune LTE2 @ 52? MHz <br /> 16 MB RAM, 512 KB iRAM, 64 MB ROM  |
| RIZR Z3  | thumb | EP1 | ? ms <br /> ? ms <br /> 926 ms <br /> 12.29 DMIPS          | 3333 ms <br /> 62.5 FPS <br /> 6012 ms <br /> 39.5 FPS <br /> 9674 ms <br /> 23.1 FPS  | ? ms <br /> ? B <br /> ? ms <br /> ? B               | ? sec <br /> ? KiB         | R452F1_09R <br /> 176x220 TFT @ 60 Hz @ RGB565 <br /> ATI Imageon W2282 <br /> T: 2293760, I: 196608, E: 2097152 <br /> I: 42112, E: 1009152, S: 0 <br /> Neptune LTE2 @ 52? MHz <br /> 16 MB RAM, 512 KB iRAM, 64 MB ROM |
| RAZR V3i | thumb | EP1 | 1313 ms <br /> 3.17 BMIPS <br /> 1929 ms <br /> 5.90 DMIPS | 3947 ms <br /> 55.5 FPS <br /> 6888 ms <br /> 35.1 FPS <br /> 13768 ms <br /> 15.9 FPS | 71 ms <br /> 1091584 B <br /> 5 ms <br /> 112384 B   | 5.44 sec <br /> 1192.0 KiB | R4441D_03R <br /> 176x220 TFT @ 81 Hz @ RGB565 <br /> ATI Imageon W2262 <br /> T: 2490368, I: 393216, E: 2097152 <br /> I: 141888, E: 1009856, S: 0 <br /> Neptune LTE2 @ 52 MHz <br /> 8 MB RAM, 512 KB iRAM, 48 MB ROM  |
| RAZR V3r | thumb | EP1 | 1304 ms <br /> 3.21 BMIPS <br /> 1914 ms <br /> 5.94 DMIPS | 4461 ms <br /> 51.5 FPS <br /> 6054 ms <br /> 42.5 FPS <br /> 13234 ms <br /> 19.0 FPS | 28 ms <br /> 385280 B <br /> 4 ms <br /> 77312 B     | 5.21 sec <br /> 1280.0 KiB | R4515_D3R <br /> 176x220 TFT @ 81 Hz @ RGB565 <br /> ATI Imageon W2250 <br /> T: 196608, I: 196608, E: 0 <br /> I: 4608, E: 0, S: 0 <br /> Neptune LTE2 @ 52 MHz <br /> 8 MB RAM, 512 KB iRAM, 32 MB ROM                  |
| V300     | thumb | EP1 | 1347 ms <br /> 3.11 BMIPS <br /> 1971 ms <br /> 5.77 DMIPS | 2668 ms <br /> 77.2 FPS <br /> 4950 ms <br /> 50.0 FPS <br /> 11888 ms <br /> 20.5 FPS | 43 ms <br /> 224000 B <br /> 2 ms <br /> 18176 B     | 1.07 sec <br /> 544.0 KiB  | TRIPLETS_72R <br /> 176x220 TFT @ 60 Hz @ RGB565 <br /> ATI Imageon W2250 <br /> T: 196608, I: 196608, E: 0 <br /> I: 4600, E: 0, S: 0 <br /> Neptune LTE @ 52 MHz <br /> 4 MB RAM, 256 KB iRAM, 32 MB ROM                |
| V600     | thumb | EP1 | 1334 ms <br /> 3.12 BMIPS <br /> 1965 ms <br /> 5.79 DMIPS | 2640 ms <br /> 82.3 FPS <br /> 4922 ms <br /> 49.9 FPS <br /> 12073 ms <br /> 20.1 FPS | 43 ms <br /> 224000 B <br /> 2 ms <br /> 18176 B     | 1.28 sec <br /> 524.0 KiB  | TRIPLETS_72R <br /> 176x220 TFT @ 60 Hz @ RGB565 <br /> ATI Imageon W2250 <br /> T: 196608, I: 196608, E: 0 <br /> I: 4600, E: 0, S: 0 <br /> Neptune LTE @ 52 MHz <br /> 4 MB RAM, 256 KB iRAM, 32 MB ROM                |

See [Overclock](../Overclock) project documents for overclocking Benchmark comparision.

### Disks/Volumes benchmarking results

Thanks to @fkcoder, @baat, and @Unreal_man for the tests.

![Screenshot of Benchmark from Motorola E1 8](../images/Screenshot_Benchmark_E1_8.png) ![Screenshot of Benchmark from Motorola E1 9](../images/Screenshot_Benchmark_E1_9.png)

| Phone             | /a/ <br /> Intel FDI <br />  128 KiB File                                             | /c/ <br /> Intel MMM <br /> 128 KiB File                                         | /b/ <br /> FAT <br /> 128 KiB File                                                  |
|-------------------|---------------------------------------------------------------------------------------|----------------------------------------------------------------------------------|-------------------------------------------------------------------------------------|
| SLVR L6i          | 4K, W:4318, R:162, ms. <br /> 8K, W:3935, R:129, ms. <br /> 16K, W:3568, R:107, ms.   | 4K, W:445, R:39, ms. <br /> 8K, W:420, R:26, ms. <br /> 16K, W:405, R:20, ms.    | (None)                                                                              |
| ROKR E1           | 4K, W:4662, R:152, ms. <br /> 8K, W:4138, R:107, ms. <br /> 16K, W:3765, R:100, ms.   | 4K, W:795, R:38, ms. <br /> 8K, W:767, R:26, ms. <br /> 16K, W:758, R:20, ms.    | 4K, W:511, R:758, ms. <br /> 8K, W:500, R:628, ms. <br /> 16K, W:421, R:560, ms.    |
| SLVR L7           | 4K, W:4886, R:161, ms. <br /> 8K, W:4618, R:123, ms. <br /> 16K, W:4315, R:100, ms.   | 4K, W:974, R:35, ms. <br /> 8K, W:959, R:25, ms. <br /> 16K, W:938, R:19, ms.    | 4K, W:308, R:190, ms. <br /> 8K, W:232, R:168, ms. <br /> 16K, W:320, R:159, ms.    |
| SLVR L7e          | 4K, W:3815, R:147, ms. <br /> 8K, W:3473, R:115, ms. <br /> 16K, W:3138, R:98, ms.    | 4K, W:843, R:38, ms. <br /> 8K, W:822, R:26, ms. <br /> 16K, W:807, R:19, ms.    | 4K, W:302, R:266, ms. <br /> 8K, W:183, R:200, ms. <br /> 16K, W:117, R:157, ms.    |
| V300              | 4K, W:8066, R:170, ms. <br /> 8K, W:6093, R:129, ms. <br /> 16K, W:5739, R:114, ms.   | (None)                                                                           | (None)                                                                              |
| V600              | 4K, W:8152, R:174, ms. <br /> 8K, W:6078, R:134, ms. <br /> 16K, W:5804, R:115, ms.   | (None)                                                                           | (None)                                                                              |
| V600 (65 MHz)     | 4K, W:6010, R:131, ms. <br /> 8K, W:6188, R:102, ms. <br /> 16K, W:5289, R:86, ms.    | (None)                                                                           | (None)                                                                              |

Benchmarking results on Motorola ROKR E1 with custom [Dream A Reality 2.x Test 16](https://forum.motofan.ru/index.php?showtopic=167677) firmware: 52 MHz vs. 65 MHz MCU overclock.

![Screenshot of Benchmark from Motorola E1 10](../images/Screenshot_Benchmark_E1_10.jpg) ![Screenshot of Benchmark from Motorola E1 11](../images/Screenshot_Benchmark_E1_11.jpg)

## Other versions

* Various signal e.g. vibromotor finding version using binary search. See [misc/VibroSignal](misc/VibroSignal) utility project in this directory.
* Some deprecated overclocking MCU hacks. See [misc/OverclockCpu](misc/OverclockCpu) utility project in this directory.
* Strange reboot case for Motorola SLVR L7e by pressing "5" button. See [misc/DebugL7e](misc/DebugL7e) utility project in this directory.

## ELF files

* Benchmark.elf (ELF for ElfPack 1.0, thumb)
* Benchmark_arm.elf (ELF for ElfPack 1.0, arm)
* Benchmark_EP2.elf (ELF for ElfPack 2.0, thumb)
* Benchmark_EM1.elf (ELF for ElfPack 1.0, M·CORE)
* Benchmark_EM2.elf (ELF for ElfPack 2.0, M·CORE)
* Benchmark_L7e.elf (ELF for ElfPack 1.0, thumb, Motorola SLVR L7e version)
* Benchmark_V600.elf (ELF for ElfPack 1.0, thumb, Motorola V600, version)
* VibroSignal.elf (ELF for ElfPack 1.0)
* OverclockCpu.elf (ELF for ElfPack 1.0)
* DebugL7e.elf (ELF for ElfPack 1.0)

## Additional information

The ELF-application has been tested on the following phones and firmware:

* Motorola SLVR L6: R3511_G_0A.52.45R_A
* Motorola SLVR L6i: R3443H1_G_0A.65.0BR
* Motorola ROKR E1: R373_G_0E.30.49R
* Motorola V360: R4513_G_08.B7.ACR
* Motorola SLVR L7: R4513_G_08.B7.E0R_RB
* Motorola SLVR L7e: R452D_G_08.01.0AR (some bugs)
* Motorola KRZR K1: R452F_G_08.03.08R (some bugs)
* Motorola RIZR Z3: R452F1_G_08.04.09R (some bugs)
* Motorola RAZR V3i: R4441D_G_08.01.03R
* Motorola RAZR V3r: R4515_G_08.BD.D3R

Application type: GUI + ATI + Java Heap.
