        .set noreorder

#include "common.s"

  STUB_START "sceUtility",0x40010000,0x00060005
    STUB_FUNC 0x4db1e739,sceUtilityNetconfInitStart
    STUB_FUNC 0xf88155f6,sceUtilityNetconfShutdownStart
    STUB_FUNC 0x91e70e35,sceUtilityNetconfUpdate
    STUB_FUNC 0x6332aa39,sceUtilityNetconfGetStatus
    STUB_FUNC 0x5eee6548,sceUtilityCheckNetParam
    STUB_FUNC 0x434d4b3a,sceUtilityGetNetParam
  STUB_END
