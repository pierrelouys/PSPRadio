/*
 * Raf: Based on scr_printf.c from pspsdk:
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * scr_printf.c - Debug screen functions.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 *
 * $Id: scr_printf.c 806 2005-07-31 11:47:10Z stefan $
 */
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <psptypes.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspge.h>
#include <stdarg.h>
#include <Screen.h>
#include <valloc.h>
#include <png.h>

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

u32 *third = NULL;

#ifdef GUSCREEN
	#include <pspgu.h>
	unsigned int __attribute__((aligned(16))) list[262144];
#endif

CScreen::CScreen(bool use_cached_vram, int width, int height, int pitch, int pixel_format)
{
	m_Width = width;
	m_Height = height;
	m_Pitch = pitch;
	m_PixelFormat = pixel_format;
	m_DrawingMode = DRMODE_TRANSPARENT;
	m_VRAMIsCached = use_cached_vram;
	switch(m_PixelFormat)
	{
	case PSP_DISPLAY_PIXEL_FORMAT_565:
		FRAMESIZE = m_Pitch * m_Height * 2;
		break;
	case PSP_DISPLAY_PIXEL_FORMAT_5551:
		FRAMESIZE = m_Pitch * m_Height * 2;
		break;
	case PSP_DISPLAY_PIXEL_FORMAT_4444:
		FRAMESIZE = m_Pitch * m_Height * 2;
		break;
	case PSP_DISPLAY_PIXEL_FORMAT_8888:
		FRAMESIZE = m_Pitch * m_Height * 4;
		break;
	};
	
	bg_col = 0; 
	fg_col = 0xFFFFFFFF;
	init = false;
	m_TextMode = TEXTMODE_NORMAL;
	m_FontWidth = 7; 
	m_FontHeight = 8;
	SetFontSize(7, 8);

	Init();
}

CScreen::~CScreen()
{
#ifdef GUSCREEN
	sceGuTerm();

	vfree(m_Buffer[ZBufferIndex]);
	m_Buffer[ZBufferIndex] = NULL;
#endif

	vfree(m_Buffer[DrawBufferIndex]);
	m_Buffer[DrawBufferIndex] = NULL;
	vfree(m_Buffer[DisplayBufferIndex]);
	m_Buffer[DisplayBufferIndex] = NULL;
}

void CScreen::Init()
{
	DrawBufferIndex    = 0;
    DisplayBufferIndex = 1;

	m_Buffer[DrawBufferIndex]    = (u32*)valloc(FRAMESIZE);   /* fb */
	m_Buffer[DisplayBufferIndex] = (u32*)valloc(FRAMESIZE);   /* bb */

#ifdef FBSCREEN	
	int CacheMask = m_VRAMIsCached?0x40000000:0;

	sceDisplaySetMode(0, m_Width, m_Height);
	sceDisplaySetFrameBuf((void *) (CacheMask + m_Buffer[DisplayBufferIndex]),
		m_Pitch, m_PixelFormat, PSP_DISPLAY_SETBUF_NEXTFRAME);
#endif

#ifdef GUSCREEN
	ZBufferIndex = 2;
	m_Buffer[ZBufferIndex] = (u32*)valloc(FRAMESIZE/2); /* zb */

	sceGuInit();

	sceGuStart(GU_DIRECT, list);
	sceGuDrawBuffer(GU_PSM_8888,       (void*)vrelptr(m_Buffer[DrawBufferIndex]),    m_Pitch);
	sceGuDispBuffer(m_Width, m_Height, (void*)vrelptr(m_Buffer[DisplayBufferIndex]), m_Pitch);
	sceGuDepthBuffer(                  (void*)vrelptr(m_Buffer[ZBufferIndex]),       m_Pitch);
	sceGuOffset(2048 - (m_Width / 2), 2048 - (m_Height / 2));
	sceGuViewport(2048, 2048, m_Width, m_Height);
	sceGuScissor(0, 0, m_Width, m_Height);
	sceGuEnable(GU_SCISSOR_TEST);
#if 0
	sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);
	sceGuDepthRange(0xc350, 0x2710);
	sceGuAlphaFunc(GU_GREATER, 0, 0xff);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_CLIP_PLANES);
	sceGuTexMode(GU_PSM_8888, 0, 0, 0);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	sceGuAmbientColor(0xffffffff);
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuFinish();
	sceGuSync(0, 0);
