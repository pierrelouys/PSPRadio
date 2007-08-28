#Note: BASE_DIR needs to be defined in the including Makefile to point to this directory.

MAKE = PSP_FW_VERSION=271 make

#GLOBAL_CFLAGS = -G0 -mno-explicit-relocs -fno-strict-aliasing -DDYNAMIC_BUILD -DPSPRADIO_VERSION='"$(PSPRADIO_VERSION)"'
GLOBAL_CFLAGS = -G0 -mno-explicit-relocs -fno-strict-aliasing -DDYNAMIC_BUILD \
								-DIF_VERSION='"$(IF_VERSION)"' -DREPO_VERSION='"$(REPO_VERSION)"'

PSP_EBOOT_ICON = "$(BASE_DIR)/Resources/Icons_Sem/fade-icon0.png"
PSP_EBOOT_PIC1 = "$(BASE_DIR)/Resources/Icons_Sem/fade-pic1.png"

RELEASE_DIR           = $(BASE_DIR)/release
PSPRADIO_RELEASE_DIR  = $(RELEASE_DIR)/PSPRadio

PSPRADIO_DIR  = $(BASE_DIR)/PSPRadio
SHAREDLIB_DIR = $(BASE_DIR)/SharedLib
PLUGINS_DIR   = $(BASE_DIR)/Plugins

# PSPRadio version is composed of the interface version + repository version (i/f='1.1', repo='12345')
IF_VERSION	 = $(shell cat $(BASE_DIR)/interface_version.dat)
REPO_VERSION = $(shell $(BASE_DIR)/get_svn_revision.sh $(BASE_DIR))
