BASE_DIR = ../..
include $(BASE_DIR)/include.mak

TARGET = APP_afkim

EXPORTED_OBJS = $(PSPRADIO_DIR)/PSPRadioExports.o 
OBJS = main.o \
      dlib/guibit.o dlib/render.o dlib/support.o dlib/textBits.o  dlib/util.o dlib/inputable.o \
      irc.o bitlbee.o \
      gui/accountsStatus.o gui/chatInput.o gui/chatSelector.o gui/newmsgBit.o \
      gui/menuPopup.o gui/accountCreateText.o gui/accountCreator.o \
      dlib/guibits/textArea.o dlib/guibits/textLineInput.o dlib/guibits/selector.o dlib/guibits/wifiSelector.o \
      dlib/keyboards/p_sprint.o dlib/keyboards/kbwrap.o dlib/keyboards/pspctrl_emu.o \
		$(EXPORTED_OBJS)

# Define to build this as a prx (instead of a static elf)
BUILD_PRX=1
USE_PSPSDK_LIBC = 0
# Define the name of our custom exports (minus the .exp extension)
PRX_EXPORTS=$(PLUGINS_DIR)/APP_Exports.exp

INCLUDES = -I./ -I$(PLUGINS_DIR) -I$(BASE_DIR) -I$(PSPRADIO_DIR) -I$(SHAREDLIB_DIR)/PSPApp -L$(SHAREDLIB_DIR)/lib \
		 -I$(SHAREDLIB_DIR)/libpspnet -I$(SHAREDLIB_DIR)/libiniparser -I$(SHAREDLIB_DIR)/libpthread \
		 -I$(SHAREDLIB_DIR)/Tools -I$(SHAREDLIB_DIR)/Logging -I$(SHAREDLIB_DIR)/danzeff/puresdl \
		 -I$(PLUGINS_DIR)/Common -L$(PLUGINS_DIR)/Common
		 
CFLAGS = -O2 -g -G0 $(GLOBAL_CFLAGS) -Wall $(INCLUDES) -I/usr/local/pspdev/psp/include/freetype2 -DPSP -DPSPRADIOPLUGIN -DDANZEFF_SDL
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =

LIBS = -lpthread -ldanzeffpuresdl -lSDL_image  -lSDL -lfreetype -lpng -lstdc++ -lz -lm -ljpeg -lpspaudio -lpsphprm -lpspgu -lpspwlan -lpsppower -lpsprtc -lPluginCommon

all: $(TARGET).prx

install: $(TARGET).prx
	mkdir -p $(PSPRADIO_RELEASE_DIR)/APP_afkim/
	cp -vf $(TARGET).prx $(PSPRADIO_RELEASE_DIR)/
	mkdir -p $(PSPRADIO_RELEASE_DIR)/APP_afkim/icons
	cp -vf icons/*.png $(PSPRADIO_RELEASE_DIR)/APP_afkim/icons
	mkdir -p $(PSPRADIO_RELEASE_DIR)/APP_afkim/pics
	cp -vf pics/*.png $(PSPRADIO_RELEASE_DIR)/APP_afkim/pics
	mkdir -p $(PSPRADIO_RELEASE_DIR)/APP_afkim/graphics
	cp -vf $(SHAREDLIB_DIR)/danzeff/graphics/*.png $(PSPRADIO_RELEASE_DIR)/APP_afkim/graphics
	cp font.ttf afkim.cfg README $(PSPRADIO_RELEASE_DIR)/APP_afkim/
	
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
