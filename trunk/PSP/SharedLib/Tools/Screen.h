#ifndef __CSCREEN__
	#define __CSCREEN__

	#include <pspdisplay.h>
	class CScreen
	{
	public:
		enum textmode
		{
			TEXTMODE_NORMAL,
			TEXTMODE_OUTLINED,
			TEXTMODE_SHADOWED
		};
				
		CScreen(int width = 480, int height = 272, 
				int pitch = 512, int pixel_format = PSP_DISPLAY_PIXEL_FORMAT_8888);
		~CScreen();
		void SetBackColor(u32 colour);
		void SetTextColor(u32 colour);
		void LoadBuffer(int iBuffer, const char *strImage);
		void SetTextMode(textmode mode){m_TextMode = mode;}
		void SetFontSize(int iWidth, int iHeight);
		int  GetFontHeight(){return m_FontHeight;}
		int  GetFontWidth(){return m_FontWidth;}
		size_t GetNumberOfTextColumns(){ return m_Width/m_FontWidth; }
		size_t GetNumberOfTextRows(){ return m_Height/m_FontHeight; }
		
	private:
		u32 bg_col, fg_col;
		bool init;
		char *m_strImage;
		u32  *m_ImageBuffer;
		textmode m_TextMode;
		int m_FontWidth, m_FontHeight;

		void Init();
		int  PrintData(const char *buff, int size);
		void LoadImage(const char* filename, u32 *ImageBuffer);
		void ShowBackgroundPng(u32 x1, u32 y1, u32 x2, u32 y2);
		
		//NEW:
	public:
		void CopyRectangle(int iFromBuffer, int iDestBuffer, u32 x1, u32 y1, u32 x2, u32 y2);
		void SetFrameBuffer(int iBuffer);
		void PrintText(int iBuffer, int pixel_x, int pixel_y, int color, char *string);
		void Effect(int iBuffer);
		void Plot(int iBuffer, int x, int y, int color);
		void VertLine(int iBuffer, int x, int y1, int y2, int color);
		int  Peek(int iBuffer, int x, int y);
		void Rectangle(int iBuffer, int x1, int y1, int x2, int y2, int color);
		void CopyFromToBuffer(int iBufferFrom, int iBufferTo);
				
	public:
		u32 *m_Buffer[4]; // 4 FRAMES in VRAM
		int FRAMESIZE;
		int m_Width;
		int m_Height;
		int m_Pitch;
		int m_PixelFormat;
		
	private:
		void PutChar(int iBuffer, int x, int y, u32 color, u8 ch);
		void PutCharWithOutline(int iBuffer, int x, int y, u32 bg_color, u32 fg_color, u8 ch);
		void PutCharWithShadow(int iBuffer, int x, int y, u32 bg_color, u32 fg_color, u8 ch);

	};
		
	
#endif
