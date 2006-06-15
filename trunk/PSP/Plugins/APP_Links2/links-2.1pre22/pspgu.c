/* framebuffer.c
 * Linux framebuffer code
 * (c) 2002 Petr 'Brain' Kulhavy
 * This file is a part of the Links program, released under GPL.
 */

#include "cfg.h"

#ifdef GRDRV_PSPGU

//#define USE_GPM_DX

/* #define pspgu_DEBUG */
/* #define SC_DEBUG */

/* note: SIGUSR1 is used by libpthread and is disabled even if no thread
   functions are called --- do not use */

#define SIG_REL	SIGUSR2
#define SIG_ACQ	SIGVTALRM

#if defined(pspgu_DEBUG) || defined(SC_DEBUG)
	#define MESSAGE(a) fprintf(stderr,"%s",a);
#endif

#ifdef TEXT
#undef TEXT
#endif

#include "links.h"

#include <pthread.h>
#include <pspctrl.h>
static int sf_danzeffOn = 0;

#define PSP_SCREEN_WIDTH 480
#define PSP_SCREEN_HEIGHT 272
#define PSP_LINE_SIZE 512
#define PSP_PIXEL_FORMAT 3
	//#include <gpm.h>

//#include <sys/mman.h>
//#include <sys/ioctl.h>

//#include <linux/fb.h>
//#include <linux/kd.h>
//#include <linux/vt.h>
#include <signal.h>

#include "arrow.inc"

#ifdef GPM_HAVE_SMOOTH
#define gpm_smooth GPM_SMOOTH
#else
#define gpm_smooth 0
#endif

#define TTY 0

#ifndef USE_GPM_DX
int pspgu_txt_xsize, pspgu_txt_ysize;
///struct winsize pspgu_old_ws;
///struct winsize pspgu_new_ws;
int pspgu_old_ws_v;
int pspgu_msetsize;
#endif
int pspgu_hgpm;

int pspgu_console;

struct itrm *pspgu_kbd;

struct graphics_device *pspgu_old_vd;

///int pspgu_handler;
char *pspgu_mem, *pspgu_vmem;
int pspgu_mem_size,pspgu_linesize,pspgu_bits_pp,pspgu_pixelsize;
int pspgu_xsize,pspgu_ysize;
int border_left, border_right, border_top, border_bottom;
int pspgu_colors, pspgu_palette_colors;
///struct pspgu_var_screeninfo vi;
///struct pspgu_fix_screeninfo fi;

void pspgu_draw_bitmap(struct graphics_device *dev,struct bitmap* hndl, int x, int y);

static unsigned char *pspgu_driver_param;
struct graphics_driver pspgu_driver;
int have_cmap=0;
volatile int pspgu_active=1;

struct palette
{
	unsigned char *alpha;
	unsigned char *blue;
	unsigned char *green;
	unsigned char *red;
};

struct palette old_palette;
struct palette global_pal;
///static struct vt_mode vt_mode,vt_omode;

///struct pspgu_var_screeninfo oldmode;

static volatile int in_gr_operation;

/* mouse */
static int mouse_x, mouse_y;		/* mouse pointer coordinates */
static int mouse_black, mouse_white;
static int background_x, background_y; /* Where was the mouse background taken from */
static unsigned char *mouse_buffer, *background_buffer, *new_background_buffer;
static struct graphics_device *mouse_graphics_device;
static int global_mouse_hidden;


#define TEST_MOUSE(xl,xh,yl,yh) if (RECTANGLES_INTERSECT(\
					(xl),(xh),\
					background_x,background_x+arrow_width,\
					(yl),(yh),\
					background_y,background_y+arrow_height)\
					&& !global_mouse_hidden){\
					mouse_hidden=1;\
					hide_mouse();\
				}else mouse_hidden=0;

#define END_MOUSE if (mouse_hidden) show_mouse();

#define START_GR in_gr_operation=1;
#define END_GR	\
		END_MOUSE\
		in_gr_operation=0;

//\
//		if (!pspgu_active)ioctl(TTY,VT_RELDISP,1);
		

#define NUMBER_OF_DEVICES	10



#define TEST_INACTIVITY if (!pspgu_active||dev!=current_virtual_device) return;

#define TEST_INACTIVITY_0 if (!pspgu_active||dev!=current_virtual_device) return 0;

#define RECTANGLES_INTERSECT(xl0, xh0, xl1, xh1, yl0, yh0, yl1, yh1) (\
				   (xl0)<(xh1)\
				&& (xl1)<(xh0)\
				&& (yl0)<(yh1)\
				&& (yl1)<(yh0))

/* This assures that x, y, xs, ys, data will be sane according to clipping
 * rectangle. If nothing lies within this rectangle, the current function
 * returns. The data pointer is automatically advanced by this macro to reflect
 * the right position to start with inside the bitmap. */
#define	CLIP_PREFACE \
	int mouse_hidden;\
	int xs=hndl->x,ys=hndl->y;\
        char *data=hndl->data;\
\
 	TEST_INACTIVITY\
        if (x>=dev->clip.x2||x+xs<=dev->clip.x1) return;\
        if (y>=dev->clip.y2||y+ys<=dev->clip.y1) return;\
        if (x+xs>dev->clip.x2) xs=dev->clip.x2-x;\
        if (y+ys>dev->clip.y2) ys=dev->clip.y2-y;\
        if (dev->clip.x1-x>0){\
                xs-=(dev->clip.x1-x);\
                data+=pspgu_pixelsize*(dev->clip.x1-x);\
                x=dev->clip.x1;\
        }\
        if (dev->clip.y1-y>0){\
                ys-=(dev->clip.y1-y);\
                data+=hndl->skip*(dev->clip.y1-y);\
                y=dev->clip.y1;\
        }\
        /* xs, ys: how much pixels to paint\
         * data: where to start painting from\
         */\
	START_GR\
	TEST_MOUSE (x,x+xs,y,y+ys)


/* fill_area: 5,5,10,10 fills in 25 pixels! */

/* This assures that left, right, top, bottom will be sane according to the
 * clipping rectangle set up by svga_driver->set_clip_area. If empty region
 * results, return from current function occurs. */
#define FILL_CLIP_PREFACE \
	int mouse_hidden;\
	TEST_INACTIVITY\
	if (left>=right||top>=bottom) return;\
	if (left>=dev->clip.x2||right<=dev->clip.x1||top>=dev->clip.y2||bottom<=dev->clip.y1) return;\
	if (left<dev->clip.x1) left=dev->clip.x1;\
	if (right>dev->clip.x2) right=dev->clip.x2;\
	if (top<dev->clip.y1) top=dev->clip.y1;\
	if (bottom>dev->clip.y2) bottom=dev->clip.y2;\
	START_GR\
	TEST_MOUSE(left,right,top,bottom)
	

#define HLINE_CLIP_PREFACE \
	int mouse_hidden;\
	TEST_INACTIVITY\
	if (y<dev->clip.y1||y>=dev->clip.y2||right<=dev->clip.x1||left>=dev->clip.x2) return;\
	if (left<dev->clip.x1) left=dev->clip.x1;\
	if (right>dev->clip.x2) right=dev->clip.x2;\
	if (left>=right) return;\
	START_GR\
	TEST_MOUSE (left,right,y,y+1)
	
#define VLINE_CLIP_PREFACE \
	int mouse_hidden;\
	TEST_INACTIVITY\
	if (x<dev->clip.x1||x>=dev->clip.x2||top>=dev->clip.y2||bottom<=dev->clip.y1) return;\
	if (top<dev->clip.y1) top=dev->clip.y1;\
	if (bottom>dev->clip.y2) bottom=dev->clip.y2;\
	if (top>=bottom) return;\
	START_GR\
	TEST_MOUSE(x,x+1,top,bottom)

#define HSCROLL_CLIP_PREFACE \
	int mouse_hidden;\
	TEST_INACTIVITY_0\
	if (!sc) return 0;\
	if (sc>(dev->clip.x2-dev->clip.x1)||-sc>(dev->clip.x2-dev->clip.x1))\
		return 1;\
	START_GR\
	TEST_MOUSE (dev->clip.x1,dev->clip.x2,dev->clip.y1,dev->clip.y2)
		
#define VSCROLL_CLIP_PREFACE \
	int mouse_hidden;\
	TEST_INACTIVITY_0\
	if (!sc) return 0;\
	if (sc>dev->clip.y2-dev->clip.y1||-sc>dev->clip.y2-dev->clip.y1) return 1;\
	START_GR\
	TEST_MOUSE (dev->clip.x1, dev->clip.x2, dev->clip.y1, dev->clip.y2)\
	
	
/* n is in bytes. dest must begin on pixel boundary. If n is not a whole number of pixels, rounding is
 * performed downwards.
 */
