BASE_DIR = $(shell pwd)
include $(BASE_DIR)/include.mak

all:
	@echo "Use: 'make static' or 'make dynamic' (for monolithic v/s modularized)"
	@echo "Also, you need to use 'make clean' when going from 'make static' to 'make dynamic'"
	@echo "or vice/versa."

# new dynamic target builds 2.0+ Compatible PSPRadio (For OE F/W)
dynamic:
	make exports
	cd $(SHAREDLIB_DIR) && $(MAKE271) GLOBAL_CFLAGS="$(DYNAMIC_GLOBAL_CFLAGS)"
	cd $(PSPRADIO_DIR)  && $(MAKE271) GLOBAL_CFLAGS="$(DYNAMIC_GLOBAL_CFLAGS)" \
								KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR)
	$(MAKE271) plugins GLOBAL_CFLAGS="$(DYNAMIC_GLOBAL_CFLAGS)" 	KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR) MAKE="$(MAKE271)"

dynamicfor15:
	make exports
	cd $(SHAREDLIB_DIR) && make GLOBAL_CFLAGS="$(DYNAMIC15_GLOBAL_CFLAGS)"
	cd $(BOOTSTRAP_DIR) && make GLOBAL_CFLAGS="$(DYNAMIC15_GLOBAL_CFLAGS)" \
								KXPLOIT_PRIMARY=$(DYNAMIC15_PRIMARY_KXPLOIT_DIR) \
								KXPLOIT_SECONDARY=$(DYNAMIC15_SECONDARY_KXPLOIT_DIR)
	cd $(PSPRADIO_DIR)  && make -f Makefile_dynamic15 GLOBAL_CFLAGS="$(DYNAMIC_GLOBAL_CFLAGS)" \
								KXPLOIT_PRIMARY=$(DYNAMIC15_PRIMARY_KXPLOIT_DIR) \
								KXPLOIT_SECONDARY=$(DYNAMIC15_SECONDARY_KXPLOIT_DIR)
	make plugins GLOBAL_CFLAGS="$(DYNAMIC_GLOBAL_CFLAGS)" 	KXPLOIT_PRIMARY=$(DYNAMIC15_PRIMARY_KXPLOIT_DIR) \
								KXPLOIT_SECONDARY=$(DYNAMIC15_SECONDARY_KXPLOIT_DIR) \
								MAKE=make

static:
	cd $(SHAREDLIB_DIR) && make GLOBAL_CFLAGS="$(STATIC_GLOBAL_CFLAGS)"
	cd $(PSPRADIO_DIR)  && make -f Makefile_static GLOBAL_CFLAGS="$(STATIC_GLOBAL_CFLAGS)" \
								KXPLOIT_PRIMARY=$(STATIC_PRIMARY_KXPLOIT_DIR) \
								KXPLOIT_SECONDARY=$(STATIC_SECONDARY_KXPLOIT_DIR)
	make plugins GLOBAL_CFLAGS="$(STATIC_GLOBAL_CFLAGS)"	KXPLOIT_PRIMARY=$(STATIC_PRIMARY_KXPLOIT_DIR) \
								KXPLOIT_SECONDARY=$(STATIC_SECONDARY_KXPLOIT_DIR) \
								MAKE=make
	cd $(STATIC_PRIMARY_KXPLOIT_DIR) && rm -f *.prx

devhook:
	cd $(SHAREDLIB_DIR) && make GLOBAL_CFLAGS="-DDEVHOOK $(STATIC_GLOBAL_CFLAGS)"
	cd $(PSPRADIO_DIR)  && make -f Makefile_devhook GLOBAL_CFLAGS="-DDEVHOOK $(STATIC_GLOBAL_CFLAGS)" \
								KXPLOIT_PRIMARY=$(DEVHOOK_RELEASE_DIR)
	make plugins GLOBAL_CFLAGS="-DDEVHOOK $(STATIC_GLOBAL_CFLAGS)"	KXPLOIT_PRIMARY=$(DEVHOOK_RELEASE_DIR) \
								MAKE=make
	cd $(DEVHOOK_RELEASE_DIR) && rm -f *.prx

packages:
	make download_shoutcast_db
	make clean
	make static
	make static_zip
	make clean_objects
	make devhook
	make devhook_zip
	make clean_objects
	make dynamic
	make extra_plugins
	make dynamic_zip
	echo "Ready."

