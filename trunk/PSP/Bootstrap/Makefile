BASE_DIR = ..
include $(BASE_DIR)/include.mak

TARGET = BootRadio
OBJS = BootstrapMain.o $(PSPRADIO_DIR)/PSPRadioExports.o

USE_PSPSDK_LIBC = 1

CFLAGS = $(GLOBAL_CFLAGS) -O2 -g -Wall -I$(PSPRADIO_DIR) -I$(BASE_DIR)/SharedLib/Logging
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS = -mno-crt0 -nostartfiles

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSPRadio

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	make kxploit
	mkdir -p $(KXPLOIT_PRIMARY)
	mkdir -p $(KXPLOIT_SECONDARY)
	cp -vf $(TARGET)/*  $(KXPLOIT_PRIMARY)
	cp -vf $(TARGET)%/* $(KXPLOIT_SECONDARY)
	
