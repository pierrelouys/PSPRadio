#!/bin/sh
svn info . | echo "`grep \"Last Changed Rev:\" | awk '{ print $4 }'`"
