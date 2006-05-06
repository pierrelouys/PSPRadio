BASE_DIR = ../..
include $(BASE_DIR)/include.mak

TARGET_LIB = ../lib/libdanzeff.a
OBJS = danzeff.o

INCDIR =
CFLAGS = $(GLOBAL_CFLAGS) -G0 -mno-explicit-relocs -Wall -g -DDANZEFF_SDL -DDANZEFF_SCEGU -DDANZEFF_USE_STD_PSPCTRL_H
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS =
LIBS =

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
