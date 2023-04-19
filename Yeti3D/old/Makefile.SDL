all: linux

linux:
	g++ -O2 -DPLATFORM_SDL data.cpp draw.cpp main.cpp yeti.cpp -o yeti3d.elf -lSDL

clean:
	-rm -Rf *.o
	-rm -Rf *.elf
