# This Makefile was edited by EXL, 26-Mar-2023.
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
ELF_NAME = Spout
PCS_NAME = Piece

all: rot90_fps30 rot90_fps15 rot0_fps30 rot0_fps15

rot90_fps30:
	$(ARM_PATH)/bin/tcc -I$(SDK_PATH) $(DEFINES) -DROT_90 -DFPS_30 -bigend -apcs /interwork -O2 -c \
		$(ELF_NAME).c -o $(ELF_NAME).o
	$(ARM_PATH)/bin/tcc -I$(SDK_PATH) $(DEFINES) -DROT_90 -DFPS_30 -bigend -apcs /interwork -O2 -c \
		$(PCS_NAME).c -o $(PCS_NAME).o
	$(ARM_PATH)/bin/armlink -nolocals -reloc -first $(LIB_MAIN)\(Lib\) $(ELF_NAME).o $(PCS_NAME).o \
		$(LIB_PATH)/$(LIB_MAIN) -o $(ELF_NAME).elf
	-rm -f *.o
	-mv $(ELF_NAME).elf $(ELF_NAME)_EP1_D90_F30.elf

rot90_fps15:
	$(ARM_PATH)/bin/tcc -I$(SDK_PATH) $(DEFINES) -DROT_90 -DFPS_15 -bigend -apcs /interwork -O2 -c \
		$(ELF_NAME).c -o $(ELF_NAME).o
	$(ARM_PATH)/bin/tcc -I$(SDK_PATH) $(DEFINES) -DROT_90 -DFPS_15 -bigend -apcs /interwork -O2 -c \
		$(PCS_NAME).c -o $(PCS_NAME).o
	$(ARM_PATH)/bin/armlink -nolocals -reloc -first $(LIB_MAIN)\(Lib\) $(ELF_NAME).o $(PCS_NAME).o \
		$(LIB_PATH)/$(LIB_MAIN) -o $(ELF_NAME).elf
	-rm -f *.o
	-mv $(ELF_NAME).elf $(ELF_NAME)_EP1_D90_F15.elf

rot0_fps30:
	$(ARM_PATH)/bin/tcc -I$(SDK_PATH) $(DEFINES) -DROT_0 -DFPS_30 -bigend -apcs /interwork -O2 -c \
		$(ELF_NAME).c -o $(ELF_NAME).o
	$(ARM_PATH)/bin/tcc -I$(SDK_PATH) $(DEFINES) -DROT_0 -DFPS_30 -bigend -apcs /interwork -O2 -c \
		$(PCS_NAME).c -o $(PCS_NAME).o
	$(ARM_PATH)/bin/armlink -nolocals -reloc -first $(LIB_MAIN)\(Lib\) $(ELF_NAME).o $(PCS_NAME).o \
		$(LIB_PATH)/$(LIB_MAIN) -o $(ELF_NAME).elf
	-rm -f *.o
	-mv $(ELF_NAME).elf $(ELF_NAME)_EP1_D0_F30.elf

rot0_fps15:
	$(ARM_PATH)/bin/tcc -I$(SDK_PATH) $(DEFINES) -DROT_0 -DFPS_15 -bigend -apcs /interwork -O2 -c \
		$(ELF_NAME).c -o $(ELF_NAME).o
	$(ARM_PATH)/bin/tcc -I$(SDK_PATH) $(DEFINES) -DROT_0 -DFPS_15 -bigend -apcs /interwork -O2 -c \
		$(PCS_NAME).c -o $(PCS_NAME).o
	$(ARM_PATH)/bin/armlink -nolocals -reloc -first $(LIB_MAIN)\(Lib\) $(ELF_NAME).o $(PCS_NAME).o \
		$(LIB_PATH)/$(LIB_MAIN) -o $(ELF_NAME).elf
	-rm -f *.o
	-mv $(ELF_NAME).elf $(ELF_NAME)_EP1_D0_F15.elf

clean:
	-rm -f *.o
	-rm -f *.elf
