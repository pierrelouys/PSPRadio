TARGET_LIB = ../lib/libsamplerate.a
OBJS = samplerate.o src_linear.o src_sinc.o src_zoh.o


INCDIR = 
CFLAGS = -O2 -Wall -D__PSP__
#-G0
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS = -lm
LIBS = 

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
