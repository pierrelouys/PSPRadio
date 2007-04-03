### 
## PSP Tools library makefile
###
BASE_DIR = ../..
include $(BASE_DIR)/include.mak

TARGET_LIB = ../lib/libTools.a
#OBJS = Tools.o Screen.o
OBJS = Screen.o

INCDIR = 
CFLAGS = $(GLOBAL_CFLAGS) -O2 -Wall -Werror -g -D__PSP__

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS =
LIBS = 

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
