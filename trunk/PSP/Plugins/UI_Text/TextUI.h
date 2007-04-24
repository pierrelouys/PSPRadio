#ifndef _PSPRADIOTEXTUI_
#define _PSPRADIOTEXTUI_

#include <UI_Interface.h>
#include "PSPRadio_Exports.h"
#include "ScFont_base.h"
#include "VIS_Plugin.h"
#include <PSPRadio.h>

/* active / dirty bits */
#define BITMASK_PCM								(1<<0)
#define BITMASK_TIME							(1<<1)
#define BITMASK_BATTERY 						(1<<2)
#define BITMASK_BUFFER_PERCENTAGE				(1<<3)
#define BITMASK_SONG_DATA						(1<<4)
#define BITMASK_STREAM_TIME						(1<<5)
#define BITMASK_CONTAINERS						(1<<6)
#define BITMASK_ELEMENTS						(1<<7)
#define BITMASK_BACKGROUND						(1<<8)
#define BITMASK_OPTIONS							(1<<9)
#define BITMASK_ACTIVE_COMMAND					(1<<10)
#define BITMASK_MESSAGE							(1<<11)
#define BITMASK_CURRENT_CONTAINER_SIDE_TITLE	(1<<12)
#define BITMASK_TIMER_1S						(1<<13)

#define BITMASK_ALL (BITMASK_PCM | BITMASK_TIME	| BITMASK_BATTERY | BITMASK_BUFFER_PERCENTAGE | BITMASK_SONG_DATA | BITMASK_STREAM_TIME | BITMASK_CONTAINERS | BITMASK_ELEMENTS | BITMASK_BACKGROUND | BITMASK_OPTIONS |  BITMASK_ACTIVE_COMMAND | BITMASK_MESSAGE | BITMASK_CURRENT_CONTAINER_SIDE_TITLE)

struct screenconfig
{
	CSFont::textmode FontMode;
	int FontWidth;
	int FontHeight;
	char *strBackground;
	int BgColor;
	int FgColor;
	int ContainerListRangeX1, ContainerListRangeX2, ContainerListRangeY1, ContainerListRangeY2;
	int EntriesListRangeX1, EntriesListRangeX2, EntriesListRangeY1, EntriesListRangeY2;
	int ContainerListTitleX, ContainerListTitleY, ContainerListTitleLen;
	int ContainerListTitleUnselectedColor, ContainerListTitleSelectedColor;
	char *strContainerListTitleUnselected, *strContainerListTitleSelected;
	int EntriesListTitleX, EntriesListTitleY, EntriesListTitleLen;
	int EntriesListTitleUnselectedColor, EntriesListTitleSelectedColor;
	char *strEntriesListTitleUnselected, *strEntriesListTitleSelected;
	int BufferPercentageX, BufferPercentageY, BufferPercentageColor;
	int MetadataX1, MetadataLength, MetadataRangeY1, MetadataRangeY2, MetadataColor, MetadataTitleColor;
	int ListsTitleColor;
	int EntriesListColor, SelectedEntryColor, PlayingEntryColor;
	int ProgramVersionX, ProgramVersionY, ProgramVersionColor;
	int StreamOpeningX, StreamOpeningY, StreamOpeningColor;
	int StreamOpeningErrorX, StreamOpeningErrorY, StreamOpeningErrorColor;
	int StreamOpeningSuccessX, StreamOpeningSuccessY, StreamOpeningSuccessColor;
	int CleanOnNewStreamRangeY1, CleanOnNewStreamRangeY2;
	int ActiveCommandX, ActiveCommandY, ActiveCommandColor;
	int ErrorMessageX, ErrorMessageY, ErrorMessageColor;
	int NetworkEnablingX, NetworkEnablingY;
	int NetworkDisablingX, NetworkDisablingY;
	int NetworkReadyX, NetworkReadyY;
	int NetworkEnablingColor,NetworkDisablingColor, NetworkReadyColor;
	int ClockX, ClockY, ClockColor, ClockFormat;
	int BatteryX, BatteryY, BatteryColor;
	int TimeX, TimeY, TimeColor;
	
};

class CTextUI : public IPSPRadio_UI
{
public:
	CTextUI();
	~CTextUI();
	
public:
	int Initialize(char *strCWD, char *strName);
	void Terminate();

