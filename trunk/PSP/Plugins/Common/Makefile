### 
## PSPRadio Plugin Common library makefile
###
BASE_DIR = ../..
include $(BASE_DIR)/include.mak

TARGET_LIB = ./libPluginCommon.a
OBJS = Common.o

INCDIR =	$(BASE_DIR)/PSPRadio $(BASE_DIR)/Plugins $(BASE_DIR)/SharedLib/PSPApp \
					$(BASE_DIR)/SharedLib/libiniparser \
					$(BASE_DIR)/SharedLib/Logging $(BASE_DIR)/SharedLib/libscreen
CFLAGS =	$(GLOBAL_CFLAGS) -O2 -Wall -Werror -g -D__PSP__

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS =
LIBS = 

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

