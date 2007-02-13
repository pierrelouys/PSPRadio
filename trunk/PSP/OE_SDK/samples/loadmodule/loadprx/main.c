#include <pspsdk.h>
#include <pspkernel.h>
#include <pspmodulemgr_kernel.h>
#include <pspumd.h>

#include <string.h>

PSP_MODULE_INFO("LoaderPRX", 0x1000, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

int WriteFile(char *file, void *addr, int size)
{
	SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	sceIoWrite(fd, addr, size);
	sceIoClose(fd);

	return 0;
}

int main_thread(SceSize args, void *argp)
{
	SceModule *mod;
	SceUID uid;

	/* Unload the loader to free user memory */
	while ((mod = sceKernelFindModuleByName("BootLoader")))
	{
		sceKernelStopModule(mod->modid, 0, NULL, NULL, NULL);
		sceKernelUnloadModule(mod->modid);
	}

	if (strstr((char *)argp, "disc") == argp)
	{
		if (!sceUmdCheckMedium(0))
		{
			sceUmdWaitDriveStat(UMD_WAITFORDISC);
		}

		sceUmdActivate(1, "disc0:"); 
		sceUmdWaitDriveStat(UMD_WAITFORINIT);

		uid = sceKernelLoadModuleDisc(argp, 0, NULL);
	}
	else if (strstr((char *)argp, ".PBP"))
	{
		uid = sceKernelLoadModuleMs2(argp, 0, NULL);
	}	
	else
	{
		// Let's try normal loadmodule
		uid = sceKernelLoadModule(argp, 0, NULL);
	}

	if (uid < 0)
	{
		Kprintf("Error %08X loading module\n", uid);		
	}

	uid = sceKernelStartModule(uid, strlen(argp)+1, argp, NULL, NULL);

	if (uid < 0)
	{
		Kprintf("Error %08X starting module.\n", uid);		
	}

	return sceKernelExitDeleteThread(0);
}

int module_start(SceSize args, void *argp)
{
	SceUID th = sceKernelCreateThread("LoadPrx", main_thread, 8, 16*1024, 0, NULL);

	if (th >= 0)
	{
		sceKernelStartThread(th, args, argp);
	}

	return 0;
}

