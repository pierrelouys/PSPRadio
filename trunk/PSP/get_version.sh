#!/bin/sh
grep PSPRADIO_VERSION PSPRadio/PSPRadio.h | head -n1 | awk -F'"' '{ print $2 }'

