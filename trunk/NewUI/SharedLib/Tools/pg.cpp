// primitive graphics for Hello World PSP

#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <string.h>
#include <stdio.h>
#include "pg.h"
#include "fonts/fontNaga10.c"
#include "fonts/font_ega.c_" // Classic EGA font - mixed case

//int analog_x=0, analog_y=0;

extern u16 _plugbmp[];

int HomeVisible=0;

//system call

extern u8 GREY32[32];

//variables
u8 *pg_vramtop=(u8*)0x04000000;
long pg_screenmode;
long pg_showframe;
long pg_drawframe;



void pgWaitVn(u32 count)
{
	for (; count>0; --count) {
		sceDisplayWaitVblankStart();
	}
}


void pgWaitV()
{
	sceDisplayWaitVblankStart();
}


u8 *pgGetVramAddr(u32 x,u32 y)
{
	return pg_vramtop+(pg_drawframe?FRAMESIZE:0)+x*PIXELSIZE*2+y*LINESIZE*2+0x40000000;
}


void pgInit()
{
	sceDisplaySetMode(0,SCREEN_WIDTH,SCREEN_HEIGHT);
	pgScreenFrame(0,0);
}


void pgPrint(u32 x,u32 y,u32 color,const char *str)
{
	while (*str!=0 && x<CMAX_X && y<CMAX_Y) {
		pgPutChar(x*8,y*8,color,0,*str,1,0,1);
		str++;
		x++;
		if (x>=CMAX_X) {
			x=0;
			y++;
		}
	}
}

void pgPrint2(u32 x,u32 y,u32 color,const char *str)
{
	while (*str!=0 && x<CMAX2_X && y<CMAX2_Y) {
		pgPutChar(x*16,y*16,color,0,*str,1,0,2);
		str++;
		x++;
		if (x>=CMAX2_X) {
			x=0;
			y++;
		}
	}
}


void pgPrint4(u32 x,u32 y,u32 color,const char *str)
{
	while (*str!=0 && x<CMAX4_X && y<CMAX4_Y) {
		pgPutChar(x*32,y*32,color,0,*str,1,0,4);
		str++;
		x++;
		if (x>=CMAX4_X) {
			x=0;
			y++;
		}
	}
}

void pgDrawFrame(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
        unsigned char *vptr0;           //pointer to vram
        unsigned long i;

        vptr0=(unsigned char *)pgGetVramAddr(0,0);
        for(i=x1; i<=x2; i++){
                ((unsigned short *)vptr0)[i*PIXELSIZE + y1*LINESIZE] = color;
                ((unsigned short *)vptr0)[i*PIXELSIZE + y2*LINESIZE] = color;
        }
        for(i=y1; i<=y2; i++){
                ((unsigned short *)vptr0)[x1*PIXELSIZE + i*LINESIZE] = color;
                ((unsigned short *)vptr0)[x2*PIXELSIZE + i*LINESIZE] = color;
        }
}

void pgFillBox(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i, j;

	vptr0=(unsigned char *)pgGetVramAddr(0,0);
	for(i=y1; i<=y2; i++){
		for(j=x1; j<=x2; j++){
			((unsigned short *)vptr0)[j*PIXELSIZE + i*LINESIZE] = color;
		}
	}
}

void pgGradientBox(u32 x1, u32 y1, u32 x2, u32 y2, u8 r1, u8 g1, u8 b1, u8 r2, u8 g2, u8 b2, u16 w)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i, j, x, z, c;

	vptr0=(unsigned char *)pgGetVramAddr(0,0);
	for(i=y1; i<=y2; i++){
		for(j=x1; j<=x2; j++){
		  z = j-x1;
		  x = x2-j;
		  if(x<w) {
                  c = RGB(((r1*z)/w), ((g1*z)/w), ((b1*z)/w)) & 0xfff;
                  c+= RGB(((r2*x)/w), ((g2*x)/w), ((b2*x)/w));
                  } else if(z<w) {
                  c = RGB(((r1*x)/w), ((g1*x)/w), ((b1*x)/w)) & 0xfff;
                  c+= RGB(((r2*z)/w), ((g2*z)/w), ((b2*z)/w));
                  } else {
                    c = RGB(r2,g2,b2);
                  }
			((unsigned short *)vptr0)[j*PIXELSIZE + i*LINESIZE] = c;
		}
	}
}

void pgFillvram(u32 color)
{
	u8 *vptr0;		//pointer to vram
	u32 i;

	vptr0=pgGetVramAddr(0,0);
	for (i=0; i<FRAMESIZE/2; i++) {
		*(u16 *)vptr0=color;
		vptr0+=PIXELSIZE*2;
	}
}

static u16 fadebuf[480*272];
void pgSetupFade() {
  int i;
  for(i=0; i<272; i++)
    memcpy(&fadebuf[i*480], pgGetVramAddr(0,i), 480*2);
}

