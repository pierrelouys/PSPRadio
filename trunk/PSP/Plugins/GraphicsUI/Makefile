BASE_DIR = ../..
include $(BASE_DIR)/include.mak

TARGET = $(KXPLOIT_PRIMARY_DIR)/TextUI

EXPORTED_OBJS = $(PSPRADIO_DIR)/PSPRadioExports.o 
#$(PSPRADIO_DIR)/pnglibExports.o
OBJS = main.o TextUI.o $(PSPRADIO_DIR)/IPSPRadio_UI.o \
		$(EXPORTED_OBJS)

# Define to build this as a prx (instead of a static elf)
BUILD_PRX=1
# Define the name of our custom exports (minus the .exp extension)
PRX_EXPORTS=$(PLUGINS_DIR)/UI_Exports.exp

INCLUDES = -I./ -I$(BASE_DIR) -I$(PSPRADIO_DIR) -I$(SHAREDLIB_DIR)/PSPApp -L$(SHAREDLIB_DIR)/lib \
		 -I$(SHAREDLIB_DIR)/libpspnet -I$(SHAREDLIB_DIR)/libiniparser \
		 -I$(SHAREDLIB_DIR)/Tools -I$(SHAREDLIB_DIR)/Logging \
		 -I$(PSPRADIO_DIR)/ScreenHandler 

		 
CFLAGS = $(GLOBAL_CFLAGS) -O2 -g -Wall $(INCLUDES) 
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =

LIBS =  -lstdc++ -liniparser -lTools -lpng -lz -lm
# LIBS =  -lsupc++ -lstdc++ \
# 		-lpspnet -lpspnet_apctl -lpspnet_inet -lpspnet_resolver \
# 		-lPSPApp \
# 		-lvorbisfile -lvorbis -logg \
# 		-lmad \
# 		-liniparser -lTools -llogging -lpspwlan -lpsputility \
# 		-lSDLmain -lSDL_image \
# 		-ljpeg -lSDL -lpsphprm -lpspaudio -lpspusb -lpspusbstor -lpsprtc \
# 		-lpsppower -lpng -lz -lm -lTools \
#		-ljpeg -lSDL -lpspgum -lpspgu -lpsphprm -lpspaudio -lpspusb -lpspusbstor -lpsprtc \
#		-lpsppower -lpng -lz -lm -lTools \

all: $(TARGET).prx
	mkdir -p $(TARGET)
	mv -vf $(TARGET).elf .
	cp -vf *.cfg $(TARGET)
	cp -vf *.png $(TARGET)

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
