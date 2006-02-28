#
# GNU C/C++ 2.6
#

LogTool : LogTool.cpp Makefile
	gcc -lstdc++ -Wall -Werror -g -o LogTool LogTool.cpp

install: LogTool
	@echo "Installing LogTool into /usr/bin/"
	@cp LogTool  /usr/bin/
	@echo "Done"

clean:
	rm -rf *.o
	rm -rf LogTool
