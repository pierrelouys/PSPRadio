#ifndef _PSPRADIOGRAPHICSUIDEFINES_
#define _PSPRADIOGRAPHICSUIDEFINES_

#define SAFE_FREE_SURFACE(surface) if(NULL != surface) { SDL_FreeSurface(surface); surface = NULL; }
#define SAFE_DELETE(obj) if(NULL != obj) { delete obj; obj = NULL; }

enum StringJustEnum
{
	JUST_LEFT,
	JUST_RIGHT,
	JUST_CENTER
};

enum StringPosEnum
{
	SP_FILENAME = 0,
	SP_FILETITLE,
	SP_URI,
	SP_BUFFER,
	SP_SAMPLERATE,
	SP_MPEGLAYER,
	SP_STREAM,
	SP_ERROR,
	SP_NETWORK,
	SP_SONGTITLE,
	SP_SONGAUTHOR,
	SP_LENGTH,
	SP_BITRATE,

	SP_ITEM_COUNT
};

enum ButtonPosEnum
{
	BP_PLAY = 0,
	BP_PAUSE,
	BP_STOP,
	BP_LOAD,
	BP_SOUND,

	BP_ITEM_COUNT
};

enum ButtonStateEnum
{
	BS_ON,
	BS_OFF,

	BS_ITEM_COUNT
};

enum ButtonTypeEnum
{
	BT_NORMAL,
	BT_MULTI_STATE,

	BT_ITEM_COUNT
};

struct StringPosType
{
	char szIniName[50];
	SDL_Rect rectPos;
	bool bEnabled;
	StringJustEnum fontJust;
	int nFontIndex;
};

struct ButtonPosType
{
	ButtonTypeEnum type;
	char szIniName[50];
//	SDL_Rect *pSrcRect;
	SDL_Rect onRect;
	SDL_Rect offRect;
	SDL_Rect dstRect;
	ButtonStateEnum currentState;
	bool bEnabled;
};

#endif