:: This make.bat script was edited by EXL, 01-Dec-2022.
:: Default platform is Motorola P2K, ElfPack v1.x, ADS1.2 [Build 848] on Windows.
:: Warning: `-nodebug` flag option for `armlink` is buggy.

:: Uncomment it for verbose output.
:: @echo off

:: Compiler path.
set ARM_PATH=C:\ARM

:: SDK path.
set SDK_PATH=%ARM_PATH%\SDK

:: Libraries path.
set LIB_PATH=%ARM_PATH%\lib

:: Main link library.
set LIB_MAIN=Lib.o

:: Defines.
set DEFINES=-D__P2K__ -DEP1

:: ELF name.
set ELF_NAME=Benchmark

:: Compiling step.
%ARM_PATH%\armcc -I%SDK_PATH% %DEFINES% -bigend -apcs /interwork -O0 -c %ELF_NAME%.c -o %ELF_NAME%.o
%ARM_PATH%\armcc -I%SDK_PATH% %DEFINES% -bigend -apcs /interwork -O0 -c Phases.c -o Phases.o

:: Linking step.
%ARM_PATH%\armlink -nolocals -reloc -first %LIB_MAIN%(Lib) %ELF_NAME%.o Phases.o %LIB_PATH%\%LIB_MAIN% -o %ELF_NAME%.elf

if /I "%1"=="clean" (
	del *.o
	del %ELF_NAME%.elf
)