#endif

	sceGuDepthRange(65535,0);
	sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
#endif
	init = true;
}

void CScreen::SetFrameBuffer(int iBuffer)
{
	sceDisplaySetFrameBuf(m_Buffer[iBuffer], 
		m_Pitch, m_PixelFormat, PSP_DISPLAY_SETBUF_NEXTFRAME);
}

void CScreen::SwapBuffers()
{
#ifdef GUSCREEN
	m_Buffer[DisplayBufferIndex] = m_Buffer[DrawBufferIndex];
	m_Buffer[DrawBufferIndex]    = (u32*)vabsptr(sceGuSwapBuffers());
#endif

#ifdef FBSCREEN
	SetFrameBuffer(DrawBufferIndex);
	DrawBufferIndex    = 1 - DrawBufferIndex;
	DisplayBufferIndex = 1 - DisplayBufferIndex;
#endif

}

typedef void (*drmodef)(u32 *pixel, int color);

void dr_mode_transparent(u32 *pixel, int color)
{
	*pixel = *pixel | color;
}
void dr_mode_opaque(u32 *pixel, int color)
{
	*pixel = color;
}

drmodef DrawingModeFunction[] = {
	dr_mode_transparent,
	dr_mode_opaque
};

void CScreen::Plot(u32 *pBuffer, int x, int y, int color)
{
	u32 *pixel = pBuffer + m_Pitch*y + x;

	DrawingModeFunction[m_DrawingMode](pixel, color);
}

void CScreen::VertLine(u32 *pBuffer, int x, int y1, int y2, int color)
{
	if (y1 < y2)
	{
		for (int y = y1<0?0:y1; y <= y2; y++)
		{
			Plot(pBuffer, x, y, color);
		}
	}
	else if (y2 < y1)
	{
		for (int y = y2<0?0:y2; y <= y1; y++)
		{
			Plot(pBuffer, x, y, color);
		}
	}
	else 
	{
		Plot(pBuffer, x, y1, color);
	}
	
}

void CScreen::HorizLine(u32 *pBuffer, int y, int x1, int x2, int color)
{
	if (x1 < x2)
	{
		for (int x = x1<0?0:x1; x <= x2; x++)
		{
			Plot(pBuffer, x, y, color);
		}
	}
	else if (x2 < x1)
	{
		for (int x = x2<0?0:x2; x <= x1; x++)
		{
			Plot(pBuffer, x, y, color);
		}
	}
	else 
	{
		Plot(pBuffer, x1, y, color);
	}
	
}

void CScreen::CopyFromToBuffer(u32 *pSource, u32 *pDest)
{
	sceKernelDcacheWritebackInvalidateAll();

#ifdef GUSCREEN
	sceGuCopyImage(GU_PSM_8888, 0,0, m_Width, m_Height, m_Pitch, pSource, 0,0, m_Pitch, pDest);
	sceGuTexSync(); /* This will stall the rendering pipeline until the current image upload initiated by sceGuCopyImage() has completed. */
#endif

#ifdef FBSCREEN
	memcpy(pDest, pSource, FRAMESIZE);
#endif

	sceKernelDcacheWritebackInvalidateAll();
}

void CScreen::Clear(int iBuffer)
{
	u32 *frame = m_Buffer[iBuffer];
	for (int i = 0; i < (m_Height * m_Pitch); i++)
	{
		*frame++ = 0;
	}
}

