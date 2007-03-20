BASE_DIR = ../..
include $(BASE_DIR)/include.mak

TARGET=VIS_ScopeBars

OBJS = Scope.o dummyimport.o

# Define to build this as a prx (instead of a static elf)
BUILD_PRX=1
# Define the name of our custom exports (minus the .exp extension)
PRX_EXPORTS=$(PLUGINS_DIR)/VIS_Exports.exp

INCLUDES = -I./ -I../
		 
CFLAGS = $(GLOBAL_CFLAGS) -O2 -g -Wall $(INCLUDES) $(SET_F)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS += -mno-crt0 -nostartfiles

#LIBS = -lm -lpng

#USE_KERNEL_LIBS=1
#USE_KERNEL_LIBC=1
#USE_PSPSDK_LIBC=1

LIBDIR =

scope_bars:
	TARGET="VIS_ScopeBars" SET_F="-DSCOPE_BARS" make the_prx
scope_dots:
	make TARGET="VIS_ScopeDots" SET_F="-DSCOPE_DOTS" the_prx
scope_lines:
	make TARGET="VIS_ScopeLines" SET_F="-DSCOPE_LINES" the_prx
scope_trail:
	make TARGET="VIS_ScopeTrail" SET_F="-DSCOPE_TRAIL" the_prx
scope_twoch:
	make TARGET="VIS_ScopeTwoCh" SET_F="-DSCOPE_TWOCH" the_prx

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
	make scope_trail 
	make clean_objs
	make scope_twoch 

install: all
	cp -vf *.prx $(PSPRADIO_RELEASE_DIR)

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
