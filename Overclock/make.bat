:: This make.bat script was edited by EXL, 07-Sep-2023.
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
:: set DEFINES=-D__P2K__ -DEP1 -DFTR_V600
:: set DEFINES=-D__P2K__ -DEP1 -DFTR_L7E
:: set DEFINES=-D__P2K__ -DEP1 -DDISABLE_ALL_INT

:: ELF name.
set ELF_NAME=Overclock

:: Compiling step.
%ARM_PATH%\tcc -I%SDK_PATH% %DEFINES% -bigend -apcs /interwork -O2 -c Clock.c -o Clock.o
%ARM_PATH%\tcc -I%SDK_PATH% %DEFINES% -bigend -apcs /interwork -O2 -c %ELF_NAME%.c -o %ELF_NAME%.o

:: Linking step.
%ARM_PATH%\armlink -nolocals -reloc -first %LIB_MAIN%(Lib) ^
	Clock.o %ELF_NAME%.o ^
	%LIB_PATH%\%LIB_MAIN% -o %ELF_NAME%.elf

if /I "%1"=="clean" (
	del *.o
	del *.elf
)