static inline void pixel_set(unsigned char *dest, int n,void * pattern)
{
	int a;

	switch(pspgu_pixelsize)
	{
		case 1:
		memset(dest,*(char *)pattern,n);
		break;

		case 2:
		{
#ifdef t2c
			short v=*(t2c *)pattern;
			int a;
			
			for (a=0;a<(n>>1);a++) ((t2c *)dest)[a]=v;
#else
			unsigned char a,b;
			int i;
			
			a=*(char*)pattern;
			b=((char*)pattern)[1];
			for (i=0;i<=n-2;i+=2){
				dest[i]=a;
				dest[i+1]=b;
			}
#endif
		}
		break;

		case 3:
		{
			unsigned char a,b,c;
			int i;
			
			a=*(char*)pattern;
			b=((char*)pattern)[1];
			c=((char*)pattern)[2];
			for (i=0;i<=n-3;i+=3){
				dest[i]=a;
				dest[i+1]=b;
				dest[i+2]=c;
			}
		}
		break;

		case 4:
		{
#ifdef t4c
			long v=*(t4c *)pattern;
			int a;
			
			for (a=0;a<(n>>2);a++) ((t4c *)dest)[a]=v;
#else
			unsigned char a,b,c,d;
			int i;
			
			a=*(char*)pattern;
			b=((char*)pattern)[1];
			c=((char*)pattern)[2];
			d=((char*)pattern)[3];
			for (i=0;i<=n-4;i+=4){
				dest[i]=a;
				dest[i+1]=b;
				dest[i+2]=c;
				dest[i+3]=d;
			}
#endif
		}
		break;

		default:
		for (a=0;a<n/pspgu_pixelsize;a++,dest+=pspgu_pixelsize) memcpy(dest,pattern,pspgu_pixelsize);
		break;
	}
}

static void redraw_mouse(void);

static void pspgu_mouse_move(int dx, int dy, int fl)
{
	struct event ev;
	mouse_x += dx;
	mouse_y += dy;
	ev.ev = EV_MOUSE;
	if (mouse_x >= pspgu_xsize) mouse_x = pspgu_xsize - 1;
	if (mouse_y >= pspgu_ysize) mouse_y = pspgu_ysize - 1;
	if (mouse_x < 0) mouse_x = 0;
	if (mouse_y < 0) mouse_y = 0;
	ev.x = mouse_x;
	ev.y = mouse_y;
	ev.b = B_MOVE;
	if (!current_virtual_device) return;
	if (current_virtual_device->mouse_handler) current_virtual_device->mouse_handler(current_virtual_device, ev.x, ev.y, fl/*ev.b*/);
	redraw_mouse();
}

static void pspgu_key_in(void *p, struct event *ev, int size)
{
#ifndef PSP
	if (size != sizeof(struct event) || ev->ev != EV_KBD) return;
	if ((ev->y & KBD_ALT) && ev->x >= '0' && ev->x <= '9') {
		switch_virtual_device((ev->x - '1' + 10) % 10);
		return;
	}
	if (!current_virtual_device) return;
	if (!ev->y && ev->x == KBD_F5) pspgu_mouse_move(-3, 0);
	else if (!ev->y && ev->x == KBD_F6) pspgu_mouse_move(0, 3);
	else if (!ev->y && ev->x == KBD_F7) pspgu_mouse_move(0, -3);
	else if (!ev->y && ev->x == KBD_F8) pspgu_mouse_move(3, 0);
	else 
	{
		if (pspgu_driver.codepage!=utf8_table&&(ev->x)>=128&&(ev->x)<=255)
			if ((ev->x=cp2u(ev->x,pspgu_driver.codepage)) == -1) return;
		if (current_virtual_device->keyboard_handler) current_virtual_device->keyboard_handler(current_virtual_device, ev->x, ev->y);
	}
#endif
}




#define mouse_getscansegment(buf,x,y,w) memcpy(buf,pspgu_vmem+y*pspgu_linesize+x*pspgu_pixelsize,w)
#define mouse_drawscansegment(ptr,x,y,w) memcpy(pspgu_vmem+y*pspgu_linesize+x*pspgu_pixelsize,ptr,w);

/* Flushes the background_buffer onscreen where it was originally taken from. */
static void place_mouse_background(void)
{
	struct bitmap bmp;

	bmp.x=arrow_width;
	bmp.y=arrow_height;
	bmp.skip=arrow_width*pspgu_pixelsize;
	bmp.data=background_buffer;

	{
		struct graphics_device * current_virtual_device_backup;

		current_virtual_device_backup=current_virtual_device;
		current_virtual_device=mouse_graphics_device;
		pspgu_draw_bitmap(mouse_graphics_device, &bmp, background_x,
			background_y);
		current_virtual_device=current_virtual_device_backup;
	}

}

/* Only when the old and new mouse don't interfere. Using it on interfering mouses would
 * cause a flicker.
 */
static void hide_mouse(void)
{

	global_mouse_hidden=1;
	place_mouse_background();
}

/* Gets background from the screen (clipping provided only right and bottom) to the
 * passed buffer.
 */
static void get_mouse_background(unsigned char *buffer_ptr)
{
	int width,height,skip,x,y;

	skip=arrow_width*pspgu_pixelsize;

	x=mouse_x;
	y=mouse_y;

	width=pspgu_pixelsize*(arrow_width+x>pspgu_xsize?pspgu_xsize-x:arrow_width);
	height=arrow_height+y>pspgu_ysize?pspgu_ysize-y:arrow_height;

	for (;height;height--){
		mouse_getscansegment(buffer_ptr,x,y,width);
		buffer_ptr+=skip;
		y++;
	}
}

/* Overlays the arrow's image over the mouse_buffer
 * Doesn't draw anything into the screen
 */
static void render_mouse_arrow(void)
{
	int x,y, reg0, reg1;
	unsigned char *mouse_ptr=mouse_buffer;
	unsigned int *arrow_ptr=arrow;

	for (y=arrow_height;y;y--){
		reg0=*arrow_ptr;
		reg1=arrow_ptr[1];
		arrow_ptr+=2;
		for (x=arrow_width;x;)
		{
			int mask=1<<(--x);

			if (reg0&mask)
				memcpy (mouse_ptr, &mouse_black, pspgu_pixelsize);
			else if (reg1&mask)
				memcpy (mouse_ptr, &mouse_white, pspgu_pixelsize);
			mouse_ptr+=pspgu_pixelsize;
		}
	}
}

static void place_mouse(void)
{
	struct bitmap bmp;

	bmp.x=arrow_width;
	bmp.y=arrow_height;
	bmp.skip=arrow_width*pspgu_pixelsize;
	bmp.data=mouse_buffer;	
	{
		struct graphics_device * current_graphics_device_backup;
//#ifndef PSP
		current_graphics_device_backup=current_virtual_device;
		current_virtual_device=mouse_graphics_device;
		pspgu_draw_bitmap(mouse_graphics_device, &bmp, mouse_x, mouse_y);
		current_virtual_device=current_graphics_device_backup;
	}
	global_mouse_hidden=0;
}

/* Only when the old and the new mouse positions do not interfere. Using this routine
 * on interfering positions would cause a flicker.
 */
static void show_mouse(void)
{

	get_mouse_background(background_buffer);
	background_x=mouse_x;
	background_y=mouse_y;
	memcpy(mouse_buffer,background_buffer,pspgu_pixelsize*arrow_area);
	render_mouse_arrow();
	place_mouse();
}

/* Doesn't draw anything into the screen
 */
static void put_and_clip_background_buffer_over_mouse_buffer(void)
{
	unsigned char *bbufptr=background_buffer, *mbufptr=mouse_buffer;
	int left=background_x-mouse_x;
	int top=background_y-mouse_y;
	int right,bottom;
	int bmpixelsizeL=pspgu_pixelsize;
	int number_of_bytes;
	int byte_skip;

	right=left+arrow_width;
	bottom=top+arrow_height;

	if (left<0){
		bbufptr-=left*bmpixelsizeL;
		left=0;
	}
	if (right>arrow_width) right=arrow_width;
	if (top<0){
		bbufptr-=top*bmpixelsizeL*arrow_width;
		top=0;
	}
	if (bottom>arrow_height) bottom=arrow_height;
	mbufptr+=bmpixelsizeL*(left+arrow_width*top);
	byte_skip=arrow_width*bmpixelsizeL;
	number_of_bytes=bmpixelsizeL*(right-left);
	for (;top<bottom;top++){
		memcpy(mbufptr,bbufptr,number_of_bytes);
		mbufptr+=byte_skip;
		bbufptr+=byte_skip;
	}
}

/* This draws both the contents of background_buffer and mouse_buffer in a scan
 * way (left-right, top-bottom), so the flicker is reduced.
 */
