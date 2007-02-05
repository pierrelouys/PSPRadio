#!/bin/sh
#grep PSPRADIO_VERSION PSPRadio/PSPRadio.h | head -n1 | awk -F'"' '{ print $2 }'
cd PSPRadio
echo `cat version.dat`.`./get_svn_revision.sh`

