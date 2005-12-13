
all:
	cd lib && rm -f *.a 
	cd Tools && make clean all
	cd libiniparser && make clean all
	cd Logging && make clean all
	cd PSPApp && make clean all && make -f Makefile.debug clean all && rm *.o 