void CScreen::StartList()
{
#ifdef GUSCREEN
	sceGuStart(GU_DIRECT, list);
#endif
}

void CScreen::EndList()
{
#ifdef GUSCREEN
	sceGuFinish();
	sceGuSync(0,0);
#endif
}

void CScreen::Clear()
{
#ifdef GUSCREEN
    sceGuClearColor(0x00000000);
    sceGuClearDepth(0);
    sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
#endif
}

int CScreen::Peek(int iBuffer, int x, int y)
{
	u32 *pixel = m_Buffer[iBuffer] + m_Pitch*y + x;
	return *pixel;
}

void CScreen::Rectangle(u32 *pBuffer, int x1, int y1, int x2, int y2, int color)
{
	for (int x = x1;x <= x2; x++)
	{
		for (int y = y1;y <= y2; y++)
		{
			Plot(pBuffer, x, y, color);
		}
	}
}

void CScreen::SetFontSize(int iWidth, int iHeight) 
{ 
	m_FontWidth = iWidth; 
	m_FontHeight = iHeight; 	
}


/** PNG Stuff **/
/** From pspsdk's libpng example. (Copyright (c) 2005 Frank Buss <fb@frank-buss.de> (aka Shine)) */
void BlitImage(u32 x1, u32 y1, u32 x2, u32 y2);
void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
	// ignore PNG warnings
}

/* Load an image and show it to screen */
void CScreen::LoadBuffer(int iBuffer, const char* filename)
{
	u32 *vram32;
	u16 *vram16;
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	size_t x, y;
	u32* line;
	FILE *fp;

	if ((fp = fopen(filename, "rb")) == NULL) return;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fclose(fp);
		return;
	}
	png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, user_warning_fn);
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return;
	}
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
	png_set_strip_16(png_ptr);
	png_set_packing(png_ptr);
	if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
	png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	line = (u32*) malloc(width * 32);//4);
	if (!line) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return;
	}
	vram16 = (u16*) m_Buffer[iBuffer];
	vram32 = m_Buffer[iBuffer];

	for (y = 0; y < height; y++) {
		png_read_row(png_ptr, (u8*) line, png_bytep_NULL);
		for (x = 0; x < width; x++) {
			u32 color32 = line[x];
			u16 color16;
			int r = color32 & 0xff;
			int g = (color32 >> 8) & 0xff;
			int b = (color32 >> 16) & 0xff;
			switch (m_PixelFormat) {
				case PSP_DISPLAY_PIXEL_FORMAT_565:
					color16 = (r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11);
					vram16[x + y * m_Pitch] = color16;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_5551:
					color16 = (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10);
					vram16[x + y * m_Pitch] = color16;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_4444:
					color16 = (r >> 4) | ((g >> 4) << 4) | ((b >> 4) << 8);
					vram16[x + y * m_Pitch] = color16;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_8888:
					color32 = r | (g << 8) | (b << 16);
					vram32[x + y * m_Pitch] = color32;
					break;
			}
		}
	}
	free(line);
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	fclose(fp);
}
/** PNG STUFF */


void CScreen::CopyRectangle(int iFromBuffer, int iDestBuffer, int x1, int y1, int x2, int y2)
{
	if (x1 == -2 || x2 == -2 || y1 == -2 || y2 == -2)
		return;

	if (x1 == -1 || x2 == -1)
		x1 = 0, x2 = m_Width;

	if (y1 == -1 || y2 == -1)
		y1 = 0, y2 = m_Height;
	
	x1 = max(x1, 0);
	x1 = min(x1, m_Width);
	x2 = max(x2, 0);
	x2 = min(x2, m_Width);

	y1 = max(y1, 0);
	y1 = min(y1, m_Height);
	y2 = max(y2, 0);
	y2 = min(y2, m_Height);

	u32 *src = m_Buffer[iFromBuffer] + x1 + (y1*m_Pitch);
	u32 *dst = m_Buffer[iDestBuffer] + x1 + (y1*m_Pitch);
	int xlen_in_bytes = (x2 - x1)*4;
	int ylen = y2 - y1;

	for (int y = 0; y < ylen; y++)
	{
		memcpy(dst + y*m_Pitch, src + y*m_Pitch, xlen_in_bytes);
	}
}

