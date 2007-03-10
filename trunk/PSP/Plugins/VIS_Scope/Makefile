BASE_DIR = ../..
include $(BASE_DIR)/include.mak

TARGET = VIS_Scope1

EXPORTED_OBJS = $(PSPRADIO_DIR)/PSPRadioExports.o 
OBJS = Scope.o $(EXPORTED_OBJS)
# $(PLUGINS_DIR)/UI_Interface.o

# Define to build this as a prx (instead of a static elf)
BUILD_PRX=1
# Define the name of our custom exports (minus the .exp extension)
PRX_EXPORTS=$(PLUGINS_DIR)/VIS_Exports.exp

INCLUDES = -I./ -I$(PLUGINS_DIR) -I$(BASE_DIR) -I$(PSPRADIO_DIR) -I$(SHAREDLIB_DIR)/PSPApp -L$(SHAREDLIB_DIR)/lib \
		 -I$(SHAREDLIB_DIR)/libpspnet -I$(SHAREDLIB_DIR)/libiniparser \
		 -I$(SHAREDLIB_DIR)/Tools -I$(SHAREDLIB_DIR)/Logging \
		 -I$(PSPRADIO_DIR)/ScreenHandler -L$(PLUGINS_DIR)/Common

		 
CFLAGS = $(GLOBAL_CFLAGS) -O2 -g -Wall $(INCLUDES) $(DEFINE)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS += -mno-crt0 -nostartfiles

LIBDIR =


#LIBS = -lstdc++ -lpthread -lTools -liniparser -lPluginCommon  -lpng -lz -lm

scope_bars:
	make TARGET="VIS_ScopeBars" DEFINE="-DSCOPE_BARS" prx
scope_dots:
	make TARGET="VIS_ScopeDots" DEFINE="-DSCOPE_DOTS" prx
scope_lines:
	make TARGET="VIS_ScopeLines" DEFINE="-DSCOPE_LINES" prx
scope_lines2:
	make TARGET="VIS_ScopeLines2" DEFINE="-DSCOPE_LINES2" prx
scope_lines3:
	make TARGET="VIS_ScopeLines3" DEFINE="-DSCOPE_LINES3" prx
scope_lines4:
	make TARGET="VIS_ScopeLines4" DEFINE="-DSCOPE_LINES4" prx

prx: $(TARGET).prx

clean_objs:
	rm -Rf *.o *.elf 

all:
	make clean_objs 
	make scope_bars 
	make clean_objs
	make scope_dots 
	make clean_objs
	make scope_lines 
	make clean_objs
	make scope_lines2 
	make clean_objs
	make scope_lines3 
	make clean_objs
	make scope_lines4

install: all
	cp -vf *.prx $(PSPRADIO_RELEASE_DIR)

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
