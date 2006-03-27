BASE_DIR = ../..
include $(BASE_DIR)/include.mak

TARGET = UI_Text

EXPORTED_OBJS = $(PSPRADIO_DIR)/PSPRadioExports.o 
COMMON_OBJS   = $(PLUGINS_DIR)/Common.o
OBJS = main.o TextUI.o $(PLUGINS_DIR)/UI_Interface.o \
		 $(EXPORTED_OBJS) $(COMMON_OBJS)

# Define to build this as a prx (instead of a static elf)
BUILD_PRX=1
# Define the name of our custom exports (minus the .exp extension)
PRX_EXPORTS=$(PLUGINS_DIR)/UI_Exports.exp

INCLUDES = -I./ -I$(PLUGINS_DIR) -I$(BASE_DIR) -I$(PSPRADIO_DIR) -I$(SHAREDLIB_DIR)/PSPApp -L$(SHAREDLIB_DIR)/lib \
		 -I$(SHAREDLIB_DIR)/libpspnet -I$(SHAREDLIB_DIR)/libiniparser \
		 -I$(SHAREDLIB_DIR)/Tools -I$(SHAREDLIB_DIR)/Logging \
		 -I$(PSPRADIO_DIR)/ScreenHandler 

		 
CFLAGS = $(GLOBAL_CFLAGS) -O2 -g -Wall $(INCLUDES) 
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =

LIBS =  -lstdc++ -liniparser -lTools -lpng -lz -lm

all: $(TARGET).prx
	rm $(COMMON_OBJS)

install:
	mkdir -p $(KXPLOIT_PRIMARY)/$(TARGET)
	cp -vf $(TARGET).prx $(KXPLOIT_PRIMARY)
	cp -vf *.cfg $(KXPLOIT_PRIMARY)/$(TARGET)
	cp -vf *.png $(KXPLOIT_PRIMARY)/$(TARGET)

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