void pgFadevram(u32 p, u16 *buf)
{
	u8 *vptr0;		//pointer to vram
	u32 x,y, r,g,b, c;
	if(!buf) buf = fadebuf;

	for (y=0; y<272; y++) {
	  vptr0=pgGetVramAddr(0,y);
 	  for (x=0; x<480; x++) {
            c = buf[y*480+x];
            b = (((c>>7) & 0xf8)*p)/100;
            g = (((c>>2) & 0xf8)*p)/100;
            r = (((c<<3) & 0xf8)*p)/100;
            *(u16 *)vptr0=RGB(r,g,b);
            vptr0+=PIXELSIZE*2;
          }
	}
}

void pgStipplevram(u32 color)
{
	u8 *vptr0;		//pointer to vram
	u32 x,y;

	for (y=0; y<272; y++) {
	  vptr0=pgGetVramAddr((y&1),y);
 	  for (x=(y&1); x<480; x+=2) {
		*(u16 *)vptr0=color;
		vptr0+=PIXELSIZE*4;
	  }
        }
}


void pgBitBltR(bltrect *sr, u16 dx, u16 dy) {
	u8 *vptr0, *vptr;
	u16 x, y;

	if(sr) {
		u16 sw = sr->w, sh = sr->h;
		u16 *sb = sr->b;

		vptr0=pgGetVramAddr(dx, dy);
		for(y=0; y<sh; y++) {
			sb = sr->b + (sr->s * (y + sr->y)) + sr->x;
			vptr=vptr0;
			for(x=0; x<sw; x++) {
				*(u16 *)vptr = *sb++;
				vptr+=PIXELSIZE*2;
			}
			vptr0+=LINESIZE*2;
		}
	}
}

void pgBitBltD(bltrect *ds, bltrect *sr) {
	u16 x, y;

	if(sr && ds) {
		u16 sw = sr->w, sh = sr->h;
		u16 *sb, *db;

		for(y=0; y<sh; y++) {
			sb = sr->b + (sr->s * (y + sr->y)) + sr->x;
			db = ds->b + (ds->s * (y + ds->y)) + ds->x;
			for(x=0; x<sw; x++) {
				*db++ = *sb++;
			}
		}
	}
}

void pgBitBltRT(bltrect *sr, u16 dx, u16 dy, u16 t) {
	u8 *vptr0, *vptr;
	u16 x, y;

	if(sr) {
		u16 sw = sr->w, sh = sr->h;
		u16 *sb = sr->b;

		vptr0=pgGetVramAddr(dx, dy);
		for(y=0; y<sh; y++, vptr0+=LINESIZE*2) {
			sb = sr->b + (sr->s * y);
			vptr=vptr0;
			for(x=0; x<sw; x++, sb++, vptr+=2) {
			        if(*sb != t)
				  *(u16 *)vptr = *sb;
			}
		}
	}
}

void pgBitBlt(u32 x,u32 y,u32 w,u32 h,u32 mag,const u16 *d)
{
	u8 *vptr0;		//pointer to vram
	u8 *vptr;		//pointer to vram
	u32 xx,yy,mx,my;
	const u16 *dd;
	
	vptr0=pgGetVramAddr(x,y);
	for (yy=0; yy<h; yy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			dd=d;
			for (xx=0; xx<w; xx++) {
				for (mx=0; mx<mag; mx++) {
					*(u16 *)vptr=*dd;
					vptr+=PIXELSIZE*2;
				}
				dd++;
			}
			vptr0+=LINESIZE*2;
		}
		d+=w;
	}
	
}


void pgPutChar(u32 x,u32 y,u32 color,u32 bgcolor,u8 ch,char drawfg,char drawbg,char mag)
{
	u8 *vptr0;		//pointer to vram
	u8 *vptr;		//pointer to vram
	const u8 *cfont;		//pointer to font
	u32 cx,cy;
	u32 b;
	char mx,my;

	// if (ch>255) return;
	cfont=font+ch*8;
	vptr0=pgGetVramAddr(x,y);
	for (cy=0; cy<8; cy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			b=0x80;
			for (cx=0; cx<8; cx++) {
				for (mx=0; mx<mag; mx++) {
					if ((*cfont&b)!=0) {
						if (drawfg) *(u16 *)vptr=color;
					} else {
						if (drawbg) *(u16 *)vptr=bgcolor;
					}
					vptr+=PIXELSIZE*2;
				}
				b=b>>1;
			}
			vptr0+=LINESIZE*2;
		}
		cfont++;
	}
}


void pgScreenFrame(long mode,long frame)
{
	pg_screenmode=mode;
	frame=(frame?1:0);
	pg_showframe=frame;
	if (mode==0) {
		//screen off
		pg_drawframe=frame;
		sceDisplaySetFrameBuf(0,0,0,1);
	} else if (mode==1) {
		//show/draw same
		pg_drawframe=frame;
		sceDisplaySetFrameBuf((char*)pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	} else if (mode==2) {
		//show/draw different
		pg_drawframe=(frame?0:1);
		sceDisplaySetFrameBuf((char*)pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	}
}


void pgScreenFlip()
{
	pg_showframe=(pg_showframe?0:1);
	pg_drawframe=(pg_drawframe?0:1);
	sceDisplaySetFrameBuf((char*)pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,0);
}


void pgScreenFlipV()
{
	pgWaitV();
	pgScreenFlip();
}
