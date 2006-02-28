BASE_DIR = .
include $(BASE_DIR)/include.mak

all:
	make exports
	cd $(SHAREDLIB_DIR) && make
	cd $(BOOTSTRAP_DIR) && make
	cd $(PSPRADIO_DIR)  && make
	make plugins

plugins:
	cd $(PLUGINS_DIR)/TextUI && make
	cd $(PLUGINS_DIR)/TextUI3D && make
	
clean:
	cd $(SHAREDLIB_DIR) && make clean
	cd $(BOOTSTRAP_DIR) && make clean
	cd $(PSPRADIO_DIR)  && make clean
	cd $(PLUGINS_DIR)/TextUI && make clean
	cd $(PLUGINS_DIR)/TextUI3D && make clean
	rm -Rfv release

exports:
	cd $(PSPRADIO_DIR) && psp-build-exports -s exports.exp
	cd $(PLUGINS_DIR) && psp-build-exports -s UI_Exports.exp
