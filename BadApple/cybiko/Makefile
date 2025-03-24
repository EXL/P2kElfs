# use 'vmake' (not 'make'!) to run this script!

# set this to where Cybiko SDK actually resides
# CYBIKO_SDK = C:\Program Files\Cybiko\Cybiko SDK

RM = vrm
VCC = vcc

APP_NAME = BadApple
APP_CLASSIC = $(APP_NAME)_Classic.app
APP_XTREME = $(APP_NAME)_Xtreme.app

all:
	2pic res/0.ico res/0.bmp
	2pic res/1.ico res/1.bmp
	2pic res/root.ico res/root.bmp

	$(VCC) -DCYOS -O -c BadApple.c -o BadApple.o
	$(VCC) BadApple.o -o bytecode.bin

	$(VCC) -R0 @res/filer_classic.list bytecode.bin -o $(APP_CLASSIC)
	$(VCC) -R0 @res/filer_xtreme.list bytecode.bin -o $(APP_XTREME)

clean:
	$(RM) -f res/*.ico
	$(RM) -f res/*.pic
	$(RM) -f *.o
	$(RM) -f *.bin
	$(RM) -f *.app

new: clean all
