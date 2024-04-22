# This Makefile was created by EXL, 03-Sep-2023.
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
DEFINES = -D__P2K__ -DEP1 -DLINUX_BOGOMIPS
#DEFINES = -D__P2K__ -DEP1 -DLINUX_BOGOMIPS -DNO_ASM
#DEFINES = -D__P2K__ -DEP1 -DPALMOS_BOGOMIPS
#DEFINES = -D__P2K__ -DEP1 -DPALMOS_BOGOMIPS -DFTR_V600
#DEFINES = -D__P2K__ -DEP1 -DPALMOS_BOGOMIPS -DFTR_L7E

# Project/ELF name.
ELF_NAME = Benchmark

all:
	$(ARM_PATH)/bin/armasm -32 -bi -apcs /interwork delay_armv4t_ADS.S delay_armv4t_ADS.o
	$(ARM_PATH)/bin/tcc -I$(SDK_PATH) $(DEFINES) -bigend -apcs /interwork -O2 -c dhry_1.c -o dhry_1.o
	$(ARM_PATH)/bin/tcc -I$(SDK_PATH) $(DEFINES) -bigend -apcs /interwork -O2 -c dhry_2.c -o dhry_2.o
	$(ARM_PATH)/bin/tcc -I$(SDK_PATH) $(DEFINES) -bigend -apcs /interwork -O2 -c Phases.c -o Phases.o
	$(ARM_PATH)/bin/tcc -I$(SDK_PATH) $(DEFINES) -bigend -apcs /interwork -O2 -c FireEffect.c -o FireEffect.o
	$(ARM_PATH)/bin/tcc -I$(SDK_PATH) $(DEFINES) -bigend -apcs /interwork -O2 -c $(ELF_NAME).c -o $(ELF_NAME).o

	$(ARM_PATH)/bin/armlink -nolocals -reloc -first $(LIB_MAIN)\(Lib\) \
		delay_armv4t_ADS.o dhry_1.o dhry_2.o Phases.o FireEffect.o $(ELF_NAME).o \
		$(LIB_PATH)/$(LIB_MAIN) -o $(ELF_NAME).elf

clean:
	-rm -f *.o
	-rm -f *.obj
	-rm -f *.elfp
	-rm -f *.elf
