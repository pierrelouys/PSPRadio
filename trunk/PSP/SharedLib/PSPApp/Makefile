TARGET_LIB = ../lib/libPSPApp.a
OBJS = PSPApp.o PSPSound.o bstdfile.o PSPSound_MP3.o

INCDIR = 
CFLAGS = -O2 -Wall
#-G0
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS = -lpspaudiolib -lpspaudio
LIBS = 

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
