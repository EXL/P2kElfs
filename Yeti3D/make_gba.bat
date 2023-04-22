@del game.gba

@set orgpath=%path%
@set path=C:\devkitadv\bin;%path%
@set CFLAGS=-marm -mthumb-interwork -mlong-calls -O1 -D __GBA__

del *.o

@REM Assemble ARM/Thumb Code
@REM =======================

@echo ====== Compiling Assembly Files
REM as -mthumb-interwork -o poly.text.iwram.o poly.text.iwram.s 

REM I still haven't been able to link this into Yeti3D
REM as -mthumb-interwork -o crt0.o crt0.s

@echo ====== Compiling C Files
gcc %CFLAGS% -c *.c ..\..\src\*.c 

@echo ====== Linking
gcc %CFLAGS% -o game.elf *.o 

objcopy -O binary game.elf game.gba

@del game.elf
@set path=%orgpath%
