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
#include <Print.h>
#include <png.h>

#define PSP_LINE_SIZE 512
#define PSP_PIXEL_FORMAT 3

/* baseado nas libs do Duke... */

static int X = 0, Y = 0;
static int MX=SCREEN_MAX_X, MY=SCREEN_MAX_Y;
static u32 bg_col = 0, fg_col = 0xFFFFFFFF;
static u32* g_vram_base = (u32 *) 0x04000000;

static bool init = false;
char *m_strImage = NULL;
u32  *m_ImageBuffer = NULL;

/** PNG Stuff **/
/** From pspsdk's libpng example. (Copyright (c) 2005 Frank Buss <fb@frank-buss.de> (aka Shine)) */
void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
	// ignore PNG warnings
}

/* Load an image and show it to screen */
void LoadImage(const char* filename, u32 *ImageBuffer)
{
	//u32* vram32;
	//u16* vram16;
	//int bufferwidth;
	//int pixelformat;
	//int unknown;
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
	line = (u32*) malloc(width * 4);
	if (!line) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return;
	}
	sceDisplayWaitVblankStart();  // if framebuf was set with PSP_DISPLAY_SETBUF_NEXTFRAME, wait until it is changed
	//u32 *vram_unused;
	//sceDisplayGetFrameBuf((void**)&vram_unused, &bufferwidth, &pixelformat, &unknown);
	//vram16 = (u16*) ImageBuffer;//vram32;
	for (y = 0; y < height; y++) 
	{
		png_read_row(png_ptr, (u8*) line, png_bytep_NULL);
		for (x = 0; x < width; x++) 
		{
			ImageBuffer[y*width+x] = line[x];
		}
	}
	free(line);
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	fclose(fp);
}
/** PNG STUFF */

static void clear_screen(u32 color)
{
    int x;
    u32 *vram = g_vram_base;

    for(x = 0; x < (PSP_LINE_SIZE * PSP_SCREEN_HEIGHT); x++)
    {
		*vram++ = color; 
    }
}

void ScreenInit()
{
	X = Y = 0;
	/* Place vram in uncached memory */
	g_vram_base = (u32 *) (0x40000000 | (u32) sceGeEdramGetAddr());
	sceDisplaySetMode(0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
	sceDisplaySetFrameBuf((void *) g_vram_base, PSP_LINE_SIZE, PSP_PIXEL_FORMAT, 1);
	clear_screen(bg_col);
	init = true;
}

void ScreenSetBackgroundImage(char *strImage)
{
	if (m_strImage)
	{
		free(m_strImage), m_strImage = NULL;
	}
	if (m_ImageBuffer)
	{
		free(m_ImageBuffer), m_ImageBuffer = NULL;
	}
	m_strImage = strdup(strImage);
	m_ImageBuffer = (u32*)malloc(PSP_SCREEN_WIDTH*PSP_SCREEN_WIDTH*sizeof(u32));
	LoadImage(m_strImage, m_ImageBuffer);
}

void ScreenSetBackColor(u32 colour)
{
   bg_col = colour;
}

void ScreenSetTextColor(u32 colour)
{
   fg_col = colour;
}

extern u8 msx[];


void ScreenPutChar( int x, int y, u32 color, u8 ch)
{
   int 	i,j, l;
   u8	*font;
   u32  pixel;
   u32 *vram_ptr;
   u32 *vram;

   if(false == init)
   {
	   return;
   }

   vram = g_vram_base + x;
   vram += (y * PSP_LINE_SIZE);
   
   font = &msx[ (int)ch * 8];
   for (i=l=0; i < 8; i++, l+= 8, font++)
   {
      vram_ptr  = vram;
      for (j=0; j < 8; j++)
	{
          if ((*font & (128 >> j)))
              pixel = color;
          else
              pixel = bg_col;

          *vram_ptr++ = pixel; 
	}
      vram += PSP_LINE_SIZE;
   }
}

void ScreenClearLine(int Y)
{
	if (NULL == m_strImage)
	{
		for (int i=0; i < MX; i++)
		{
			ScreenPutChar( i*7 , Y * 8, bg_col, 219);
		}
	}
	else
	{
	//Get Line from bg image..
	}
}

/* Print non-nul terminated strings */
int ScreenPrintData(const char *buff, int size)
{
	int i;
	int j;
	char c;

	for (i = 0; i < size; i++)
	{
		c = buff[i];
		switch (c)
		{
			case '\n':
						X = 0;
						Y ++;
						if (Y == MY)
							Y = 0;
						ScreenClearLine(Y);
						break;
			case '\t':
						for (j = 0; j < 5; j++) {
							ScreenPutChar( X*7 , Y * 8, fg_col, ' ');
							X++;
						}
						break;
			default:
						ScreenPutChar( X*7 , Y * 8, fg_col, c);
						X++;
						if (X == MX)
						{
							X = 0;
							Y++;
							if (Y == MY)
								Y = 0;
							ScreenClearLine(Y);
						}
		}
	}

	return i;
}

void ScreenPrintf(const char *format, ...)
{
   va_list	opt;
   char     buff[2048];
   int		bufsz;
   
   if(!init)
   {
	   return;
   }
   
   va_start(opt, format);
   bufsz = vsnprintf( buff, (size_t) sizeof(buff), format, opt);
   ScreenPrintData(buff, bufsz);
}


void ScreenSetXY(int x, int y)
{
	if( x<MX && x>=0 ) X=x;
	if( y<MY && y>=0 ) Y=y;
}

int ScreenGetX()
{
	return X;
}

int ScreenGetY()
{
	return Y;
}

void BlitWholeImage()
{
	u16* vram16;
	int bufferwidth;
	int pixelformat;
	u32 x,y;
	u32 *vram32 = NULL;
	int unknown;
	sceDisplayWaitVblankStart();  // if framebuf was set with PSP_DISPLAY_SETBUF_NEXTFRAME, wait until it is changed
	sceDisplayGetFrameBuf((void**)&vram32, &bufferwidth, &pixelformat, &unknown); 
	//vram32 = g_vram_base;//
	vram16 = (u16*) vram32;
	for (y = 0; y < PSP_SCREEN_HEIGHT; y++) {
		for (x = 0; x < PSP_SCREEN_WIDTH; x++) {
			u32 color32 = m_ImageBuffer[y*PSP_SCREEN_WIDTH+x];
			u16 color16;
			int r = color32 & 0xff;
			int g = (color32 >> 8) & 0xff;
			int b = (color32 >> 16) & 0xff;
			switch (pixelformat) {
				case PSP_DISPLAY_PIXEL_FORMAT_565:
					color16 = (r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11);
					vram16[x + y * bufferwidth] = color16;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_5551:
					color16 = (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10);
					vram16[x + y * bufferwidth] = color16;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_4444:
					color16 = (r >> 4) | ((g >> 4) << 4) | ((b >> 4) << 8);
					vram16[x + y * bufferwidth] = color16;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_8888:
					color32 = r | (g << 8) | (b << 16);
					vram32[x + y * bufferwidth] = color32;
					break;
			}
		}
	}
}

void ScreenClear()
{
	int y;

	if(!init)
	{
		return;
	}

	if (m_strImage)
	{
		BlitWholeImage();
		ScreenSetXY(0,0);
	}
	else
	{
		for(y=0;y<MY;y++)
			ScreenClearLine(y);
		ScreenSetXY(0,0);
		clear_screen(bg_col);
	}
}