static inline void place_mouse_composite(void)
{
	int mouse_left=mouse_x;
	int mouse_top=mouse_y;
	int background_left=background_x;
	int background_top=background_y;
	int mouse_right=mouse_left+arrow_width;
	int mouse_bottom=mouse_top+arrow_height;
	int background_right=background_left+arrow_width;
	int background_bottom=background_top+arrow_height;
	int skip=arrow_width*pspgu_pixelsize;
	int background_length,mouse_length;
	unsigned char *mouse_ptr=mouse_buffer,*background_ptr=background_buffer;
	int yend;

	if (mouse_bottom>pspgu_ysize) mouse_bottom=pspgu_ysize;
	if (background_bottom>pspgu_ysize) background_bottom=pspgu_ysize;

	/* Let's do the top part */
	if (background_top<mouse_top){
		/* Draw the background */
		background_length=background_right>pspgu_xsize?pspgu_xsize-background_left
			:arrow_width;
		for (;background_top<mouse_top;background_top++){
			mouse_drawscansegment(background_ptr,background_left
				,background_top,background_length*pspgu_pixelsize);
			background_ptr+=skip;
		}
			
	}else if (background_top>mouse_top){
		/* Draw the mouse */
		mouse_length=mouse_right>pspgu_xsize
			?pspgu_xsize-mouse_left:arrow_width;
		for (;mouse_top<background_top;mouse_top++){
			mouse_drawscansegment(mouse_ptr,mouse_left,mouse_top,mouse_length*pspgu_pixelsize);
			mouse_ptr+=skip;
		}
	}

	/* Let's do the middle part */
	yend=mouse_bottom<background_bottom?mouse_bottom:background_bottom;
	if (background_left<mouse_left){
		/* Draw background, mouse */
		mouse_length=mouse_right>pspgu_xsize?pspgu_xsize-mouse_left:arrow_width;
		for (;mouse_top<yend;mouse_top++){
			mouse_drawscansegment(background_ptr,background_left,mouse_top
				,(mouse_left-background_left)*pspgu_pixelsize);
			mouse_drawscansegment(mouse_ptr,mouse_left,mouse_top,mouse_length*pspgu_pixelsize);
			mouse_ptr+=skip;
			background_ptr+=skip;
		}
			
	}else{
		int l1, l2, l3;
		
		/* Draw mouse, background */
		mouse_length=mouse_right>pspgu_xsize?pspgu_xsize-mouse_left:arrow_width;
		background_length=background_right-mouse_right;
		if (background_length+mouse_right>pspgu_xsize)
			background_length=pspgu_xsize-mouse_right;
		l1=mouse_length*pspgu_pixelsize;
		l2=(mouse_right-background_left)*pspgu_pixelsize;
		l3=background_length*pspgu_pixelsize;
		for (;mouse_top<yend;mouse_top++){
			mouse_drawscansegment(mouse_ptr,mouse_left,mouse_top,l1);
			if (background_length>0)
				mouse_drawscansegment(
					background_ptr +l2,
				       	mouse_right,mouse_top ,l3);
			mouse_ptr+=skip;
			background_ptr+=skip;
		}
	}

	if (background_bottom<mouse_bottom){
		/* Count over bottoms! tops will be invalid! */
		/* Draw mouse */
		mouse_length=mouse_right>pspgu_xsize?pspgu_xsize-mouse_left
			:arrow_width;
		for (;background_bottom<mouse_bottom;background_bottom++){
			mouse_drawscansegment(mouse_ptr,mouse_left,background_bottom
				,mouse_length*pspgu_pixelsize);
			mouse_ptr+=skip;
		}
	}else{
		/* Draw background */
		background_length=background_right>pspgu_xsize?pspgu_xsize-background_left
			:arrow_width;
		for (;mouse_bottom<background_bottom;mouse_bottom++){
			mouse_drawscansegment(background_ptr,background_left,mouse_bottom
				,background_length*pspgu_pixelsize);
			background_ptr+=skip;
		}
	}
}

/* This moves the mouse a sophisticated way when the old and new position of the
 * cursor overlap.
 */
static inline void redraw_mouse_sophisticated(void)
{
	int new_background_x, new_background_y;

	get_mouse_background(mouse_buffer);
	put_and_clip_background_buffer_over_mouse_buffer();
	memcpy(new_background_buffer,mouse_buffer,pspgu_pixelsize*arrow_area);
	new_background_x=mouse_x;
	new_background_y=mouse_y;
	render_mouse_arrow();
	place_mouse_composite();
	memcpy(background_buffer,new_background_buffer,pspgu_pixelsize*arrow_area);
	background_x=new_background_x;
	background_y=new_background_y;
}

static void redraw_mouse(void){
	
	if (!pspgu_active) return; /* We are not drawing */
	if (mouse_x!=background_x||mouse_y!=background_y){
		if (RECTANGLES_INTERSECT(
			background_x, background_x+arrow_width,
			mouse_x, mouse_x+arrow_width,
			background_y, background_y+arrow_height,
			mouse_y, mouse_y+arrow_height)){
			redraw_mouse_sophisticated();
		}else{
			/* Do a normal hide/show */
			get_mouse_background(mouse_buffer);
			memcpy(new_background_buffer,
				mouse_buffer,arrow_area*pspgu_pixelsize);
			render_mouse_arrow();
			hide_mouse();
			place_mouse();
			memcpy(background_buffer,new_background_buffer
				,arrow_area*pspgu_pixelsize);
			background_x=mouse_x;
			background_y=mouse_y;
		}
	}
}

/* This is an empiric magic that ensures
 * Good white purity
 * Correct rounding and dithering prediction
 * And this is the cabbala:
 * 063 021 063 
 * 009 009 021
 * 255 085 255
 * 036 036 084
 */
static void generate_palette(struct palette *palette)
{
        int a;

	switch (pspgu_colors)
	{
		case 16:
               	for (a=0;a<pspgu_palette_colors;a++)
                {
       	                palette->red[a]=(a&8)?65535:0;
               	        palette->green[a]=((a>>1)&3)*(65535/3);
                       	palette->blue[a]=(a&1)?65535:0;
		}
		break;
		case 256:
                for (a=0;a<pspgu_palette_colors;a++){
                       	palette->red[a]=((a>>5)&7)*(65535/7);
                        palette->green[a]=((a>>2)&7)*(65535/7);
       	                palette->blue[a]=(a&3)*(65535/3);
                }
		break;
		case 32768:
                for (a=0;a<pspgu_palette_colors;a++){
			/*
                       	palette->red[a]=((a>>10)&31)*(65535/31);
                        palette->green[a]=((a>>5)&31)*(65535/31);
       	                palette->blue[a]=(a&31)*(65535/31);
			*/
                        palette->red[a]=
                        palette->green[a]=
                        palette->blue[a]=(((a&31)*255)/31)*257;
                }
		break;
		case 65536:
                for (a=0;a<pspgu_palette_colors;a++){
			/*
                       	palette->red[a]=((a>>11)&31)*(65535/31);
                        palette->green[a]=((a>>5)&63)*(65535/63);
       	                palette->blue[a]=(a&31)*(65535/31);
			*/
                        palette->green[a]=(((a&63)*255)/64)*257;
                        palette->red[a]=
                        palette->blue[a]=(((a&31)*255)/32)*257;
                }
		break;
                default:
                for (a=0;a<pspgu_palette_colors;a++){
                        palette->red[a]=
                        palette->green[a]=
                        palette->blue[a]=a*257;
                        /* stuff it in both high and low byte */
                }
	}
}

static void alloc_palette(struct palette *pal)
{
	pal->red=mem_calloc(sizeof(unsigned short)*pspgu_palette_colors);
	pal->green=mem_calloc(sizeof(unsigned short)*pspgu_palette_colors);
	pal->blue=mem_calloc(sizeof(unsigned short)*pspgu_palette_colors);

	if (!pal->red||!pal->green||!pal->blue) {
		/*internal("Cannot create palette.\n")*/;
	}
}


static void free_palette(struct palette *pal)
{
	mem_free(pal->red);
	mem_free(pal->green);
	mem_free(pal->blue);
}

