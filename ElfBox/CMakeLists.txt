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
# Project section: ElfBox
#
project(ElfBox LANGUAGES C VERSION "1.0.0")

include_directories(${PROJECT_SOURCE_DIR})

add_definitions("-DFTR_V600")
#add_definitions("-DSHOW_ELF_ICONS")
#add_definitions("-DASYNC")
#add_definitions("-DFTR_C650")
#add_definitions("-DDEBUG")

set(SOURCE_FILES
	ElfBox.c)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

########################################################################################################################
# Linking and postlinking section
#
if(ELFPACK STREQUAL "EP1")
	#target_link_libraries(${PROJECT_NAME} "${P2K_DIRECTORY_LIBRARIES}/dbg/dbg.o")
elseif(ELFPACK STREQUAL "EP2")
	#target_link_libraries(${PROJECT_NAME} "${P2K_DIRECTORY_LIBRARIES}/libgfx.a")
	include("${P2K_EP2_TOOLCHAIN_ROOT}/cmake/PostLink.cmake")
elseif(ELFPACK STREQUAL "EM1")
	#target_link_libraries(${PROJECT_NAME} "${P2K_DIRECTORY_LIBRARIES}/libc.a")
elseif(ELFPACK STREQUAL "EM2")
	#target_link_libraries(${PROJECT_NAME} "${P2K_DIRECTORY_LIBRARIES}/libc.a")
endif()
