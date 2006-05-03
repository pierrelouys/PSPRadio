BASE_DIR = ../..
include $(BASE_DIR)/include.mak

TARGET = APP_Retawq

EXPORTED_OBJS = $(PSPRADIO_DIR)/PSPRadioExports.o 
COMMON_OBJS   = $(PLUGINS_DIR)/Common.o
OBJS = plugin_main.o init.o main.o parser.o resource.o stuff.o pspcurses.o psp.o \
		$(COMMON_OBJS) $(EXPORTED_OBJS)

USE_PSPSDK_LIBC = 0
# Define the name of our custom exports (minus the .exp extension)
PRX_EXPORTS=$(PLUGINS_DIR)/APP_Exports.exp

INCLUDES = -I./ -I$(PLUGINS_DIR) -I$(BASE_DIR) -I$(PSPRADIO_DIR) -I$(PSPRADIO_DIR)/ScreenHandler \
		   -I$(SHAREDLIB_DIR)/PSPApp -L$(SHAREDLIB_DIR)/lib \
		   -I$(SHAREDLIB_DIR)/libpspnet -I$(SHAREDLIB_DIR)/libiniparser \
		   -I$(SHAREDLIB_DIR)/Tools -I$(SHAREDLIB_DIR)/Logging -I$(SHAREDLIB_DIR)/danzeff
		 
#GLOBAL_CFLAGS = -G0 -mno-explicit-relocs -Wall -Werror -DDYNAMIC_BUILD
#MY_CFLAGS = -G0 -mno-explicit-relocs  -DDYNAMIC_BUILD -DSTAND_ALONE_APP
MY_CFLAGS = -G0 -mno-explicit-relocs  -DDYNAMIC_BUILD -DUSE_DANZEFF

CFLAGS = $(MY_CFLAGS) $(EXTRA_FLAGS) -g -Wall 	$(INCLUDES)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =

DANZEFF_LIBS = -ldanzeff -lSDL_image -lSDL -lpng -lz -lm  -lpspgu -lpspaudio -ljpeg -lpsphprm

LIBS =    $(DANZEFF_LIBS) -lpspsdk -lpspwlan -lpspnet

the_pbp: $(OBJS) EBOOT.PBP

the_prx: $(TARGET).prx

prx:
	make BUILD_PRX=1 the_prx

stand_alone: 
	make EXTRA_FLAGS="-DSTAND_ALONE_APP" EXPORTED_OBJS="" COMMON_OBJS="" the_pbp

install:
	cp -vf $(TARGET).prx $(KXPLOIT_PRIMARY)
	mkdir -p $(KXPLOIT_PRIMARY)/$(TARGET)
	cp -vf tips.html $(KXPLOIT_PRIMARY)/$(TARGET)
#	cp -vf *.cfg $(KXPLOIT_PRIMARY)/$(TARGET)

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
