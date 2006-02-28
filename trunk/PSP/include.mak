#Note: BASE_DIR needs to be defined in the including Makefile to point to this directory.

GLOBAL_CFLAGS = -G0 -mno-explicit-relocs

PSP_EBOOT_ICON = "$(BASE_DIR)/Resources/Icons_Sem/fade-icon0.png"
PSP_EBOOT_PIC1 = "$(BASE_DIR)/Resources/Icons_Sem/fade-pic1.png"

RELEASE_DIR           = $(BASE_DIR)/release
KXPLOIT_PRIMARY_DIR   = "$(RELEASE_DIR)/v1.5/PSPRadio"
KXPLOIT_SECONDARY_DIR = "$(RELEASE_DIR)/v1.5/PSPRadio%"
RELEASE_DIR_FOR_V1    = "$(RELEASE_DIR)/v1.0/PSPRadio"

BOOTSTRAP_DIR = $(BASE_DIR)/Bootstrap
PSPRADIO_DIR  = $(BASE_DIR)/PSPRadio
SHAREDLIB_DIR = $(BASE_DIR)/SharedLib
PLUGINS_DIR   = $(BASE_DIR)/Plugins
