TARGET = MadEx
OBJS = example-mad.o

INCDIR = ../
CFLAGS = -O2 -G0 -Wall -L../../lib -I../../libnet -I../../libiniparser -I../../Tools
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = 
LDFLAGS = 
LIBS = -lPSPApp -lpspaudio -lsupc++ -lstdc++ -lnet -lmad -liniparser -lTools

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSPApp MAD example App

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
