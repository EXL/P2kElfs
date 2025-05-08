# This Makefile was created by EXL, 07-Sep-2023.
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
DEFINES = -D__P2K__ -DEP1
#DEFINES = -D__P2K__ -DEP1 -DFTR_V600
#DEFINES = -D__P2K__ -DEP1 -DFTR_V635
#DEFINES = -D__P2K__ -DEP1 -DFTR_L7E
#DEFINES = -D__P2K__ -DEP1 -DDISABLE_ALL_INT

# Project/ELF name.

ifeq ($(ARGON), 1)
	ELF_NAME = OverclockLV
else
	ELF_NAME = Overclock
endif

all:
	$(ARM_PATH)/bin/tcc -I$(SDK_PATH) $(DEFINES) -bigend -apcs /interwork -O2 -c Clock.c \
		-o Clock.o
	$(ARM_PATH)/bin/tcc -I$(SDK_PATH) $(DEFINES) -bigend -apcs /interwork -O2 -c Overclock.c \
		-o Overclock.o

	$(ARM_PATH)/bin/armlink -nolocals -reloc -first $(LIB_MAIN)\(Lib\) \
		Clock.o Overclock.o \
		$(LIB_PATH)/$(LIB_MAIN) -o $(ELF_NAME).elf

clean:
	-rm -f *.o
	-rm -f *.obj
	-rm -f *.elfp
	-rm -f *.elf