dynamic_zip:
	cd $(RELEASE_DIR) && zip -r PSPRadio$(PSPRADIO_VERSION)_Dynamic.zip *.txt dynamic/
	
static_zip:	
	cd $(RELEASE_DIR) && zip -r PSPRadio$(PSPRADIO_VERSION)_Static.zip *.txt static/

devhook_zip:	
	cd $(RELEASE_DIR) && zip -r PSPRadio$(PSPRADIO_VERSION)_DH.zip *.txt dh/

exports:
	cd $(PSPRADIO_DIR) && psp-build-exports -s exports.exp
	cd $(PLUGINS_DIR) && psp-build-exports -s UI_Exports.exp
	cd $(PLUGINS_DIR) && psp-build-exports -s FSS_Exports.exp
	cd $(PLUGINS_DIR) && psp-build-exports -s APP_Exports.exp
	cd $(PLUGINS_DIR) && psp-build-exports -s GAME_Exports.exp

plugins:
	$(MAKE) clean_plugin_common_objects
	cd $(PLUGINS_DIR)/UI_Text && $(MAKE) && $(MAKE) install
	$(MAKE) clean_plugin_common_objects
	cd $(PLUGINS_DIR)/UI_Text3D && $(MAKE) && $(MAKE) install

extra_plugins:
	$(MAKE271) clean_plugin_common_objects
	cd $(PLUGINS_DIR)/FSServer_FTPD && $(MAKE271) GLOBAL_CFLAGS="$(DYNAMIC_GLOBAL_CFLAGS)" \
								KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR)
	cd $(PLUGINS_DIR)/FSServer_FTPD && $(MAKE271) install KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR)
	$(MAKE271) clean_plugin_common_objects
	cd $(PLUGINS_DIR)/APP_Retawq    && $(MAKE271) prx install KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR)
	mkdir -p $(DYNAMIC_PRIMARY_KXPLOIT_DIR)/graphics
	cp -vf $(SHAREDLIB_DIR)/danzeff/graphics/*.png $(DYNAMIC_PRIMARY_KXPLOIT_DIR)/graphics/.
	$(MAKE271) clean_plugin_common_objects
	cd $(PLUGINS_DIR)/APP_NetScan   && $(MAKE271) GLOBAL_CFLAGS="$(DYNAMIC_GLOBAL_CFLAGS)" \
								KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR)
	cd $(PLUGINS_DIR)/APP_NetScan   && $(MAKE271) install KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR)

app_afkim:
	cd $(PLUGINS_DIR)/APP_afkim/ && $(MAKE271) GLOBAL_CFLAGS="$(DYNAMIC_GLOBAL_CFLAGS)" \
								KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR) install
app_links2:
	cd $(PLUGINS_DIR)/APP_Links2/links-2.1pre23/build-psp && $(MAKE271) GLOBAL_CFLAGS="$(DYNAMIC_GLOBAL_CFLAGS)" \
								KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR) prepare prx install_prx

game_psptris:
	cd $(PLUGINS_DIR)/GAME_PSPTris && $(MAKE271) -f Makefile.plugin GLOBAL_CFLAGS="$(DYNAMIC_GLOBAL_CFLAGS)" \
								KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR)
	cd $(PLUGINS_DIR)/GAME_PSPTris && $(MAKE271) -f Makefile.plugin install KXPLOIT_PRIMARY=$(DYNAMIC_PRIMARY_KXPLOIT_DIR)

download_shoutcast_db:
	wget -O PSPRadio/SHOUTcast/newdb.xml "http://www.shoutcast.com/sbin/newxml.phtml?genre=Top500"
	chmod a+w PSPRadio/SHOUTcast/newdb.xml

clean_plugin_common_objects:
	rm -fv $(PLUGINS_DIR)/*.o

clean:
	make clean_objects
	rm -Rfv release
	
clobber:
	make clean
	find . -name "*.log" -exec rm {} \;
	find . -name "*.prx" -exec rm {} \;

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
	cd $(PLUGINS_DIR)/APP_Retawq    && make clean2
	cd $(PLUGINS_DIR)/APP_Links2/links-2.1pre23/build-psp && make clean
	cd $(PLUGINS_DIR)/GAME_PSPTris && make -f Makefile.plugin clean

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
