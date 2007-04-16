BASE_DIR = ../..
include $(BASE_DIR)/include.mak

TARGET = UI_Text3D

EXPORTED_OBJS = $(PSPRADIO_DIR)/PSPRadioExports.o 
OBJS =	$(PLUGINS_DIR)/UI_Interface.o $(EXPORTED_OBJS) \
		main.o TextUI3D.o TextUI3D_Panel.o TextUI3D_WindowManager_HSM.o \
		jsaVRAMManager.o jsaTextureCache.o

# Define to build this as a prx (instead of a static elf)
BUILD_PRX=1
PRX_EXPORTS=$(PLUGINS_DIR)/UI_Exports.exp

INCLUDES = -I./ -I$(PLUGINS_DIR) -I$(BASE_DIR) -I$(PSPRADIO_DIR) -I$(SHAREDLIB_DIR)/PSPApp \
		-L$(SHAREDLIB_DIR)/lib -L$(PLUGINS_DIR)/Common\
		-I$(SHAREDLIB_DIR)/libpspnet -I$(SHAREDLIB_DIR)/libiniparser \
		-I$(SHAREDLIB_DIR)/libscreen -I$(SHAREDLIB_DIR)/Logging \
		-I$(PSPRADIO_DIR)/ScreenHandler 

		 
CFLAGS = $(GLOBAL_CFLAGS) -O2 -g -Wall $(INCLUDES) 
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS =  -lstdc++ -liniparser \
		-lpspgum -lpspgu -lpng -lz -lm -lPluginCommon -lpthreadlite

all: $(TARGET).prx

install:
	mkdir -p $(PSPRADIO_RELEASE_DIR)/$(TARGET)
	cp -vf $(TARGET).prx $(PSPRADIO_RELEASE_DIR)
	cp -vf *.cfg $(PSPRADIO_RELEASE_DIR)/$(TARGET)
	cp -vf Graphics/*.png $(PSPRADIO_RELEASE_DIR)/$(TARGET)

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
