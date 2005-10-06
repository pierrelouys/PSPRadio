TARGET = PSPRadio
KXPLOIT_PRIMARY_DIR = "PSPRadio"
KXPLOIT_SECONDARY_DIR = "PSPRadio%"

OBJS = PSPRadio.o PlayList.o TextUI.o GraphicsUITheme.o GraphicsUI.o

INCDIR = ../

CFLAGS = -O2 -G2 -Wall -I../SharedLib/PSPApp -L../SharedLib/lib \
		 -I../SharedLib/libpspnet -I../SharedLib/libiniparser \
		 -I../SharedLib/Tools -I../SharedLib/Logging
		 
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = 
LDFLAGS = 

LIBS = -lPSPApp -lsupc++ -lstdc++ -lpspnet -lmad \
		-liniparser -lTools -llogging -lpspwlan -lpsputility \
		-lSDLmain -lSDL_image \
		-ljpeg -lSDL -lpspgu -lpsphprm -lpspaudio -lpspusb -lpspusbstor
#		-lsamplerate -lm \

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSPRadio

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak


kxploit: $(TARGET).elf $(PSP_EBOOT_SFO) $(EXTRA_TARGETS) $(FINAL_TARGET)
	mkdir -p $(KXPLOIT_PRIMARY_DIR)
	$(STRIP) $(TARGET).elf -o $(KXPLOIT_PRIMARY_DIR)/$(PSP_EBOOT)
	mkdir -p $(KXPLOIT_SECONDARY_DIR)
	$(PACK_PBP) $(KXPLOIT_SECONDARY_DIR)/$(PSP_EBOOT) $(PSP_EBOOT_SFO) $(PSP_EBOOT_ICON)  \
		$(PSP_EBOOT_ICON1) $(PSP_EBOOT_UNKPNG) $(PSP_EBOOT_PIC1)  \
		$(PSP_EBOOT_SND0) NULL $(PSP_EBOOT_PSAR)
	cp -vf *.cfg $(KXPLOIT_PRIMARY_DIR)
	mkdir -p $(KXPLOIT_PRIMARY_DIR)/THEME
	cp -vf THEME/* $(KXPLOIT_PRIMARY_DIR)/THEME

