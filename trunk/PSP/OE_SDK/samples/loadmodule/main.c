#include <pspsdk.h>
#include <pspuser.h>
#include <pspctrl.h>
#include <systemctrl.h>
#include <systemctrl_se.h>

#include <string.h>

PSP_MODULE_INFO("BootLoader", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

int main_thread(SceSize args, void *argp)
{
	SceUID mod;
	char file[256];

	if (argp)
	{
		char *p = argp+strlen((char *)argp)-10;
		*p = 0;
		sceIoChdir(argp);
	}

	pspDebugScreenInit();

	pspDebugScreenPrintf("Press X to load umd EBOOT.BIN.\n");
	pspDebugScreenPrintf("Press O to load the homebrew at ms0:/PSP/GAME/HOMEBREW/EBOOT.PBP\n");
	pspDebugScreenPrintf("Press square to load EBOOT.BIN of iso ""ms0:/ISO/myumd.iso""\n");


	while (1)
	{
		SceCtrlData pad;

		sceCtrlReadBufferPositive(&pad, 1);

		if (pad.Buttons & PSP_CTRL_CROSS)
		{
			strcpy(file, "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN");
			break;
		}
		else if (pad.Buttons & PSP_CTRL_CIRCLE)
		{
			strcpy(file, "ms0:/PSP/GAME/HOMEBREW/EBOOT.PBP");
			break;
		}
		else if (pad.Buttons & PSP_CTRL_SQUARE)
		{
			if (sctrlHENIsSE() && !sctrlHENIsDevhook())
			{
				SEConfig config;

				sctrlSEGetConfig(&config);

				if (config.usenoumd)
				{
					sctrlSEMountUmdFromFile("ms0:/ISO/myumd.iso", 1, 1);
				}
				else
				{
					sctrlSEMountUmdFromFile("ms0:/ISO/myumd.iso", 0, config.useisofsonumdinserted);
				}

				strcpy(file, "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN");
				break;
			}
			else
			{
				pspDebugScreenPrintf("\nThis operation can only be performed in SE without devhook running.\n");
				sceKernelDelayThread(200000);
			}
		}

		sceKernelDelayThread(10000);
	}

	pspDebugScreenPrintf("\nLoading. Wait...\n");

	mod = sceKernelLoadModule("loadprx.prx", 0, NULL);
	
	if (mod < 0)
	{
		pspDebugScreenPrintf("Cannot load module: 0x%08X\n", mod);
	}
	else
	{
		mod = sceKernelStartModule(mod, strlen(file)+1, file, NULL, NULL);

		if (mod < 0)
		{
			pspDebugScreenPrintf("Cannot start module: 0x%08X\n", mod);
		}
	}	

	return sceKernelExitDeleteThread(0);
}

int module_start(SceSize args, void *argp)
{
	SceUID th = sceKernelCreateThread("main_thread", main_thread, 0x20, 0x10000, 0, NULL);

	if (th >= 0)
	{
		sceKernelStartThread(th, args, argp);
	}

	return 0;
}

