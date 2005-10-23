#ifndef _PSPRADIOGRAPHICSUIDEFINES_
#define _PSPRADIOGRAPHICSUIDEFINES_

#define SAFE_FREE_SURFACE(surface) if(NULL != surface) { SDL_FreeSurface(surface); surface = NULL; }
#define SAFE_DELETE(obj) if(NULL != obj) { delete obj; obj = NULL; }
#define SAFE_FREE(obj) if(NULL != obj) { free(obj); obj = NULL; }

enum ScreenTypeEnum
{
	ST_MAINSCREEN,
	ST_SHOUTCASTSCREEN,
	ST_SETTINGSSCREEN
};

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
	SP_META_FILETITLE = 0,
	SP_META_URI,
	SP_META_URL,
	SP_META_SAMPLERATE,
	SP_META_MPEGLAYER,
	SP_META_GENRE,
	SP_META_SONGAUTHOR,
	SP_META_LENGTH,
	SP_META_BITRATE,
	SP_META_CHANNELS,
	SP_ERROR,

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
	BP_BUFFER,
	BP_NETWORK,
	BP_STREAM,

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
	SDL_Rect selectorPos;
	bool bHasSelector;
	bool bEnabled;
	int nFontIndex;
};

#endif
