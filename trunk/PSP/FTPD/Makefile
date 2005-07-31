##
## FTPD Makefile
##

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = FTPDRafTest v0.21raf1
PSP_EBOOT_ICON = resources/logo.png

GCC = psp-gcc
PSP_LD = psp-ld

CFLAGS = $(INCLUDES) -O1 -G0 -Wall -Werror -g -mgp32 -mlong32 -DVERSIONSTRING="\"FTPD v0.21\""
#CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
#ASFLAGS = $(CFLAGS)
LIBS = -L../SharedLib/lib libnet.a ../SharedLib/lib/Tools.a
INCLUDES = -I../SharedLib/libnet -I../SharedLib/Tools

TARGET = ftpd
OBJS = main.o pg.o sutils.o ftp.o _itoa.o log.o 

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
