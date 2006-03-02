BASE_DIR = ../..
include $(BASE_DIR)/include.mak

TARGET = FSS_FTPD

EXPORTED_OBJS = $(PSPRADIO_DIR)/PSPRadioExports.o 
OBJS = main.o ftp_main.o ftp.o _itoa.o sutils.o  \
		$(EXPORTED_OBJS)

# Define to build this as a prx (instead of a static elf)
BUILD_PRX=1
# Define the name of our custom exports (minus the .exp extension)
PRX_EXPORTS=$(PLUGINS_DIR)/FSS_Exports.exp

INCLUDES = -I./ -I$(BASE_DIR) -I$(PSPRADIO_DIR) -I$(SHAREDLIB_DIR)/PSPApp -L$(SHAREDLIB_DIR)/lib \
		 -I$(SHAREDLIB_DIR)/libpspnet -I$(SHAREDLIB_DIR)/libiniparser \
		 -I$(SHAREDLIB_DIR)/Tools -I$(SHAREDLIB_DIR)/Logging
		 
CFLAGS = $(GLOBAL_CFLAGS) -O2 -g -Wall $(INCLUDES) 
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =

LIBS =  -lstdc++ -liniparser -lTools -lpng -lz -lm

all: $(TARGET).prx

install:
	mkdir -p $(KXPLOIT_PRIMARY)/$(TARGET)
	cp -vf $(TARGET).prx $(KXPLOIT_PRIMARY)
	cp -vf *.cfg $(KXPLOIT_PRIMARY)/$(TARGET)

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
