##
## FTPD Makefile
##

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = FTPDRafTest v0.21raf1
PSP_EBOOT_ICON = resources/logo.png

GCC = psp-gcc
PSP_LD = psp-ld

CFLAGS = $(INCLUDES) -O1 -G0 -Wall -Werror -g -mgp32 -mlong32 -DVERSIONSTRING="\"FTPD v0.21\""
LIBS = -L../SharedLib/lib -lnet ../SharedLib/lib/Tools.a ../SharedLib/lib/iniparser.a
INCLUDES = -I../SharedLib/libnet -I../SharedLib/Tools -I../SharedLib/iniparser

TARGET = ftpd
OBJS = main.o sutils.o ftp.o _itoa.o log.o 

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
