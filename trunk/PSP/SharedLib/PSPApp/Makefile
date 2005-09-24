TARGET_LIB = ../lib/libPSPApp.a
OBJS = PSPApp.o PSPSound.o bstdfile.o PSPSound_MP3.o httpget.o

INCDIR = 
CFLAGS = -O2 -Wall -I../libpspnet -D__PSP__
#-G0
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS = -lpspaudiolib -lpspaudio -lpspnet 
LIBS = 

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
