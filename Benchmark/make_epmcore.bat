:: This make.bat script was edited by EXL, 03-Jun-2023.
:: Default platform is Motorola P2K, ElfPack M*CORE, mcore-elf-gcc (GCC) 3.4.6 on Windows.

:: Uncomment it for disable verbose output.
:: @echo off

:: Compiler path.
set MCORE_PATH=C:\MCORE

:: SDK path.
set SDK_PATH=%MCORE_PATH%\SDK

:: Defines.
set DEFINES=-D__P2K__ -DEPMCORE

:: Includes.
set INCLUDES=-I.

:: Optimization.
set OPTIM=-O2

:: Output names.
set ELF_NAME=Benchmark
set FINAL_NAME=Benchmark_MCORE


:: Compiling step.
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% ^
	-c %ELF_NAME%.c -o %ELF_NAME%.o

:: Linking step.
%MCORE_PATH%\bin\mcore-elf-ld -d -EB -N -r -s -Bstatic -Bsymbolic -Bsymbolic-functions ^
	--gc-sections -nostdinc -nostdlib --unresolved-symbols=report-all -z muldefs -z combreloc -z nodefaultlib ^
	%ELF_NAME%.o -T%MCORE_PATH%\mcore-elf\lib\linker_elf.ld -o %ELF_NAME%.elf

:: Renaming step.
move /y %ELF_NAME%.elf %FINAL_NAME%.elf

if /I "%1"=="clean" (
	del *.o
	del *.elf
)
