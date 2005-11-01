### 
## PSP Tools library makefile
###
TARGET_LIB = ../lib/libTools.a
OBJS = pg.o Tools.o Print.o

INCDIR = 
CFLAGS = -O2 -Wall -D__PSP__
#-G0
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS =
LIBS = 

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
