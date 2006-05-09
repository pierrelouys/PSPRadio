#!/bin/sh
export SHAREDLIB_DIR="../.."
rm Makefile config.cache config.h config.log config.status  stamp-h 
export psp=yes
export INCL="-I$SHAREDLIB_DIR/Tools"
export CC="psp-gcc -I$SHAREDLIB_DIR/libpthread -I$PSPDEV/psp/include/ -I$PSPDEV/psp/sdk/include/ -I$PSPDEV/psp/include/machine/" 
export CFLAGS="-I$PSPDEV/psp/sdk/include/ -I$PSPDEV/psp/include/machine/ $INCL" 
export LDFLAGS="-L$(psp-config --pspsdk-path)/lib -L$(psp-config --pspsdk-path)/sdk/lib -L$SHAREDLIB_DIR/lib" 
export LIBS="-lpspdebug -lpspdisplay -lpspge -lpspctrl -lpspsdk -lc -lpspnet -lpspnet_inet -lpspnet_apctl -lpspnet_resolver -lpsputility -lpspuser -liberty -lpspsdk -lpspnet -lpspaudio -lpspgu -lpspge -lpsphprm -lpspkernel" 

echo "Building internal configure dependencies..."
cd $SHAREDLIB_DIR/libpthread && pwd && make all || (echo "Error building libpthread"; exit 0);
cd -

echo "Calling configure..."
./Configure psp --prefix=$(psp-config --psp-prefix) threads zlib no-shared no-asm

echo "Done."


