BASE_DIR = ../..
include $(BASE_DIR)/include.mak

TARGET = APP_NetScan

EXPORTED_OBJS = $(PSPRADIO_DIR)/PSPRadioExports.o 
COMMON_OBJS   = $(PLUGINS_DIR)/Common.o
OBJS = main.o \
		$(COMMON_OBJS) $(EXPORTED_OBJS)

# Define to build this as a prx (instead of a static elf)
BUILD_PRX=1
USE_PSPSDK_LIBC = 0
# Define the name of our custom exports (minus the .exp extension)
PRX_EXPORTS=$(PLUGINS_DIR)/APP_Exports.exp

INCLUDES = -I./ -I$(PLUGINS_DIR) -I$(BASE_DIR) -I$(PSPRADIO_DIR) -I$(SHAREDLIB_DIR)/PSPApp -L$(SHAREDLIB_DIR)/lib \
		 -I$(SHAREDLIB_DIR)/libpspnet -I$(SHAREDLIB_DIR)/libiniparser \
		 -I$(SHAREDLIB_DIR)/Tools -I$(SHAREDLIB_DIR)/Logging
		 
CFLAGS = $(GLOBAL_CFLAGS) -O2 -g -Wall $(INCLUDES)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =

LIBS =  -lpspwlan -lpspnet
#-lstdc++  -lPSPApp -liniparser
#-lstdc++ -lPSPApp
# -lTools

all: $(TARGET).prx
	rm $(COMMON_OBJS)

install:
	cp -vf $(TARGET).prx $(KXPLOIT_PRIMARY)
#	mkdir -p $(KXPLOIT_PRIMARY)/$(TARGET)
#	cp -vf $(TARGET).prx $(KXPLOIT_PRIMARY)
#	cp -vf *.cfg $(KXPLOIT_PRIMARY)/$(TARGET)

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
