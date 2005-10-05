TARGET = PSPRadio
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
