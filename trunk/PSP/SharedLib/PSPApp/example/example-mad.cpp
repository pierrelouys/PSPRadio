/* 
	PSPApp Example 2
	Using MAD
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
PSP_MODULE_INFO("MADEXAMPLE", 0x1000, 1, 1);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_VFPU);

#define CFG_FILENAME "madraf.cfg"

class myPSPApp : public CPSPApp
{
private:
	CIniParser *config;
	CPSPSound_MP3 *MP3;
	
public:
	/** Setup */
	int Setup(int argc, char **argv)
	{
		printf("PSPApp MAD Example...\n");
		
		//open config file
		char strCfgFile[256];
		char strDir[256];
		strcpy(strDir, argv[0]);
		dirname(strDir); /** Retrieve the directory name */
		sprintf(strCfgFile, "%s/%s", strDir, CFG_FILENAME);

		config = new CIniParser(strCfgFile);
		
		//sceKernelDelayThread(1000000);
		printf("Starting Network...\n");
		EnableNetwork(config->GetInteger("WIFI:PROFILE", 0));

		MP3 = new CPSPSound_MP3();
		if (MP3)
		{
			MP3->SetFile(config->GetStr("MUSIC:FILE"));
			//MP3->Play();
			pspDebugScreenSetXY(8,30);
			printf("O or X = Play/Pause | [] = Stop | ^ = Reconnect\n");
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
				printf("Disabling Network...");
				DisableNetwork();
				printf("Enabling Network...");
				EnableNetwork(config->GetInteger("WIFI:PROFILE", 0));
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

