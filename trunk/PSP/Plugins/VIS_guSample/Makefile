TARGET=VIS_guSample

OBJS = VisguSample.o dummyimport.o

# Define to build this as a prx (instead of a static elf)
BUILD_PRX=1
# Define the name of our custom exports (minus the .exp extension)
PRX_EXPORTS=VIS_Exports.exp
		 
CFLAGS = -G0 -mno-explicit-relocs -O2 -g -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS += -mno-crt0 -nostartfiles

LIBS = -lpspgu

LIBDIR =


all: exports $(TARGET).prx

exports: 
	psp-build-exports -s VIS_Exports.exp

make_in_pspradio_repo:
	cp ../VIS_Plugin.h ../VIS_Exports.exp .
	make all #clean
	rm PSPRadio_VIS.S
	mkdir -p ../../release/PSPRadio/Sample_guVisualizer_Sources
	cp -vf * ../../release/PSPRadio/Sample_guVisualizer_Sources
	rm VIS_Plugin.h VIS_Exports.exp

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
