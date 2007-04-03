BASE_DIR = ../..
include $(BASE_DIR)/include.mak

TARGET = APP_Retawq

EXPORTED_OBJS = $(PSPRADIO_DIR)/PSPRadioExports.o 
OBJS = plugin_main.o init.o main.o parser.o resource.o stuff.o pspcurses.o psp.o \
		$(EXPORTED_OBJS)

USE_PSPSDK_LIBC = 0
# Define the name of our custom exports (minus the .exp extension)
PRX_EXPORTS=$(PLUGINS_DIR)/APP_Exports.exp

INCLUDES = -I./ -I$(PLUGINS_DIR) -I$(BASE_DIR) -I$(PSPRADIO_DIR) -I$(PSPRADIO_DIR)/ScreenHandler \
		   -I$(SHAREDLIB_DIR)/PSPApp -L$(SHAREDLIB_DIR)/lib \
		   -I$(SHAREDLIB_DIR)/libpspnet -I$(SHAREDLIB_DIR)/libiniparser \
		   -I$(SHAREDLIB_DIR)/Logging -I$(SHAREDLIB_DIR)/danzeff \
		   -I$(PLUGINS_DIR)/Common -L$(PLUGINS_DIR)/Common

MY_CFLAGS = $(GLOBAL_CFLAGS) -DUSE_DANZEFF

CFLAGS = $(MY_CFLAGS) $(EXTRA_FLAGS) -g -Wall 	$(INCLUDES)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =

DANZEFF_LIBS = -ldanzeffgu -lpng -lz -lm  -lpspgu -lpspaudio -ljpeg -lpsphprm

LIBS =    $(DANZEFF_LIBS) -lpspsdk -lpspwlan -lpspnet -lPluginCommon

the_pbp: $(OBJS) EBOOT.PBP

the_prx: $(TARGET).prx

prx:
	make BUILD_PRX=1 the_prx

stand_alone: 
	make EXTRA_FLAGS="-DSTAND_ALONE_APP" EXPORTED_OBJS="" COMMON_OBJS="" the_pbp

clean2:
	rm -f *.o *.elf *.prx *.pbp *.PBP

install:
	cp -vf $(TARGET).prx $(PSPRADIO_RELEASE_DIR)
	mkdir -p $(PSPRADIO_RELEASE_DIR)/$(TARGET)
	cp -vf tips.html $(PSPRADIO_RELEASE_DIR)/$(TARGET)

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
