This is the SDK for 3.03 OE-A.

Changes from the SDK of 2.71 SE-C:

sctrlSESetRebootType -> removed

If you want to execute an application in the 1.50 kernel from the 3.03 kernel,
load an start the module "flash0:/kd/reboot150.prx" (or "flash0:/kn/reboot150.prx") before
using the function sceKernelLoadExecVSHMs*

kuKernelInitKeyConfig -> added
sctrlKernelSetInitKeyConfig -> added
sctrlKernelLoadExecVSHMs4 -> added
sctrlHENSetOnApplyPspRelSectionEvent -> added

sctrlKernelSetInitApitype and sctrlKernelSetDevkitVersion now return the previous value.

