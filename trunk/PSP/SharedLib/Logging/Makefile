BASE_DIR = ../..
include $(BASE_DIR)/include.mak

TARGET_LIB = ../lib/liblogging.a
OBJS = Logging.o

INCDIR = ../PSPApp
CFLAGS = $(GLOBAL_CFLAGS) -O2 -Wall -g -D__PSP__
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS = 
LIBS = 

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
