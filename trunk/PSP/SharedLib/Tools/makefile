### 
## PSP Libnet makefile
###

GCC = psp-gcc
LD = psp-ld
AR = psp-ar
RANLIB = psp-ranlib

PSPSDK		= $(shell psp-config --pspsdk-path)

CFLAGS		=-march=allegrex -O0 -G0 -Wall -g -mgp32 -mlong32 -I. -I$(PSPSDK)/include
ASFLAGS		=-march=allegrex -g -O2 -G0 -Wall -I.

LIB =../lib/Tools.a
OBJS  = Tools.o pg.o

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) r $(LIB) $(OBJS)

.c.o:
	$(GCC) $(CFLAGS) -c $*.c

.S.o:
	$(GCC) $(ASFLAGS) -c $*.S

clean:
	rm -f *.o $(LIB)
