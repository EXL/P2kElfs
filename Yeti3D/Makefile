all: linux

linux:
	gcc `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c data.c -o data.o
	gcc `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c draw.c -o draw.o
	gcc `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c extra.c -o extra.o
	gcc `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c font.c -o font.o
	gcc `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c game.c -o game.o
	gcc `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c main.c -o main.o
	gcc `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c maps.c -o maps.o
	gcc `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c model.c -o model.o
	gcc `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c sprites.c -o sprites.o
	gcc `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c yeti.c -o yeti.o
	gcc -O2 \
		data.o draw.o extra.o font.o game.o main.o maps.o model.o sprites.o yeti.o \
		-o yeti3d.elf `sdl-config --libs`

clean:
	-rm -f *.o
	-rm -f *.elf
	-rm -f *.exe
