#!/bin/sh
svn info $1 | echo "`grep \"Last Changed Rev:\" | awk '{ print $4 }'`"
