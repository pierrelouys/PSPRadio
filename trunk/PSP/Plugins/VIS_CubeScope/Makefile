TARGET=VIS_CubeScope

OBJS = VisCubeScope.o dummyimport.o

# Define to build this as a prx (instead of a static elf)
BUILD_PRX=1
# Define the name of our custom exports (minus the .exp extension)
PRX_EXPORTS=../VIS_Exports.exp
		 
CFLAGS = -G0 -mno-explicit-relocs -O2 -g -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS += -mno-crt0 -nostartfiles

LIBS = -lpspmath -lpspgum -lpspgu -lm

LIBDIR =


all: exports $(TARGET).prx
	cp *.prx ../../release/PSPRadio

exports: 
	psp-build-exports -s VIS_Exports.exp

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
