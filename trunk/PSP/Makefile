BASE_DIR = $(shell pwd)
include $(BASE_DIR)/include.mak

all:
	@echo "Use: 'make static' or 'make dynamic' (for monolithic v/s modularized)"
	@echo "Also, you need to use 'make clean' when going from 'make static' to 'make dynamic'"
	@echo "or vice/versa."

dynamic:
	make exports
	cd $(SHAREDLIB_DIR) && make GLOBAL_CFLAGS="$(DYNAMIC_GLOBAL_CFLAGS)"
	cd $(BOOTSTRAP_DIR) && make GLOBAL_CFLAGS="$(DYNAMIC_GLOBAL_CFLAGS)" \
								KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR) \
								KXPLOIT_SECONDARY=$(DYNAMIC_SECONDARY_KXPLOIT_DIR)
	cd $(PSPRADIO_DIR)  && make GLOBAL_CFLAGS="$(DYNAMIC_GLOBAL_CFLAGS)" \
								KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR) \
								KXPLOIT_SECONDARY=$(DYNAMIC_SECONDARY_KXPLOIT_DIR)
	make plugins GLOBAL_CFLAGS="$(DYNAMIC_GLOBAL_CFLAGS)" 	KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR) \
								KXPLOIT_SECONDARY=$(DYNAMIC_SECONDARY_KXPLOIT_DIR)
	
static:
	cd $(SHAREDLIB_DIR) && make GLOBAL_CFLAGS="$(STATIC_GLOBAL_CFLAGS)"
	cd $(PSPRADIO_DIR)  && make -f Makefile_static GLOBAL_CFLAGS="$(STATIC_GLOBAL_CFLAGS)" \
								KXPLOIT_PRIMARY=$(STATIC_PRIMARY_KXPLOIT_DIR) \
								KXPLOIT_SECONDARY=$(STATIC_SECONDARY_KXPLOIT_DIR)
	make plugins GLOBAL_CFLAGS="$(STATIC_GLOBAL_CFLAGS)"	KXPLOIT_PRIMARY=$(STATIC_PRIMARY_KXPLOIT_DIR) \
								KXPLOIT_SECONDARY=$(STATIC_SECONDARY_KXPLOIT_DIR)
	cd $(STATIC_PRIMARY_KXPLOIT_DIR) && rm -f *.prx
	
packages: 
	make clean
	make static
	cd $(RELEASE_DIR) && zip -r PSPRadio$(PSPRADIO_VERSION)_Static.zip *.txt static/
	make clean_objects
	make dynamic
	cd $(RELEASE_DIR) && zip -r PSPRadio$(PSPRADIO_VERSION)_Dynamic.zip *.txt dynamic/
	echo "Ready."
	
exports:
	cd $(PSPRADIO_DIR) && psp-build-exports -s exports.exp
	cd $(PLUGINS_DIR) && psp-build-exports -s UI_Exports.exp
	cd $(PLUGINS_DIR) && psp-build-exports -s FSS_Exports.exp
	cd $(PLUGINS_DIR) && psp-build-exports -s APP_Exports.exp
	
plugins:
	make clean_plugin_common_objects
	cd $(PLUGINS_DIR)/UI_Text && make && make install
	make clean_plugin_common_objects
	cd $(PLUGINS_DIR)/UI_Text3D && make && make install

extra_plugins:
	make clean_plugin_common_objects
	cd $(PLUGINS_DIR)/FSServer_FTPD && make GLOBAL_CFLAGS="$(DYNAMIC_GLOBAL_CFLAGS)" \
								KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR)
	cd $(PLUGINS_DIR)/FSServer_FTPD && make install KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR)
	make clean_plugin_common_objects
	cd $(PLUGINS_DIR)/APP_NetScan   && make GLOBAL_CFLAGS="$(DYNAMIC_GLOBAL_CFLAGS)" \
								KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR)
	cd $(PLUGINS_DIR)/APP_NetScan   && make install KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR)
	make clean_plugin_common_objects
	cd $(PLUGINS_DIR)/APP_Retawq    && make GLOBAL_CFLAGS="$(DYNAMIC_GLOBAL_CFLAGS)" \
								KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR)
	cd $(PLUGINS_DIR)/APP_Retawq    && make install KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR)
	mkdir -p $(DYNAMIC_PRIMARY_KXPLOIT_DIR)/graphics
	cp -vf $(SHAREDLIB_DIR)/danzeff/graphics/*.png $(DYNAMIC_PRIMARY_KXPLOIT_DIR)/graphics/.

clean_plugin_common_objects:
	rm -fv $(PLUGINS_DIR)/*.o

clean:
	make clean_objects
	rm -Rfv release
	
clean_objects:
	cd $(SHAREDLIB_DIR) && make clean
	cd $(BOOTSTRAP_DIR) && make clean
	cd $(PSPRADIO_DIR)  && make clean
	cd $(PLUGINS_DIR)/UI_Text   && make clean
	cd $(PLUGINS_DIR)/UI_Text3D && make clean
	make clean_extra_plugins
	
clean_extra_plugins:
	cd $(PLUGINS_DIR)/FSServer_FTPD && make clean
	cd $(PLUGINS_DIR)/APP_NetScan   && make clean
	cd $(PLUGINS_DIR)/APP_Retawq    && make clean

skin_packs: 
	make KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR) albadross_skin_pack

albadross_skin_pack:
	cp -R Resources/AlbaSkins/*					$(KXPLOIT_PRIMARY)
	cp $(PLUGINS_DIR)/UI_Text/UI_Text.prx 		$(KXPLOIT_PRIMARY)/UI_PSPRadio.prx
	cp $(PLUGINS_DIR)/UI_Text/UI_Text.prx 		$(KXPLOIT_PRIMARY)/UI_Metalle.prx
	cp $(PLUGINS_DIR)/UI_Text/UI_Text.prx 		$(KXPLOIT_PRIMARY)/UI_Premiere.prx
	cp $(PLUGINS_DIR)/UI_Text/UI_Text.prx 		$(KXPLOIT_PRIMARY)/UI_Selector.prx
	cp $(PLUGINS_DIR)/UI_Text3D/UI_Text3D.prx 	$(KXPLOIT_PRIMARY)/UI_3D_Radio.prx
	cp $(PLUGINS_DIR)/UI_Text3D/UI_Text3D.prx 	$(KXPLOIT_PRIMARY)/UI_3D_Blue.prx
	cp $(PLUGINS_DIR)/UI_Text3D/UI_Text3D.prx 	$(KXPLOIT_PRIMARY)/UI_3D_Mario.prx
	rm $(KXPLOIT_PRIMARY)/UI_Text.prx
	rm $(KXPLOIT_PRIMARY)/UI_Text3D.prx
	sed -e 's/UI=UI_Text/UI=UI_PSPRadio/g' PSPRadio/PSPRadio.cfg > $(KXPLOIT_PRIMARY)/PSPRadio.cfg
