#Note: BASE_DIR needs to be defined in the including Makefile to point to this directory.

MAKE = make

GLOBAL_CFLAGS = -G0 -mno-explicit-relocs -DDYNAMIC_BUILD -DPSPRADIO_VERSION='"$(PSPRADIO_VERSION)"'

PSP_EBOOT_ICON = "$(BASE_DIR)/Resources/Icons_Sem/fade-icon0.png"
PSP_EBOOT_PIC1 = "$(BASE_DIR)/Resources/Icons_Sem/fade-pic1.png"

RELEASE_DIR           = $(BASE_DIR)/release
PSPRADIO_RELEASE_DIR  = $(RELEASE_DIR)/PSPRadio

PSPRADIO_DIR  = $(BASE_DIR)/PSPRadio
SHAREDLIB_DIR = $(BASE_DIR)/SharedLib
PLUGINS_DIR   = $(BASE_DIR)/Plugins

PSPRADIO_VERSION = $(shell $(BASE_DIR)/get_version.sh $(PSPRADIO_DIR))
