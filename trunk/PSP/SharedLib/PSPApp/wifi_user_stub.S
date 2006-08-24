    .set noreorder

#include "pspstub.s"

    STUB_START   "sceUtility",0x40010000,0x00020005
    STUB_FUNC   0x1579a159,sceUtilityLoadNetModule
    STUB_FUNC   0x64d50c56,sceUtilityUnloadNetModule
    STUB_END
