BASE_DIR = ../..
include $(BASE_DIR)/include.mak

TARGET=VIS_ScopeBars

EXPORTED_OBJS = $(PSPRADIO_DIR)/PSPRadioExports.o 
OBJS = Scope.o $(EXPORTED_OBJS)

# Define to build this as a prx (instead of a static elf)
BUILD_PRX=1
# Define the name of our custom exports (minus the .exp extension)
PRX_EXPORTS=$(PLUGINS_DIR)/VIS_Exports.exp

INCLUDES = -I./ -I$(PLUGINS_DIR) -I$(BASE_DIR) -I$(PSPRADIO_DIR) -I$(SHAREDLIB_DIR)/PSPApp -L$(SHAREDLIB_DIR)/lib \
		 -I$(SHAREDLIB_DIR)/libpspnet -I$(SHAREDLIB_DIR)/libiniparser \
		 -I$(SHAREDLIB_DIR)/Tools -I$(SHAREDLIB_DIR)/Logging \
		 -I$(PSPRADIO_DIR)/ScreenHandler -L$(PLUGINS_DIR)/Common $(SET_F)

		 
CFLAGS = $(GLOBAL_CFLAGS) -O2 -g -Wall $(INCLUDES) $(SET_F)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS += -mno-crt0 -nostartfiles

LIBDIR =

scope_bars:
	TARGET="VIS_ScopeBars" SET_F="-DSCOPE_BARS" make the_prx
scope_dots:
	make TARGET="VIS_ScopeDots" SET_F="-DSCOPE_DOTS" the_prx
scope_lines:
	make TARGET="VIS_ScopeLines" SET_F="-DSCOPE_LINES" the_prx
scope_lines2:
	make TARGET="VIS_ScopeLines2" SET_F="-DSCOPE_LINES2" the_prx
scope_lines3:
	make TARGET="VIS_ScopeLines3" SET_F="-DSCOPE_LINES3" the_prx
scope_lines4:
	make TARGET="VIS_ScopeLines4" SET_F="-DSCOPE_LINES4" the_prx

the_prx: $(TARGET).prx

clean_objs:
	rm -Rf *.o *.elf 

clobber:
	rm -Rf *.o *.elf *.prx
	
all: scope_bars
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
