# This Makefile was edited by EXL, 05-Dec-2022.
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

# ELF name.
ELF_NAME = FireEffect

all:
	$(ARM_PATH)/bin/tcc -I$(SDK_PATH) $(DEFINES) -bigend -apcs /interwork -O2 -c $(ELF_NAME).c \
		-o $(ELF_NAME).o
	$(ARM_PATH)/bin/armlink -nolocals -reloc -first $(LIB_MAIN)\(Lib\) $(ELF_NAME).o $(LIB_PATH)/$(LIB_MAIN) \
		-o $(ELF_NAME).elf

clean:
	-rm -f $(ELF_NAME).o
	-rm -f $(ELF_NAME).elf
