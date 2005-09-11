#ifndef _TOOLS_
#define _TOOLS_


char *dirname (char *strPath);
char *basename (char *strPath);

#ifndef LINUX

#ifndef __PSP__
	#define __PSP__
#endif

//typedef signed long long   int64_t;
//typedef signed int         int32_t;
//typedef signed short       int16_t;
//typedef signed char        int8_t;
typedef unsigned long long u_int64_t;
typedef unsigned int       u_int32_t;
typedef unsigned short     u_int16_t;
typedef unsigned char      u_int8_t;

#include <machine/setjmp.h>
#define sigjmp_buf jmp_buf
#define sigsetjmp(a,b) setjmp(a)
#define siglongjmp(a,b) longjmp(a,b)
//#define jabort 1

/* Define printf, just to make typing easier */
#define printf	pspDebugScreenPrintf

//#define getpass(a) "password" //Need to implement!

/** BSD Style seek macros */
#define L_SET	SEEK_SET
#define L_INCR	SEEK_CUR
#define L_XTND	SEEK_END

/** Dirent/glib defines */
#define ARG_MAX	4096

#define PATH_MAX MAXPATHLEN
#define SO_DEBUG 1

#endif
#endif
