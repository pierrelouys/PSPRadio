TARGET = PSPRadio
KXPLOIT_PRIMARY_DIR = "PSPRadio"
KXPLOIT_SECONDARY_DIR = "PSPRadio%"

OBJS = Main.o PSPRadio.o MetaDataContainer.o \
		./ScreenHandler/ScreenHandler.o ./ScreenHandler/OptionsScreen.o ./ScreenHandler/SHOUTcastScreen.o ./ScreenHandler/PlayListScreen.o ./ScreenHandler/LocalFilesScreen.o \
		./IPSPRadio_UI.o \
		./TextUI/TextUI.o \
		./TextUI3D/TextUI3D.o ./TextUI3D/TextUI3D_Panel.o ./TextUI3D/TextUI3D_WindowManager.o ./TextUI3D/TextUI3D_WindowManager_HSM.o ./TextUI3D/jsaVRAMManager.o ./TextUI3D/jsaTextureCache.o

INCDIR = ../

CFLAGS = -O2 -Wall -Werror $(INCLUDES)

INCLUDES = -I../SharedLib/PSPApp -L../SharedLib/lib \
		 -I../SharedLib/libpspnet -I../SharedLib/libiniparser \
		 -I../SharedLib/Tools -I../SharedLib/Logging \
		 -I./ScreenHandler \
		 -I./TextUI \
		 -I./GraphicsUI \
		 -I./TextUI3D

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
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
		-ljpeg -lSDL -lpspgum -lpspgu -lpsphprm -lpspaudio -lpspusb -lpspusbstor -lpsprtc \
		-lpsppower -lpng -lz -lm -lTools \

#Note: For Tremor use:
#-lvorbisidec \		
#For regular vorbis:
#-lvorbisfile -lvorbis -logg \


EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSPRadio

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

clean: 
	rm -fv *.o *.PBP *.SFO $(OBJS) ./TextUI/*.o ./GraphicsUI/*.o ./TextUI3D/*.o
	rm -Rf $(KXPLOIT_PRIMARY_DIR) $(KXPLOIT_SECONDARY_DIR)

kxploit: $(TARGET).elf $(PSP_EBOOT_SFO) $(EXTRA_TARGETS) $(FINAL_TARGET)
	mkdir -p $(KXPLOIT_PRIMARY_DIR)
	$(STRIP) $(TARGET).elf -o $(KXPLOIT_PRIMARY_DIR)/$(PSP_EBOOT)
	mkdir -p $(KXPLOIT_SECONDARY_DIR)
	$(PACK_PBP) $(KXPLOIT_SECONDARY_DIR)/$(PSP_EBOOT) $(PSP_EBOOT_SFO) $(PSP_EBOOT_ICON)  \
		$(PSP_EBOOT_ICON1) $(PSP_EBOOT_UNKPNG) $(PSP_EBOOT_PIC1)  \
		$(PSP_EBOOT_SND0) NULL $(PSP_EBOOT_PSAR)
	cp -vf *.cfg $(KXPLOIT_PRIMARY_DIR)
	mkdir -p $(KXPLOIT_PRIMARY_DIR)/PlayLists
	mkdir -p $(KXPLOIT_PRIMARY_DIR)/SHOUTcast
	cp -vf SHOUTcast/* $(KXPLOIT_PRIMARY_DIR)/SHOUTcast
	mkdir -p $(KXPLOIT_PRIMARY_DIR)/Screenshots
	mkdir -p $(KXPLOIT_PRIMARY_DIR)/TextUI
	cp -vf ./TextUI/*.cfg $(KXPLOIT_PRIMARY_DIR)/TextUI
	cp -vf ./TextUI/*.png $(KXPLOIT_PRIMARY_DIR)/TextUI
	cp -vf ./Init.png ./Shutdown.png $(KXPLOIT_PRIMARY_DIR)
	mkdir -p $(KXPLOIT_PRIMARY_DIR)/TextUI3D
	cp -vf ./TextUI3D/*.cfg $(KXPLOIT_PRIMARY_DIR)/TextUI3D
	cp -vf ./TextUI3D/Graphics/*.png $(KXPLOIT_PRIMARY_DIR)/TextUI3D
