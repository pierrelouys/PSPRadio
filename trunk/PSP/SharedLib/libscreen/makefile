### 
## Screen library makefile
###
BASE_DIR = ../..
include $(BASE_DIR)/include.mak

TARGET_LIB = libscreen.a
OBJS = Screen.o ScFont_base.o valloc.o

INCDIR = 
CFLAGS = $(GLOBAL_CFLAGS) -O2 -Wall -Werror -g -D__PSP__ $(EXTRAFLAGS)

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS =
LIBS = 

guscreen:
	rm -f *.o libguscreen.a
	make EXTRAFLAGS=-DGUSCREEN TARGET_LIB=libguscreen.a libguscreen.a

fbscreen:
	rm -f *.o libfbscreen.a
	make EXTRAFLAGS=-DFBSCREEN TARGET_LIB=libfbscreen.a libfbscreen.a

all: fbscreen guscreen

install: all
	cp libguscreen.a libfbscreen.a ../lib

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