typedef unsigned short __u16;
struct pspgu_cmap {
	int start,
	len;
	__u16 *red,
	*green,
	*blue,
	*transp;
};
static void set_palette(struct palette *pal)
{
#ifndef PSP
	struct pspgu_cmap cmap;
	int i;
	unsigned short *red=pal->red;
	unsigned short *green=pal->green;
	unsigned short *blue=pal->blue;
	__u16 *r, *g, *b, *t;

	r=mem_alloc(pspgu_palette_colors*sizeof(__u16));
	g=mem_alloc(pspgu_palette_colors*sizeof(__u16));
	b=mem_alloc(pspgu_palette_colors*sizeof(__u16));
	t=mem_calloc(pspgu_palette_colors*sizeof(__u16));

	if (!r||!g||!b||!t) {
		/*internal("Cannot allocate memory.\n")*/;
	}

	for (i = 0; i < pspgu_palette_colors; i++)
	{
	        r[i] = red[i];
	        g[i] = green[i];
	        b[i] = blue[i];
		/*fprintf(stderr, "%d %d %d\n", r[i], g[i], b[i]);*/
                /*fprintf(stderr, "%5x: %5x\t%5x\t%5x\t%5x\n",i,r[i],g[i],b[i],t[i]);*/

	}

	cmap.start = 0;
	cmap.len = pspgu_palette_colors;
	cmap.red = r;
	cmap.green = g;
	cmap.blue = b;
	cmap.transp = t;
	if ((ioctl(pspgu_handler, FBIOPUTCMAP, &cmap))==-1) {
		/*internal("Cannot set palette\n")*/;
	}
	mem_free(r);mem_free(g);mem_free(b);mem_free(t);
#endif
}


static void get_palette(struct palette *pal)
{
#ifndef PSP
	struct pspgu_cmap cmap;
	int i;
	__u16 *r, *g, *b, *t;

	r=mem_alloc(pspgu_palette_colors*sizeof(__u16));
	g=mem_alloc(pspgu_palette_colors*sizeof(__u16));
	b=mem_alloc(pspgu_palette_colors*sizeof(__u16));
	t=mem_alloc(pspgu_palette_colors*sizeof(__u16));

	if (!r||!g||!b||!t) {
		/*internal("Cannot allocate memory.\n")*/;
	}

	cmap.start = 0;
	cmap.len = pspgu_palette_colors;
	cmap.red = r;
	cmap.green = g;
	cmap.blue = b;
	cmap.transp = t;

	if (ioctl(pspgu_handler, FBIOGETCMAP, &cmap)) {
		/*internal("Cannot get palette\n")*/;
	}

	for (i = 0; i < pspgu_palette_colors; i++)
	{
		/*printf("%d %d %d\n",r[i],g[i],b[i]);*/
	        pal->red[i] = r[i];
	        pal->green[i] = g[i];
	        pal->blue[i] = b[i];
	}

	mem_free(r);mem_free(g);mem_free(b);mem_free(t);
#endif
}


static void pspgu_switch_signal(void *data)
{
#ifndef PSP
	struct vt_stat st;
	struct rect r;
	int signal=(int)(unsigned long)data;

	switch(signal)
	{
		case SIG_REL: /* release */
		pspgu_active=0;
		if (!in_gr_operation)ioctl(TTY,VT_RELDISP,1);
		break;

		case SIG_ACQ: /* acq */
		if (ioctl(TTY,VT_GETSTATE,&st)) return;
		if (st.v_active != pspgu_console) return;
		pspgu_active=1;
		ioctl(TTY,VT_RELDISP,VT_ACKACQ);
		if (have_cmap && current_virtual_device)
			set_palette(&global_pal);
		r.x1=0;
		r.y1=0;
		r.x2=pspgu_xsize;
		r.y2=pspgu_ysize;
		if (border_left | border_top | border_right | border_bottom) memset(pspgu_mem,0,pspgu_mem_size);
		if (current_virtual_device) current_virtual_device->redraw_handler(current_virtual_device,&r);
		break;
	}
#endif
}


static unsigned char *pspgu_switch_init(void)
{
#ifndef PSP
	install_signal_handler(SIG_REL, pspgu_switch_signal, (void*)SIG_REL, 1);
	install_signal_handler(SIG_ACQ, pspgu_switch_signal, (void*)SIG_ACQ, 0);
	if (-1 == ioctl(TTY,VT_GETMODE, &vt_omode)) {
		return stracpy("Could not get VT mode.\n");
	}
	memcpy(&vt_mode, &vt_omode, sizeof(vt_mode));

	vt_mode.mode   = VT_PROCESS;
	vt_mode.waitv  = 0;
	vt_mode.relsig = SIG_REL;
	vt_mode.acqsig = SIG_ACQ;

	if (-1 == ioctl(TTY,VT_SETMODE, &vt_mode)) {
		return stracpy("Could not set VT mode.\n");
	}
	return NULL;
#endif
}

static void pspgu_switch_shutdown(void)
{
#ifndef PSP
	ioctl(TTY,VT_SETMODE, &vt_omode);
#endif
}

static void pspgu_shutdown_palette(void)
{
	if (have_cmap)
	{
		set_palette(&old_palette);
		free_palette(&old_palette);
		free_palette(&global_pal);
	}
}

static void pspgu_ctrl_c(struct itrm *i)
{
	kbd_ctrl_c();
}

#if !defined(USE_GPM_DX) && !defined(PSP)
void pspgu_mouse_setsize()
{
	struct vt_stat vs;
	if (!ioctl(0, VT_GETSTATE, &vs)) {
		fd_set zero;
		struct timeval tv;
		FD_ZERO(&zero);
		memset(&tv, 0, sizeof tv);
		ioctl(0, VT_ACTIVATE, vs.v_active > 1 ? 1 : 2);
		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		select(0, &zero, &zero, &zero, &tv);
		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		select(0, &zero, &zero, &zero, &tv);
		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		select(0, &zero, &zero, &zero, &tv);
		ioctl(0, VT_ACTIVATE, vs.v_active);
	}
}
#endif

void unhandle_pspgu_mouse(void);

#ifndef PSP
static void pspgu_gpm_in(void *nic)
{
#ifndef USE_GPM_DX
	static int lx = -1, ly = -1;
#endif
	struct event ev;
	Gpm_Event gev;
	again:
	if (Gpm_GetEvent(&gev) <= 0) {
		unhandle_pspgu_mouse();
		return;
	}

	/*fprintf(stderr, "%d %d\n", gev.x, gev.y);*/
#ifndef USE_GPM_DX
	if (gev.x != lx || gev.y != ly) {
		mouse_x = (gev.x - 1) * pspgu_xsize / pspgu_txt_xsize + pspgu_xsize / pspgu_txt_xsize / 2 - 1;
		mouse_y = (gev.y - 1) * pspgu_ysize / pspgu_txt_ysize + pspgu_ysize / pspgu_txt_ysize / 2 - 1;
		lx = gev.x, ly = gev.y;
	}
#else
	if (gev.dx || gev.dy) {
		if (!(gev.type & gpm_smooth)) {
			mouse_x += gev.dx * 8;
			mouse_y += gev.dy * 8;
		} else {
			mouse_x += gev.dx;
			mouse_y += gev.dy;
		}
	}
#endif
	ev.ev = EV_MOUSE;
	if (mouse_x >= pspgu_xsize) mouse_x = pspgu_xsize - 1;
	if (mouse_y >= pspgu_ysize) mouse_y = pspgu_ysize - 1;
	if (mouse_x < 0) mouse_x = 0;
	if (mouse_y < 0) mouse_y = 0;

	if (!(gev.type & gpm_smooth) && (gev.dx || gev.dy)) {
		mouse_x = (mouse_x + 8) / 8 * 8 - 4;
		mouse_y = (mouse_y + 8) / 8 * 8 - 4;
		if (mouse_x >= pspgu_xsize) mouse_x = pspgu_xsize - 1;
		if (mouse_y >= pspgu_ysize) mouse_y = pspgu_ysize - 1;
		if (mouse_x < 0) mouse_x = 0;
		if (mouse_y < 0) mouse_y = 0;
	}

	ev.x = mouse_x;
	ev.y = mouse_y;
	if (gev.buttons & GPM_B_LEFT) ev.b = B_LEFT;
	else if (gev.buttons & GPM_B_MIDDLE) ev.b = B_MIDDLE;
	else if (gev.buttons & GPM_B_RIGHT) ev.b = B_RIGHT;
	else ev.b = 0;
	if (gev.type & GPM_DOWN) ev.b |= B_DOWN;
	else if (gev.type & GPM_UP) ev.b |= B_UP;
	else if (gev.type & GPM_DRAG) ev.b |= B_DRAG;
	else ev.b |= B_MOVE;

#ifndef USE_GPM_DX
	if (pspgu_msetsize < 0) {
	} else if (pspgu_msetsize < 10) {
		pspgu_msetsize++;
	} else if ((ev.b & BM_ACT) == B_MOVE && !(ev.b & BM_BUTT)) {
		pspgu_mouse_setsize();
		pspgu_msetsize = -1;
	}
#endif

	if (((ev.b & BM_ACT) == B_MOVE && !(ev.b & BM_BUTT)) || (ev.b & BM_ACT) == B_DRAG) {
		if (can_read(pspgu_hgpm)) goto again;
	}

	if (!current_virtual_device) return;
	if (current_virtual_device->mouse_handler) current_virtual_device->mouse_handler(current_virtual_device, ev.x, ev.y, ev.b);
	redraw_mouse();
}
#endif

