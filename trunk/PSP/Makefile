BASE_DIR = .
include $(BASE_DIR)/include.mak

all:
	@echo "Use: 'make static' or 'make dynamic' (for monolithic v/s modularized)"
	@echo "Also, you need to use 'make clean' when going from 'make static' to 'make dynamic'"
	@echo "or vice/versa."

dynamic:
	make exports
	cd $(SHAREDLIB_DIR) && make
	cd $(BOOTSTRAP_DIR) && make
	cd $(PSPRADIO_DIR)  && make
	make plugins
	
static:
	cd $(SHAREDLIB_DIR) && make
	cd $(PSPRADIO_DIR)  && make -f Makefile_static
	
exports:
	cd $(PSPRADIO_DIR) && psp-build-exports -s exports.exp
	cd $(PLUGINS_DIR) && psp-build-exports -s UI_Exports.exp

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