	int SetTitle(char *strTitle);
	int DisplayMessage_EnablingNetwork();
	int DisplayMessage_DisablingNetwork();
	int DisplayMessage_NetworkReady(char *strIP);
	int DisplayMainCommands();
	int DisplayActiveCommand(CPSPSound::pspsound_state playingstate);
	int DisplayErrorMessage(char *strMsg);
	int DisplayMessage(char *strMsg);
	int DisplayBufferPercentage(int a);

	/** these are listed in sequential order */
	int OnNewStreamStarted();
	int OnStreamOpening();
	int OnConnectionProgress();
	int OnStreamOpeningError();
	int OnStreamOpeningSuccess();
	int OnNewSongData(MetaData *pData);
	int OnStreamTimeUpdate(MetaData *pData);
	
	/** Screen Handling */
	void Initialize_Screen(IScreen *Screen);
	void UpdateOptionsScreen(list<OptionsScreen::Options> &OptionsList, 
										 list<OptionsScreen::Options>::iterator &CurrentOptionIterator);

	void DisplayContainers(CMetaDataContainer *Container);
	void DisplayElements(CMetaDataContainer *Container);
	void OnCurrentContainerSideChange(CMetaDataContainer *Container);
	void OnScreenshot(CScreenHandler::ScreenShotState state) { m_ScreenShotState = state; }
	
	void OnBatteryChange(int Percentage);
	void OnTimeChange(pspTime *LocalTime);

	void OnButtonReleased(int buttonmask);
	
public:
	int  m_dirtybitmask;
	int  m_activebitmask;
	int  m_refreshbitmask;

	CScreen *m_Screen;
	CSFont  *m_FontEngine;
	CScreenHandler::ScreenShotState m_ScreenShotState;
private:
	CIniParser 	*m_Config;
	char  		*m_strTitle;
	CScreenHandler::Screen m_CurrentScreen;
	screenconfig m_ScreenConfig;
	int 		m_LastBatteryPercentage;
	pspTime 	m_LastLocalTime;
	char 		*m_strCWD;
	char 		*m_strConfigDir;
	CPSPSound *m_Sound;
	//helpers
	enum uicolors
	{
		COLOR_BLACK = 0x00000000,
		COLOR_WHITE = 0x00FFFFFF,
		COLOR_RED   = 0x000000FF,
		COLOR_GREEN = 0x0000FF00,
		COLOR_BLUE  = 0x00FF0000,
		COLOR_CYAN  = 0x00AABB00,
		COLOR_YELLOW= 0x00559999
	};
	void uiPrintf(int iBuffer, int x, int y, int color, char *strFormat, ...);

	int ClearErrorMessage();
	int GetConfigColor(char *strKey);
	void GetConfigPair(char *strKey, int *x, int *y);
	
	void PrintOption(int iBuffer, int x, int y, int c, 
					 char *strName, char *strStates[], 
					 int iNumberOfStates, int iSelectedState, int iActiveState);
	void LoadConfigSettings(IScreen *Screen);
	
	//NEW:
	public:
	int m_iBufferPercentage;
	CMetaDataContainer *m_Container;
	void CTextUI::NewPCMBuffer(short *PCMBuffer);
	list<OptionsScreen::Options> m_OptionsList;
	int m_CurrentOptionIterator;
	CPSPSound::pspsound_state m_CurrentPlayingState;
	char *m_strMessage;
	CLock *m_RenderLock;
	/* For renderloop */
	bool m_bDisplayFPS;
	bool m_IsPlaying;
	clock_t m_TimeStartedPlaying;	
	VisPluginConfig m_vis_cfg;
	VisPluginConfig m_fs_vis_cfg;
	clock_t m_FullscreenWait;
	CPSPRadio *m_PSPRadio;

	void PrintMessage(char *message);
	void PrintTime(int iBuffer, bool draw_background);
	void PrintBattery(int iBuffer, bool draw_background);
	int  PrintBufferPercentage(int iBuffer, bool draw_background);
	int  PrintSongData(int iBuffer, bool draw_background);
	int  PrintStreamTime(int iBuffer, bool draw_background);
	void PrintContainers(int iBuffer, bool draw_background);
	void PrintElements(int iBuffer, bool draw_background);
	void PrintProgramVersion(int iBuffer);
	void PrintOptionsScreen(int iBuffer, bool draw_background);
	int  PrintActiveCommand(int iBuffer, bool draw_background);
	void RenderMessage(u32 *pBuffer);
	void PrintCurrentContainerSideTitle(int iBuffer, bool draw_background);

	static void render_thread(void *);
	void RenderLoop(); /* Called from render_thread */
	void RenderNormal();
	void RenderFullscreenVisualizer();
};



#endif
