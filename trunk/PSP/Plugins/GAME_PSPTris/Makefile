TARGET = PSPTris
KXPLOIT_PRIMARY_DIR = "__SCE__PSPTris"
KXPLOIT_SECONDARY_DIR = "%__SCE__PSPTris"
PSP_EBOOT_ICON = eboot_resource/icon0.png
PSP_EBOOT_PIC1 = eboot_resource/pic1.png

OBJS = PSPTris_main.o PSPTris_intro.o PSPTris_audio.o PSPTris_menu.o PSPTris_game.o PSPTris_game_classic.o PSPTris_game_color.o PSPTris_credits.o PSPTris_highscore.o jsaParticle.o jsaTextureCache.o valloc.o PSPKeyHandler.o

INCDIR = ../

INCLUDES = -I../../SharedLib/danzeff

CFLAGS = -G0 -Wall -ggdb -O2 -fno-strict-aliasing -Werror -DDANZEFF_SCEGU $(INCLUDES)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = ../../SharedLib/lib/
LDFLAGS =

DANZEFF_LIBS = -ldanzeff -lSDL_image -lSDL -lpng -lz -lm  -lpspgu -lpspaudio -ljpeg -lpsphprm
LIBS = $(DANZEFF_LIBS) -lsupc++ -lstdc++ -lpspgum -lpspgu -lpsprtc -lpsppower -lpng -lz -lm -lmikmod -lpspaudio

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSPTris

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

clean:
	rm -fv *.o *.PBP *.SFO $(OBJS)
	rm -Rf $(KXPLOIT_PRIMARY_DIR) $(KXPLOIT_SECONDARY_DIR)

kxploit: $(TARGET).elf $(PSP_EBOOT_SFO) $(EXTRA_TARGETS) $(FINAL_TARGET)
	mkdir -p $(KXPLOIT_PRIMARY_DIR)
	$(STRIP) $(TARGET).elf -o $(KXPLOIT_PRIMARY_DIR)/$(PSP_EBOOT)
	mkdir -p $(KXPLOIT_SECONDARY_DIR)
	$(PACK_PBP) $(KXPLOIT_SECONDARY_DIR)/$(PSP_EBOOT) $(PSP_EBOOT_SFO) $(PSP_EBOOT_ICON)  \
		$(PSP_EBOOT_ICON1) $(PSP_EBOOT_UNKPNG) $(PSP_EBOOT_PIC1)  \
		$(PSP_EBOOT_SND0) NULL $(PSP_EBOOT_PSAR)
	mkdir -p $(KXPLOIT_PRIMARY_DIR)/Music
	cp -vf audio/*.mod $(KXPLOIT_PRIMARY_DIR)/Music
	mkdir -p $(KXPLOIT_PRIMARY_DIR)/Samples
	cp -vf audio/*.wav $(KXPLOIT_PRIMARY_DIR)/Samples
	mkdir -p $(KXPLOIT_PRIMARY_DIR)/Textures
	cp -vf graphics/*.png $(KXPLOIT_PRIMARY_DIR)/Textures
	cp -vf readme.txt $(KXPLOIT_PRIMARY_DIR)/.
	cp -vf whatsnew.txt $(KXPLOIT_PRIMARY_DIR)/.
# copy files used for Danzeff OSK
	mkdir -p $(KXPLOIT_PRIMARY_DIR)/graphics
	cp -vf ../../SharedLib/danzeff/graphics/*.png $(KXPLOIT_PRIMARY_DIR)/graphics
	rm -rf $(KXPLOIT_PRIMARY_DIR)/graphics/keys_c_t.png
	rm -rf $(KXPLOIT_PRIMARY_DIR)/graphics/keys_t.png
	rm -rf $(KXPLOIT_PRIMARY_DIR)/graphics/nums_c_t.png
	rm -rf $(KXPLOIT_PRIMARY_DIR)/graphics/nums_t.png