#ifndef PSP
static int handle_pspgu_mouse(void)
{
	Gpm_Connect conn;
#ifndef USE_GPM_DX
	int gpm_ver = 0;
	struct winsize ws;
	pspgu_old_ws_v = 0;
#endif
	pspgu_hgpm = -1;
#ifndef USE_GPM_DX
	Gpm_GetLibVersion(&gpm_ver);
	if (gpm_ver >= 11900 && ioctl(1, TIOCGWINSZ, &ws) != -1) {
		memcpy(&pspgu_old_ws, &ws, sizeof(struct winsize));
		pspgu_old_ws_v = 1;
		ws.ws_row *= 2;
		ioctl(1, TIOCSWINSZ, &ws);
		pspgu_msetsize = 0;
		memcpy(&pspgu_new_ws, &ws, sizeof ws);
	} else pspgu_msetsize = -1;
	get_terminal_size(1, &pspgu_txt_xsize, &pspgu_txt_ysize);
#endif
	conn.eventMask = ~0;
	conn.defaultMask = gpm_smooth;
	conn.minMod = 0;
	conn.maxMod = -1;
	if ((pspgu_hgpm = Gpm_Open(&conn, 0)) < 0) {
		unhandle_pspgu_mouse();
		return -1;
	}
	set_handlers(pspgu_hgpm, pspgu_gpm_in, NULL, NULL, NULL);
#ifdef SIGTSTP
	install_signal_handler(SIGTSTP, (void (*)(void *))sig_tstp, NULL, 0);
#endif
#ifdef SIGCONT
	install_signal_handler(SIGCONT, (void (*)(void *))sig_cont, NULL, 0);
#endif
#ifdef SIGTTIN
	install_signal_handler(SIGTTIN, (void (*)(void *))sig_tstp, NULL, 0);
#endif

	return 0;
}
#endif

#ifndef PSP
void unhandle_pspgu_mouse(void)
{
	if (pspgu_hgpm >= 0) set_handlers(pspgu_hgpm, NULL, NULL, NULL, NULL);
#ifndef USE_GPM_DX
	pspgu_hgpm = -1;
	if (pspgu_old_ws_v) {
		ioctl(1, TIOCSWINSZ, &pspgu_old_ws);
		pspgu_old_ws_v = 0;
	}
#endif
	Gpm_Close();
#ifdef SIGTSTP
	install_signal_handler(SIGTSTP, (void (*)(void *))sig_tstp, NULL, 0);
#endif
#ifdef SIGCONT
	install_signal_handler(SIGCONT, (void (*)(void *))sig_cont, NULL, 0);
#endif
#ifdef SIGTTIN
	install_signal_handler(SIGTTIN, (void (*)(void *))sig_tstp, NULL, 0);
#endif
}
#endif

#ifndef PSP
#ifndef USE_GPM_DX
static void block_pspgu_mouse(void)
{
	if (pspgu_hgpm >= 0) set_handlers(pspgu_hgpm, NULL, NULL, NULL, NULL);
#ifndef USE_GPM_DX
	if (pspgu_old_ws_v) {
		ioctl(1, TIOCSWINSZ, &pspgu_old_ws);
	}
#endif
}

static void unblock_pspgu_mouse(void)
{
	if (pspgu_hgpm >= 0) set_handlers(pspgu_hgpm, pspgu_gpm_in, NULL, NULL, NULL);
#ifndef USE_GPM_DX
	if (pspgu_old_ws_v) {
		ioctl(1, TIOCSWINSZ, &pspgu_new_ws);
		pspgu_msetsize = 0;
	}
#endif
}
#endif
#endif


void pspInputThread()
{
	for(;;)
	{
		static int oldButtonMask = 0;
		int deltax = 0, deltay = 0;
		static int danzeff_x = PSP_SCREEN_WIDTH/2-(64*3/2), danzeff_y = PSP_SCREEN_HEIGHT/2-(64*3/2);
		SceCtrlData pad;
		SceCtrlLatch latch; 
		unsigned short	fl	= 0;
				
		if ( g_PSPEnableInput == truE )
		{
			
			if (g_PSPEnableRendering == falsE)
			{
				g_PSPEnableRendering = truE;
				cls_redraw_all_terminals();
			}
			
			sceCtrlReadBufferPositive(&pad, 1);
			sceCtrlReadLatch(&latch);
			
			RAND_add(&pad, sizeof(pad), sizeof(pad)); /** Add more randomness to SSL */
				
			if (sf_danzeffOn)
			{
				if (danzeff_dirty())
				{
					///??sdl_register_update(dev, 0, 0, sdl_VIDEO_WIDTH, sdl_VIDEO_HEIGHT, 0);
				}
				
				if (latch.uiMake)
				{
					// Button Pressed 
					oldButtonMask = latch.uiPress;
				}
				else if (latch.uiBreak) /** Button Released */
				{
					if (oldButtonMask & PSP_CTRL_START)
					{
						/* Enter input */
						sf_danzeffOn = 0;
						cls_redraw_all_terminals();
					}
					else if (oldButtonMask & PSP_CTRL_SELECT)
					{
						TakeScreenShot();
						wait_for_triangle("");
						cls_redraw_all_terminals();
					}
					oldButtonMask = 0;
				}
				
				if (pad.Buttons & PSP_CTRL_LEFT)
				{
					danzeff_x-=5;
					cls_redraw_all_terminals();
				}
				else if (pad.Buttons & PSP_CTRL_RIGHT)
				{
					danzeff_x+=5;
					cls_redraw_all_terminals();
				}
				else
				{
					int key = 0;
					key = danzeff_readInput(pad);
					if (key) 
					{
						switch (key)
						{
							case '\n':
								key = KBD_ENTER;
								break;
							case 8:
								key = KBD_BS;
								break;
						}
						current_virtual_device->keyboard_handler(current_virtual_device, key, 0);
						//sdl_register_update(dev, 0, 0, sdl_VIDEO_WIDTH, sdl_VIDEO_HEIGHT, 0);
					}
				}
				danzeff_moveTo(danzeff_x, danzeff_y);
				
			}
			else
			{
				if  (pad.Lx < 128)
				{
					//newx = mouse_x - (128 - pad.Lx)/30;
					deltax = -(128 - pad.Lx)/30;
				}
				else
				{
					//newx = mouse_x + (pad.Lx - 128)/30;
					deltax = (pad.Lx - 128)/30;
				}
			
				if  (pad.Ly < 128)
				{
					//newy = mouse_y - (128 - pad.Ly)/30;
					deltay = - (128 - pad.Ly)/30;
				}
				else
				{
					//newy = mouse_y + (pad.Ly - 128)/30;
					deltay = (pad.Ly - 128)/30;
				}
	
				
				//if (mouse_x != newx || mouse_y != newy)
				{
					//if (newx >= 0 && newx < 480)
					//	mouse_x = newx;
					//if (newy >=0 && newy < 272)
					//	mouse_y = newy;
	
					///SDL_WarpMouse(mouse_x, mouse_y);
					
					fl	= B_MOVE;
					if (pad.Buttons & PSP_CTRL_CROSS)
					{
						fl = B_DRAG | B_LEFT;
					}
					else if (pad.Buttons & PSP_CTRL_TRIANGLE)
					{
						fl = B_DRAG | B_RIGHT;
					}
						
					/* call handler */
					//current_virtual_device->mouse_handler(current_virtual_device, mouse_x, mouse_y, fl);
					pspgu_mouse_move(deltax, deltay, fl);
				}
				
				if (latch.uiMake)
				{
					// Button Pressed 
					oldButtonMask = latch.uiPress;
					if (oldButtonMask & PSP_CTRL_CROSS)
					{
						fl	= B_DOWN | B_LEFT;
						if (current_virtual_device) current_virtual_device->mouse_handler(current_virtual_device, mouse_x, mouse_y, fl);
					}
					else if (oldButtonMask & PSP_CTRL_TRIANGLE)
					{
						fl	= B_DOWN | B_RIGHT;
						if (current_virtual_device) current_virtual_device->mouse_handler(current_virtual_device, mouse_x, mouse_y, fl);
					}
				}
				else if (latch.uiBreak) /** Button Released */
				{
					if (oldButtonMask & PSP_CTRL_SELECT && oldButtonMask & PSP_CTRL_CROSS)
					{
						pspDebugScreenInit();
						wifiChooseConnect();
						cls_redraw_all_terminals();
					}
					else if (oldButtonMask & PSP_CTRL_DOWN)
					{
						if (oldButtonMask & PSP_CTRL_RTRIGGER)
						{
							if (current_virtual_device) current_virtual_device->keyboard_handler(current_virtual_device, KBD_DEL, fl);
						}
						else if (oldButtonMask & PSP_CTRL_LTRIGGER)
						{
							if (current_virtual_device) current_virtual_device->keyboard_handler(current_virtual_device, KBD_PAGE_DOWN, fl);
						}
						else
						{
							if (current_virtual_device) current_virtual_device->keyboard_handler(current_virtual_device, KBD_DOWN, fl);
						}
					}
					else if (oldButtonMask & PSP_CTRL_UP)
					{
						if (oldButtonMask & PSP_CTRL_RTRIGGER)
						{
							if (current_virtual_device) current_virtual_device->keyboard_handler(current_virtual_device, KBD_INS, fl);
						}
						else if (oldButtonMask & PSP_CTRL_LTRIGGER)
						{
							if (current_virtual_device) current_virtual_device->keyboard_handler(current_virtual_device, KBD_PAGE_UP, fl);
						}
						else
						{
							if (current_virtual_device) current_virtual_device->keyboard_handler(current_virtual_device, KBD_UP, fl);
						}
					}
					else if (oldButtonMask & PSP_CTRL_LEFT)
					{
						if (oldButtonMask & PSP_CTRL_RTRIGGER)
						{
							if (current_virtual_device) current_virtual_device->keyboard_handler(current_virtual_device, '[', fl);
						}
						else
						{
							if (current_virtual_device) current_virtual_device->keyboard_handler(current_virtual_device, KBD_LEFT, fl);
						}
					}
					else if (oldButtonMask & PSP_CTRL_RIGHT)
					{
						if (oldButtonMask & PSP_CTRL_RTRIGGER)
						{
							if (current_virtual_device) current_virtual_device->keyboard_handler(current_virtual_device, ']', fl);
						}
						else
						{
							if (current_virtual_device) current_virtual_device->keyboard_handler(current_virtual_device, KBD_RIGHT, fl);
						}
					}
					else if (oldButtonMask & PSP_CTRL_LTRIGGER)
					{
					}
					else if (oldButtonMask & PSP_CTRL_RTRIGGER)
					{
					}
					else if (oldButtonMask & PSP_CTRL_CROSS)
					{
						fl	= B_UP | B_LEFT;
						if (current_virtual_device) current_virtual_device->mouse_handler(current_virtual_device, mouse_x, mouse_y, fl);
					}
					else if (oldButtonMask & PSP_CTRL_SQUARE)
					{
						if (current_virtual_device) current_virtual_device->keyboard_handler(current_virtual_device, KBD_ESC, fl);
					}
					else if (oldButtonMask & PSP_CTRL_TRIANGLE)
					{
						fl	= B_UP | B_RIGHT;
						if (current_virtual_device) current_virtual_device->mouse_handler(current_virtual_device, mouse_x, mouse_y, fl);
					}
					else if (oldButtonMask & PSP_CTRL_CIRCLE)
					{
						if (current_virtual_device) current_virtual_device->keyboard_handler(current_virtual_device, KBD_ENTER, fl);
					}
					else if (oldButtonMask & PSP_CTRL_START)
					{
						//current_virtual_device->keyboard_handler(current_virtual_device, 'g', fl);
						if (danzeff_isinitialized())
						{
							///???danzeff_set_screen(sdl_SURFACE(dev));
							danzeff_moveTo(danzeff_x, danzeff_y);
							danzeff_render();
							///???sdl_register_update(dev, 0, 0, sdl_VIDEO_WIDTH, sdl_VIDEO_HEIGHT, 0);
							sf_danzeffOn = 1;
						}
						else
						{
							wait_for_triangle("Error loading danzeff OSK");
						}
					}
					else if (oldButtonMask & PSP_CTRL_SELECT)
					{
						TakeScreenShot();
						wait_for_triangle("");
						cls_redraw_all_terminals();
					}
					oldButtonMask = 0;
				}
			}
		}
		//sceKernelDelayThread(50*1000); /* Wait 50ms */
		sceDisplayWaitVblankStart();
	}
}


