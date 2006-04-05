BASE_DIR = ../..
include $(BASE_DIR)/include.mak

TARGET = APP_Retawq

EXPORTED_OBJS = $(PSPRADIO_DIR)/PSPRadioExports.o 
COMMON_OBJS   = $(PLUGINS_DIR)/Common.o
OBJS = plugin_main.o init.o main.o parser.o resource.o stuff.o psp_curses.o\
		$(COMMON_OBJS) $(EXPORTED_OBJS)

# Define to build this as a prx (instead of a static elf)
BUILD_PRX=1
USE_PSPSDK_LIBC = 0
# Define the name of our custom exports (minus the .exp extension)
PRX_EXPORTS=$(PLUGINS_DIR)/APP_Exports.exp

INCLUDES = -I./ -I$(PLUGINS_DIR) -I$(BASE_DIR) -I$(PSPRADIO_DIR) -I$(PSPRADIO_DIR)/ScreenHandler -I$(SHAREDLIB_DIR)/PSPApp -L$(SHAREDLIB_DIR)/lib \
		 -I$(SHAREDLIB_DIR)/libpspnet -I$(SHAREDLIB_DIR)/libiniparser \
		 -I$(SHAREDLIB_DIR)/Tools -I$(SHAREDLIB_DIR)/Logging
		 
#GLOBAL_CFLAGS = -G0 -mno-explicit-relocs -Wall -Werror -DDYNAMIC_BUILD
#MY_CFLAGS = -G0 -mno-explicit-relocs  -DDYNAMIC_BUILD -DSTAND_ALONE_APP
MY_CFLAGS = -G0 -mno-explicit-relocs  -DDYNAMIC_BUILD 

CFLAGS = $(MY_CFLAGS) $(EXTRA_FLAGS) -g -Wall 	$(INCLUDES)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =

LIBS =   -lpspsdk -lpspwlan -lpspnet
#-lstdc++  -lPSPApp -liniparser
#-lstdc++ -lPSPApp
# -lTools

all: cursesbi.o $(TARGET).prx 

stand_alone: 
	make clean	
	make EXTRA_FLAGS="-DSTAND_ALONE_APP"

install:
	cp -vf $(TARGET).prx $(KXPLOIT_PRIMARY)
	mkdir -p $(KXPLOIT_PRIMARY)/$(TARGET)
	cp -vf tips.html $(KXPLOIT_PRIMARY)/$(TARGET)
#	cp -vf *.cfg $(KXPLOIT_PRIMARY)/$(TARGET)

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
