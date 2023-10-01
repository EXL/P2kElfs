del game.gba

set path=C:\devkitARM\bin;%path%

arm-eabi-gcc -Wl,--allow-multiple-definition -DPLATFORM_GBA -marm -mthumb-interwork -mlong-calls -O3 ^
	data.c yeti.c draw.c main.c -o game.elf

arm-eabi-objcopy -O binary game.elf game.gba

del game.elf
