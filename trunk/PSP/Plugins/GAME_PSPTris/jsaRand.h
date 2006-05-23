/*
	Random number generator for the Sony PSP.
	Copyright (C) 2006 Jesper Sandberg


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
#ifndef _JSA_RAND_H_
#define _JSA_RAND_H_

void jsaRandInit(void);
u32 jsaRand(void);
int jsaRandRange(int min, int max);
float jsaRandRangef(float min, float max);

#endif /* _JSA_RAND_H_ */
