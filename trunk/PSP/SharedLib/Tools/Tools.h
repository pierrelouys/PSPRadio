/* 
	PSP Tools. (Initial Release: Sept. 2005)
	Copyright (C) 2005  Rafael Cabezas a.k.a. Raf
	
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef _TOOLS_
	#define _TOOLS_
	
	char *dirname (char *strPath);
	char *basename (char *strPath);
	
	#ifndef LINUX
	
		#ifndef __PSP__
			#define __PSP__
		#endif
		
		typedef unsigned long long u_int64_t;
		typedef unsigned int       u_int32_t;
		typedef unsigned short     u_int16_t;
		typedef unsigned char      u_int8_t;
		
		#include <machine/setjmp.h>
		#define sigjmp_buf jmp_buf
		#define sigsetjmp(a,b) setjmp(a)
		#define siglongjmp(a,b) longjmp(a,b)
		//#define jabort 1
		
		//#define getpass(a) "password" //Need to implement!
		
		/** BSD Style seek macros */
		#define L_SET	SEEK_SET
		#define L_INCR	SEEK_CUR
		#define L_XTND	SEEK_END
		
		/** Dirent/glib defines */
		#define ARG_MAX	4096
		
		#define PATH_MAX MAXPATHLEN
		//#define SO_DEBUG 1
			
	#endif
#endif
