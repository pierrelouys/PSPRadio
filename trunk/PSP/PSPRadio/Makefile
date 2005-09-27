TARGET = PSPRadio
OBJS = PSPRadio.o TextUI.o
PSP_EBOOT_ICON = resources/icon0.png
#PSP_EBOOT_ICON1 = resources/icon0.png
#PSP_EBOOT_SND0 = resources/SND0.at3
PSP_EBOOT_PIC1 = resources/pic1.png

INCDIR = ../
CFLAGS = -O2 -G0 -Wall -I../SharedLib/PSPApp -L../SharedLib/lib -I../SharedLib/libpspnet -I../SharedLib/libiniparser -I../SharedLib/Tools \
		-I../SharedLib/Logging
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = 
LDFLAGS = 
LIBS = -lPSPApp -lpspaudio -lsupc++ -lstdc++ -lpspnet -lmad -liniparser -lTools -llogging

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSPRadio

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