static unsigned char *pspgu_init_driver(unsigned char *param, unsigned char *ignore)
{
	unsigned char *e;
	struct stat st;
#ifndef PSP
	kbd_set_raw = 1;
#endif
	pspgu_old_vd = NULL;
	ignore=ignore;
	pspgu_driver_param=NULL;
	if(param != NULL)
		pspgu_driver_param=stracpy(param);

#ifndef PSP
	border_left = border_right = border_top = border_bottom = 0;
	if (!param) param="";
	if (*param) {
		if (*param < '0' || *param > '9')
			{ bad_p:
				if(pspgu_driver_param) { mem_free(pspgu_driver_param); pspgu_driver_param=NULL; }
				return stracpy("-mode syntax is left_border[,top_border[,right_border[,bottom_border]]]\n"); }
		border_left = strtoul(param, (char **)(void *)&param, 10);
		if (*param == ',') param++;
	} else {
		border_left = 0;
	}
	if (*param) {
		if (*param < '0' || *param > '9') goto bad_p;
		border_top = strtoul(param, (char **)(void *)&param, 10);
		if (*param == ',') param++;
	} else {
		border_top = border_left;
	}
	if (*param) {
		if (*param < '0' || *param > '9') goto bad_p;
		border_right = strtoul(param, (char **)(void *)&param, 10);
		if (*param == ',') param++;
	} else {
		border_right = border_left;
	}
	if (*param) {
		if (*param < '0' || *param > '9') goto bad_p;
		border_bottom = strtoul(param, (char **)(void *)&param, 10);
		if (*param == ',') param++;
	} else {
		border_bottom = border_top;
	}
	if (*param) goto bad_p;

	if (fstat(TTY, &st)) {
		if(pspgu_driver_param) { mem_free(pspgu_driver_param); pspgu_driver_param=NULL; }
		return stracpy("Cannon stat stdin.\n");
	}

	pspgu_console = st.st_rdev & 0xff;

	ioctl(TTY, VT_WAITACTIVE, pspgu_console);
	if ((e = pspgu_switch_init())) {
		if(pspgu_driver_param) { mem_free(pspgu_driver_param); pspgu_driver_param=NULL; }
		return e;
	}

	pspgu_handler=open("/dev/fb0",O_RDWR);
	if (pspgu_handler==-1) {
		pspgu_switch_shutdown();
		if(pspgu_driver_param) { mem_free(pspgu_driver_param); pspgu_driver_param=NULL; }
		return stracpy("Cannot open /dev/fb0.\n");
	}

	if ((ioctl (pspgu_handler, FBIOGET_VSCREENINFO, &vi))==-1)
	{
		close(pspgu_handler);
		pspgu_switch_shutdown();
		if(pspgu_driver_param) { mem_free(pspgu_driver_param); pspgu_driver_param=NULL; }
		return stracpy("Cannot get FB VSCREENINFO.\n");
	}
	oldmode=vi;

	if ((ioctl (pspgu_handler, FBIOGET_FSCREENINFO, &fi))==-1)
	{
		close(pspgu_handler);
		pspgu_switch_shutdown();
		if(pspgu_driver_param) { mem_free(pspgu_driver_param); pspgu_driver_param=NULL; }
		return stracpy("Cannot get FB FSCREENINFO.\n");
	}

	pspgu_xsize=vi.xres;
	pspgu_ysize=vi.yres;
	pspgu_bits_pp=vi.bits_per_pixel;
#endif

	pspgu_xsize=PSP_SCREEN_WIDTH;
	pspgu_ysize=PSP_SCREEN_HEIGHT;
	pspgu_bits_pp=16;
#define pspgu_VISUAL_DIRECTCOLOR 10
#define pspgu_VISUAL_PSEUDOCOLOR 20

	///fi.visual = pspgu_VISUAL_DIRECTCOLOR;
	///fi.line_length = PSP_LINE_SIZE;
	///fi.smem_len = pspgu_xsize * pspgu_ysize * pspgu_bits_pp;

	if (pspgu_xsize <= border_left + border_right) border_left = border_right = 0;
	pspgu_xsize -= border_left + border_right;
	if (pspgu_ysize <= border_top + border_bottom) border_top = border_bottom = 0;
	pspgu_ysize -= border_top + border_bottom;

	pspgu_driver.x=pspgu_xsize;
	pspgu_driver.y=pspgu_ysize;

	 switch(pspgu_bits_pp)
	{
		case 4:
		pspgu_pixelsize=1;
		pspgu_palette_colors=16;
		break;
		
		case 8:
		pspgu_pixelsize=1;
		pspgu_palette_colors=256;
		break;

		case 15:
		case 16:
		pspgu_pixelsize=2;
		pspgu_palette_colors=64;
		break;

		case 24:
		pspgu_palette_colors=256;
		pspgu_pixelsize=3;
		break;

		case 32:
		pspgu_palette_colors=256;
		pspgu_pixelsize=4;
		pspgu_bits_pp=24;
		break;

		default:
#ifndef PSP
		close(pspgu_handler);
		pspgu_switch_shutdown();
		if(pspgu_driver_param) { mem_free(pspgu_driver_param); pspgu_driver_param=NULL; }
#endif
		return stracpy("Unknown bit depth");
	}
	pspgu_colors=1<<pspgu_bits_pp;

#ifndef PSP
	if (fi.visual==pspgu_VISUAL_PSEUDOCOLOR && pspgu_colors <= 0x1000000) /* set palette */
	{
		have_cmap=1;
		pspgu_palette_colors=pspgu_colors;
		alloc_palette(&old_palette);
		get_palette(&old_palette);

		alloc_palette(&global_pal);
		generate_palette(&global_pal);
		set_palette(&global_pal);
	}
	if (fi.visual==pspgu_VISUAL_DIRECTCOLOR) /* set pseudo palette */
#endif
	{
		have_cmap=2;
		alloc_palette(&old_palette);
		get_palette(&old_palette);

		alloc_palette(&global_pal);
		generate_palette(&global_pal);
		set_palette(&global_pal);
	}
	
	pspgu_linesize=PSP_LINE_SIZE;//fi.line_length;
	pspgu_mem_size=pspgu_xsize * pspgu_ysize * pspgu_bits_pp;

#ifndef PSP
	vi.xoffset=0;
	vi.yoffset=0;
	if ((ioctl(pspgu_handler, FBIOPAN_DISPLAY, &vi))==-1)
	{
	/* mikulas : nechodilo mi to, tak jsem tohle vyhodil a ono to chodi */
		/*pspgu_shutdown_palette();
		close(pspgu_handler);
		return stracpy("Cannot pan display.\n");
		*/
	}
#endif

	if (init_virtual_devices(&pspgu_driver, NUMBER_OF_DEVICES)){
		pspgu_shutdown_palette();
#ifndef PSP	
		close(pspgu_handler);
#endif
		pspgu_switch_shutdown();
		if(pspgu_driver_param) { mem_free(pspgu_driver_param); pspgu_driver_param=NULL; }
		return stracpy("Allocation of virtual devices failed.\n");
	}
#ifndef PSP
	pspgu_kbd = handle_svgalib_keyboard((void (*)(void *, unsigned char *, int))pspgu_key_in);
#else
	
	{
		pthread_t pthid;
		pthread_attr_t pthattr;
		struct sched_param shdparam;
		pthread_attr_init(&pthattr);
		shdparam.sched_policy = SCHED_OTHER;
		shdparam.sched_priority = 45;
		pthread_attr_setschedparam(&pthattr, &shdparam);
		pthread_create(&pthid, &pthattr, pspInputThread, NULL);
	}
#endif

	/* Mikulas: nechodi to na sparcu */
	if (pspgu_mem_size < pspgu_linesize * pspgu_ysize)
	{
		pspgu_shutdown_palette();
		///svgalib_free_trm(pspgu_kbd);
		shutdown_virtual_devices();
#ifndef PSP
		close(pspgu_handler);
#endif
		pspgu_switch_shutdown();
		if(pspgu_driver_param) { mem_free(pspgu_driver_param); pspgu_driver_param=NULL; }
		return stracpy("Nonlinear mapping of graphics memory not supported.\n");
	}
		
	
#ifndef PSP
	if ((pspgu_mem=mmap(0,pspgu_mem_size,PROT_READ|PROT_WRITE,MAP_SHARED,pspgu_handler,0))==MAP_FAILED)
	{
		pspgu_shutdown_palette();
		svgalib_free_trm(pspgu_kbd);
		shutdown_virtual_devices();

		close(pspgu_handler);
		pspgu_switch_shutdown();
		if(pspgu_driver_param) { mem_free(pspgu_driver_param); pspgu_driver_param=NULL; }
		return stracpy("Cannot mmap graphics memory.\n");
	}
#else /* PSP */

	/* Place vram in uncached memory */
	pspgu_mem = (u32 *) (0x40000000 | (u32) sceGeEdramGetAddr());
	sceDisplaySetMode(0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
	sceDisplaySetFrameBuf((void *) pspgu_mem, PSP_LINE_SIZE, PSP_PIXEL_FORMAT, 1);
	pspgu_mem_size = PSP_SCREEN_WIDTH * PSP_SCREEN_HEIGHT * 16;
#endif

	pspgu_vmem = pspgu_mem + border_left * pspgu_pixelsize + border_top * pspgu_linesize;
	pspgu_driver.depth=pspgu_pixelsize&7;
	pspgu_driver.depth|=(pspgu_bits_pp&31)<<3;
	pspgu_driver.depth|=(!!(1/*?*/))<<8;	/* nonstd byte order */
	
	pspgu_driver.get_color=get_color_fn(pspgu_driver.depth);
	/*pspgu_switch_init();*/
	///install_signal_handler(SIGINT, (void (*)(void *))pspgu_ctrl_c, pspgu_kbd, 0);

	/* mouse */
	mouse_buffer=mem_alloc(pspgu_pixelsize*arrow_area);
	background_buffer=mem_alloc(pspgu_pixelsize*arrow_area);
	new_background_buffer=mem_alloc(pspgu_pixelsize*arrow_area);
	background_x=mouse_x=pspgu_xsize>>1;
	background_y=mouse_y=pspgu_ysize>>1;
	mouse_black=pspgu_driver.get_color(0);
	mouse_white=pspgu_driver.get_color(0xffffff);
	mouse_graphics_device=pspgu_driver.init_device();
	virtual_devices[0] = NULL;
	global_mouse_hidden=1;
#ifndef PSP
	if (handle_pspgu_mouse()) {
		pspgu_driver.shutdown_device(mouse_graphics_device);
		mem_free(mouse_buffer);
		mem_free(background_buffer);
		mem_free(new_background_buffer);
		pspgu_shutdown_palette();
		///svgalib_free_trm(pspgu_kbd);
		shutdown_virtual_devices();

		///close(pspgu_handler);
		pspgu_switch_shutdown();
		if(pspgu_driver_param) { mem_free(pspgu_driver_param); pspgu_driver_param=NULL; }
		return stracpy("Cannot open GPM mouse.\n");
	}
	/* hide cursor */
	///printf("\033[?25l");
	///fflush(stdout);
#endif
	if (border_left | border_top | border_right | border_bottom) memset(pspgu_mem,0,pspgu_mem_size);
		
	show_mouse();
	return NULL;
}

static void pspgu_shutdown_driver(void)
{
	mem_free(mouse_buffer);
	mem_free(background_buffer);
	mem_free(new_background_buffer);
	pspgu_driver.shutdown_device(mouse_graphics_device);
	///unhandle_pspgu_mouse();
	///ioctl (pspgu_handler, FBIOPUT_VSCREENINFO, &oldmode);
	pspgu_shutdown_palette();
	///install_signal_handler(SIGINT, NULL, NULL, 0);

	///close(pspgu_handler);

	memset(pspgu_mem,0,pspgu_mem_size);
	///munmap(pspgu_mem,pspgu_mem_size);
	shutdown_virtual_devices();
	pspgu_switch_shutdown();
	///svgalib_free_trm(pspgu_kbd);
	if(pspgu_driver_param) mem_free(pspgu_driver_param);
	/* show cursor */
	///printf("\033[?25h");
	///fflush(stdout);
}


static unsigned char *pspgu_get_driver_param(void)
{
        return pspgu_driver_param;
}


/* Return value:        0 alloced on heap
 *                      1 alloced in vidram
 *                      2 alloced in X server shm
 */
static int pspgu_get_empty_bitmap(struct bitmap *dest)
{
	if (dest->x && (unsigned)dest->x * (unsigned)dest->y / (unsigned)dest->x != (unsigned)dest->y) overalloc();
	if ((unsigned)dest->x * (unsigned)dest->y > (unsigned)MAXINT / pspgu_pixelsize) overalloc();
	dest->data=mem_alloc(dest->x*dest->y*pspgu_pixelsize);
	dest->skip=dest->x*pspgu_pixelsize;
	dest->flags=0;
	return 0;
}

/* Return value:        0 alloced on heap
 *                      1 alloced in vidram
 *                      2 alloced in X server shm
 */
/*
static int pspgu_get_filled_bitmap(struct bitmap *dest, long color)
{
	int n;
	
	if (dest->x && (unsigned)dest->x * (unsigned)dest->y / (unsigned)dest->x != (unsigned)dest->y) overalloc();
	if ((unsigned)dest->x * (unsigned)dest->y > MAXINT / pspgu_pixelsize) overalloc();
	n=dest->x*dest->y*pspgu_pixelsize;
	dest->data=mem_alloc(n);
	pixel_set(dest->data,n,&color);
	dest->skip=dest->x*pspgu_pixelsize;
	dest->flags=0;
	return 0;
}
*/

static void pspgu_register_bitmap(struct bitmap *bmp)
{
}

static void pspgu_unregister_bitmap(struct bitmap *bmp)
{
	mem_free(bmp->data);
}

static void *pspgu_prepare_strip(struct bitmap *bmp, int top, int lines)
{
	return ((char *)bmp->data)+bmp->skip*top;
}


static void pspgu_commit_strip(struct bitmap *bmp, int top, int lines)
{
	return;
}


void pspgu_draw_bitmap(struct graphics_device *dev,struct bitmap* hndl, int x, int y)
{
	unsigned char *scr_start;

	CLIP_PREFACE

	scr_start=pspgu_vmem+y*pspgu_linesize+x*pspgu_pixelsize;
	for(;ys;ys--){
		memcpy(scr_start,data,xs*pspgu_pixelsize);
		data+=hndl->skip;
		scr_start+=pspgu_linesize;
	}
	END_GR
}


static void pspgu_draw_bitmaps(struct graphics_device *dev, struct bitmap **hndls, int n, int x, int y)
{
	TEST_INACTIVITY

	if (x>=pspgu_xsize||y>pspgu_ysize) return;
	while(x+(*hndls)->x<=0&&n){
		x+=(*hndls)->x;
		n--;
		hndls++;
	}
	while(n&&x<=pspgu_xsize){
		pspgu_draw_bitmap(dev, *hndls, x, y);
		x+=(*hndls)->x;
		n--;
		hndls++;
	}
}



static void pspgu_fill_area(struct graphics_device *dev, int left, int top, int right, int bottom, long color)
{
	unsigned char *dest;
	int y;

	FILL_CLIP_PREFACE

	dest=pspgu_vmem+top*pspgu_linesize+left*pspgu_pixelsize;
	for (y=bottom-top;y;y--){
		pixel_set(dest,(right-left)*pspgu_pixelsize,&color);
		dest+=pspgu_linesize;
	}
	END_GR
}


static void pspgu_draw_hline(struct graphics_device *dev, int left, int y, int right, long color)
{
	unsigned char *dest;
	HLINE_CLIP_PREFACE
	
	dest=pspgu_vmem+y*pspgu_linesize+left*pspgu_pixelsize;
	pixel_set(dest,(right-left)*pspgu_pixelsize,&color);
	END_GR
}


static void pspgu_draw_vline(struct graphics_device *dev, int x, int top, int bottom, long color)
{
	unsigned char *dest;
	int y;
	VLINE_CLIP_PREFACE

	dest=pspgu_vmem+top*pspgu_linesize+x*pspgu_pixelsize;
	for (y=(bottom-top);y;y--){
		memcpy(dest,&color,pspgu_pixelsize);
		dest+=pspgu_linesize;
	}
	END_GR
}


static int pspgu_hscroll(struct graphics_device *dev, struct rect_set **ignore, int sc)
{
	unsigned char *dest, *src;
	int y;
	int len;
	HSCROLL_CLIP_PREFACE
	
	ignore=NULL;
	if (sc>0){
		len=(dev->clip.x2-dev->clip.x1-sc)*pspgu_pixelsize;
		src=pspgu_vmem+pspgu_linesize*dev->clip.y1+dev->clip.x1*pspgu_pixelsize;
		dest=src+sc*pspgu_pixelsize;
		for (y=dev->clip.y2-dev->clip.y1;y;y--){
			memmove(dest,src,len);
			dest+=pspgu_linesize;
			src+=pspgu_linesize;
		}
	}else{
		len=(dev->clip.x2-dev->clip.x1+sc)*pspgu_pixelsize;
		dest=pspgu_vmem+pspgu_linesize*dev->clip.y1+dev->clip.x1*pspgu_pixelsize;
		src=dest-sc*pspgu_pixelsize;
		for (y=dev->clip.y2-dev->clip.y1;y;y--){
			memmove(dest,src,len);
			dest+=pspgu_linesize;
			src+=pspgu_linesize;
		}
	}
	END_GR
	return 1;
}


static int pspgu_vscroll(struct graphics_device *dev, struct rect_set **ignore, int sc)
{
	unsigned char *dest, *src;
	int y;
	int len;

	VSCROLL_CLIP_PREFACE

	ignore=NULL;
	len=(dev->clip.x2-dev->clip.x1)*pspgu_pixelsize;
	if (sc>0){
		/* Down */
		dest=pspgu_vmem+(dev->clip.y2-1)*pspgu_linesize+dev->clip.x1*pspgu_pixelsize;
		src=dest-pspgu_linesize*sc;
		for (y=dev->clip.y2-dev->clip.y1-sc;y;y--){
			memcpy(dest,src,len);
			dest-=pspgu_linesize;
			src-=pspgu_linesize;
		}
	}else{
		/* Up */
		dest=pspgu_vmem+dev->clip.y1*pspgu_linesize+dev->clip.x1*pspgu_pixelsize;
		src=dest-pspgu_linesize*sc;
		for (y=dev->clip.y2-dev->clip.y1+sc;y;y--){
			memcpy(dest,src,len);
			dest+=pspgu_linesize;
			src+=pspgu_linesize;
		}
	}
	END_GR
	return 1;
}


static void pspgu_set_clip_area(struct graphics_device *dev, struct rect *r)
{
	memcpy(&dev->clip, r, sizeof(struct rect));
	if (dev->clip.x1>=dev->clip.x2||dev->clip.y2<=dev->clip.y1||dev->clip.y2<=0||dev->clip.x2<=0||dev->clip.x1>=pspgu_xsize
			||dev->clip.y1>=pspgu_ysize){
		/* Empty region */
		dev->clip.x1=dev->clip.x2=dev->clip.y1=dev->clip.y2=0;
	}else{
		if (dev->clip.x1<0) dev->clip.x1=0;
		if (dev->clip.x2>pspgu_xsize) dev->clip.x2=pspgu_xsize;
		if (dev->clip.y1<0) dev->clip.y1=0;
		if (dev->clip.y2>pspgu_ysize) dev->clip.y2=pspgu_ysize;
	}
}

static int pspgu_block(struct graphics_device *dev)
{
	if (pspgu_old_vd) return 1;
	///unhandle_pspgu_mouse();
	pspgu_old_vd = current_virtual_device;
	current_virtual_device=NULL;
	///svgalib_block_itrm(pspgu_kbd);
	if (have_cmap) set_palette(&old_palette);
	///printf("\033[?25h");
	///fflush(stdout);
	return 0;
}

static void pspgu_unblock(struct graphics_device *dev)
{
#ifdef DEBUG
	if (current_virtual_device) {
		/*internal("pspgu_unblock called without pspgu_block");*/
		return;
	}
#endif /* #ifdef DEBUG */
	///if (svgalib_unblock_itrm(pspgu_kbd)) return;
	current_virtual_device = pspgu_old_vd;
	pspgu_old_vd = NULL;
	if (have_cmap) set_palette(&global_pal);
	///printf("\033[?25l");
	///fflush(stdout);
	///handle_pspgu_mouse();
	if (border_left | border_top | border_right | border_bottom) memset(pspgu_mem,0,pspgu_mem_size);
	if (current_virtual_device) current_virtual_device->redraw_handler(current_virtual_device
			,&current_virtual_device->size);
}


struct graphics_driver pspgu_driver={
	"pspgu",
	pspgu_init_driver,
	init_virtual_device,
	shutdown_virtual_device,
	pspgu_shutdown_driver,
	pspgu_get_driver_param,
	pspgu_get_empty_bitmap,
	/*pspgu_get_filled_bitmap,*/
	pspgu_register_bitmap,
	pspgu_prepare_strip,
	pspgu_commit_strip,
	pspgu_unregister_bitmap,
	pspgu_draw_bitmap,
	pspgu_draw_bitmaps,
	NULL,	/* pspgu_get_color */
	pspgu_fill_area,
	pspgu_draw_hline,
	pspgu_draw_vline,
	pspgu_hscroll,
	pspgu_vscroll,
	pspgu_set_clip_area,
	pspgu_block,
	pspgu_unblock,
	NULL,	/* set_title */
	NULL, /* exec */
	0,				/* depth (filled in pspgu_init_driver function) */
	0, 0,				/* size (in X is empty) */
	GD_DONT_USE_SCROLL|GD_NEED_CODEPAGE,		/* flags */
	0,				/* codepage */
	NULL,				/* shell */
};

#endif /* GRDRV_PSPGU */
