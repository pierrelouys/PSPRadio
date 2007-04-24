#ifndef __CSCREEN__
	#define __CSCREEN__

	#include <pspdisplay.h>

	#define MAX_BUFFERS 16

	class CScreen
	{
	public:
		enum drawingmode
		{
			DRMODE_TRANSPARENT,
			DRMODE_OPAQUE,
		};
		enum drawingmodeinternal
		{
			DRMODE_TRANSPARENT32,
			DRMODE_OPAQUE32,
			DRMODE_TRANSPARENT16,
			DRMODE_OPAQUE16,
		};
				
		CScreen(bool use_cached_vram = false, int width = 480, int height = 272, int pitch = 512, 
				int pixel_format = PSP_DISPLAY_PIXEL_FORMAT_8888);
		~CScreen();
		void SetDrawingMode(drawingmode newmode);
		void LoadBuffer(int iBuffer, const char *strImage);
		
	private:
		drawingmodeinternal m_DrawingMode;
		int m_CacheMask;
		bool init;

		void Init();
		void LoadImage(const char* filename, u32 *ImageBuffer);
		void ShowBackgroundPng(u32 x1, u32 y1, u32 x2, u32 y2);
		
	public:
		void CopyRectangle(int iFromBuffer, int iDestBuffer, int x1, int y1, int x2, int y2);
		void SetFrameBuffer(int iBuffer);
		void StartList(unsigned int *list); /* Specify list */
		void StartList(); /* Use internal list */
		void EndList();
		void SwapBuffers();
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
		int m_Bpp; /* Bytes per pixel */
		int m_GUPixelFormat;
		int m_TextureFormat;
	};
		
	
#endif
