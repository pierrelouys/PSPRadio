/* 
	PSPRadio
	Author: Rafael Cabezas.
	Initial Release: Sept. 2005
*/
#include <list>
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

using namespace std;

asm(".global __lib_stub_top");
asm(".global __lib_stub_bottom");

/* Define the module info section */
PSP_MODULE_INFO("PSPRADIO", 0x1000, 1, 1);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_VFPU);

#define CFG_FILENAME "PSPRadio.cfg"
#define GOTO_ERROR	pspDebugScreenSetXY(0,20); printf("% 40c",' '); pspDebugScreenSetXY(0,20);
#define PL_TERMINATOR "!!**!!"

class CPlayList
{
public:
	CPlayList()
	{
		m_songiterator = m_playlist.begin();
	};
	
	~CPlayList()
	{
		Clear();
	};
	
	char *GetCurrentFileName()
	{
		return (*m_songiterator).strFileName?(*m_songiterator).strFileName:(char*)"";
	};
	
	char *GetCurrentTitle()
	{
		return (*m_songiterator).strFileTitle?(*m_songiterator).strFileTitle:(char*)"";
	};

	void Next()
	{
		m_songiterator++;
		//if (m_songiterator == m_playlist.end())
		if (strcmp((*m_songiterator).strFileName, PL_TERMINATOR) == 0)
		{
			m_songiterator = m_playlist.begin();
		}
	}
	
	void Prev()
	{
		if (m_songiterator == m_playlist.begin())
		{
			m_songiterator = m_playlist.end();
			m_songiterator--;
			m_songiterator--;
		}
		else
		{
			m_songiterator--;
		}
	}
	
	int GetNumberOfSongs()
	{
		return m_playlist.size();
	}

	void InsertFile(char *strFileName)
	{
		songmetadata songdata;// = NULL;
		//songdata = (songmetadata*)malloc(sizeof(songdata));
		memset(&songdata, 0, sizeof(songdata));
		strncpy(songdata.strFileName, strFileName, 256);
		m_playlist.push_back(songdata);
	}
	
	void LoadPlayListFile(char *strFileName)
	{
		FILE *fd = NULL;
		char strLine[256];
		int iLines = 0;
		int iFormatVersion = 1;
		songmetadata songdata;// = NULL;
		
		Clear();
		
		fd = fopen(strFileName, "r");
		
		pspDebugScreenSetXY(0,0);

		if(fd != NULL)
		{
			while (!feof(fd))
			{
				fgets(strLine, 256, fd);
				if (strlen(strLine) == 0)
				{
					continue;
				}
				if ((iLines == 0) && (strLine[0] == '['))
				{
					iFormatVersion = 2;
					continue;
				}
				
				strLine[strlen(strLine)-1] = 0; /** Remove LF 0D*/
				if (strLine[strlen(strLine)-1] == 0x0D) 
					strLine[strlen(strLine)-1] = 0; /** Remove CR 0A*/
				
				/** We have a line with data here */
				
				switch(iFormatVersion)
				{
					case 1:
						//printf("inserting element %d\n", iLines);
						//songdata = (songmetadata*)malloc(sizeof(songmetadata));
						//if (songdata)
						{
							memset(&songdata, 0, sizeof(songmetadata));
						//	printf("file=%s\n",strLine);
							memcpy(songdata.strFileName, strLine, 256);
						//	printf("inserting in the list\n");
							m_playlist.push_back(songdata);
						//	printf("insertion complete\n");
						}
						//else
						//{
						//	printf("Memory Error \n");
						//}
						break;
					case 2:
						//switch(state) //file/title/length
						break;
				}
						
				iLines++;
			}
			fclose(fd), fd = NULL;
			
			/** Add terminator */
			memset(&songdata, 0, sizeof(songmetadata));
			memcpy(songdata.strFileName, PL_TERMINATOR, 256);
			m_playlist.push_back(songdata);
			
			m_songiterator = m_playlist.begin();
		}
		else
		{
			GOTO_ERROR;
			printf("Unable to open '%s'", strFileName);
		}
	}
	
	void Clear()
	{
		while(m_playlist.size())
		{
			//free(m_playlist.front());
			m_playlist.pop_front();
		}
		m_songiterator = m_playlist.begin();
	};
	
private:
	struct songmetadata
	{
		char strFileName[300];
		char strFileTitle[256];
		char songTitle[256];
		char songAuthor[256];
	};
	list<songmetadata> m_playlist; 
	list<songmetadata>::iterator m_songiterator;
};

class myPSPApp : public CPSPApp
{
private:
	CIniParser *config;
	CPSPSound_MP3 *MP3;
	CPlayList *m_PlayList;
	
public:
	/** Setup */
	int Setup(int argc, char **argv)
	{
		printf("PSPRadio by Raf (http://rafpsp.blogspot.com/) WIP version 0.1a\n");
		
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
		m_PlayList = new CPlayList();

		if (MP3 && m_PlayList)
		{
			if (strlen(config->GetStr("MUSIC:PLAYLIST")))
			{
				m_PlayList->LoadPlayListFile(config->GetStr("MUSIC:PLAYLIST"));
			}
			else if (strlen(config->GetStr("MUSIC:FILE")))
			{
				m_PlayList->InsertFile(config->GetStr("MUSIC:PLAYLIST"));
			}

			MP3->SetFile(m_PlayList->GetCurrentFileName());
			
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
			if (iButtonMask & PSP_CTRL_LTRIGGER)
			{
				MP3->Stop();
				m_PlayList->Prev();
				MP3->SetFile(m_PlayList->GetCurrentFileName());
				sceKernelDelayThread(500000);  
				pspDebugScreenSetXY(30,25);
				printf ("PLAY   ");
				MP3->Play();
			}
			else if (iButtonMask & PSP_CTRL_RTRIGGER)
			{
				MP3->Stop();
				m_PlayList->Next();
				MP3->SetFile(m_PlayList->GetCurrentFileName());
				sceKernelDelayThread(500000);  
				pspDebugScreenSetXY(30,25);
				printf ("PLAY   ");
				MP3->Play();
			}
			else if (iButtonMask & PSP_CTRL_CROSS || iButtonMask & PSP_CTRL_CIRCLE) 
			{
				switch(playingstate)
				{
					case CPSPSound::STOP:
					case CPSPSound::PAUSE:
						printf ("PLAY   ");
						MP3->Play();
						break;
					case CPSPSound::PLAY:
						printf ("PAUSE   ");
						MP3->Pause();
						break;
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

