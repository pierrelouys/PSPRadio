BASE_DIR = ../..
include $(BASE_DIR)/include.mak

TARGET_LIB = ../lib/libPSPApp.a

OBJS =  PSPApp.o PSPThread.o PSPUSBStorage.o \
		PSPApp_Network.o PSPEventQ.o  PSPKeyHandler.o \
		PSPStream.o httpget.o bstdfile.o \
		PSPSoundBuffer.o \
		PSPSound.o PSPSoundDecoder.o \
		PSPSoundDecoder_MAD.o \
		PSPSoundDecoder_OGG.o \
		ID3Parser.o PRXLoader.o 

INCDIR =
INCLUDES= -I../libpspnet -I../Logging
CFLAGS = $(GLOBAL_CFLAGS) -O2 -Wall -Werror -g -D__PSP__ $(INCLUDES) -D_NO_INIT_

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS = -lpspaudiolib -lpspaudio -lPSPNet -llogging
LIBS = 

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
