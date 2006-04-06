#Note: BASE_DIR needs to be defined in the including Makefile to point to this directory.

STATIC_GLOBAL_CFLAGS  = -G0 -mno-explicit-relocs -Wall -Werror
DYNAMIC_GLOBAL_CFLAGS = -G0 -mno-explicit-relocs -Wall -Werror -DDYNAMIC_BUILD

PSP_EBOOT_ICON = "$(BASE_DIR)/Resources/Icons_Sem/fade-icon0.png"
PSP_EBOOT_PIC1 = "$(BASE_DIR)/Resources/Icons_Sem/fade-pic1.png"

RELEASE_DIR           = $(BASE_DIR)/release

DYNAMIC_PRIMARY_KXPLOIT_DIR    = $(RELEASE_DIR)/dynamic/__SCE__PSPRadio
DYNAMIC_SECONDARY_KXPLOIT_DIR  = $(RELEASE_DIR)/dynamic/%__SCE__PSPRadio
STATIC_PRIMARY_KXPLOIT_DIR     = $(RELEASE_DIR)/static/__SCE__PSPRadio
STATIC_SECONDARY_KXPLOIT_DIR   = $(RELEASE_DIR)/static/%__SCE__PSPRadio

BOOTSTRAP_DIR = $(BASE_DIR)/Bootstrap
PSPRADIO_DIR  = $(BASE_DIR)/PSPRadio
SHAREDLIB_DIR = $(BASE_DIR)/SharedLib
PLUGINS_DIR   = $(BASE_DIR)/Plugins

PSPRADIO_VERSION = $(shell $(BASE_DIR)/get_version.sh)