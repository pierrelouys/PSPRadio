/* 
	PSPRadio
	Author: Rafael Cabezas.
	Initial Release: Sept. 2005
*/
#include <PSPApp.h>
#include <PSPSound_MP3.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <iniparser.h>
#include <Tools.h>

asm(".global __lib_stub_top");
asm(".global __lib_stub_bottom");

/* Define the module info section */
PSP_MODULE_INFO("PSPRADIO", 0x1000, 1, 1);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_VFPU);

#define CFG_FILENAME "PSPRadio.cfg"

class myPSPApp : public CPSPApp
{
private:
	CIniParser *config;
	CPSPSound_MP3 *MP3;
	
public:
	/** Setup */
	int Setup(int argc, char **argv)
	{
		printf("PSPRadio\n");
		
		//open config file
		char strCfgFile[256];
		char strDir[256];
		strcpy(strDir, argv[0]);
		dirname(strDir); /** Retrieve the directory name */
		sprintf(strCfgFile, "%s/%s", strDir, CFG_FILENAME);

		config = new CIniParser(strCfgFile);
		
		pspDebugScreenSetXY(10,32);
		printf("Enabling Network... ");
		EnableNetwork(config->GetInteger("WIFI:PROFILE", 0));
		pspDebugScreenSetXY(10,32);
		printf("Ready               ");
		printf("IP = %s", GetMyIP());
		
		MP3 = new CPSPSound_MP3();
		if (MP3)
		{
			MP3->SetFile(config->GetStr("MUSIC:FILE"));
			pspDebugScreenSetXY(8,30);
			printf("O or X = Play/Pause | [] = Stop | ^ = Reconnect\n");
			//if (strcmp(config->GetStr("INITIAL:AUTOPLAY"), "TRUE") == 0)
			//	MP3->Play();

		}
		else
			printf("Error creating mp3 object\n");
			
	
		
		return 0;
	}
	
	void OnExit()
	{
		delete(MP3);
		delete(config);
	}

 
	void OnButtonReleased(int iButtonMask)
	{
		if (MP3)
		{
			CPSPSound::pspsound_state playingstate = MP3->GetPlayState();
			pspDebugScreenSetXY(30,25);
			if (iButtonMask & PSP_CTRL_CROSS || iButtonMask & PSP_CTRL_CIRCLE) 
			{
				if (playingstate == CPSPSound::STOP || playingstate == CPSPSound::PAUSE)
				{
					printf ("PLAY   ");
					MP3->Play();
				}
				else //play
				{
					printf ("PAUSE   ");
					MP3->Pause();
				}
			}
			else if (iButtonMask & PSP_CTRL_SQUARE)
			{
				if (playingstate == CPSPSound::PLAY || playingstate == CPSPSound::PAUSE)
				{
					printf ("STOP   ");
					MP3->Stop();
				}
			}
			else if (iButtonMask & PSP_CTRL_TRIANGLE)
			{
				printf ("STOP   ");
				MP3->Stop();
				sceKernelDelayThread(50000);  
				
				pspDebugScreenSetXY(10,32);
				printf("Disabling Network...");
				DisableNetwork();
				sceKernelDelayThread(500000);  
				pspDebugScreenSetXY(10,32);
				printf("Enabling Network... ");
				EnableNetwork(config->GetInteger("WIFI:PROFILE", 0));
				pspDebugScreenSetXY(10,32);
				printf("Ready               ");
			}
		}
	};
	
};
	
/** main */
int main(int argc, char **argv) 
{
	myPSPApp *PSPApp  = new myPSPApp();
	PSPApp->Setup(argc, argv);
	PSPApp->Run();
	
	return 0;
}

