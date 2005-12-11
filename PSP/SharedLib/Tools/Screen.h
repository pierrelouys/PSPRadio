#ifndef __CSCREEN__
	#define __CSCREEN__

	#define PSP_SCREEN_WIDTH 480
	#define PSP_SCREEN_HEIGHT 272
	#define PSP_LINE_SIZE 512
	#define PSP_PIXEL_FORMAT 3
	
	

	class CScreen
	{
	public:
		enum textmode
		{
			TEXTMODE_NORMAL,
			TEXTMODE_OUTLINED,
			TEXTMODE_SHADOWED
		};
				
		CScreen();
		void SetBackColor(u32 colour);
		void SetTextColor(u32 colour);
		void SetBackgroundImage(char *strImage);
		void SetTextMode(textmode mode){m_TextMode = mode;}
		void SetFontSize(int iWidth, int iHeight);
		int  GetFontHeight(){return m_FontHeight;}
		int  GetFontWidth(){return m_FontWidth;}
		size_t GetNumberOfTextColumns(){ return PSP_SCREEN_WIDTH/m_FontWidth; }
		size_t GetNumberOfTextRows(){ return PSP_SCREEN_HEIGHT/m_FontHeight; }
		void Printf(const char *format, ...);
		void PrintText(int pixel_x, int pixel_y, int color, char *string);
		void Clear();
		void SetXY(int x, int y);
		int  GetX();
		int  GetY();
		void ClearNChars(int X, int Y, int N);
		void ClearLine(int Y);
		void ClearCharsAtYFromX1ToX2(int pixel_y, int pixel_x1, int pixel_x2);
		
	private:
		int X, Y;
		int MX, MY;
		u32 bg_col, fg_col;
		u32* g_vram_base;
		bool init;
		char *m_strImage;
		u32  *m_ImageBuffer;
		textmode m_TextMode;
		int m_FontWidth, m_FontHeight;

		void Init();
		void PutChar( int x, int y, u32 color, u8 ch, bool do_background = true);
		int  PrintData(const char *buff, int size);
		void LoadImage(const char* filename, u32 *ImageBuffer);
		void clear_screen(u32 color);
		void PutEraseChar( int x, int y, u32 color);
		void PutCharWithOutline(int x, int y, u32 bg_color, u32 fg_color, u8 ch);
		void PutCharWithShadow(int x, int y, u32 bg_color, u32 fg_color, u8 ch);
		void ShowBackgroundPng(u32 x1, u32 y1, u32 x2, u32 y2);

	};
		
	
#endif
