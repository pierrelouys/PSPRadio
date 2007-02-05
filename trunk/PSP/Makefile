BASE_DIR = $(shell pwd)
include $(BASE_DIR)/include.mak

all:
	make download_shoutcast_db
	make clean
	make pspradio
	make ZIPFILE="PSPRadio$(PSPRADIO_VERSION).zip" zip_release
	make extra_plugins
	make ZIPFILE="PSPRadio$(PSPRADIO_VERSION)_with_extras.zip" zip_release
	echo "Ready."

# new dynamic target builds 2.0+ Compatible PSPRadio (For OE F/W)
pspradio:
	$(MAKE) exports
	cd $(SHAREDLIB_DIR) && $(MAKE)
	cd $(PSPRADIO_DIR)  && $(MAKE)
	$(MAKE) plugins

zip_release:
	cd $(RELEASE_DIR) && zip -r $(ZIPFILE) *.txt PSPRadio/
	
exports:
	cd $(PSPRADIO_DIR) && psp-build-exports -s exports.exp
	cd $(PLUGINS_DIR)  && psp-build-exports -s UI_Exports.exp
	cd $(PLUGINS_DIR)  && psp-build-exports -s FSS_Exports.exp
	cd $(PLUGINS_DIR)  && psp-build-exports -s APP_Exports.exp
	cd $(PLUGINS_DIR)  && psp-build-exports -s GAME_Exports.exp

plugins:
	$(MAKE) clean_plugin_common_objects
	cd $(PLUGINS_DIR)/UI_Text && $(MAKE) && $(MAKE) install
	$(MAKE) clean_plugin_common_objects
	cd $(PLUGINS_DIR)/UI_Text3D && $(MAKE) && $(MAKE) install

extra_plugins:
	$(MAKE) clean_plugin_common_objects
	cd $(PLUGINS_DIR)/FSServer_FTPD && $(MAKE)
	cd $(PLUGINS_DIR)/FSServer_FTPD && $(MAKE) install
	$(MAKE) clean_plugin_common_objects
	cd $(PLUGINS_DIR)/APP_Retawq    && $(MAKE) prx install
	mkdir -p $(PSPRADIO_RELEASE_DIR)/graphics
	cp -vf $(SHAREDLIB_DIR)/danzeff/graphics/*.png $(PSPRADIO_RELEASE_DIR)/graphics/.
	$(MAKE) clean_plugin_common_objects
	cd $(PLUGINS_DIR)/APP_NetScan   && $(MAKE)
	cd $(PLUGINS_DIR)/APP_NetScan   && $(MAKE) install

app_afkim:
	cd $(PLUGINS_DIR)/APP_afkim/ && $(MAKE)
	
app_links2:
	cd $(PLUGINS_DIR)/APP_Links2/links-2.1pre23/build-psp && $(MAKE) -f Makefile.psp.plugin prepare all install

game_psptris:
	cd $(PLUGINS_DIR)/GAME_PSPTris && $(MAKE) -f Makefile.plugin 
	cd $(PLUGINS_DIR)/GAME_PSPTris && $(MAKE) -f Makefile.plugin install
	
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
	make albadross_skin_pack

albadross_skin_pack:
	cp -R Resources/AlbaSkins/*					$(PSPRADIO_RELEASE_DIR)
	cp $(PLUGINS_DIR)/UI_Text/UI_Text.prx 		$(PSPRADIO_RELEASE_DIR)/UI_PSPRadio.prx
	cp $(PLUGINS_DIR)/UI_Text/UI_Text.prx 		$(PSPRADIO_RELEASE_DIR)/UI_Metalle.prx
	cp $(PLUGINS_DIR)/UI_Text/UI_Text.prx 		$(PSPRADIO_RELEASE_DIR)/UI_Premiere.prx
	cp $(PLUGINS_DIR)/UI_Text/UI_Text.prx 		$(PSPRADIO_RELEASE_DIR)/UI_Selector.prx
	cp $(PLUGINS_DIR)/UI_Text3D/UI_Text3D.prx 	$(PSPRADIO_RELEASE_DIR)/UI_3D_Radio.prx
	cp $(PLUGINS_DIR)/UI_Text3D/UI_Text3D.prx 	$(PSPRADIO_RELEASE_DIR)/UI_3D_Blue.prx
	cp $(PLUGINS_DIR)/UI_Text3D/UI_Text3D.prx 	$(PSPRADIO_RELEASE_DIR)/UI_3D_Mario.prx
	rm $(PSPRADIO_RELEASE_DIR)/UI_Text.prx
	rm $(PSPRADIO_RELEASE_DIR)/UI_Text3D.prx
	sed -e 's/UI=UI_Text/UI=UI_PSPRadio/g' PSPRadio/PSPRadio.cfg > $(PSPRADIO_RELEASE_DIR)/PSPRadio.cfg
