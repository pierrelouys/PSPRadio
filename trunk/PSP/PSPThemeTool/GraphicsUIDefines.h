#ifndef _PSPRADIOGRAPHICSUIDEFINES_
#define _PSPRADIOGRAPHICSUIDEFINES_

#define SAFE_FREE_SURFACE(surface) if(NULL != surface) { SDL_FreeSurface(surface); surface = NULL; }
#define SAFE_DELETE(obj) if(NULL != obj) { delete obj; obj = NULL; }

enum StringJustEnum
{
	JUST_LEFT,
	JUST_RIGHT,
	JUST_CENTER,

	JUST_ITEM_COUNT
};

enum OutputAreaEnum
{
	OA_PLAYLIST,
	OA_PLAYLISTITEM,
	OA_SETTINGS,

	OA_ITEM_COUNT
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
	BP_VOLUME,

	BP_ITEM_COUNT
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
	char szIniName[50];
	SDL_Rect *pSrcRect;
	SDL_Rect dstRect;
	int nCurrentState;
	bool bEnabled;
	int nButtonCount;
};

struct OutputAreaType
{
	char szIniName[50];
	int nLineCount;
	SDL_Rect srcRect;
	SDL_Rect dstRect;
	SDL_Rect lineSize;
	bool bEnabled;
	int nFontIndex;
};

#endif