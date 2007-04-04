#ifndef __CSCREEN__
	#define __CSCREEN__

	#include <pspdisplay.h>

	#define MAX_BUFFERS 16

	class CScreen
	{
	public:
		enum textmode
		{
			TEXTMODE_NORMAL,
			TEXTMODE_OUTLINED,
			TEXTMODE_SHADOWED
		};
		enum drawingmode
		{
			DRMODE_TRANSPARENT,
			DRMODE_OPAQUE,
		};
				
		CScreen(bool use_cached_vram = false, int width = 480, int height = 272, int pitch = 512, 
				int pixel_format = PSP_DISPLAY_PIXEL_FORMAT_8888);
		~CScreen();
		void SetDrawingMode(drawingmode newmode) { m_DrawingMode = newmode; };
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
		drawingmode m_DrawingMode;
		int m_CacheMask;
		u32 bg_col, fg_col;
		bool init;
		textmode m_TextMode;
		int m_FontWidth, m_FontHeight;

		void Init();
		int  PrintData(const char *buff, int size);
		void LoadImage(const char* filename, u32 *ImageBuffer);
		void ShowBackgroundPng(u32 x1, u32 y1, u32 x2, u32 y2);
		
		//NEW:
	public:
		void CopyRectangle(int iFromBuffer, int iDestBuffer, int x1, int y1, int x2, int y2);
		void SetFrameBuffer(int iBuffer);
		void StartList();
		void EndList();
		void SwapBuffers();
		void PrintText(u32 *pBuffer, int pixel_x, int pixel_y, int color, char *string);
		void PrintText(int iBuffer, int pixel_x, int pixel_y, int color, char *string) 
			{ PrintText(m_Buffer[iBuffer], pixel_x, pixel_y, color, string); }
		void Effect(int iBuffer);
		void Plot(u32 *pBuffer, int x, int y, int color);
		void VertLine(u32 *pBuffer, int x, int y1, int y2, int color);
		void HorizLine(u32 *pBuffer, int y, int x1, int x2, int color);
		int  Peek(int iBuffer, int x, int y);
		void Rectangle(u32 *pBuffer, int x1, int y1, int x2, int y2, int color);
		void CopyFromToBuffer(u32 *pSource, u32 *pDest);
		void CopyFromToBuffer(int iBufferFrom, int iBufferTo)
			{ CopyFromToBuffer(m_Buffer[iBufferFrom], m_Buffer[iBufferTo]); }
		void Clear(int iBuffer);
		void Clear(); /* Clear backbuffer */
		u32* GetBufferAddress(int iBuffer){ return m_Buffer[iBuffer]; };
				
	public:
		u32 *m_Buffer[MAX_BUFFERS];
		int DrawBufferIndex, DisplayBufferIndex, ZBufferIndex;
		int FRAMESIZE;
		int m_Width;
		int m_Height;
		int m_Pitch;
		int m_PixelFormat;
		
	private:
		void PutChar(u32 *pBuffer, int x, int y, u32 color, u8 ch);
		void PutCharWithOutline(u32 *pBuffer, int x, int y, u32 bg_color, u32 fg_color, u8 ch);
		void PutCharWithShadow(u32 *pBuffer, int x, int y, u32 bg_color, u32 fg_color, u8 ch);

	};
		
	
#endif
