TARGET_LIB = ../lib/libPSPApp.a
OBJS = PSPApp.o PSPSound.o bstdfile.o PSPSoundDecoder_MAD.o httpget.o PSPEventQ.o

INCDIR =
INCLUDES= -I../libpspnet -I../Logging -I../libsamplerate
CFLAGS = -O2 -Wall -D__PSP__ $(INCLUDES)

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS = -lpspaudiolib -lpspaudio -lpspnet -llogging
LIBS = 

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
