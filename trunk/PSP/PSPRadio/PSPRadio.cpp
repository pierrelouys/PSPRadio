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
#include <Logging.h>

using namespace std;

asm(".global __lib_stub_top");
asm(".global __lib_stub_bottom");

/* Define the module info section */
PSP_MODULE_INFO("PSPRADIO", 0x1000, 1, 1);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_VFPU);

#define CFG_FILENAME "PSPRadio.cfg"
#define PL_TERMINATOR "!!**!!"

//#define Log(level, format, args...) pPSPApp->m_Log.Log("PSPRadio", level, format, ## args)
#define ReportError pPSPApp->ReportError

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
		
		/** Remove current terminator */
		if (m_playlist.size() > 0)
		{
			m_playlist.pop_back();
		}
		
		//songdata = (songmetadata*)malloc(sizeof(songdata));
		Log(LOG_INFO, "Adding '%s' to the list.", strFileName);
		memset(&songdata, 0, sizeof(songdata));
		strncpy(songdata.strFileName, strFileName, 256);
		m_playlist.push_back(songdata);
		
		/** Add terminator */
		memset(&songdata, 0, sizeof(songmetadata));
		memcpy(songdata.strFileName, PL_TERMINATOR, 256);
		m_playlist.push_back(songdata);
	}
	
	void LoadPlayListFile(char *strFileName)
	{
		FILE *fd = NULL;
		char strLine[256];
		int iLines = 0;
		int iFormatVersion = 1;
		songmetadata songdata;// = NULL;
		
//		Clear();
		/** Remove current terminator */
		if (m_playlist.size() > 0)
		{
			m_playlist.pop_back();
		}

		
		fd = fopen(strFileName, "r");
		
		pspDebugScreenSetXY(0,0);

		if(fd != NULL)
		{
			while (!feof(fd))
			{
				strLine[0] = 0;
				fgets(strLine, 256, fd);
				if (strlen(strLine) == 0 || strLine[0] == '\r' || strLine[0] == '\n')
				{
					continue;
				}
				if ((iLines == 0) && (strLine[0] == '['))
				{
					iFormatVersion = 2;
					ReportError("This is a version 2 playlist. This is not supported at the moment!");
					
					continue;
				}
				if (strLine[0] == ';')
				{
					/** A comment!, ignore */
					continue;
				}
				
				strLine[strlen(strLine)-1] = 0; /** Remove LF 0D*/
				if (strLine[strlen(strLine)-1] == 0x0D) 
					strLine[strlen(strLine)-1] = 0; /** Remove CR 0A*/
				
				/** We have a line with data here */
				
				switch(iFormatVersion)
				{
					case 1:
						memset(&songdata, 0, sizeof(songmetadata));
						memcpy(songdata.strFileName, strLine, 256);
						m_playlist.push_back(songdata);
						Log(LOG_INFO, "Adding '%s' to the list.", strLine);
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
			ReportError("Unable to open playlist '%s'", strFileName);
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
	myPSPApp(): CPSPApp("PSPRadio", "0.3b"){};

	/** Setup */
	int Setup(int argc, char **argv)
	{
		printf("%s by Raf (http://rafpsp.blogspot.com/) WIP version %s\n",
				GetProgramName(),
				GetProgramVersion());
		
				
		/** open config file */
		char strCfgFile[256];
		char strDir[256];
		strcpy(strDir, argv[0]);
		dirname(strDir); /** Retrieve the directory name */
		sprintf(strCfgFile, "%s/%s", strDir, CFG_FILENAME);

		config = new CIniParser(strCfgFile);

		if (config->GetInteger("DEBUGGING:LOGFILE_ENABLED", 0) == 1)
		{
			/** Set Logging Global Object to use the configured logfile and loglevels */
			Logging.Set(config->GetStr("DEBUGGING:LOGFILE"), (loglevel_enum)config->GetInteger("DEBUGGING:LOGLEVEL", 10));
			Log(LOG_ALWAYS, "%s Version %s Starting", GetProgramName(), GetProgramVersion());
		}
			
		pspDebugScreenSetXY(10,26);
		printf("Enabling Network... ");
		Log(LOG_INFO, "Enabling Network");
		EnableNetwork(config->GetInteger("WIFI:PROFILE", 0));
		pspDebugScreenSetXY(10,26);
		printf("Ready               ");
		printf("IP = %s", GetMyIP());
		Log(LOG_INFO, "Enabling Network: Done. IP='%s'", GetMyIP());
		
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
			
			pspDebugScreenSetXY(8,25);
			printf("O or X = Play/Pause | [] = Stop | ^ = Reconnect");
		}
		else
		{
			printf("Error creating mp3 object\n");
			Log(LOG_ERROR, "Error creating CPSPSound_MP3 object, or CPlaylist object.");
		}
	
		Log(LOG_LOWLEVEL, "Exiting Setup()");


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
			pspDebugScreenSetXY(30,20);
			if (iButtonMask & PSP_CTRL_LTRIGGER)
			{
				MP3->Stop();
				m_PlayList->Prev();
				MP3->SetFile(m_PlayList->GetCurrentFileName());
				sceKernelDelayThread(500000);  
				pspDebugScreenSetXY(30,20);
				printf("PLAY   ");
				MP3->Play();
			}
			else if (iButtonMask & PSP_CTRL_RTRIGGER)
			{
				MP3->Stop();
				m_PlayList->Next();
				MP3->SetFile(m_PlayList->GetCurrentFileName());
				sceKernelDelayThread(500000);  
				pspDebugScreenSetXY(30,20);
				printf("PLAY   ");
				MP3->Play();
			}
			else if (iButtonMask & PSP_CTRL_CROSS || iButtonMask & PSP_CTRL_CIRCLE) 
			{
				switch(playingstate)
				{
					case CPSPSound::STOP:
					case CPSPSound::PAUSE:
						printf("PLAY   ");
						MP3->Play();
						break;
					case CPSPSound::PLAY:
						printf("PAUSE   ");
						MP3->Pause();
						break;
				}
			}
			else if (iButtonMask & PSP_CTRL_SQUARE)
			{
				if (playingstate == CPSPSound::PLAY || playingstate == CPSPSound::PAUSE)
				{
					printf("STOP   ");
					MP3->Stop();
				}
			}
			else if (iButtonMask & PSP_CTRL_TRIANGLE)
			{
				printf("STOP   ");
				MP3->Stop();
				sceKernelDelayThread(50000);  
				
				pspDebugScreenSetXY(10,26);
				printf("Disabling Network...");
				Log(LOG_INFO, "Triangle Pressed. Restarting networking...");

				DisableNetwork();
				sceKernelDelayThread(500000);  
				pspDebugScreenSetXY(10,26);
				printf("Enabling Network... ");
				EnableNetwork(config->GetInteger("WIFI:PROFILE", 0));
				pspDebugScreenSetXY(10,26);
				printf("Ready               ");
			}
		}
	};
	
	int OnMessage(int iMessageId, void *pMessage, int iSenderId)
	{
		if (iMessageId == MID_ERROR)
		{
			pspDebugScreenSetXY(0,30);
			printf("% 70c", ' ');
			pspDebugScreenSetXY(0,30);
			printf("Error: %s", (char*)pMessage);
			Log(LOG_ERROR, (char*)pMessage);
		}
		else
		{
			switch (iSenderId)
			{
			//case PSPAPP_SENDER_ID:
			//	switch(iMessageId)
			//	{
			//	case ERROR
			//case SID_PSPSOUND:
			default:
				switch(iMessageId)
				{
				case MID_THPLAY_BEGIN:
					pspDebugScreenSetXY(30,4);
					printf("Starting Play Thread\n");
					break;
				case MID_THPLAY_END:
					pspDebugScreenSetXY(30,4);
					printf("                    ");
					pspDebugScreenSetXY(0,11);      
					printf("                        ");
					break;
				case MID_THPLAY_BUFCYCLE:
					pspDebugScreenSetXY(0,11);
					printf("Out Buffer: %03d/%03d   ", MP3->GetBufferPopPos(), NUM_BUFFERS);
					break;
				case MID_THPLAY_DONE: /** Done with the current stream! */
					/** If it was playing, then start next song in playlist 
					 *  (Don't do it if the user pressed STOP
					 */
					if (MP3->GetPlayState() == CPSPSound::PLAY)
					{
						//IF URL, then restart instead of going to next!
						sceKernelDelayThread(50000);  
						MP3->Stop();
						m_PlayList->Next();
						MP3->SetFile(m_PlayList->GetCurrentFileName());
						sceKernelDelayThread(500000);  
						pspDebugScreenSetXY(30,20);
						printf("PLAY   ");
						MP3->Play();
					}
					break;
					
				case MID_THDECODE_AWOKEN:
					pspDebugScreenSetXY(0,4);
					printf("Starting Decoding Thread");
					break;
				case MID_THDECODE_ASLEEP:
					pspDebugScreenSetXY(0,4);
					printf("                        ");
					pspDebugScreenSetXY(0,4);        
					printf("                        ");
					pspDebugScreenSetXY(0,10);      
					printf("                        ");
					//pspDebugScreenSetXY(0,18);
					//ReportError("% 70c", ' ');
					break;
					
				case MID_DECODE_STREAM_OPENING:
					pspDebugScreenSetXY(0,18);
					printf("% 70c", 0x20);
					pspDebugScreenSetXY(0,18);
					printf("Stream: %s (Opening)", MP3->GetFile());
					break;
				case MID_DECODE_STREAM_OPEN_ERROR:
					pspDebugScreenSetXY(0,18);
					printf("% 70c", ' ');
					pspDebugScreenSetXY(0,18);
					printf("Stream: %s (Error Opening)", MP3->GetFile());
					break;
				case MID_DECODE_STREAM_OPEN:
					pspDebugScreenSetXY(0,18);
					printf("% 70c", ' ');
					pspDebugScreenSetXY(0,18);
					printf("Stream: %s (Open)", MP3->GetFile());
					break;
				case MID_DECODE_BUFCYCLE:
					pspDebugScreenSetXY(0,10);
					printf("In Buffer:  %03d/%03d   ", MP3->GetBufferPushPos(), NUM_BUFFERS);
					break;
				case MID_DECODE_METADATA_INFO:
					pspDebugScreenSetXY(0,12);
					printf("MetaData '%s'", (char*)pMessage);
					break;
				//case MID_DECODE_DONE:
				case MID_DECODE_FRAME_INFO_HEADER:
					struct mad_header *Header;
					Header = (struct mad_header *)pMessage;
					pspDebugScreenSetXY(0,9);
					printf("%lukbps %dHz ",
							Header->bitrate, 
							Header->samplerate);
					break;
				case MID_DECODE_FRAME_INFO_LAYER:
					printf("MPEG layer %s stream. ", (char*)pMessage);
					break;
					
				}
			}
			
		}
		
		return 0;
	}
	
};

/** main */
int main(int argc, char **argv) 
{
	myPSPApp *PSPApp  = new myPSPApp();
	PSPApp->Setup(argc, argv);
	PSPApp->Run();
	
	return 0;
}

