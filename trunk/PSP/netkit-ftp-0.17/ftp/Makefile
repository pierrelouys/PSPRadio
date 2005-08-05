##
## FTPD Makefile
##

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = netkitFTPraf1
#PSP_EBOOT_ICON = resources/logo.png

GCC = psp-gcc
PSP_LD = psp-ld

CFLAGS = $(INCLUDES) -O1 -G0 -Wall -g -mgp32 -mlong32 -DVERSIONSTRING="\"netkitFTPraf1\""
#CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti -Werror
#ASFLAGS = $(CFLAGS)
LIBS = -L../../SharedLib/lib ../../SharedLib/lib/libnet.a ../../SharedLib/lib/Tools.a
INCLUDES = -I../../SharedLib/libnet -I../../SharedLib/Tools

TARGET = ftp
OBJS = main-psp.o cmds.o cmdtab.o domacro.o ftp.o glob.o main.o ruserpass.o

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
