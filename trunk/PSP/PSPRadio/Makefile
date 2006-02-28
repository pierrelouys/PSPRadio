BASE_DIR = ..
include $(BASE_DIR)/include.mak

TARGET = $(KXPLOIT_PRIMARY_DIR)/PSPRadio

# Define to build this as a prx (instead of a static elf)
BUILD_PRX=1
PRX_EXPORTS=exports.exp

IMPORTED_OBJS = $(PLUGINS_DIR)/PSPRadio_UI.o

OBJS = Main.o PSPRadio.o MetaDataContainer.o \
		./ScreenHandler/ScreenHandler.o ./ScreenHandler/OptionsScreen.o ./ScreenHandler/SHOUTcastScreen.o  \
		./ScreenHandler/PlayListScreen.o ./ScreenHandler/LocalFilesScreen.o \
		./IPSPRadio_UI.o PSPRadio_Exports.o \
		$(IMPORTED_OBJS)

#INCDIR = ..

CFLAGS = $(GLOBAL_CFLAGS) -O2 -Wall -Werror -g $(INCLUDES)

INCLUDES = -I$(SHAREDLIB_DIR)/PSPApp -L$(SHAREDLIB_DIR)/lib \
		 -I$(SHAREDLIB_DIR)/libpspnet -I$(SHAREDLIB_DIR)/libiniparser \
		 -I$(SHAREDLIB_DIR)/Tools -I$(SHAREDLIB_DIR)/Logging \
		 -I./ScreenHandler \
		 -I$(PLUGINS_DIR)/TextUI \
		 -I$(PLUGINS_DIR)/GraphicsUI \
		 -I$(PLUGINS_DIR)/TextUI3D


CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti -D_PRX_
ASFLAGS = $(CFLAGS)

LIBDIR = 
LDFLAGS = 

LIBS =  -lsupc++ -lstdc++ \
		-lpspnet -lpspnet_apctl -lpspnet_inet -lpspnet_resolver \
		-lPSPApp \
		-lvorbisfile -lvorbis -logg \
		-lmad \
		-liniparser -lTools -llogging -lpspwlan -lpsputility \
		-lSDLmain -lSDL_image \
		-ljpeg -lSDL -lpsphprm -lpspaudio -lpspusb -lpspusbstor -lpsprtc \
		-lpsppower -lpng -lz -lm -lTools \
#		-ljpeg -lSDL -lpspgum -lpspgu -lpsphprm -lpspaudio -lpspusb -lpspusbstor -lpsprtc \
#		-lpsppower -lpng -lz -lm -lTools \

all: $(TARGET).prx
	mv -vf $(TARGET).elf .
	cp -vf *.cfg $(KXPLOIT_PRIMARY_DIR)
	mkdir -p $(KXPLOIT_PRIMARY_DIR)/PlayLists
	mkdir -p $(KXPLOIT_PRIMARY_DIR)/SHOUTcast
	cp -vf SHOUTcast/* $(KXPLOIT_PRIMARY_DIR)/SHOUTcast
	mkdir -p $(KXPLOIT_PRIMARY_DIR)/Screenshots
	cp -vf ./Init.png ./Shutdown.png $(KXPLOIT_PRIMARY_DIR)
	cp -vf readme.txt whatsnew.txt $(RELEASE_DIR)

#Note: For Tremor use:
#-lvorbisidec \		
#For regular vorbis:
#-lvorbisfile -lvorbis -logg \

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
