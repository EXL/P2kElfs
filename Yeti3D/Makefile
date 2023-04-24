# This Makefile was edited by EXL, 23-Apr-2023.
# Default platform is Motorola P2K, ElfPack v1.x, ADS1.2 [Build 848] on Linux.
# Warning: `-nodebug` flag option for `armlink` is buggy.

# Compiler path.
ARM_PATH = /opt/arm

# SDK path.
SDK_PATH = $(ARM_PATH)/SDK

# Libraries path.
LIB_PATH = $(ARM_PATH)/lib

# Main link library.
LIB_MAIN = Lib.o

# Defines.
DEFINES = -D__P2K__ -DEP1 -DROT_90 -DFPS_30

# ELF name.
ELF_NAME = Yeti3D

all:
	$(ARM_PATH)/bin/armcc -I$(SDK_PATH) $(DEFINES) -bigend -apcs /interwork -O2 -c draw.c -o draw.o
	$(ARM_PATH)/bin/armcc -I$(SDK_PATH) $(DEFINES) -bigend -apcs /interwork -O2 -c game.c -o game.o
	$(ARM_PATH)/bin/armcc -I$(SDK_PATH) $(DEFINES) -bigend -apcs /interwork -O2 -c yeti.c -o yeti.o
	$(ARM_PATH)/bin/armcc -I$(SDK_PATH) $(DEFINES) -bigend -apcs /interwork -O2 -c main_p2k.c -o main_p2k.o
	$(ARM_PATH)/bin/armlink -nolocals -reloc -first $(LIB_MAIN)\(Lib\) draw.o game.o yeti.o main_p2k.o \
		$(LIB_PATH)/$(LIB_MAIN) -o $(ELF_NAME).elf

clean:
	-rm -f *.o
	-rm -f $(ELF_NAME).elf
