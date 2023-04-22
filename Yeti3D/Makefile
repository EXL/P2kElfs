all: linux

linux:
	gcc -g `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c data.c -o data.o
	gcc -g `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c draw.c -o draw.o
	gcc -g `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c extra.c -o extra.o
	gcc -g `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c font.c -o font.o
	gcc -g `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c game.c -o game.o
	gcc -g `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c maps.c -o maps.o
	gcc -g `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c model.c -o model.o
	gcc -g `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c sprites.c -o sprites.o
	gcc -g `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c yeti.c -o yeti.o
	gcc -g `sdl-config --cflags` -O2 -DPLATFORM_SDL -D__GBA__ -c main_sdl.c -o main_sdl.o
	gcc -g -O2 \
		data.o draw.o extra.o font.o game.o maps.o model.o sprites.o yeti.o main_sdl.o \
		-o yeti3d.elf `sdl-config --libs`

clean:
	-rm -f *.o
	-rm -f *.elf
	-rm -f *.exe
