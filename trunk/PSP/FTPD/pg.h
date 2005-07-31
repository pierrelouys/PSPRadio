#ifndef __PG_H
#define __PG_H

// primitive graphics for Hello World PSP

#define WHITE 0xffff
#define GREY  0xbdef
#define RED   0x801f
#define GREEN 0x83e0
#define BLUE  0xfc00
#define BLACK 0x8000
#define YELLOW 0x83ff
#define ORANGE 0x81ff
#define VIOLET 0xfc1f
#define TEAL 0xffe0

#define RGB(r,g,b) ((((b>>3) & 0x1F)<<10)|(((g>>3) & 0x1F)<<5)|(((r>>3) & 0x1F)<<0)|0x8000)

int analog_x, analog_y;
 
typedef struct {
	u16 x, y;	// origin
	u16 w, h;	// clip size
	u16 s;		// skew
	u16 *b;		// buffer
} bltrect;

u8 *pgGetVramAddr(u32 x,u32 y);
void pgInit();
void pgWaitV();
void pgWaitVn(u32 count);
void pgScreenFrame(long mode,long frame);
void pgScreenFlip();
void pgScreenFlipV();
void pgPrint(u32 x,u32 y,u32 color,const char *str);
void pgPrint2(u32 x,u32 y,u32 color,const char *str);
void pgPrint4(u32 x,u32 y,u32 color,const char *str);
void pgFillvram(u32 color);
void pgBitBltD(bltrect *ds, bltrect *sr);
void pgBitBltR(bltrect *sr, u16 dx, u16 dy);
void pgBitBlt(u32 x,u32 y,u32 w,u32 h,u32 mag,const u16 *d);
void pgPutChar(u32 x,u32 y,u32 color,u32 bgcolor,u8 ch,char drawfg,char drawbg,char mag);
void pgPrintCursor(int* xRef, int* yRef, u32 color, const char *str);

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 272
#define         PIXELSIZE       1                               //in short
#define         LINESIZE        512                             //in short
#define         FRAMESIZE       0x44000                 //in byte
#define CMAX_X 60
#define CMAX_Y 34
#define CMAX2_X 30
#define CMAX2_Y 17
#define CMAX4_X 15
#define CMAX4_Y 8 

#define UPPER_THRESHOLD  0xcf
#define LOWER_THRESHOLD  0x2f

#endif
