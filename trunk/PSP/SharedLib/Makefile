
all:
	cd libscreen && make install
	cd libiniparser && make
	cd Logging && make
	cd PSPApp && make
	cd danzeff && make -f Makefile.gu
	cd danzeff && make -f Makefile.sdl
	cd danzeff && make -f Makefile.vram
	cd danzeff/puresdl && make -f Makefile.pspradio

clean:
	cd lib && rm -f *.a 
	cd libscreen && make clean
	cd libiniparser && make clean
	cd Logging && make clean
	cd PSPApp && make clean 
	cd danzeff && make -f Makefile.gu  clean
	cd danzeff && make -f Makefile.vram  clean
	cd danzeff && make -f Makefile.sdl clean

