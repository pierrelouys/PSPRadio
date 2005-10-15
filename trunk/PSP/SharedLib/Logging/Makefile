TARGET_LIB = ../lib/liblogging.a
OBJS = Logging.o

INCDIR = ../PSPApp
CFLAGS = -O2 -Wall -D__PSP__
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS = 
LIBS = 

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
