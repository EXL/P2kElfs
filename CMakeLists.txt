########################################################################################################################
# CMake build script for Motorola P2K ELFs: 02-Oct-2023
#
cmake_minimum_required(VERSION 3.16)
if(NOT ELFPACK)
	set(ELFPACK "EP1")
endif()
if(NOT ARM_MODE)
	set(ARM_MODE "Thumb")
endif()
if(ELFPACK STREQUAL "EP1") # Motorola P2K, ARM/Thumb, ElfPack v1.x, ADS1.2 [Build 848] on Windows and Linux.
	if (WIN32)
		set(CMAKE_TOOLCHAIN_FILE "C:/ARM/cmake/P2K_EP1_Toolchain.cmake")
	else()
		set(CMAKE_TOOLCHAIN_FILE "/opt/arm/cmake/P2K_EP1_Toolchain.cmake")
	endif()
elseif(ELFPACK STREQUAL "EP2") # ElfPack v2.x, ARM/Thumb, devkitARM release 26 (GCC 4.4.0) on Windows.
	set(CMAKE_TOOLCHAIN_FILE "C:/devkitARM/cmake/P2K_EP2_Toolchain.cmake")
elseif(ELFPACK STREQUAL "EM1") # ElfPack v1.x, M*CORE, mcore-elf-gcc (GCC) 3.4.6 on Windows.
	set(CMAKE_TOOLCHAIN_FILE "C:/MCORE_EM1/cmake/P2K_EM1_Toolchain.cmake")
elseif(ELFPACK STREQUAL "EM2") # ElfPack v2.x, M*CORE, mcore-elf-gcc (GCC) 3.4.6 on Windows.
	set(CMAKE_TOOLCHAIN_FILE "C:/MCORE_EM2/cmake/P2K_EM2_Toolchain.cmake")
endif()

########################################################################################################################
# Project section: General
#
project(P2kElfs)

add_subdirectory(Ambilight)
add_subdirectory(BattDump)
add_subdirectory(Benchmark)
add_subdirectory(Dumper)
add_subdirectory(ElfBox)
add_subdirectory(FireEffect)
add_subdirectory(Overclock)
add_subdirectory(Perlin)
add_subdirectory(PowerAlert)
add_subdirectory(Screenshot)
add_subdirectory(Spout)
add_subdirectory(VibroHaptic)
add_subdirectory(Yeti3D-Old)
add_subdirectory(Yeti3D)
