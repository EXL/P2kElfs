:: This make_em2.bat script was created by EXL, 03-Sep-2023.
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
set DEFINES=-D__P2K__ -DEM2 -DLINUX_BOGOMIPS
:: set DEFINES=-D__P2K__ -DEM2 -DPALMOS_BOGOMIPS

:: Includes.
set INCLUDES=-I.

:: Optimization.
set OPTIM=-O2

:: Project/ELF name.
set ELF_NAME=Benchmark

:: Compiling step.
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c delay_mcore340_GCC.S -o delay_mcore340_GCC.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c dhry_1.c -o dhry_1.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c dhry_2.c -o dhry_2.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c Phases.c -o Phases.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c FireEffect.c -o FireEffect.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c %ELF_NAME%.c -o %ELF_NAME%.o

:: Linking step.
%MCORE_PATH%\bin\mcore-elf-ld -d -EB -N -r -s -Bstatic -Bsymbolic -Bsymbolic-functions ^
	--gc-sections -nostdinc -nostdlib --unresolved-symbols=report-all -z muldefs -z combreloc -z nodefaultlib ^
	delay_mcore340_GCC.o dhry_1.o dhry_2.o Phases.o FireEffect.o %ELF_NAME%.o ^
	-T%MCORE_PATH%\mcore-elf\lib\linker_elf.ld -o %ELF_NAME%.elf
