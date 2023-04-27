:: This make.bat script was edited by EXL, 26-Mar-2023.
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
set ELF_NAME=Spout
set PCS_NAME=Piece

:: Compiling step.
%ARM_PATH%\tcc -I%SDK_PATH% %DEFINES% -DROT_90 -DFPS_30 -bigend -apcs /interwork -O2 -c %ELF_NAME%.c ^
	-o %ELF_NAME%.o
%ARM_PATH%\tcc -I%SDK_PATH% %DEFINES% -DROT_90 -DFPS_30 -bigend -apcs /interwork -O2 -c %PCS_NAME%.c ^
	-o %PCS_NAME%.o
:: Linking step.
%ARM_PATH%\armlink -nolocals -reloc -first %LIB_MAIN%(Lib) %ELF_NAME%.o %PCS_NAME%.o %LIB_PATH%\%LIB_MAIN% ^
	-o %ELF_NAME%.elf
:: Renaming step.
del *.o
move /y %ELF_NAME%.elf %ELF_NAME%_RH1.elf

:: Compiling step.
%ARM_PATH%\tcc -I%SDK_PATH% %DEFINES% -DROT_90 -DFPS_15 -bigend -apcs /interwork -O2 -c %ELF_NAME%.c ^
	-o %ELF_NAME%.o
%ARM_PATH%\tcc -I%SDK_PATH% %DEFINES% -DROT_90 -DFPS_15 -bigend -apcs /interwork -O2 -c %PCS_NAME%.c ^
	-o %PCS_NAME%.o
:: Linking step.
%ARM_PATH%\armlink -nolocals -reloc -first %LIB_MAIN%(Lib) %ELF_NAME%.o %PCS_NAME%.o %LIB_PATH%\%LIB_MAIN% ^
	-o %ELF_NAME%.elf
:: Renaming step.
del *.o
move /y %ELF_NAME%.elf %ELF_NAME%_RL1.elf

:: Compiling step.
%ARM_PATH%\tcc -I%SDK_PATH% %DEFINES% -DROT_0 -DFPS_30 -bigend -apcs /interwork -O2 -c %ELF_NAME%.c ^
	-o %ELF_NAME%.o
%ARM_PATH%\tcc -I%SDK_PATH% %DEFINES% -DROT_0 -DFPS_30 -bigend -apcs /interwork -O2 -c %PCS_NAME%.c ^
	-o %PCS_NAME%.o
:: Linking step.
%ARM_PATH%\armlink -nolocals -reloc -first %LIB_MAIN%(Lib) %ELF_NAME%.o %PCS_NAME%.o %LIB_PATH%\%LIB_MAIN% ^
	-o %ELF_NAME%.elf
:: Renaming step.
del *.o
move /y %ELF_NAME%.elf %ELF_NAME%_OH1.elf

:: Compiling step.
%ARM_PATH%\tcc -I%SDK_PATH% %DEFINES% -DROT_0 -DFPS_15 -bigend -apcs /interwork -O2 -c %ELF_NAME%.c ^
	-o %ELF_NAME%.o
%ARM_PATH%\tcc -I%SDK_PATH% %DEFINES% -DROT_0 -DFPS_15 -bigend -apcs /interwork -O2 -c %PCS_NAME%.c ^
	-o %PCS_NAME%.o
:: Linking step.
%ARM_PATH%\armlink -nolocals -reloc -first %LIB_MAIN%(Lib) %ELF_NAME%.o %PCS_NAME%.o %LIB_PATH%\%LIB_MAIN% ^
	-o %ELF_NAME%.elf
:: Renaming step.
del *.o
move /y %ELF_NAME%.elf %ELF_NAME%_OL1.elf

if /I "%1"=="clean" (
	del *.o
	del *.elf
)
