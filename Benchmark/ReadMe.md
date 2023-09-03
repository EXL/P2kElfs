Benchmark
=========

A simple ELF benchmarking application for Motorola P2K phones.

## Screenshots from Motorola SLVR L6

![Screenshot of Benchmark from Motorola L6 1](../images/Screenshot_Benchmark_L6_1.png) ![Screenshot of Benchmark from Motorola L6 2](../images/Screenshot_Benchmark_L6_2.png) ![Screenshot of Benchmark from Motorola L6 3](../images/Screenshot_Benchmark_L6_3.png) ![Screenshot of Benchmark from Motorola L6 4](../images/Screenshot_Benchmark_L6_4.png) ![Screenshot of Benchmark from Motorola L6 5](../images/Screenshot_Benchmark_L6_5.png) ![Screenshot of Benchmark from Motorola L6 6](../images/Screenshot_Benchmark_L6_6.png)

## Screenshots from Motorola ROKR E1

![Screenshot of Benchmark from Motorola E1 1](../images/Screenshot_Benchmark_E1_1.png) ![Screenshot of Benchmark from Motorola E1 2](../images/Screenshot_Benchmark_E1_2.png) ![Screenshot of Benchmark from Motorola E1 3](../images/Screenshot_Benchmark_E1_3.png) ![Screenshot of Benchmark from Motorola E1 4](../images/Screenshot_Benchmark_E1_4.png) ![Screenshot of Benchmark from Motorola E1 5](../images/Screenshot_Benchmark_E1_5.png) ![Screenshot of Benchmark from Motorola E1 6](../images/Screenshot_Benchmark_E1_6.png)

## Videos of benchmarking Motorola phones

See [Benchmark ELF on Motorola P2K phones](https://www.youtube.com/TODO) video on YouTube.

## Benchmarking results

| Phone    | SET    | CPU (MCU)               | GPU (IPU)                      | RAM (SRAM)                            | HEAP (J2ME)          | Hardware                                                                                 |
| ---      | ---    | ---                     | ---                            | ---                                   | ---                  | ---                                                                                      |
| SLVR L6  | thumb  | 2.85 BMIPS | 5.34 DMIPS | 52.6 FPS | 25.7 FPS | 17.1 FPS | 158 ms - 1222912 B | 12 ms - 117504 B | 3.7 sec | 1108.0 KiB | 128x160 cSTN @ 51 Hz, ATI Imageon W2240, Neptune LTE @ 52 MHz, 8 MB RAM, 256 KB iRAM     |
| ROKR E1  | thumb  | 2.85 BMIPS | 5.43 DMIPS | 62.5 FPS | 35.0 FPS | 17.1 FPS | 30 ms - 505088 B   | 3 ms 77312 B     | 6.9 sec | 1060.0 KiB | 176x220 TFT @ 60 Hz, ATI Imageon W2250, Neptune LTE @ 52 MHz, 8 MB RAM, 256 KB iRAM      |


* Various signal e.g. vibromotor finding version using binary search. See [find_vibromotor_signal](https://github.com/EXL/P2kElfs/tree/find_vibromotor_signal) branch in this repository.

## ELF files

* Benchmark_EP1.elf (ELF for ElfPack 1.0, thumb)
* Benchmark_EP1_arm.elf (ELF for ElfPack 1.0, arm)
* Benchmark_EP2.elf (ELF for ElfPack 2.0, thumb)
* Benchmark_EP2_arm.elf (ELF for ElfPack 2.0, arm)
* Benchmark_EM1.elf (ELF for ElfPack 1.0, M·CORE)
* Benchmark_EM2.elf (ELF for ElfPack 2.0, M·CORE)
* Benchmark_L7e.elf (ELF for ElfPack 1.0, thumb, Motorola SLVR L7e version)
* Benchmark_V300.elf (ELF for ElfPack 1.0, thumb, Motorola V300, version)

## Additional information

The ELF-application has been tested on the following phones and firmware:

* Motorola SLVR L6: R3443H1_G_0A.65.0BR
* Motorola ROKR E1: R373_G_0E.30.49R

Application type: GUI + ATI + Java Heap.
