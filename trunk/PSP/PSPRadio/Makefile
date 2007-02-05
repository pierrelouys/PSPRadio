BASE_DIR = ..
include $(BASE_DIR)/include.mak

TARGET = PSPRadio
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSPRadio Dynamic

# Define to build this as a prx (instead of a static elf)
BUILD_PRX=1
PRX_EXPORTS=exports.exp

IMPORTED_OBJS = $(PLUGINS_DIR)/UI_Interface.o $(PLUGINS_DIR)/PSPRadio_UI.o \
				$(PLUGINS_DIR)/PSPRadio_FSS.o \
				$(PLUGINS_DIR)/PSPRadio_APP.o \
				$(PLUGINS_DIR)/PSPRadio_GAME.o

OBJS = Main.o PSPRadio.o MetaDataContainer.o \
		./ScreenHandler/ScreenHandler.o ./ScreenHandler/SHOUTcastScreen.o  \
		./ScreenHandler/OptionsScreen.o ./ScreenHandler/OptionsPluginMenuScreen.o \
		./ScreenHandler/PlayListScreen.o ./ScreenHandler/LocalFilesScreen.o \
		PSPRadio_Exports.o \
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

LIBS =	-lstdc++ \
		-lPSPApp -liniparser -llogging -lTools\
 		-lmad \
		-lvorbisfile -lvorbis -logg \
		-lpspwlan \
		-lpsphprm -lpspaudio -lpspusb -lpspusbstor -lpsprtc -lpsppower \
		-lpng -lz -lm \

all: $(EXTRA_TARGETS) $(FINAL_TARGET)
	mkdir -p $(PSPRADIO_RELEASE_DIR)
	cp -vf EBOOT.PBP $(PSPRADIO_RELEASE_DIR)
	cp -vf *.cfg $(PSPRADIO_RELEASE_DIR)
	mkdir -p $(PSPRADIO_RELEASE_DIR)/PlayLists
	mkdir -p $(PSPRADIO_RELEASE_DIR)/SHOUTcast
	cp -vf SHOUTcast/* $(PSPRADIO_RELEASE_DIR)/SHOUTcast
	mkdir -p $(PSPRADIO_RELEASE_DIR)/Screenshots
	cp -vf ./Init.png ./Shutdown.png $(PSPRADIO_RELEASE_DIR)
	cp -vf readme.txt whatsnew.txt $(RELEASE_DIR)

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak


