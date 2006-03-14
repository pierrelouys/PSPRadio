BASE_DIR = ..
include $(BASE_DIR)/include.mak

TARGET = PSPRadio

# Define to build this as a prx (instead of a static elf)
BUILD_PRX=1
PRX_EXPORTS=exports.exp

IMPORTED_OBJS = $(PLUGINS_DIR)/PSPRadio_UI.o $(PLUGINS_DIR)/PSPRadio_FSS.o

OBJS = Main.o PSPRadio.o MetaDataContainer.o \
		./ScreenHandler/ScreenHandler.o ./ScreenHandler/SHOUTcastScreen.o  \
		./ScreenHandler/OptionsScreen.o ./ScreenHandler/OptionsPluginMenuScreen.o \
		./ScreenHandler/PlayListScreen.o ./ScreenHandler/LocalFilesScreen.o \
		./IPSPRadio_UI.o PSPRadio_Exports.o \
		$(IMPORTED_OBJS)

CFLAGS = $(GLOBAL_CFLAGS) -O2 -Wall -Werror -g $(INCLUDES)

INCLUDES = -I$(SHAREDLIB_DIR)/PSPApp -L$(SHAREDLIB_DIR)/lib \
		 -I$(SHAREDLIB_DIR)/libpspnet -I$(SHAREDLIB_DIR)/libiniparser \
		 -I$(SHAREDLIB_DIR)/Tools -I$(SHAREDLIB_DIR)/Logging \
		 -I./ScreenHandler \
		 -I$(PLUGINS_DIR) \
		 -I$(PLUGINS_DIR)/TextUI \
		 -I$(PLUGINS_DIR)/GraphicsUI \
		 -I$(PLUGINS_DIR)/TextUI3D


CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = 
LDFLAGS = 

# LIBS =  -lsupc++ -lstdc++ \
# 		-lpspnet -lpspnet_apctl -lpspnet_inet -lpspnet_resolver \
# 		-lPSPApp \
# 		-lvorbisfile -lvorbis -logg \
# 		-lmad \
# 		-liniparser -lTools -llogging -lpspwlan -lpsputility \
# 		-lSDLmain -lSDL_image \
# 		-ljpeg -lSDL -lpsphprm -lpspaudio -lpspusb -lpspusbstor -lpsprtc \
# 		-lpsppower -lpng -lz -lm -lTools \

LIBS =	-lstdc++ \
		-lPSPApp -liniparser -llogging -lTools\
 		-lmad \
		-lvorbisfile -lvorbis -logg \
		-lpspwlan \
		-lpsphprm -lpspaudio -lpspusb -lpspusbstor -lpsprtc -lpsppower \
		-lpng -lz -lm \
		


all: $(TARGET).prx
	mkdir -p $(KXPLOIT_PRIMARY)
	cp -vf $(TARGET).prx $(KXPLOIT_PRIMARY)
	cp -vf *.cfg $(KXPLOIT_PRIMARY)
	mkdir -p $(KXPLOIT_PRIMARY)/PlayLists
	mkdir -p $(KXPLOIT_PRIMARY)/SHOUTcast
	cp -vf SHOUTcast/* $(KXPLOIT_PRIMARY)/SHOUTcast
	mkdir -p $(KXPLOIT_PRIMARY)/Screenshots
	cp -vf ./Init.png ./Shutdown.png $(KXPLOIT_PRIMARY)
	cp -vf readme.txt whatsnew.txt $(RELEASE_DIR)
	
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
