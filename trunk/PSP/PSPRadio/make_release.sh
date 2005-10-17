#!/bin/sh
cd SharedLib && make && cd .. && \
cd PSPRadio && make clean all && make -f Makefile.halfast2 kxploit && make -f Makefile.handojin kxploit && make -f Makefile.harbringer kxploit && make -f Makefile.mrpochi kxploit && cd ..
echo "Done!"

