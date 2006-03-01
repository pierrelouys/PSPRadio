BASE_DIR = $(shell pwd)
include $(BASE_DIR)/include.mak

all:
	@echo "Use: 'make static' or 'make dynamic' (for monolithic v/s modularized)"
	@echo "Also, you need to use 'make clean' when going from 'make static' to 'make dynamic'"
	@echo "or vice/versa."

dynamic:
	make exports
	cd $(SHAREDLIB_DIR) && make
	cd $(BOOTSTRAP_DIR) && make KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR) \
								KXPLOIT_SECONDARY=$(DYNAMIC_SECONDARY_KXPLOIT_DIR)
	cd $(PSPRADIO_DIR)  && make KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR) \
								KXPLOIT_SECONDARY=$(DYNAMIC_SECONDARY_KXPLOIT_DIR)
	make plugins				KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR) \
								KXPLOIT_SECONDARY=$(DYNAMIC_SECONDARY_KXPLOIT_DIR)
	
static:
	cd $(SHAREDLIB_DIR) && make
	cd $(PSPRADIO_DIR)  && make -f Makefile_static
	make plugins				KXPLOIT_PRIMARY=$(STATIC_PRIMARY_KXPLOIT_DIR) \
								KXPLOIT_SECONDARY=$(STATIC_SECONDARY_KXPLOIT_DIR)
	cd $(PLUGINS_DIR) && rm -f *.prx
	
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

