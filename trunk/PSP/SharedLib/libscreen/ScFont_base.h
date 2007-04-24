#ifndef __CSCREEN_FONT_BASE__
	#define __CSCREEN_FONT_BASE__

	#include <Screen.h>

	class CSFont
	{
	public:
		enum textmode
		{
			TEXTMODE_NORMAL,
			TEXTMODE_OUTLINED,
			TEXTMODE_SHADOWED
		};
				
		CSFont();
		~CSFont();

		void SetSurface(u32 *addr, u32 visible_width, u32 height, u32 width, u32 Bpp) { m_sfc_addr = addr; m_sfc_width = width; m_sfc_visible_width = visible_width; m_sfc_height = height; m_sfc_Bpp = Bpp; }
	
		void SetBackColor(u32 colour);
		void SetTextColor(u32 colour);
		void SetTextMode(textmode mode){m_TextMode = mode;}
		void SetFontSize(int iWidth, int iHeight);
		int  GetFontHeight(){return m_FontHeight;}
		int  GetFontWidth(){return m_FontWidth;}
		size_t GetNumberOfTextColumns(){ return m_sfc_visible_width/m_FontWidth; }
		size_t GetNumberOfTextRows(){ return m_sfc_height/m_FontHeight; }
		
	private:
		/* Drawing Surface details */
		u32 *m_sfc_addr, m_sfc_width, m_sfc_visible_width, m_sfc_height, m_sfc_Bpp;
		u32 bg_col, fg_col;
		textmode m_TextMode;
		int m_FontWidth, m_FontHeight;

		int  PrintData(const char *buff, u32 size);
		void PutChar(u32 x, u32 y, u32 color, u8 ch);
		void PutCharWithOutline(u32 x, u32 y, u32 bg_color, u32 fg_color, u8 ch);
		void PutCharWithShadow(u32 x, u32 y, u32 bg_color, u32 fg_color, u8 ch);
	
	public:	
		void PrintText(u32 pixel_x, u32 pixel_y, u32 color, const char *string);
		void PrintText(u32 *sfc_addr, u32 pixel_x, u32 pixel_y, u32 color, const char *string)
			{ m_sfc_addr = sfc_addr; PrintText(pixel_x, pixel_y, color, string); }
	};
	
#endif