void CScreen::SetBackColor(u32 color)
{
   bg_col = color;
}

void CScreen::SetTextColor(u32 color)
{
   fg_col = color;
}

void CScreen::Effect(int iBuffer)
{
	u32 *pixel = NULL;
	for (int y = 0; y < 100; y++)
	{
		for (int x = 0; x < 100 /*m_Pitch*/; x++)
		{
			pixel = m_Buffer[iBuffer] + m_Pitch*y + x;
			*pixel = *pixel * 2;
		}
	}
	
}

extern u8 msx[];

void CScreen::PutChar(u32 *pBuffer, int x, int y, u32 color, u8 ch)
{
	int 	i,j;
	u8	*font;
	u32 *vram_ptr;
	
	if(false == init)
	{
	   return;
	}
	
	u32 *vram = pBuffer + x + (y * m_Pitch);
	
	
	font = &msx[ (int)ch * 8];
	for (i=0; i < 8; i++, font++)
	{
		vram_ptr  = vram;
		for (j=0; j < 8; j++)
		{
			if ((*font & (128 >> j)))
				*vram_ptr++ = color;
			else
				vram_ptr++;
		}
		vram += m_Pitch;
	}
}

void CScreen::PutCharWithOutline(u32 *pBuffer, int x, int y, u32 bg_color, u32 fg_color, u8 ch)
{
	/** y */	
	PutChar(pBuffer, x,   y, bg_color, ch);
	PutChar(pBuffer, x+1, y, bg_color, ch);	
	PutChar(pBuffer, x+2, y, bg_color, ch);
	/** y + 2 */
	PutChar(pBuffer, x,   y+2, bg_color, ch);
	PutChar(pBuffer, x+1, y+2, bg_color, ch);	
	PutChar(pBuffer, x+2, y+2, bg_color, ch);
	/** y + 1 */
	PutChar(pBuffer, x,   y+1, bg_color, ch);
	PutChar(pBuffer, x+2, y+1, bg_color, ch);
	PutChar(pBuffer, x+1, y+1, fg_color, ch);	
}

void CScreen::PutCharWithShadow(u32 *pBuffer, int x, int y, u32 bg_color, u32 fg_color, u8 ch)
{
	/** x+1,y+1 */
	PutChar(pBuffer, x+1, y+1, bg_color, ch);
	/** x,y */
	PutChar(pBuffer, x, y, fg_color, ch);	
}


void CScreen::PrintText(u32 *pBuffer, int pixel_x, int pixel_y, int color, char *string)
{
	int i = 0;
	char c;

	for (;;i++)
	{
		c = string[i];
		if (0 == c || pixel_x > m_Width || pixel_y > m_Height)
			break;
			
		switch (c)
		{
			case '\n':
			case '\t':
				PutChar(pBuffer, pixel_x, pixel_y, color, ' ' );
				pixel_x+=m_FontWidth;
				break;
			default: 
				switch(m_TextMode)
				{
					case TEXTMODE_NORMAL:
						PutChar(pBuffer, pixel_x, pixel_y, color, c );
						break;
					case TEXTMODE_OUTLINED:
						PutCharWithOutline(pBuffer, pixel_x, pixel_y, 0, color, c );
						break;
					case TEXTMODE_SHADOWED:
						PutCharWithShadow(pBuffer, pixel_x, pixel_y, 0, color, c );
						break;
					default:
						PutChar(pBuffer, pixel_x, pixel_y, color, c );
						break;
				}
				pixel_x+=m_FontWidth;
		}
	}
}
