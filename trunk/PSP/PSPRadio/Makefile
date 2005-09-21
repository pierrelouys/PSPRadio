TARGET = PSPRadio
OBJS = PSPRadio.o

INCDIR = ../
CFLAGS = -O2 -G0 -Wall -I../SharedLib/PSPApp -L../SharedLib/lib -I../SharedLib/libnet -I../SharedLib/libiniparser -I../SharedLib/Tools
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = 
LDFLAGS = 
LIBS = -lPSPApp -lpspaudio -lsupc++ -lstdc++ -lnet -lmad -liniparser -lTools

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSPRadio

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
