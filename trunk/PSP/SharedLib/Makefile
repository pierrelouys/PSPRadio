
all:
	cd Tools && make
	cd libiniparser && make
	cd Logging && make
	cd PSPApp && make
	cd libpthread && make
	cd danzeff && make -f Makefile.gu
	cd danzeff && make -f Makefile.sdl

clean:
	cd lib && rm -f *.a 
	cd Tools && make clean
	cd libiniparser && make clean
	cd Logging && make clean
	cd PSPApp && make clean 
	cd libpthread && make clean
	cd danzeff && make -f Makefile.gu  clean
	cd danzeff && make -f Makefile.sdl clean

