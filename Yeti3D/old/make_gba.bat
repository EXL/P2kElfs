del game.gba

set path=C:\devkitARM\bin;%path%

gcc -DPLATFORM_GBA -marm -mthumb-interwork -mlong-calls -O3 -o game.elf data.c yeti.c draw.c main.c

objcopy -O binary game.elf game.gba

del game.elf
