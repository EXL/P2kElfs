:: This make_em2.bat script was created by EXL, 02-Dec-2024.
:: Default platform is Motorola P2K, EM2 ElfPack M*CORE, mcore-elf-gcc (GCC) 3.4.6 on Windows.

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
set MCORE_PATH=C:\MCORE_EM2

:: SDK path.
set SDK_PATH=%MCORE_PATH%\SDK

:: Defines.
set DEFINES=-D__P2K__ -DEM2 -DROT_0 -DFPS_30
:: set DEFINES=-D__P2K__ -DEM2 -DROT_0 -DFPS_30 -DFTR_V3X
:: set DEFINES=-D__P2K__ -DEM2 -DROT_0 -DFPS_30 -DG_DEFAULT

:: Includes.
set INCLUDES=-I.

:: Optimization.
set OPTIM=-O3

:: Project/ELF name.
set ELF_NAME=vNesC

:: Compiling step.
%MCORE_PATH%\bin\mcore-elf-gcc -std=c99 -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c vNesC.c -o vNesC.o
%MCORE_PATH%\bin\mcore-elf-gcc -std=c99 -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c vP2k.c -o vP2k.o

:: Linking step.
%MCORE_PATH%\bin\mcore-elf-ld -d -EB -N -r -s -Bstatic -Bsymbolic -Bsymbolic-functions ^
	--gc-sections -nostdinc -nostdlib --unresolved-symbols=report-all -z muldefs -z combreloc -z nodefaultlib ^
	vNesC.o vP2k.o ^
	-T%MCORE_PATH%\mcore-elf\lib\linker_elf.ld -o %ELF_NAME%.elf
