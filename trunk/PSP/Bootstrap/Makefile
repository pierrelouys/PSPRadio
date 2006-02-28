BASE_DIR = ..
include $(BASE_DIR)/include.mak

TARGET = BootRadio
OBJS = BootstrapMain.o $(PSPRADIO_DIR)/PSPRadioExports.o

USE_PSPSDK_LIBC = 1

INCDIR = 
CFLAGS = $(GLOBAL_CFLAGS) -O2 -g -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS = -mno-crt0 -nostartfiles

LIBDIR =

LIBS = 

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSPRadio

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	#make v1
	make kxploit
	
v1:
	#make 
	
kxploit: $(TARGET).elf $(PSP_EBOOT_SFO) $(EXTRA_TARGETS) $(FINAL_TARGET)
	mkdir -p $(KXPLOIT_PRIMARY_DIR)
	$(STRIP) $(TARGET).elf -o $(KXPLOIT_PRIMARY_DIR)/$(PSP_EBOOT)
	mkdir -p $(KXPLOIT_SECONDARY_DIR)
	$(PACK_PBP) $(KXPLOIT_SECONDARY_DIR)/$(PSP_EBOOT) $(PSP_EBOOT_SFO) $(PSP_EBOOT_ICON)  \
		$(PSP_EBOOT_ICON1) $(PSP_EBOOT_UNKPNG) $(PSP_EBOOT_PIC1)  \
		$(PSP_EBOOT_SND0) NULL $(PSP_EBOOT_PSAR)
	
