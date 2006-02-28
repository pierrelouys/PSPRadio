### 
## PSP Tools library makefile
###
TARGET_LIB = ../lib/libTools.a
OBJS = pg.o Tools.o Screen.o

INCDIR = 
CFLAGS = $(GLOBAL_CFLAGS)-O2 -Wall -g -D__PSP__

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS =
LIBS = 

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
