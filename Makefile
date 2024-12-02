# This Makefile was created by EXL, 03-Sep-2024
# Default platform is Linux, Ubuntu 22.04 LTS

CC=gcc
STRIP=strip
CFLAGS=`sdl2-config --cflags` -O2 -g -DUSE_SDL2 -Wall -pedantic -Wno-maybe-uninitialized -Wno-misleading-indentation
#CFLAGS+=-DREPAINT_HOOK
LFLAGS=`sdl2-config --libs` -Wl,--allow-multiple-definition
NAME=vNesC
ifeq ($(OS), Windows_NT)
	EXECUTABLE=$(NAME).exe
else
	EXECUTABLE=$(NAME).elf
endif

all: $(EXECUTABLE)

$(EXECUTABLE): vNesC.o vSdl2.o
	$(CC) $(CFLAGS) vNesC.o vSdl2.o -o $(EXECUTABLE) $(LFLAGS)
#	$(STRIP) -s $(EXECUTABLE)

clean:
	-rm -f *.o
	-rm -f *.elf
	-rm -f *.exe

vNesC.o: Makefile vNesC.c vNesC.h
	$(CC) $(CFLAGS) -c vNesC.c -o vNesC.o

vSdl2.o: Makefile vSdl2.c vNesC.h
	$(CC) $(CFLAGS) -c vSdl2.c -o vSdl2.o
