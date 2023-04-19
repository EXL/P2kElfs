del game.gba

set path=C:\devkitadv\bin;%path%

g++ -DPLATFORM_GBA -marm -mthumb-interwork -mlong-calls -O3 -o game.elf data.cpp yeti.cpp draw.cpp main.cpp

objcopy -O binary game.elf game.gba

del game.elf
