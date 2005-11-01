#ifndef __PRINT__
	#define __PRINT__

	#define PSP_SCREEN_WIDTH 480
	#define PSP_SCREEN_HEIGHT 272
	#define SCREEN_MAX_X 68
	#define SCREEN_MAX_Y 34
	
	void ScreenInit();
	void ScreenSetBackColor(u32 colour);
	void ScreenSetTextColor(u32 colour);
	void ScreenSetBackgroundImage(char *strImage);
	void ScreenPutChar( int x, int y, u32 color, u8 ch);
	int  ScreenPrintData(const char *buff, int size);
	void ScreenPrintf(const char *format, ...);
	void ScreenSetXY(int x, int y);
	int ScreenGetX();
	int ScreenGetY();
	void ScreenClear();
	void ScreenClearNChars(int X, int Y, int N);
	void ScreenClearLine(int Y);
	
	
#endif
