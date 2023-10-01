:: This make.bat script was created by EXL, 03-Sep-2023.
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
set DEFINES=-D__P2K__ -DEP1 -DLINUX_BOGOMIPS
:: set DEFINES=-D__P2K__ -DEP1 -DPALMOS_BOGOMIPS
:: set DEFINES=-D__P2K__ -DEP1 -DPALMOS_BOGOMIPS -DFTR_V600
:: set DEFINES=-D__P2K__ -DEP1 -DPALMOS_BOGOMIPS -DFTR_L7E

:: Project/ELF name.
set ELF_NAME=Benchmark

:: Compiling step.
%ARM_PATH%\armasm -32 -bi -apcs /interwork delay_armv4t_ADS.s delay_armv4t_ADS.o
%ARM_PATH%\tcc -I%SDK_PATH% %DEFINES% -bigend -apcs /interwork -O2 -c dhry_1.c -o dhry_1.o
%ARM_PATH%\tcc -I%SDK_PATH% %DEFINES% -bigend -apcs /interwork -O2 -c dhry_2.c -o dhry_2.o
%ARM_PATH%\tcc -I%SDK_PATH% %DEFINES% -bigend -apcs /interwork -O2 -c Phases.c -o Phases.o
%ARM_PATH%\tcc -I%SDK_PATH% %DEFINES% -bigend -apcs /interwork -O2 -c FireEffect.c -o FireEffect.o
%ARM_PATH%\tcc -I%SDK_PATH% %DEFINES% -bigend -apcs /interwork -O2 -c %ELF_NAME%.c -o %ELF_NAME%.o

:: Linking step.
%ARM_PATH%\armlink -nolocals -reloc -first %LIB_MAIN%(Lib) ^
	delay_armv4t_ADS.o dhry_1.o dhry_2.o Phases.o FireEffect.o %ELF_NAME%.o ^
	%LIB_PATH%\%LIB_MAIN% -o %ELF_NAME%.elf
