	.set noreorder

#include "common.s"

  STUB_START "sceNet_Library",0x90000,0x00050005
    STUB_FUNC 0x39af39a6,sceNetInit
    STUB_FUNC 0x281928a9,sceNetTerm
    STUB_FUNC 0xad6844c6,sceNetThreadAbort
    STUB_FUNC 0x0bf0a3ae,sceNetGetLocalEtherAddr
    STUB_FUNC 0xcc393e48,sceNetGetMallocStat

  STUB_END
