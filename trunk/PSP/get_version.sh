#!/bin/sh
#grep PSPRADIO_VERSION PSPRadio/PSPRadio.h | head -n1 | awk -F'"' '{ print $2 }'
#First argument must be PSPRadio directory
cd $1
echo `cat version.dat`.`./get_svn_revision.sh`

