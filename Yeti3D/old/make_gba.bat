del game.gba

set path=C:\devkitARM\bin;%path%

arm-eabi-gcc -Wl,--allow-multiple-definition -DPLATFORM_GBA -marm -mthumb-interwork -mlong-calls -O3 -o game.elf data.c yeti.c draw.c main.c

arm-eabi-objcopy -O binary game.elf game.gba

del game.elf
