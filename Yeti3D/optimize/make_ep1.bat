:: This make.bat script was created by EXL, 23-Apr-2023.
:: Default platform is Motorola P2K, ElfPack v1.x, ADS1.2 [Build 848] on Windows.
:: Warning: `-nodebug` flag option for `armlink` is buggy.

:: Uncomment it for disable verbose output.
:: @echo off

if /I "%1"=="clean" (
	if exist *.o    del *.o
	if exist *.obj  del *.obj
	if exist *.elfp del *.elfp
	if exist *.elf  del *.elf
	exit /b 0
)

:: Compiler path.
set ARM_PATH=C:\ARM

:: SDK path.
set SDK_PATH=%ARM_PATH%\SDK

:: Libraries path.
set LIB_PATH=%ARM_PATH%\lib

:: Main link library.
set LIB_MAIN=Lib.o

:: Defines.
set DEFINES=-D__P2K__ -DEP1 -DROT_90 -DFPS_30
:: set DEFINES=-D__P2K__ -DEP1 -DROT_90 -DFPS_30 -DFTR_V600 -DJAVA_HEAP
:: set DEFINES=-DDEBUG
:: set DEFINES=-DG_DEFAULT
:: set DEFINES=-DFPS_METER
:: set DEFINES=-DMEMORY_MANUAL_ALLOCATION
:: set DEFINES=-DSEARCH_LONG_RANGE

:: Project/ELF name.
set ELF_NAME=Yeti3D

:: Compiling step.
%ARM_PATH%\tcc -I%SDK_PATH% %DEFINES% -bigend -apcs /interwork -O2 -c draw.c -o draw.o
%ARM_PATH%\tcc -I%SDK_PATH% %DEFINES% -bigend -apcs /interwork -O2 -c game.c -o game.o
%ARM_PATH%\tcc -I%SDK_PATH% %DEFINES% -bigend -apcs /interwork -O2 -c yeti.c -o yeti.o
%ARM_PATH%\tcc -I%SDK_PATH% %DEFINES% -bigend -apcs /interwork -O2 -c main_p2k.c -o main_p2k.o

:: Linking step.
%ARM_PATH%\armlink -nolocals -reloc -first %LIB_MAIN%(Lib) draw.o game.o yeti.o main_p2k.o ^
	%LIB_PATH%\%LIB_MAIN% -o %ELF_NAME%.elf
